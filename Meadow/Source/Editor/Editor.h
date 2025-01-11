#pragma once

namespace Zahra
{
	struct EditorConfig
	{
		float SceneCacheInterval = 300.f;
		uint32_t MaxCachedScenes = 5;
	};

	struct EditAction
	{
		std::function<void()> Do, Undo;
	};

	class Editor
	{
	public:
		static void Reset();
		static void OnSave();

		static EditorConfig& GetConfig();

		static void NewAction(EditAction& action);

		static void Undo();
		static void Redo();

		static bool CanUndo();
		static bool CanRedo();
		static bool UnsavedChanges();
	};
}
