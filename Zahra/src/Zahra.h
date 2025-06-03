#pragma once

// For use by client apps

//------------CORE---------------------
#include "Zahra/Core/Application.h"
#include "Zahra/Core/Assert.h"
#include "Zahra/Core/Buffer.h"
#include "Zahra/Core/UUID.h"
#include "Zahra/Core/Input.h"
#include "Zahra/Core/KeyCodes.h"
#include "Zahra/Core/Layer.h"
#include "Zahra/Core/Log.h"
#include "Zahra/Core/Memory.h"
#include "Zahra/Core/MouseCodes.h"
#include "Zahra/Core/Ref.h"
#include "Zahra/Core/Scope.h"
#include "Zahra/Core/Thread.h"
#include "Zahra/Core/Timer.h"

//------------ASSET--------------------
#include "Zahra/Assets/Asset.h"
#include "Zahra/Assets/AssetLoader.h"
#include "Zahra/Assets/AssetManager.h"
#include "Zahra/Assets/AssetManagerBase.h"
#include "Zahra/Assets/AssetType.h"
#include "Zahra/Assets/EditorAssetManager.h"
#include "Zahra/Assets/RuntimeAssetManager.h"

//------------DEBUG--------------------
#include "Zahra/Debug/Profiling.h"

//------------IMGUI--------------------
#include "Zahra/ImGui/ImGuiLayer.h"

//------------PROJECT------------------
#include "Zahra/Projects/Project.h"
#include "Zahra/Projects/ProjectSerialiser.h"

//------------RENDERER-----------------
#include "Zahra/Renderer/Cameras/Camera.h"
#include "Zahra/Renderer/Cameras/EditorCamera.h"
#include "Zahra/Renderer/Cameras/SceneCamera.h"
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
#include "Zahra/Scene/Components.h"
#include "Zahra/Scene/Entity.h"
#include "Zahra/Scene/Scene.h"
#include "Zahra/Scene/ScriptableEntity.h"

//------------SCRIPTING----------------
#include "Zahra/Scripting/ScriptEngine.h"
