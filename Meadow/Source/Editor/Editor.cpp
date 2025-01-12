#include "zpch.h"
#include "Editor.h"

namespace Zahra
{
	struct EditHistoryData
	{
		EditorConfig Config;

		std::vector<EditAction> UndoStack, RedoStack;
		bool Dirty = false;
		int32_t Balance;
	};
	static EditHistoryData s_EditorData;

	void Editor::Reset()
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

	EditorConfig& Editor::GetConfig()
	{
		return s_EditorData.Config;
	}

	void Editor::NewAction(EditAction& action)
	{
		action.Do();
		s_EditorData.UndoStack.push_back(action);
		s_EditorData.RedoStack.clear();

		if (s_EditorData.Balance < 0)
			s_EditorData.Dirty = true;

		s_EditorData.Balance++;
	}

	void Editor::Undo()
	{
		if (s_EditorData.UndoStack.empty())
			return;

		auto action = s_EditorData.UndoStack.back();
		s_EditorData.UndoStack.pop_back();
		action.Undo();
		s_EditorData.RedoStack.push_back(action);

		s_EditorData.Balance--;
	}

	void Editor::Redo()
	{
		if (s_EditorData.RedoStack.empty())
			return;

		auto& action = s_EditorData.RedoStack.back();
		action.Do();
		s_EditorData.UndoStack.push_back(action);
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
	}

}
