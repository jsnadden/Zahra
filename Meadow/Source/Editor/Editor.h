#pragma once

#include "Editor/TypeDefs.h"
#include "Zahra/Renderer/Cameras/EditorCamera.h"
#include "Zahra/Scene/Entity.h"
#include "Zahra/Scene/Scene.h"

namespace Zahra
{
	struct EditorConfig
	{
		float SceneCacheInterval = 300.f;
		uint32_t MaxCachedScenes = 5;
		bool ShowSavePrompt = true;
	};

	class Edit : public RefCounted
	{
	public:
		virtual void Do() = 0;
		virtual void Undo() = 0;

		// check to see if we can merge subsequent edits into a single action
		virtual bool CanMergeWith(const Edit& other) { return false; }
	};

	class Editor
	{
	public:
		// TODO: give this another go, I previously overreacted XD
#if 0
		static void Reset();
		static void OnSave();

		static void MakeEdit(Ref<Edit>& edit);

		static void Undo();
		static void Redo();

		static bool CanUndo();
		static bool CanRedo();
		static bool UnsavedChanges();
#endif

		static EditorConfig& GetConfig();

		static SceneState GetSceneState();
		static void SetSceneState(const SceneState& sceneState);

		static WeakRef<Scene> GetSceneContext();
		static void SetSceneContext(Ref<Scene> scene);

		static Entity GetSelectedEntity();
		static void SelectEntity(Entity entity);
		static bool IsSelected(ZGUID entityID);

		static WeakRef<EditorCamera> GetPrimaryEditorCamera();
		static void SetPrimaryEditorCamera(EditorCamera& camera);
		static void CenterPrimaryEditorCamera(const glm::vec3& point);



	};
}
