#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic script -- name, source, and the script data itself
//============================================================================================================

class Script
{
protected:

	String				mName;
	String				mSource;
	TreeNode			mContents;
	bool				mSerializable;
	Thread::Lockable	mLock;

public:

	Script(const String& name);

	R5_DECLARE_SOLO_CLASS("Script");

	void Lock()		const	{ mLock.Lock();	}
	void Unlock()	const	{ mLock.Unlock();	}

	const String&	GetName() const { return mName; }
	const TreeNode&	GetRoot() const { return mContents; }

	bool IsValid() const { return mName.IsValid() && mContents.mChildren.IsValid(); }
	bool Load (const char* source);

	// Serialization functions
	void SetSerializable(bool val) { mSerializable = val; }
	bool IsSerializable() const { return mSerializable && mSource.IsValid(); }
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& root) const;
};