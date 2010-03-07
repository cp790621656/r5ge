#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Most basic game object -- can be positioned somewhere in the scene and can contain children.
//============================================================================================================

class Core;
class Object
{
	friend class Script;	// Script needs access to 'mScripts' so it can remove itself
	friend class Scene;		// Scene needs to be able to use 'mCore'
	friend class Core;		// Core needs to be able to set 'mCore'

public:

	// Object creation delegate
	typedef FastDelegate<Object* (void)> CreateDelegate;

	// Flags used by the Object
	struct Flag
	{
		enum
		{
			Enabled			= 1 << 1,
			Visible			= 1 << 2,
			BoxCollider		= 1 << 3,
		};
	};

	// Types used by this class
	typedef Object*					ObjectPtr;
	typedef PointerArray<Object>	Children;
	typedef PointerArray<Script>	Scripts;
	typedef Thread::Lockable		Lockable;

protected:

	String		mName;				// Name of this object
	Flags		mFlags;				// Internal flags associated with this object
	Object*		mParent;			// Object's parent
	void*		mSubParent;			// Optional abstract sub-parent the object belongs to (such as a QuadTree node)
	Core*		mCore;				// Engine's core that the object was created with
	byte		mLayer;				// Draw layer on which this object resides

	Vector3f	mRelativePos;		// Local space position
	Quaternion	mRelativeRot;		// Local space rotation
	float		mRelativeScale;		// Local space scale
	Vector3f	mAbsolutePos;		// World space position, calculated every Update()
	Quaternion	mAbsoluteRot;		// World space rotation
	float		mAbsoluteScale;		// World space scale
	
	Bounds		mRelativeBounds;	// Local bounds
	Bounds		mAbsoluteBounds;	// Calculated bounds that include only this object
	Bounds		mCompleteBounds;	// Calculated bounds that include the bounds of all children
	bool		mCalcAbsBounds;		// Whether absolute bounds will be auto-calculated ('true' in most cases)
	bool		mIncChildBounds;	// Whether to include children when re-calculating the bounds ('true' in most cases)

	bool		mIsDirty;			// Whether the object's absolute coordinates should be recalculated
	bool		mHasMoved;			// Whether the object has moved since last update
	bool		mSerializable;		// Whether the object will be serialized out
	bool		mShowOutline;		// Whether to show the bounding outline -- useful for debugging

	Lockable	mLock;
	Children	mChildren;
	Children	mDeletedObjects;
	Scripts		mScripts;
	Scripts		mDeletedScripts;

private:

	// In order to eliminate virtual function calls, all virtual functions in this class
	// automatically set these flags, thus marking themselves as not being overwritten,
	// which in turn will make sure that they are never called again.

	struct Ignore
	{
		enum
		{
			Fill			= 1 << 1,
			PreUpdate		= 1 << 2,
			Update			= 1 << 3,
			PostUpdate		= 1 << 4,
			Raycast			= 1 << 5,
			Draw			= 1 << 6,
			SerializeFrom	= 1 << 7,
			SerializeTo		= 1 << 8,
		};
	};

	mutable Flags mIgnore;

protected:

	// Objects should never be created manually. Use AddObject<> template instead.
	Object();

public:

	virtual ~Object() { Release(false); }

	// This is a top-level base class
	R5_DECLARE_BASE_CLASS("Object", Object);

private:

	// INTERNAL Registers a new object of specified type
	static void _Register (const String& type, const CreateDelegate& callback);

	// INTERNAL: Creates a new object of specified type
	static Object* _Create (const String& type);

	Object*	_AddObject	(const String& type, const String& name);
    Object*	_FindObject	(const String& name, bool recursive = true, bool threadSafe = true);
	Script* _AddScript	(const String& type);
	Script* _GetScript	(const String& type);

	void _Add	(Object* ptr);
	void _Remove(Object* ptr);

	// Draws the outline of the bounding box
	uint _DrawOutline (IGraphics* graphics, const ITechnique* tech);

public:

	// Destroys the object -- this action is queued until next update
	void DestroySelf (bool threadSafe = true);

	// Release all resources associated with this object
	void Release (bool threadSafe = true);

	// Thread-safe locking functionality
	void Lock()		const	{ mLock.Lock(); }
	void Unlock()	const	{ mLock.Unlock(); }

