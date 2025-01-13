#pragma once

#include "Utils/TypeDefs.h"

namespace Zahra
{
	struct EditorConfig
	{
		float SceneCacheInterval = 300.f;
		uint32_t MaxCachedScenes = 5;
	};

	class Edit : public RefCounted
	{
	public:
		virtual void Do() = 0;
		virtual void Undo() = 0;
	};

	class Editor
	{
	public:
		static void Reset();
		static void OnSave();

		static EditorConfig& GetConfig();

		static void MakeEdit(Ref<Edit>& edit);

		static void Undo();
		static void Redo();

		static bool CanUndo();
		static bool CanRedo();
		static bool UnsavedChanges();

		static SceneState GetSceneState();
		static void SetSceneState(const SceneState& sceneState);
	};
}
