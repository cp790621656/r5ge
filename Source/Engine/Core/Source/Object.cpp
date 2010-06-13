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
	Object::Register<Octree>();
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
	Script::Register<OSAttachToBone>();
}

//============================================================================================================
// Each object is enabled by default
//============================================================================================================

Object::Object() :
	mParent			(0),
	mSubParent		(0),
	mCore			(0),
	mLayer			(10),
	mRelativeScale	(1.0f),
	mAbsoluteScale	(1.0f),
	mCalcRelBounds	(true),
	mCalcAbsBounds	(true),
	mIncChildBounds	(true),
	mIsDirty		(false),
	mHasMoved		(true),
	mSerializable	(true),
	mShowOutline	(false)
{
	mFlags.Set(Flag::Enabled | Flag::BoxCollider);
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
			ptr->mIsDirty = true;
			mChildren.Expand() = ptr;

			// Inform the listeners that a new child has been added
			OnAddChild(ptr);

			// Initialize the object
			ptr->OnInit();
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Finds an object in the scene
//============================================================================================================

const Object* Object::_FindObject (const String& name, bool recursive, bool threadSafe) const
{
	const Object* node (0);

	if (mChildren.IsValid())
	{
		if (threadSafe) mChildren.Lock();
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
		if (threadSafe) mChildren.Unlock();
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

	FOREACH(i, mScripts)
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

const Script* Object::_GetScript (const String& type) const
{
	const Script* ptr (0);

	FOREACH(i, mScripts)
	{
		const Script* script = mScripts[i];

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
		obj->mAbsoluteVel	= mAbsoluteVel + mRelativeVel;

		obj->mRelativePos	= Interpolation::GetDifference(mAbsolutePos, obj->mAbsolutePos) / mAbsoluteScale;
		obj->mRelativeRot	= Interpolation::GetDifference(mAbsoluteRot, obj->mAbsoluteRot);
		obj->mRelativeScale	= obj->mAbsoluteScale / mAbsoluteScale;
		obj->mLastPos		= obj->mRelativePos;

		mChildren.Expand() = obj;
		mIsDirty = true;

		OnAddChild(obj);
	}
}

//============================================================================================================
// INTERNAL: Removes a child object
//============================================================================================================

void Object::_Remove (Object* obj)
{
	if (obj != 0 && mChildren.Remove(obj))
	{
		OnRemoveChild(obj);

		obj->mIsDirty		= true;
		obj->mSubParent		= 0;
		obj->mRelativePos	= obj->mAbsolutePos;
		obj->mRelativeRot	= obj->mAbsoluteRot;
		obj->mRelativeScale	= obj->mAbsoluteScale;
		obj->mRelativeVel	= obj->mAbsoluteVel;
		obj->mLastPos		= obj->mRelativePos;

		mIsDirty = true;
	}
}

//============================================================================================================
// Debugging section used to show the actual bounding box itself
//============================================================================================================

void GetBoundingBoxLines (Array<Vector3f>& v, const Vector3f& min, const Vector3f& max)
{
	// Bottom X
	v.Expand().Set(min.x, min.y, min.z);
	v.Expand().Set(max.x, min.y, min.z);
	v.Expand().Set(min.x, max.y, min.z);
	v.Expand().Set(max.x, max.y, min.z);
	
	// Top X
	v.Expand().Set(min.x, min.y, max.z);
	v.Expand().Set(max.x, min.y, max.z);
	v.Expand().Set(min.x, max.y, max.z);
	v.Expand().Set(max.x, max.y, max.z);

	// Bottom Y
	v.Expand().Set(min.x, min.y, min.z);
	v.Expand().Set(min.x, max.y, min.z);
	v.Expand().Set(max.x, min.y, min.z);
	v.Expand().Set(max.x, max.y, min.z);

	// Top Y
	v.Expand().Set(min.x, min.y, max.z);
	v.Expand().Set(min.x, max.y, max.z);
	v.Expand().Set(max.x, min.y, max.z);
	v.Expand().Set(max.x, max.y, max.z);

	// Bottom Z
	v.Expand().Set(min.x, min.y, min.z);
	v.Expand().Set(min.x, min.y, max.z);
	v.Expand().Set(max.x, min.y, min.z);
	v.Expand().Set(max.x, min.y, max.z);

	// Top Z
	v.Expand().Set(min.x, max.y, min.z);
	v.Expand().Set(min.x, max.y, max.z);
	v.Expand().Set(max.x, max.y, min.z);
	v.Expand().Set(max.x, max.y, max.z);
}

//============================================================================================================
// Draws the outline of the bounding box
//============================================================================================================

uint Object::_DrawOutline (IGraphics* graphics, const ITechnique* tech)
{
	uint result(0);

	// If the model's outline is requested, draw it now
	if (tech->GetColorWrite())
	{
		graphics->ResetModelMatrix();

		Array<Vector3f> v;
		const Bounds& complete = GetCompleteBounds();
		GetBoundingBoxLines(v, complete.GetMin(), complete.GetMax());

		float factor = Float::Abs(Float::Cos(4.0f * Time::GetTime()));
		bool stencil = graphics->GetStencilTest();

		graphics->SetActiveMaterial (0);
		graphics->SetActiveColor( Color4f(factor, 1.0f - factor, 0, 1) );

		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Normal,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Color,		0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord0,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord1,	0);
		graphics->SetActiveVertexAttribute(IGraphics::Attribute::Position,	v);

		result += graphics->DrawVertices( IGraphics::Primitive::Line, v.GetSize() );

		// Restore the active technique
		graphics->SetStencilTest(stencil);
		graphics->SetActiveColor( Color4f(1.0f) );
		graphics->SetActiveTechnique(tech);
	}
	return result;
}

//============================================================================================================
// Destroys the object
//============================================================================================================

void Object::DestroySelf (bool threadSafe)
{
	if (mParent != 0)
	{
		if (threadSafe) Lock();
		{
			mScripts.Lock();
			{
				FOREACH(i, mScripts)
				{
					Script* script = mScripts[i];
					script->OnDestroy();
				}
			}
			mScripts.Unlock();

			mParent->_Remove(this);
			mParent->mDeletedObjects.Expand() = this;
			mParent = 0;
		}
		if (threadSafe) Unlock();
	}
}

//============================================================================================================
// Releases the object and all its children
//============================================================================================================

void Object::Release (bool threadSafe)
{
	if (threadSafe) Lock();
	{
		mCalcRelBounds = true;
		mCalcAbsBounds = true;

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
	if (threadSafe) Unlock();
}

//============================================================================================================
// Convenience functionality
//============================================================================================================

IGraphics* Object::GetGraphics()
{
	return (mCore != 0) ? mCore->GetGraphics() : 0;
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
			if (mParent != 0) mParent->_Remove(this);

			mParent		= ptr;
			mSubParent	= 0;
			mIsDirty	= true;

			if (mParent != 0) mParent->_Add(this);
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
// Convenience function -- force-updates the object based on the parent's absolute values
//============================================================================================================

void Object::Update (bool threadSafe)
{
	if (mParent != 0 && mFlags.Get(Flag::Enabled))
	{
		if (threadSafe) Lock();
		{
			Update(
				mParent->GetAbsolutePosition(),
				mParent->GetAbsoluteRotation(),
				mParent->GetAbsoluteScale(), false, false);
		}
		if (threadSafe) Unlock();
	}
}

//============================================================================================================
// INTERNAL: Updates the object, calling appropriate virtual functions
//============================================================================================================

bool Object::Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved, bool threadSafe)
{
	mHasMoved = false;
	bool retVal (false);

	if (mDeletedObjects.IsValid()) mDeletedObjects.Clear();
	if (mDeletedScripts.IsValid()) mDeletedScripts.Clear();

	if (mFlags.Get(Flag::Enabled))
	{
		if (threadSafe) Lock();
		{
			// Calculate the velocity since last update
			mRelativeVel = (mRelativePos - mLastPos) / Time::GetDelta();
			mAbsoluteVel = mRelativeVel;
			if (mParent != 0) mAbsoluteVel += mParent->GetAbsoluteVelocity();
			mLastPos = mRelativePos;

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
				mHasMoved = true;
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
						mIsDirty |= obj->Update(mAbsolutePos, mAbsoluteRot, mAbsoluteScale, parentMoved, threadSafe);
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
				if (mIncChildBounds)
				{
					for (uint i = 0; i < mChildren.GetSize(); ++i)
					{
						Object* obj = mChildren[i];

						if (obj != 0 && obj->GetFlag(Flag::Enabled))
						{
							mCompleteBounds.Include(obj->GetCompleteBounds());
						}
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
		if (threadSafe) Unlock();
	}
	else
	{
		// The object is disabled -- no local velocity, and use parent's velocity for absolute
		mRelativeVel.Set(0.0f, 0.0f, 0.0f);
		mAbsoluteVel = (mParent != 0) ? mParent->GetAbsoluteVelocity() : mRelativeVel;
	}
	return retVal;
}

//============================================================================================================
// Fills the render queues
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

				if (mShowOutline)
				{
					static ITechnique* wireframe = mCore->GetGraphics()->GetTechnique("Wireframe");
					params.mDrawQueue.Add(mLayer, this, wireframe->GetMask(), 0, 0.0f);
				}
			}
			Unlock();
		}
	}
}

//============================================================================================================
// Draws the object with the specified technique
//============================================================================================================

uint Object::Draw (const Deferred::Storage& storage, uint group, const ITechnique* tech, bool insideOut)
{
	uint result (0);

	if (!mIgnore.Get(Ignore::Draw))
	{
		result += OnDraw(storage, group, tech, insideOut);
	}

	// Debugging functionality
	if (mShowOutline && group == 0)
	{
		static IGraphics*  graphics  = mCore->GetGraphics();
		static ITechnique* wireframe = graphics->GetTechnique("Wireframe");

		if (tech == wireframe)
		{
			result += _DrawOutline(graphics, tech);
		}
	}
	return result;
}

//============================================================================================================
// Cast a ray into space and fill the list with objects that it intersected with
//============================================================================================================

void Object::Raycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits, bool threadSafe)
{
	if (mFlags.Get(Flag::BoxCollider) && Intersect::RayBounds(pos, dir, mCompleteBounds))
	{
		bool considerChildren = true;

		// Try the custom virtual functionality first
		if (!mIgnore.Get(Ignore::Raycast))
		{
			considerChildren = OnRaycast(pos, dir, hits);
		}

		// If we're ignoring virtual functionality and we have a collider flag set
		if (mIgnore.Get(Ignore::Raycast) && mFlags.Get(Flag::BoxCollider))
		{
			// If the ray intersects with our bounds, join the hit list
			if (Intersect::RayBounds(pos, dir, mAbsoluteBounds))
			{
				RaycastHit& hit = hits.Expand();
				hit.mObject = this;
				hit.mSqrCamDist = (pos - mAbsoluteBounds.GetCenter()).Dot();
			}
		}

		// Continue on to children
		if (considerChildren)
		{
			if (threadSafe) Lock();
			{
				for (uint i = mChildren.GetSize(); i > 0; )
					mChildren[--i]->Raycast(pos, dir, hits, threadSafe);
			}
			if (threadSafe) Unlock();
		}
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

			if (!mRelativePos.IsZero())		node.AddChild("Position", mRelativePos);
			if (!mRelativeRot.IsIdentity()) node.AddChild("Rotation", mRelativeRot);
			if (mRelativeScale != 1.0f)		node.AddChild("Scale", mRelativeScale);

			if (!mCalcRelBounds && mRelativeBounds.IsValid())
			{
				node.AddChild("Min", mRelativeBounds.GetMin());
				node.AddChild("Max", mRelativeBounds.GetMax());
			}

			if (mLayer != 10) node.AddChild("Layer", mLayer);

			if (mShowOutline) node.AddChild("Show Outline", mShowOutline);

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

bool Object::SerializeFrom (const TreeNode& root, bool forceUpdate, bool threadSafe)
{
	if (threadSafe) Lock();
	{
		bool serializable = true;

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if (tag == "Serializable")
			{
				value >> serializable;
				mSerializable = serializable;
			}
			else if	(tag == "Position")		{ if (value >> mRelativePos)	mIsDirty = true; }
			else if (tag == "Rotation")		{ if (value >> mRelativeRot)	mIsDirty = true; }
			else if (tag == "Scale")		{ if (value >> mRelativeScale)	mIsDirty = true; }
			else if (tag == "Min" || tag == "Max")
			{
				Vector3f v;

				if (value >> v)
				{
					mCalcRelBounds = false;
					mRelativeBounds.Include(v);
					mIsDirty = true;
				}
			}
			else if (tag == "Layer")
			{
				uint temp;

				if (value >> temp)
				{
					mLayer = (byte)(temp & 31);
					mIsDirty = true;
				}
			}
			else if (tag == "Show Outline")
			{
				mShowOutline = value.IsBool() ? value.AsBool() : true;
			}
			else if (tag == Script::ClassID())
			{
				// The 'contains' check is here because scripts can self-destruct inside OnInit()
				Script* ptr = _AddScript(value.IsString() ? value.AsString() : value.GetString());

				if (ptr != 0 && mScripts.Contains(ptr))
				{
					// The object should not remain locked during script serialization
					if (threadSafe) Unlock();
					ptr->SerializeFrom(node);
					if (!serializable && mParent == 0) ptr->SetSerializable(false);
					if (threadSafe) Lock();
				}
			}
			else if (mIgnore.Get(Ignore::SerializeFrom) || !OnSerializeFrom(node))
			{
				Object* ptr = _AddObject(tag, value.IsString() ? value.AsString() : value.GetString());

				if (ptr != 0)
				{
					// The object should not remain locked during child serialization
					if (threadSafe) Unlock();
					ptr->SerializeFrom(node, forceUpdate);
					if (!serializable && mParent == 0) ptr->SetSerializable(false);
					ptr->SetDirty();
					if (threadSafe) Lock();
				}
			}
		}
	}
	if (threadSafe) Unlock();
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
// Called when the object is being raycast into -- should return 'false' if children were already considered
//============================================================================================================

bool Object::OnRaycast (const Vector3f& pos, const Vector3f& dir, Array<RaycastHit>& hits)
{
	mIgnore.Set(Ignore::Raycast, true);
	return true;
}

//============================================================================================================
// Called when the object is being loaded
//============================================================================================================

bool Object::OnSerializeFrom (const TreeNode& node)
{
	mIgnore.Set(Ignore::SerializeFrom, true);
	return false;
}

//============================================================================================================
// Called when the object is being saved
//============================================================================================================

void Object::OnSerializeTo (TreeNode& node) const
{
	mIgnore.Set(Ignore::SerializeTo, true);
}