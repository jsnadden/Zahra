#pragma once

// This is for use by client apps

//------------CORE---------------------
#include "Zahra/Core/Application.h"
#include "Zahra/Core/Assert.h"
#include "Zahra/Core/GUID.h"
#include "Zahra/Core/Input.h"
#include "Zahra/Core/KeyCodes.h"
#include "Zahra/Core/Layer.h"
#include "Zahra/Core/Log.h"
#include "Zahra/Core/MouseCodes.h"
#include "Zahra/Core/Ref.h"
#include "Zahra/Core/Scope.h"
#include "Zahra/Core/Timer.h"

//------------DEBUG--------------------
#include "Zahra/Debug/Profiling.h"

//------------IMGUI--------------------
#include "Zahra/ImGui/ImGuiLayer.h"

//------------RENDERING----------------
#include "Zahra/Renderer/Camera.h"
#include "Zahra/Renderer/EditorCamera.h"
#include "Zahra/Renderer/Framebuffer.h"
#include "Zahra/Renderer/Image.h"
#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/Material.h"
#include "Zahra/Renderer/Mesh.h"
#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Renderer/Renderer2D.h"
#include "Zahra/Renderer/RenderPass.h"
#include "Zahra/Renderer/SceneRenderer.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/ShaderResourceManager.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Renderer/UniformBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

//------------SCENE--------------------
#include "Zahra/Scene/Scene.h"
#include "Zahra/Scene/Components.h"
#include "Zahra/Scene/Entity.h"
#include "Zahra/Scene/ScriptableEntity.h"

