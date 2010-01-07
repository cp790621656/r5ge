#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// NOTE: Resource is just a TreeNode structure with a name. This class precedes the "Script" class, and its
// future is questionable. For now it's just used to execute TreeNodes on different threads.
//============================================================================================================

class Resource
{
protected:

	String				mName;
	String				mSource;
	TreeNode			mContents;
	bool				mSerializable;
	Thread::Lockable	mLock;

public:

	Resource(const String& name);

	R5_DECLARE_SOLO_CLASS("Resource");

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