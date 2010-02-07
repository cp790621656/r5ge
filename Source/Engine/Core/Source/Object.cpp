#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Register default object types
//============================================================================================================

void RegisterDefaultObjects()
{
	Object::Register<Object>();
	Object::Register<Camera>();
	Object::Register<DebugCamera>();
	Object::Register<AnimatedCamera>();
	Object::Register<ModelInstance>();
	Object::Register<DirectionalLight>();
	Object::Register<PointLight>();
	Object::Register<Terrain>();
	Object::Register<Decal>();
	Object::Register<Billboard>();
	Object::Register<Glare>();
	Object::Register<DirectionalBillboard>();
	Object::Register<DirectionalGlare>();
}

//============================================================================================================
// Registers all default scripts
//============================================================================================================

void RegisterDefaultScripts()
{
	Script::Register<BoneAttachment>();
}

//============================================================================================================
// Each object is enabled by default
//============================================================================================================

Object::Object() :
	mParent			(0),
	mCore			(0),
	mLayer			(10),
	mRelativeScale	(1.0f),
	mAbsoluteScale	(1.0f),
	mCalcAbsBounds	(true),
	mIsDirty		(false),
	mSerializable	(true)
{
	mFlags.Set(Flag::Enabled);
}

//============================================================================================================
// List of registered object and script types
//============================================================================================================

Hash<Object::CreateDelegate> gObjectTypes;

//============================================================================================================
// INTERNAL: Registers a new object
//============================================================================================================

void Object::_Register (const String& type, const CreateDelegate& callback)
{
	gObjectTypes.Lock();
	gObjectTypes[type] = callback;
	gObjectTypes.Unlock();
}

//============================================================================================================
// INTERNAL: Creates a new object of specified type
//============================================================================================================

Object* Object::_Create (const String& type)
{
	Object* ptr (0);
	gObjectTypes.Lock();
	{
		const CreateDelegate* callback = gObjectTypes.GetIfExists(type);
		if (callback != 0) ptr = (*callback)();
	}
	gObjectTypes.Unlock();
	return ptr;
}

//============================================================================================================
// INTERNAL: Adds a child object of specified type (or returns an existing one)
//============================================================================================================

