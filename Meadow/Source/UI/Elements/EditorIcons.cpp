#include "zpch.h"
#include "EditorIcons.h"

namespace Zahra
{
	static std::map<std::string, Ref<Texture2D>> s_Icons;
	static std::map<std::string, ImGuiTextureHandle> s_ImGuiHandles;

	void EditorIcons::Init()
	{
		// Generic
		s_Icons["Generic/BrokenImage"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/Generic/broken_image.png");
		
		// Scene control panel
		s_Icons["SceneControls/Play"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/play.png");
		s_Icons["SceneControls/Step"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/step.png");
		s_Icons["SceneControls/Reset"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/reset.png");
		s_Icons["SceneControls/Stop"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/stop.png");
		s_Icons["SceneControls/Pause"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/pause.png");
		s_Icons["SceneControls/PhysicsOn"]			= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/physics_on.png");
		s_Icons["SceneControls/PhysicsOff"]			= TextureLoader::LoadTexture2DFromSource("Resources/Icons/SceneControls/physics_off.png");

		// Content browser panel
		s_Icons["ContentBrowser/DirectoryThumb"]	= TextureLoader::LoadTexture2DFromSource("Resources/Icons/ContentBrowser/folder.png");
		s_Icons["ContentBrowser/DefaultFileThumb"]	= TextureLoader::LoadTexture2DFromSource("Resources/Icons/ContentBrowser/default_file.png");
		s_Icons["ContentBrowser/Back"]				= TextureLoader::LoadTexture2DFromSource("Resources/Icons/ContentBrowser/back_arrow.png");
		s_Icons["ContentBrowser/Forward"]			= TextureLoader::LoadTexture2DFromSource("Resources/Icons/ContentBrowser/forward_arrow.png");

		auto imguiLayer = ImGuiLayer::GetOrCreate();

		for (auto& [name, texture] : s_Icons)
		{
			s_ImGuiHandles[name] = imguiLayer->RegisterTexture(texture);
		}
	}

	void EditorIcons::Shutdown()
	{
		auto imguiLayer = ImGuiLayer::GetOrCreate();
		for (auto& [name, handle] : s_ImGuiHandles)
			imguiLayer->DeregisterTexture(handle);

		s_ImGuiHandles.clear();
		s_Icons.clear();
	}

	ImGuiTextureHandle EditorIcons::GetIconHandle(const std::string& iconName)
	{
		auto it = s_ImGuiHandles.find(iconName);
		if (it == s_ImGuiHandles.end())
			return s_ImGuiHandles.at("Generic/BrokenImage");

		return it->second;
	}
}
