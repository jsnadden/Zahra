#pragma once

#include "Utils/TypeDefs.h"

namespace Zahra
{
	struct EditorConfig
	{
		float SceneCacheInterval = 300.f;
		uint32_t MaxCachedScenes = 5;
		bool ShowSavePrompt = true;
	};

#if 0
	class Edit : public RefCounted
	{
	public:
		virtual void Do() = 0;
		virtual void Undo() = 0;
	};
#endif

	class Editor
	{
	public:
		static EditorConfig& GetConfig();
		static SceneState GetSceneState();
		static void SetSceneState(const SceneState& sceneState);

		///////////////////////////
		// MOTHBALLED 13/01/2025:
		// The difficulties of this undertaking made for a fun weekend challenge, but I've come to understand
		// why this is a bad idea. For a start, you have to keep track of A LOT of state, and a great deal of that
		// is highly unpredictable (as it is with any non-trivial program). Even with what seemed like the easiest
		// thing to be able to undo/redo, namely data fields in ECS components, the fact that you have no long-term
		// guarantee of the existence of those fields means you have either let the undo/redo stacks manage the
		// lifetimes of all those data, or do frequent/thorough garbage collection to clear out dangling refs.
		// Unless I wanted to write an ECS system from scratch, I should avoid doing things using this command
		// pattern approach. If I return to this at some point, I should go the photoshop route: cache binary images
		// and/or diffs to keep track of ALL relevant state. It might be slower, and certainly will put a limit on the
		// undo/redo stack sizes, but there's a reason that's the standard for a lot of technical applications. In
		// any case, I need to move on now. This was, at the very least, a great learning experience.
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

	};
}