Object* Object::_AddObject (const String& type, const String& name)
{
	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;
		RegisterDefaultObjects();
	}

	Object* ptr (0);

	for (uint i = mChildren.GetSize(); i > 0; )
	{
		Object* obj = mChildren[--i];

		if ( obj != 0 && name == obj->GetName() && type == obj->GetClassID() )
		{
			ptr = obj;
			break;
		}
	}

	if (ptr == 0)
	{
		ptr = Object::_Create(type);

		if (ptr != 0)
		{
			ptr->mCore = mCore;
			ptr->SetName(name);
			ptr->mParent = this;
			mChildren.Expand() = ptr;
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Finds an object in the scene
//============================================================================================================

Object* Object::_FindObject (const String& name, bool recursive)
{
	Object* node (0);

	if (mChildren.IsValid())
	{
		for (uint i = mChildren.GetSize(); i > 0; )
		{
			if ( name == mChildren[--i]->GetName() )
			{
				node = mChildren[i];
				break;
			}
		}

		if ( node == 0 && recursive )
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
				if ( node = mChildren[i]->_FindObject(name, recursive) )
					break;
		}
	}
	return node;
}

//============================================================================================================
// INTERNAL: Adds a script of specified type (or returns an existing one).
//============================================================================================================

Script* Object::_AddScript (const String& type)
{
	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;
		RegisterDefaultScripts();
	}

	Script* ptr (0);

	for (uint i = mScripts.GetSize(); i > 0; )
	{
		Script* script = mScripts[i];

		if ( script != 0 && type == script->GetClassID() )
		{
			ptr = script;
			break;
		}
	}

	if (ptr == 0)
	{
		ptr = Script::_Create(type);

		if (ptr != 0)
		{
			ptr->mObject = this;
			mScripts.Expand() = ptr;
			ptr->OnInit();
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Retrieves a script of specified type
//============================================================================================================

Script* Object::_GetScript (const String& type)
{
	Script* ptr (0);

	for (uint i = mScripts.GetSize(); i > 0; )
	{
		Script* script = mScripts[i];

		if ( script != 0 && type == script->GetClassID() )
		{
			ptr = script;
			break;
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Adds a child object
//============================================================================================================

void Object::_Add (Object* obj)
{
	if (obj != 0)
	{
		obj->mAbsolutePos	= obj->mRelativePos;
		obj->mAbsoluteRot	= obj->mRelativeRot;
		obj->mAbsoluteScale	= obj->mRelativeScale;

		obj->mRelativePos	= Interpolation::GetDifference(mAbsolutePos, obj->mAbsolutePos) / mAbsoluteScale;
		obj->mRelativeRot	= Interpolation::GetDifference(mAbsoluteRot, obj->mAbsoluteRot);
		obj->mRelativeScale	= obj->mAbsoluteScale / mAbsoluteScale;

		mChildren.Expand() = obj;
		mIsDirty = true;
	}
}

//============================================================================================================
// INTERNAL: Removes a child object
//============================================================================================================

void Object::_Remove (Object* obj)
{
	if (obj != 0)
	{
		mChildren.Remove(obj);

		obj->mIsDirty		= true;
		obj->mRelativePos	= obj->mAbsolutePos;
		obj->mRelativeRot	= obj->mAbsoluteRot;
		obj->mRelativeScale	= obj->mAbsoluteScale;

		mIsDirty = true;
	}
}

//============================================================================================================
// Releases the object and all its children
//============================================================================================================

void Object::Release()
{
	Lock();
	{
		mRelativeBounds.Reset();
		mCompleteBounds.Reset();

		if (mParent != 0)
		{
			mParent->_Remove(this);
			mParent = 0;
		}

		if (mScripts.IsValid())
		{
			// Run through all scripts
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				// Remove the reference to this object
				if (mScripts[--i] != 0)
				{
					mScripts[i]->mObject = 0;
				}
			}
			
			// Release all scripts
			mScripts.Release();
		}

		if (mChildren.IsValid())
		{
			// Run through all children
			for (uint i = mChildren.GetSize(); i > 0; )
			{
				// Remove all references to this object
				if (mChildren[--i] != 0)
				{
					mChildren[i]->mParent = 0;
				}
			}

			// Release all children
			mChildren.Release();
		}
	}
	Unlock();
}

//============================================================================================================
// Changes the parent object
//============================================================================================================

void Object::SetParent (Object* ptr)
{
	if (mParent != ptr)
    {
        Lock();
		{
			if (mParent) mParent->_Remove(this);
			mParent = ptr;
			if (mParent) mParent->_Add(this);
		}
        Unlock();
    }
}

//============================================================================================================
// Sets the position using an absolute value
//============================================================================================================

void Object::SetAbsolutePosition (const Vector3f& pos)
{
	mIsDirty = true;
	mAbsolutePos = pos;

	if (mParent != 0)
	{
		float invScale = 1.0f / mParent->GetAbsoluteScale();
		mRelativePos = Interpolation::GetDifference(mParent->GetAbsolutePosition(), mAbsolutePos) * invScale;
	}
	else mRelativePos = mAbsolutePos;
}

//============================================================================================================
// Sets the rotation using an absolute value
//============================================================================================================

void Object::SetAbsoluteRotation (const Quaternion& rot)
{
	mIsDirty = true;
	mAbsoluteRot = rot;

	if (mParent != 0)
	{
		mRelativeRot = Interpolation::GetDifference(mParent->GetAbsoluteRotation(), mAbsoluteRot);
	}
	else mRelativeRot = mAbsoluteRot;
}

//============================================================================================================
// Sets the position using an absolute value
//============================================================================================================

void Object::SetAbsoluteScale (float scale)
{
	mIsDirty = true;
	mAbsoluteScale = scale;

	if (mParent != 0)
	{
		mRelativeScale = mAbsoluteScale / mParent->GetAbsoluteScale();
	}
	else mRelativeScale = mAbsoluteScale;
}

//============================================================================================================
// OnKeyPress event notifier
//============================================================================================================

bool Object::KeyPress (const Vector2i& pos, byte key, bool isDown)
{
	bool retVal (false);

	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];
				retVal |= script->OnKeyPress(pos, key, isDown);
			}
			retVal |= OnKeyPress(pos, key, isDown);
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// Mouse movement event notification
//============================================================================================================

bool Object::MouseMove (const Vector2i& pos, const Vector2i& delta)
{
	bool retVal (false);

	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];
				retVal |= script->OnMouseMove(pos, delta);
			}
			retVal |= OnMouseMove(pos, delta);
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// Mouse scrolling event notification
//============================================================================================================

bool Object::Scroll (const Vector2i& pos, float delta)
{
	bool retVal (false);

	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];
				retVal |= script->OnScroll(pos, delta);
			}
			retVal |= OnScroll(pos, delta);
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// INTERNAL: Updates the object, calling appropriate virtual functions
//============================================================================================================

bool Object::Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved)
{
	bool retVal (false);

	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			// If the parent has moved then we need to recalculate the absolute values
			if (parentMoved) mIsDirty = true;

			// Run through all scripts and notify them
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];

				// If the script is listening to pre-update events
				if (!script->mIgnore.Get(Script::Ignore::PreUpdate))
				{
					script->OnPreUpdate();
				}
			}

			// Call the pre-update function
			if (!mIgnore.Get(Ignore::PreUpdate)) OnPreUpdate();

			// If something has changed, update the absolute values
			if (mIsDirty)
			{
				mAbsolutePos = (mRelativePos * scale) * rot + pos;
				mAbsoluteScale = mRelativeScale * scale;
				mAbsoluteRot.Combine(rot, mRelativeRot);
			}

			// Run through all scripts and notify them
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];

				// If the script is listening to pre-update events
				if (!script->mIgnore.Get(Script::Ignore::Update))
				{
					script->OnUpdate();
				}
			}

			// Call the update function
			if (!mIgnore.Get(Ignore::Update)) OnUpdate();

			// Update all children
			if (mChildren.IsValid())
			{
				parentMoved = mIsDirty;

				for (uint i = 0; i < mChildren.GetSize(); ++i)
				{
					Object* obj = mChildren[i];

					if (obj != 0 && obj->GetFlag(Flag::Enabled))
					{
						mIsDirty |= obj->Update(mAbsolutePos, mAbsoluteRot, mAbsoluteScale, parentMoved);
					}
				}
			}

			if (mIsDirty)
			{
				// Remember that there was a change
				retVal = true;

				// Recalculate absolute bounds
				if (mCalcAbsBounds)
				{
					if (mRelativeBounds.IsValid())
					{
						mAbsoluteBounds = mRelativeBounds;
						mAbsoluteBounds.Transform(mAbsolutePos, mAbsoluteRot, mAbsoluteScale);
					}
					else
					{
						mAbsoluteBounds.Reset();
					}
				}
				
				// Start with absolute bounds
				mCompleteBounds = mAbsoluteBounds;

				// Run through all children and include their bounds as well
				for (uint i = 0; i < mChildren.GetSize(); ++i)
				{
					Object* obj = mChildren[i];

					if (obj != 0 && obj->GetFlag(Flag::Enabled))
					{
						mCompleteBounds.Include(obj->GetCompleteBounds());
					}
				}
			}

			// Run through all scripts and notify them
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];

				// If the script is listening to pre-update events
				if (!script->mIgnore.Get(Script::Ignore::PostUpdate))
				{
					script->OnPostUpdate();
				}
			}

			// Call the post-update function
			if (!mIgnore.Get(Ignore::PostUpdate)) OnPostUpdate();

			// All absolute values have now been calculated
			mIsDirty = false;
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// INTERNAL: Fills the render queues
//============================================================================================================

