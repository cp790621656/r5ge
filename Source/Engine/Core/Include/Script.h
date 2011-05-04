#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Scripts can be attached to game objects and are the ideal way of inserting code into the scene.
// Author: Michael Lyashenko
//============================================================================================================

class Script
{
public:

	// Allow Object class to create scripts and access internal members in order to simplify code
	friend class Object;
	friend class Core;

	// Script creation function delegate
	typedef FastDelegate<Script* (void)>  CreateDelegate;
	typedef Hash<CreateDelegate> List;

protected:

	struct Ignore
	{
		enum
		{
			PreUpdate		= 1 << 0,
			Update			= 1 << 1,
			PostUpdate		= 1 << 2,
			Fill			= 1 << 3,
			KeyPress		= 1 << 4,
			MouseMove		= 1 << 5,
			Scroll			= 1 << 6,
		};
	};

	Object*		mObject;
	Flags		mIgnore;
	bool		mEnabled;
	bool		mSerializable;

private:

	// INTERNAL: Registers a new script of the specified type
	static void _Register(const String& type, const CreateDelegate& func);

	// INTERNAL: Registers a new script of the specified type
	static void _Register(List& list, const String& type, const CreateDelegate& func);

	// INTERNAL: Removes the specified script from the list of registered scripts
	static void _UnRegister(const String& type);

	// INTERNAL: Removes the specified script from the list of registered scripts
	static void _UnRegister(List& list, const String& type);

	// INTERNAL: Creates a new script of the specified type
	static Script* _Create (const String& type);

	// INTERNAL: Creates a new script of the specified type
	static Script* _Create (List& list, const String& type);

protected:

	// Use the AddScript<> template to add new scripts
	Script() : mEnabled(true), mSerializable(true) {}

public:

	// Registers a new script
	template <typename Type> static void Register() { _Register( Type::ClassName(), &Type::_CreateNew ); }
	template <typename Type> static void UnRegister() { _UnRegister( Type::ClassName() ); }

	// Sets the replacement script type list that should be used instead of the built-in one
	static void SetTypeList (List* list);

	// Retrieves the local type list
	static List* GetTypeList();

public:

	// This is a top-level base class
	R5_DECLARE_INTERFACE_CLASS(Script);

	// Scripts should be removed via DestroySelf() or using the RemoveScript<> template
	virtual ~Script();

	// Access to the owner of the script
	Object* GetOwner() { return mObject; }
	const Object* GetOwner() const { return mObject; }

	// Convenience function: mObject->GetCore()->IsKeyDown(key);
	bool IsKeyDown (uint key);

	// Destroys this script - this action is queued until next update
	void DestroySelf();

	// It's possible to choose not to serialize certain scripts
	bool IsSerializable() const { return mSerializable; }
	void SetSerializable (bool val) { mSerializable = val; }

	// Serialization
	void SerializeTo (TreeNode& root) const;
	void SerializeFrom (const TreeNode& root);

protected:

	// Initialization function is called once the script has been created
	virtual void OnInit() {}

	// Callback triggered when DestroySelf() gets called
	virtual void OnDestroy() {}

	// Called prior to object's Update function
	virtual void OnPreUpdate() { mIgnore.Set(Ignore::PreUpdate, true); }

	// Called after the object's absolute coordinates have been calculated
	virtual void OnUpdate() { mIgnore.Set(Ignore::Update, true); }

	// Called after the object has updated all of its children
	virtual void OnPostUpdate() { mIgnore.Set(Ignore::PostUpdate, true); }

	// Called when the scene draw queue is being filled
	virtual void OnFill (FillParams& params) { mIgnore.Set(Ignore::Fill, true); }

	// Key and mouse events
	virtual bool OnKeyPress (const Vector2i& pos, byte key, bool isDown) { mIgnore.Set(Ignore::KeyPress,  true); return false; }
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta) { mIgnore.Set(Ignore::MouseMove, true); return false; }
	virtual bool OnScroll	(const Vector2i& pos, float delta)			 { mIgnore.Set(Ignore::Scroll,	  true); return false; }

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const {}
	virtual void OnSerializeFrom(const TreeNode& node) {}
};