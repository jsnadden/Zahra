#include "zpch.h"
#include "SceneRenderer.h"

namespace Zahra
{

	SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification)
	{
		ShaderSpecification shaderSpec{};
		// TODO: the app should send the shader details prior to this (in Renderer::Init() e.g.)
		// (and they should really be owned by a shader library)
		shaderSpec.Name = "vulkan_tutorial";
		shaderSpec.SourceDirectory = "Resources/Shaders";
		m_Shader = Shader::Create(shaderSpec);

		PipelineSpecification pipelineSpec{};
		pipelineSpec.Shader = m_Shader;
		m_Pipeline = Pipeline::Create(pipelineSpec);

		CommandBufferSpecification commandBufferSpec{};
		commandBufferSpec.Pipeline = m_Pipeline;
		m_CommandBuffer = CommandBuffer::Create(commandBufferSpec);
	}

	SceneRenderer::~SceneRenderer()
	{
		m_Shader.Reset();
		m_Pipeline.Reset();
		m_CommandBuffer.Reset();
	}

	void SceneRenderer::DrawStuff()
	{
		


	}

}
