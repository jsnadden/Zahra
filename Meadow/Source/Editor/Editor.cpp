#include "zpch.h"
#include "Editor.h"

namespace Zahra
{
	struct EditHistoryData
	{
		EditorConfig Config;

		//std::vector<Ref<Edit>> UndoStack, RedoStack;
		bool Dirty = false;
		int32_t Balance = 0;

		SceneState EditorSceneState = SceneState::Edit;
	};
	static EditHistoryData s_EditorData;

	EditorConfig& Editor::GetConfig()
	{
		return s_EditorData.Config;
	}

	SceneState Editor::GetSceneState()
	{
		return s_EditorData.EditorSceneState;
	}

	void Editor::SetSceneState(const SceneState& sceneState)
	{
		s_EditorData.EditorSceneState = sceneState;
	}

	/*void Editor::Reset()
	{
		s_EditorData.UndoStack.clear();
		s_EditorData.RedoStack.clear();
		s_EditorData.Dirty = false;
		s_EditorData.Balance = 0;
	}

	void Editor::OnSave()
	{
		s_EditorData.Dirty = false;
		s_EditorData.Balance = 0;
	}

	void Editor::MakeEdit(Ref<Edit>& edit)
	{
		if (!edit)
			return;

		edit->Do();

		if (s_EditorData.EditorSceneState != SceneState::Edit)
			return;

		s_EditorData.UndoStack.push_back(edit);
		s_EditorData.RedoStack.clear();

		if (s_EditorData.Balance < 0)
			s_EditorData.Dirty = true;

		s_EditorData.Balance++;
	}

	void Editor::Undo()
	{
		if (s_EditorData.UndoStack.empty() || s_EditorData.EditorSceneState != SceneState::Edit)
			return;

		auto& edit = s_EditorData.UndoStack.back();
		edit->Undo();
		s_EditorData.RedoStack.push_back(edit);
		s_EditorData.UndoStack.pop_back();

		s_EditorData.Balance--;
	}

	void Editor::Redo()
	{
		if (s_EditorData.RedoStack.empty() || s_EditorData.EditorSceneState != SceneState::Edit)
			return;

		auto& edit = s_EditorData.RedoStack.back();
		edit->Do();
		s_EditorData.UndoStack.push_back(edit);
		s_EditorData.RedoStack.pop_back();

		s_EditorData.Balance++;
	}

	bool Editor::CanUndo()
	{
		return s_EditorData.UndoStack.size() > 0;
	}

	bool Editor::CanRedo()
	{
		return s_EditorData.RedoStack.size() > 0;
	}

	bool Editor::UnsavedChanges()
	{
		return s_EditorData.Dirty || s_EditorData.Balance;
	}*/

}
