#include "zpch.h"
#include "Font.h"

#include "Zahra/Renderer/Text/MSDF.h"
#include "Zahra/Renderer/Text/UnicodeRanges.h"
#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	struct UnicodeRange
	{
		uint32_t Start, End;
	};

	struct AtlasSpecification
	{
		std::string Name;
		float EmSize;
		int Width, Height;
	};

	static const std::map<Font::CharacterSet, std::vector<UnicodeRange>> s_UnicodeRanges =
	{
		// (partially) obtained from imgui_draw.cpp
		{
			Font::CharacterSet::Latin,
			{
				Z_UNICODE_RANGE_Latin,
				Z_UNICODE_RANGE_LatinSupp,
			}
		},
		{
			Font::CharacterSet::Chinese,
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
	static Ref<Texture2D> CreateAndCacheAtlas(const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& fontGeometry, const AtlasSpecification& atlasSpec)
	{
		msdf_atlas::ImmediateAtlasGenerator<S, N, Fn, msdf_atlas::BitmapAtlasStorage<T, N>> generator(atlasSpec.Width, atlasSpec.Height);
		
		msdf_atlas::GeneratorAttributes attributes{};
		{
			attributes.config.overlapSupport = true;
			attributes.scanlinePass = true;
		}
		
		generator.setAttributes(attributes);
		generator.setThreadCount(8);
		generator.generate(glyphs.data(), glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap =(msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();
		//msdfgen::savePng(bitmap, "atlas.png");

		TextureSpecification spec{};
		{
			spec.Width = atlasSpec.Width;
			spec.Height = atlasSpec.Height;
			spec.GenerateMips = false;
		}

		Buffer pixelBuffer(bitmap.pixels, atlasSpec.Width * atlasSpec.Height * Image::BytesPerPixel(spec.Format));

		return Texture2D::CreateFromBuffer(spec, pixelBuffer);
	}

	Font::Font(const std::filesystem::path& fontFilepath, CharacterSet characterSet)
		: m_Data(new MSDFData()), m_CharacterSet(characterSet)
	{
		msdfgen::FreetypeHandle* freetype = msdfgen::initializeFreetype();
		Z_CORE_VERIFY(freetype, "Failed to initialise FreeType library");

		/////////////////////////////////////////////////////////////////////////////////////
		// LOAD FONT
		std::string fontFileString = fontFilepath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(freetype, fontFileString.c_str());

		if (!font)
		{
			Z_CORE_ERROR("Failed to load font from '{0}'", fontFileString.c_str());
			return;
		}

		Z_CORE_INFO("Font '{0}' was successfully loaded", fontFileString.c_str());

		/////////////////////////////////////////////////////////////////////////////////////
		// LOAD GLYPHS
		msdf_atlas::Charset charSet;

		Z_CORE_ASSERT(s_UnicodeRanges.find(m_CharacterSet) != s_UnicodeRanges.end(),
			"This writing system/script is not currently supported");

		for (UnicodeRange range : s_UnicodeRanges.at(m_CharacterSet))
		{
			for (uint32_t character = range.Start; character <= range.End; character++)
				charSet.add(character);
		}		

		double fontScale = 1.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->GlyphGeometries);
		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charSet);
		Z_CORE_INFO("{0} out of {1} glyphs were successfully loaded", glyphsLoaded, charSet.size());

		/////////////////////////////////////////////////////////////////////////////////////
		// GENERATE ATLAS
		int emSize = 40.0; // TODO: play around with this (though 40.0 was recommended)

		msdf_atlas::TightAtlasPacker packer;
		packer.setPixelRange(2.0);
		packer.setMiterLimit(1.0);
		packer.setPadding(0.0);
		packer.setScale(emSize);
		int leftover = packer.pack(m_Data->GlyphGeometries.data(), (int)m_Data->GlyphGeometries.size());
		Z_CORE_ASSERT(leftover <= 0);

		AtlasSpecification atlasSpec{};
		{
			atlasSpec.Name = "TEMPORARY_ATLAS_NAME";
			atlasSpec.EmSize = packer.getScale();
			packer.getDimensions(atlasSpec.Width, atlasSpec.Height);
		}

#pragma region Set up edge colouring (copied from msdf_atlas_gen main.cpp)
		#define MSDF_DEFAULT_ANGLE_THRESHOLD 3.0
		#define MSDF_LCG_MULTIPLIER 6364136223846793005ull
		#define MSDF_LCG_INCREMENT 1442695040888963407ull
		#define MSDF_THREAD_COUNT 8

		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = m_Data->GlyphGeometries, &coloringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (MSDF_LCG_MULTIPLIER * (coloringSeed ^ i) + MSDF_LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, MSDF_DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, m_Data->GlyphGeometries.size()).finish(MSDF_THREAD_COUNT);
		}
		else {
			unsigned long long glyphSeed = coloringSeed;
			for (auto& glyph : m_Data->GlyphGeometries)
			{
				glyphSeed *= MSDF_LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, MSDF_DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}
#pragma endregion

		m_AtlasTexture = CreateAndCacheAtlas<byte, float, 4, msdf_atlas::mtsdfGenerator>(m_Data->GlyphGeometries, m_Data->FontGeometry, atlasSpec);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(freetype);
	}

	Font::~Font()
	{
		delete m_Data;
		m_Data = nullptr;
	}

}