void Object::Fill (FillParams& params)
{
	if (mFlags.Get(Flag::Enabled))
	{
		// Cull the bounding volume if it's available. Note that we check the absolute bounds but actually
		// use the complete bounds. This is because complete bounds can be valid even if absolute aren't.
		bool isVisible = mAbsoluteBounds.IsValid() ? params.mFrustum.IsVisible(mCompleteBounds) : true;

		mFlags.Set(Flag::Visible, isVisible);

		if (isVisible)
		{
			Lock();
			{
				// Inform script listeners of this event
				for (uint i = mScripts.GetSize(); i > 0; )
				{
					Script* script = mScripts[--i];

					if (!script->mIgnore.Get(Script::Ignore::Fill))
					{
						script->OnFill(params);
					}
				}

				// Trigger the virtual function if it's available
				bool considerChildren = mIgnore.Get(Ignore::Fill) ? true : OnFill(params);

				if (considerChildren)
				{
					// Recurse through all children
					for (uint i = 0; i < mChildren.GetSize(); ++i)
					{
						mChildren[i]->Fill(params);
					}
				}
			}
			Unlock();
		}
	}
}

//============================================================================================================
// INTERNAL: Recursive Object::OnSelect caller
//============================================================================================================

void Object::Select (const Vector3f& pos, ObjectPtr& ptr, float& distance)
{
	if (mFlags.Get(Flag::Visible) && mCompleteBounds.Contains(pos))
	{
		Lock();
		{
			// If this object is marked as 'selectable', try to select it
			if (mFlags.Get(Flag::Selectable))
			{
				float current = mAbsolutePos.GetDistanceTo(pos);

				if (current < distance)
				{
					distance = current;
					ptr = this;
				}
			}

			bool considerChildren = true;

			if (!mIgnore.Get(Ignore::Select))
			{
				considerChildren = OnSelect(pos, ptr, distance);
			}

			if (considerChildren)
			{
				for (uint i = mChildren.GetSize(); i > 0; )
					mChildren[--i]->Select(pos, ptr, distance);
			}
		}
		Unlock();
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Object::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		Lock();
		{
			TreeNode& node = root.AddChild( GetClassID(), mName );
			node.AddChild("Position", mRelativePos);
			node.AddChild("Rotation", mRelativeRot);
			node.AddChild("Scale", mRelativeScale);
			node.AddChild("Layer", mLayer);

			if (!mIgnore.Get(Ignore::SerializeTo))
				OnSerializeTo(node);

			for (uint i = 0; i < mScripts.GetSize(); ++i)
				mScripts[i]->SerializeTo(node);

			for (uint i = 0; i < mChildren.GetSize(); ++i)
				mChildren[i]->SerializeTo(node);
		}
		Unlock();
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Object::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	Lock();
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if		(tag == "Serializable")		  value >> mSerializable;
			else if	(tag == "Position")		{ if (value >> mRelativePos)	mIsDirty = true; }
			else if (tag == "Rotation")		{ if (value >> mRelativeRot)	mIsDirty = true; }
			else if (tag == "Scale")		{ if (value >> mRelativeScale)	mIsDirty = true; }
			else if (tag == "Layer")
			{
				uint temp;
				if (value >> temp) mLayer = (byte)(temp & 31);
			}
			else if (tag == Script::ClassID())
			{
				// The 'contains' check is here because scripts can self-destruct inside OnInit()
				Script* ptr = _AddScript(value.IsString() ? value.AsString() : value.GetString());
				if (ptr != 0 && mScripts.Contains(ptr)) ptr->SerializeFrom(node);
			}
			else if (mIgnore.Get(Ignore::SerializeFrom) || !OnSerializeFrom(node))
			{
				Object* ptr = _AddObject(tag, value.IsString() ? value.AsString() : value.GetString());
				if (ptr != 0) ptr->SerializeFrom(node, forceUpdate);
			}
		}
	}
	Unlock();
	return true;
}

