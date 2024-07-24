#pragma once

#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Zahra
{
	namespace OpenGLUtils
	{
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
			{
				return GL_VERTEX_SHADER;
			}
			else if (type == "fragment" || type == "pixel")
			{
				return GL_FRAGMENT_SHADER;
			}
			// TODO: add other shader types if they come into play

			Z_CORE_WARN("Unknown shader type");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
				{
					return shaderc_glsl_vertex_shader;
				}
				case GL_FRAGMENT_SHADER:
				{
					return shaderc_glsl_fragment_shader;
				}
			}

			Z_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
				{
					return "GL_VERTEX_SHADER";
				}
				case GL_FRAGMENT_SHADER:
				{
					return "GL_FRAGMENT_SHADER";
				}
			}

			Z_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: validate assets directory
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();

			if (!std::filesystem::exists(cacheDirectory)) std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
				{
					return ".cached_opengl.vert";
				}
				case GL_FRAGMENT_SHADER:
				{
					return ".cached_opengl.frag";
				}
			}

			Z_CORE_ASSERT(false);
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
				{
					return ".cached_vulkan.vert";
				}
				case GL_FRAGMENT_SHADER:
				{
					return ".cached_vulkan.frag";
				}
			}

			Z_CORE_ASSERT(false);
			return "";
		}

	}
}