	// Retrieves the Core that was ultimately owns this object
	inline Core* GetCore() { return mCore; }

	// Convenience functionality
	IGraphics* GetGraphics();

	// Returns whether the object is currently visible
	bool IsVisible() const { return mFlags.Get(Flag::Visible) && (mParent == 0 && mParent->IsVisible()); }

	// Name and flag field retrieval
	const String&		GetName()				const	{ return mName;				}
	const Object*		GetParent()				const	{ return mParent;			}
	void*				GetSubParent()					{ return mSubParent;		}
	const void*			GetSubParent()			const	{ return mSubParent;		}
	byte				GetLayer()				const	{ return mLayer;			}
	const Flags&		GetFlags()				const	{ return mFlags;			}
	bool				GetFlag (uint flag)		const	{ return mFlags.Get(flag);	}
	bool				IsDirty()				const	{ return mIsDirty;			}
	bool				IsShowingOutline()		const	{ return mShowOutline;		}
	bool				HasMoved()				const	{ return mHasMoved;			}
	bool				IsSerializable()		const	{ return mSerializable;		}
	Children&			GetChildren()					{ return mChildren;			}
	const Children&		GetChildren()			const	{ return mChildren;			}
	const Scripts&		GetScripts()			const	{ return mScripts;			}

	// Retrieves relative (local space) position / rotation
	const Vector3f&		GetRelativePosition()	const	{ return mRelativePos;		}
	const Quaternion&	GetRelativeRotation()	const	{ return mRelativeRot;		}
	float				GetRelativeScale()		const	{ return mRelativeScale;	}

	// Retrieves absolute (world space) position / rotation
	const Vector3f&		GetAbsolutePosition()	const	{ return mAbsolutePos;		}
	const Quaternion&	GetAbsoluteRotation()	const	{ return mAbsoluteRot;		}
	float				GetAbsoluteScale()		const	{ return mAbsoluteScale;	}

	// Retrieves object bounds
	const Bounds&		GetRelativeBounds()		const	{ return mRelativeBounds;	}
	const Bounds&		GetAbsoluteBounds()		const	{ return mAbsoluteBounds;	}
	const Bounds&		GetCompleteBounds()		const	{ return mCompleteBounds;	}

public:

	// Every object has a name that can be changed
	void SetName (const String& name) { mName = name; }

	// Every object can have object-specific flags that can be changed
	void SetFlag (uint flag, bool val) { mFlags.Set(flag, val); }

	// Marks the object as dirty, making it recalculate its absolute position next update
	void SetDirty() { mIsDirty = true; }

	// It's possible to switch object's parents
	void SetParent (Object* ptr);

	// Sets the optional sub-parent
	void SetSubParent (void* ptr) { mSubParent = ptr; }

	// Changes this object's draw layer
	void SetLayer (byte layer) { mLayer = (byte)(layer & 31); }

	// Whether the object will be saved out
	void SetSerializable (bool val) { mSerializable = val; }

	// Whether to show the object's bounding box
	void SetShowOutline	(bool val) { mShowOutline = val; }

	// Sets all relative values
	void SetRelativePosition ( const Vector3f& pos )	{ mRelativePos	  = pos;	mIsDirty = true; }
	void SetRelativeRotation ( const Quaternion& rot )	{ mRelativeRot	  = rot;	mIsDirty = true; }
	void SetRelativeScale	 ( float scale )			{ mRelativeScale  = scale;	mIsDirty = true; }
	void SetRelativeBounds	 ( const Bounds& b )		{ mRelativeBounds = b;		mIsDirty = true; }

	// Sets both relative and absolute values using provided absolute values
	void SetAbsolutePosition ( const Vector3f& pos );
	void SetAbsoluteRotation ( const Quaternion& rot );
	void SetAbsoluteScale	 ( float scale );

	// It should be possible to temporarily override the absolute values
	void OverrideAbsolutePosition (const Vector3f&   pos)	{ mAbsolutePos	 = pos;		}
	void OverrideAbsoluteRotation (const Quaternion& rot)	{ mAbsoluteRot	 = rot;		}
	void OverrideAbsoluteScale	  (float scale)				{ mAbsoluteScale = scale;	}

	// Key press event notifier
	bool KeyPress (const Vector2i& pos, byte key, bool isDown);

	// Mouse movement event notification
	bool MouseMove (const Vector2i& pos, const Vector2i& delta);