//============================================================================================================
// Called prior to object's Update function
//============================================================================================================

void Object::OnPreUpdate()
{
	mIgnore.Set(Ignore::PreUpdate, true);
}

//============================================================================================================
// Called after the object's absolute coordinates have been calculated
//============================================================================================================

void Object::OnUpdate()
{
	mIgnore.Set(Ignore::Update, true);
}

//============================================================================================================
// Called after the object has updated all of its children
//============================================================================================================

void Object::OnPostUpdate()
{
	mIgnore.Set(Ignore::PostUpdate, true);
}

//============================================================================================================
// Called when the scene is being culled. Should update the 'mask' parameter.
//============================================================================================================

bool Object::OnFill (FillParams& params)
{
	mIgnore.Set(Ignore::Fill, true);
	return true;
}

//============================================================================================================
// Called when the object is being selected -- may update the referenced values
//============================================================================================================

bool Object::OnSelect (const Vector3f& pos, ObjectPtr& ptr, float& radius)
{
	mIgnore.Set(Ignore::Select, true);
	return true;
}

//============================================================================================================
// Called when the object is being loaded
//============================================================================================================

bool Object::OnSerializeFrom (const TreeNode& root)
{
	mIgnore.Set(Ignore::SerializeFrom, true);
	return false;
}

//============================================================================================================
// Called when the object is being saved
//============================================================================================================

void Object::OnSerializeTo (TreeNode& root) const
{
	mIgnore.Set(Ignore::SerializeTo, true);
}