#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Scripts can be attached to game objects and are the ideal way of inserting code into the scene.
//============================================================================================================

class Object;
class Script
{
private:

	struct Ignore
	{
		enum
		{
			PreUpdate		= 1 << 1,
			Update			= 1 << 2,
			PostUpdate		= 1 << 3,
		};
	};

protected:

	// Allow Entity class to access private data members in order to simplify code
	friend class Object;

	Object*		mObject;
	Flags		mIgnore;
	bool		mEnabled;

protected:

	// It's not possible to create just plain scripts -- they need to be derived from
	Script() : mEnabled(true) {}

	// Destroys this script, deleting it from memory and from its owner Object
	void DestroySelf() { delete this; }

public:

	// This is a top-level base class
	R5_DECLARE_INTERFACE_CLASS("Script");

	// Scripts should be removed via DestroySelf() or using the RemoveScript<> template
	virtual ~Script();

	// Initialization function is called once the script has been created
	virtual void Init() {}

	// Called prior to object's Update function
	virtual void OnPreUpdate() { mIgnore.Set(Ignore::PreUpdate, true); }

	// Called after the object's absolute coordinates have been calculated
	virtual void OnUpdate() { mIgnore.Set(Ignore::Update, true); }

	// Called after the object has updated all of its children
	virtual void OnPostUpdate() { mIgnore.Set(Ignore::PostUpdate, true); }

	// Serialization
	virtual bool SerializeTo	(TreeNode& root) const { return false; }
	virtual bool SerializeFrom	(const TreeNode& root) { return false; }
};