	// Mouse scrolling event notification
	bool Scroll (const Vector2i& pos, float delta);

	// Convenience function -- force-updates the object based on the parent's absolute values
	void Update (bool threadSafe = true);

	// Updates the object, calling appropriate virtual functions
	bool Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved, bool threadSafe = true);

	// Fills the render queues and updates the visibility mask
	void Fill (FillParams& params);

	// Draws the object with the specified technique
	uint Draw (uint group, const ITechnique* tech, bool insideOut);

	// Cast a ray into space and fill the list with objects that it intersected with
	void Raycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits);

	// Serialization
	bool SerializeTo (TreeNode& root) const;
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);

protected:

	// NOTE: Don't forget to set 'mIsDirty' to 'true' if you modify relative coordinates
	// in your virtual functions, or absolute coordinates won't be recalculated!

	// The very first function called on the object -- called after the object's parent and root have been set
	virtual void OnInit() {}

	// Function called when a new child object has been added
	virtual void OnAddChild (Object* obj) {}

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj) {}

	// Called by a manually triggered function: key press event notification
	virtual bool OnKeyPress (const Vector2i& pos, byte key, bool isDown) { return false; }

	// Called by a manually triggered function: mouse movement event notification
	virtual bool OnMouseMove (const Vector2i& pos, const Vector2i& delta) { return false; }

	// Called by a manually triggered function: mouse scroll event notification
	virtual bool OnScroll (const Vector2i& pos, float delta) { return false; }

	// Called prior to object's Update function, before absolute coordinates are calculated
	virtual void OnPreUpdate();

	// Called after the object's absolute coordinates have been calculated
	// but before all children and absolute bounds have been updated.
	virtual void OnUpdate();

	// Called after the object has updated all of its children and absolute bounds
	virtual void OnPostUpdate();

	// Called when the scene is being culled. Should update the 'mask' parameter.
	// Return 'true' if the Object should recurse through children, 'false' if the function already did.
	virtual bool OnFill (FillParams& params);

	// Draw the object using the specified technique. This function will only be
	// called if this object has been added to the list of drawable objects in
	// OnFill. It should return the number of triangles rendered.
	virtual uint OnDraw (uint group, const ITechnique* tech, bool insideOut) { mIgnore.Set(Ignore::Draw, true); return 0; }

	// Called when the object is being raycast into -- should return 'false' if children were already considered
	virtual bool OnRaycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits);

	// Called when the object is being saved
	virtual void OnSerializeTo (TreeNode& node) const;

	// Called when the object is being loaded
	virtual bool OnSerializeFrom (const TreeNode& node);

public:

	// Registers a new object of the specified type
	template <typename Type> static void Register() { _Register( Type::ClassID(), &Type::_CreateNew ); }

	// Finds a child object of the specified name and type
	template <typename Type> Type* FindObject (const String& name, bool recursive = true, bool threadSafe = true)
	{
		if (threadSafe) Lock();
		Object* obj = _FindObject(name, recursive, threadSafe);
		if (threadSafe) Unlock();
		return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
	}

	// Creates a new child of specified type and name
	template <typename Type> Type* AddObject (const String& name, bool threadSafe = true)
	{
		if (threadSafe) Lock();
		Object* obj = _AddObject(Type::ClassID(), name);
		if (threadSafe) Unlock();
		return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
	}

	// Retrieves an existing script on the object
	template <typename Type> Type* GetScript(bool threadSafe = true)
	{
		if (threadSafe) Lock();
		Script* script = _GetScript(Type::ClassID());
		if (threadSafe) Unlock();
		return ( script != 0 && script->IsOfClass(Type::ClassID()) ) ? (Type*)script : 0;
	}

	// Adds a new script to the object (only one script of each type can be active on the object)
	template <typename Type> Type* AddScript(bool threadSafe = true)
	{
		if (threadSafe) Lock();
		Script* script = _AddScript(Type::ClassID());
		if (threadSafe) Unlock();
		return ( script != 0 && script->IsOfClass(Type::ClassID()) ) ? (Type*)script : 0;
	}

	// Removes the specified script from the game object
	template <typename Type> void RemoveScript(bool threadSafe = true)
	{
		if (threadSafe) Lock();
		Script* script = _GetScript(Type::ClassID());
		if (script != 0) delete script;
		if (threadSafe) Unlock();
	}
};