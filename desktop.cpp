#include "desktop.h"
#include "workspace.h"
#include "option.h"
#include <process.h>

//-----------------------------------------------------------------------------
// Thread functions for [create], [remove] and [jumpto].

unsigned int __stdcall createThread(void *arg)
{
	((Desktop *)arg)->__create();
	return 0;
}

unsigned int __stdcall removeThread(void *arg)
{
	((Desktop *)arg)->__remove();
	return 0;
}

unsigned int __stdcall jumptoThread(void *arg)
{
	((Desktop *)arg)->__jumpto();
	return 0;
}

//-----------------------------------------------------------------------------

Desktop::Desktop()
: index_(0), jumptoIndex_(0), workspaces_(option.number)
{
	for (size_t i = 0; i < workspaces_.size(); ++i) {
		workspaces_[i] = new Workspace;
	}
}

Desktop::~Desktop()
{
	for (size_t i = 0; i < workspaces_.size(); ++i) {
		workspaces_[i]->show(); // Show them all before exit
		delete workspaces_[i];
	}
	workspaces_.clear();
}

//-----------------------------------------------------------------------------

// Create a new workspace (with an extra thread)
void Desktop::create()
{
	unsigned int id_; // 95 & 98 need this
	_beginthreadex(0, 0, createThread, this, 0, &id_);
}

// Remove the current workspace (with an extra thread).
bool Desktop::remove()
{
	// At least [MinNumber] workspaces
	if (workspaces_.size() <= MinNumber) { return false; }

	unsigned int id_; // 95 & 98 need this.
	_beginthreadex(0, 0, removeThread, this, 0, &id_);

	return true;
}

// Jump to the workspace with the specified index (with an extra thread).
bool Desktop::jumpto(size_t index)
{
	if (index >= workspaces_.size() || index_ == index) { return false; }
	jumptoIndex_ = index; // Save the index jumped to

	unsigned int id_; // 95 & 98 need this
	_beginthreadex(0, 0, jumptoThread, this, 0, &id_);

	return true;
}

//-----------------------------------------------------------------------------
// The ones that cost time and will be invoked by the related thread function.

void Desktop::__create()
{
	// Some windows may have been opened or closed in the workspace,
	// so it's necessary to re-enumerate them.
	workspaces_[index_]->enumWindows();
	workspaces_[index_]->hide();
	workspaces_.push_back(new Workspace());
	index_ = workspaces_.size() - 1;
}

// The case that only one workspace is left is not considered here. It's done
// in function bool remove().
void Desktop::__remove()
{
	size_t i = index_; // The workspace will be removed.
	size_t d = index_; // The workspace the removed one will be merged to.

	if (index_ == 0) { // If it's going to remove the head, show the next workspace.
		++d; // No need to update index_ in this case.
	} else { // Otherwise, show the previous workspace.
		--d;
		--index_; // Update index_
	}

	// Some windows may have been opened or closed in the workspace, so it's
	// necessary to re-enumerate them.
	workspaces_[i]->enumWindows();

	Workspace *tmp = workspaces_[i];
	workspaces_[d]->merge(*tmp);
	workspaces_[d]->show();
	workspaces_.erase(workspaces_.begin() + i);
	delete tmp;
}

// The index jumped to is specified by memeber variable "jumpto_", and
// the validation for it is not considered here. It's done in function
// bool jumpto(size_t index).
void Desktop::__jumpto()
{
	workspaces_[index_]->enumWindows();
	workspaces_[index_]->hide();
	index_ = jumptoIndex_;
	workspaces_[index_]->show();
}
