#include "zpch.h"
#include "Font.h"

#include "Zahra/Renderer/Text/UnicodeRanges.h"
#include "Zahra/Renderer/Texture.h"

#undef INFINITE // avoid clash with unnecessary windows definition
#include "msdf-atlas-gen.h"
#include "msdfgen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

namespace Zahra
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> GlyphGeometries;
		msdf_atlas::FontGeometry FontGeometry;
	};

	struct UnicodeRange
	{
		uint32_t Start, End;
	};

	static const std::map<Font::WritingSystem, std::vector<UnicodeRange>> s_UnicodeRanges =
	{
		// (partially) obtained from imgui_draw.cpp
		{
			Font::WritingSystem::Latin,
			{
				Z_UNICODE_RANGE_Latin,
				Z_UNICODE_RANGE_LatinSupp,
			}
		},
		{
			Font::WritingSystem::Chinese,
			{
				Z_UNICODE_RANGE_Latin,
				Z_UNICODE_RANGE_LatinSupp,
				Z_UNICODE_RANGE_CJKSymbols,
				Z_UNICODE_RANGE_CJKIdeograms,
				Z_UNICODE_RANGE_Hiragana,
				Z_UNICODE_RANGE_Katakana,
				Z_UNICODE_RANGE_KatakanaExt,
				Z_UNICODE_RANGE_HalfWidth,
				Z_UNICODE_RANGE_Specials,
			}
		}
	};

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> Fn>
	static Ref<Texture2D> CreateAndCacheAtlas(const MSDFData* data, const std::string& fontName, float fontSize, int width, int height)
	{
		msdf_atlas::ImmediateAtlasGenerator<S, N, Fn, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		
		msdf_atlas::GeneratorAttributes attributes{};
		{
			attributes.config.overlapSupport = true;
			attributes.scanlinePass = true;
		}
		
		generator.setAttributes(attributes);
		generator.setThreadCount(8);
		generator.generate(data->GlyphGeometries.data(), data->GlyphGeometries.size());

		msdfgen::BitmapConstRef<T, N> bitmap =(msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec{};
		{
			spec.Format = ImageFormat::RGB;
			spec.Width = width;
			spec.Height = height;
			spec.GenerateMips = true;
		}

		Buffer pixelBuffer(bitmap.pixels, width * height * Image::BytesPerPixel(spec.Format));

		return Texture2D::CreateFromBuffer(spec, pixelBuffer);
	}

	Font::Font(const std::filesystem::path& fontFilepath, WritingSystem writingSystem)
		: m_Data(new MSDFData()), m_WritingSystem(writingSystem)
	{
		msdfgen::FreetypeHandle* freetype = msdfgen::initializeFreetype();
		Z_CORE_VERIFY(freetype, "Failed to initialise FreeType library");

		/////////////////////////////////////////////////////////////////////////////////////
		// LOAD FONT FILE
		std::string fontFileString = fontFilepath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(freetype, fontFileString.c_str());

		if (!font)
		{
			Z_CORE_ERROR("Failed to load font from '{0}'", fontFileString.c_str());
			return;
		}

		Z_CORE_INFO("Loaded font from '{0}'", fontFileString.c_str());

		/////////////////////////////////////////////////////////////////////////////////////
		// GENERATE CHARACTER SET
		msdf_atlas::Charset characterSet;

		Z_CORE_ASSERT(s_UnicodeRanges.find(m_WritingSystem) != s_UnicodeRanges.end(),
			"This writing system/script is not currently supported");

		for (UnicodeRange range : s_UnicodeRanges.at(m_WritingSystem))
		{
			for (uint32_t character = range.Start; character <= range.End; character++)
				characterSet.add(character);
		}		

		double fontScale = 1.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->GlyphGeometries);
		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, characterSet);
		Z_CORE_INFO("Successfully loaded glyphs for {0} out of {1} characters", glyphsLoaded, characterSet.size());

		/////////////////////////////////////////////////////////////////////////////////////
		// GENERATE ATLAS
		int emSize = 40.0; // TODO: play around with this (though 40.0 was recommended)

		msdf_atlas::TightAtlasPacker packer;
		packer.setPixelRange(2.0);
		packer.setMiterLimit(1.0);
		packer.setPadding(0.0);
		packer.setScale(emSize);
		int leftover = packer.pack(m_Data->GlyphGeometries.data(), m_Data->GlyphGeometries.size());
		Z_CORE_ASSERT(leftover <= 0);

		int atlasWidth, atlasHeight;
		packer.getDimensions(atlasWidth, atlasHeight);
		emSize = packer.getScale();

		Ref<Texture2D> atlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(m_Data, "test", emSize, atlasWidth, atlasHeight);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(freetype);
	}

	Font::~Font()
	{
		delete m_Data;
		m_Data = nullptr;
	}

}
