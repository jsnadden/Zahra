#include "zpch.h"
#include "OpenGLShader.h"

#include "Zahra/Core/Timer.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>



namespace Zahra
{	

	namespace OpenGLShaderUtils
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
			return "Resources/cache/shader/opengl";
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

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		OpenGLShaderUtils::CreateCacheDirectoryIfNeeded();

		std::string sourceString= ReadFile(filepath);
		auto shaderSources = ParseShaderSrc(sourceString);
		
		Timer timer;
		{
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
		}
		Z_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());

		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind(".");
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
		: m_Name(name)
	{
		std::unordered_map<GLenum, std::string> shaderSrcs;
		shaderSrcs[GL_VERTEX_SHADER] = vertexSource;
		shaderSrcs[GL_FRAGMENT_SHADER] = fragmentSource;

		CompileOrGetVulkanBinaries(shaderSrcs);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(NULL);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string fileContents;

		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			fileContents.resize(in.tellg());
			in.seekg(0, std::ios::beg);

			in.read(&fileContents[0], fileContents.size());

			in.close();
		}
		else
		{
			Z_CORE_ERROR("Couldn't read shader file '{0}'", filepath);
		}
		
		return fileContents;

	}

	std::unordered_map<GLenum, std::string> OpenGLShader::ParseShaderSrc(std::string& shaderSource)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";

		size_t typeTokenLength = strlen(typeToken);
		size_t typeTokenPosition = shaderSource.find(typeToken, 0);

		// TODO: make this a little more flexible, ignoring whitespace etc.
		while (typeTokenPosition != std::string::npos)
		{
			size_t endOfLine = shaderSource.find_first_of("\r\n", typeTokenPosition);
			Z_CORE_ASSERT(endOfLine != std::string::npos, "Syntax error.");

			size_t shaderTypePosition = typeTokenPosition + typeTokenLength + 1;

			std::string type = shaderSource.substr(shaderTypePosition, endOfLine - shaderTypePosition);
			Z_CORE_ASSERT(OpenGLShaderUtils::ShaderTypeFromString(type), "Invalid shader type specified.");

			size_t shaderCodeStart = shaderSource.find_first_not_of("\r\n", endOfLine);
			typeTokenPosition = shaderSource.find(typeToken, shaderCodeStart);

			shaderSources[OpenGLShaderUtils::ShaderTypeFromString(type)] = (typeTokenPosition == std::string::npos) ?
				shaderSource.substr(shaderCodeStart) : shaderSource.substr(shaderCodeStart, typeTokenPosition - shaderCodeStart);
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = OpenGLShaderUtils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_Filepath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + OpenGLShaderUtils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);

			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);
				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, OpenGLShaderUtils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
			
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					Z_CORE_ERROR(module.GetErrorMessage());
					Z_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
		{
			Z_CORE_TRACE("====================================================");
			Reflect(stage, data);
		}
		Z_CORE_TRACE("====================================================");


	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = OpenGLShaderUtils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();

		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_Filepath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + OpenGLShaderUtils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, OpenGLShaderUtils::GLShaderStageToShaderC(stage), m_Filepath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					Z_CORE_ERROR(module.GetErrorMessage());
					Z_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();
		std::vector<GLuint> shaderIDs;

		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), static_cast<GLsizei>(spirv.size() * sizeof(uint32_t)));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			Z_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_Filepath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);

		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		Z_CORE_TRACE("OpenGLShader::Reflect: {0} ({1})", OpenGLShaderUtils::GLShaderStageToString(stage), m_Filepath);
		Z_CORE_TRACE("---------------------------------------------------");
		Z_CORE_TRACE("  |  {0} resource(s)", resources.sampled_images.size());
		Z_CORE_TRACE("  |  {0} uniform buffer(s)", resources.uniform_buffers.size());

		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t memberCount = bufferType.member_types.size();

			Z_CORE_TRACE("===================================================");
			Z_CORE_TRACE("    Uniform buffer {0}", resource.name);
			Z_CORE_TRACE("---------------------------------------------------");
			Z_CORE_TRACE("      |  Size = {0}", bufferSize);
			Z_CORE_TRACE("      |  Binding = {0}", binding);
			Z_CORE_TRACE("      |  Members = {0}", memberCount);
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SET UNIFORMS

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, uint32_t count, int* values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec4& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::SetMat2(const std::string& name, const glm::mat2& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}

