#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Find an area by position that will respond to events
//============================================================================================================

Area* Area::_FindChild (const Vector2i& pos)
{
	Area* ptr (0);
	if ( mReceivesEvents && mRegion.Contains(pos) && mRegion.IsVisible() )
	{
		mChildren.Lock();
		{
			for (uint i = mChildren.GetSize(); i > 0; )
				if (mChildren[--i] && (ptr = mChildren[i]->_FindChild(pos)))
					break;
		}
		mChildren.Unlock();

		// No child area claims to own the position, but it's still inside this area's
		if (ptr == 0) ptr = this;
	}
	return ptr;
}

//============================================================================================================
// Simple search
//============================================================================================================

Area* Area::_FindChild (const String& name, bool recursive)
{
	Area* ptr (0);
	mChildren.Lock();
	{
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			if (mChildren[i] && mChildren[i]->GetName() == name)
			{
				ptr = mChildren[i];
				break;
			}
		}

		if (recursive && ptr == 0)
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
			{
				if (mChildren[i] && (ptr = mChildren[i]->_FindChild(name)) )
				{
					break;
				}
			}
		}
	}
	mChildren.Unlock();
	return ptr;
}

//============================================================================================================
// Adds a unique named child, or returns an existing area if it is already present
//============================================================================================================

Area* Area::_AddChild (const String& type, const String& name, bool unique)
{
	Area* ptr (0);
	mChildren.Lock();
	{
		if (unique)
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
			{
				if (mChildren[i] && mChildren[i]->GetName() == name)
				{
					ptr = mChildren[i];
					break;
				}
			}
		}

		if (ptr == 0)
		{
			if ( ptr = mRoot->_CreateArea(type, name, this) )
			{
				if (mRegion.IsVisible()) ptr->Update(mRegion);
				mChildren.Expand() = ptr;
			}
		}
	}
	mChildren.Unlock();
	return ptr;
}

//============================================================================================================
// Layer manipulation
//============================================================================================================

void Area::SetLayer (int layer, bool setDirty)
{
	if (mLayer != layer)
	{
		if (setDirty) SetDirty();
		mLayer = layer;
		OnLayerChanged();
		if (setDirty) SetDirty();

		// NOTE: I do not recurse through children, which in essence makes this layer only local.
		// If in the future I decide to implement local / absolute layers, recurse here.
	}
}

//============================================================================================================
// Deletes all child areas
//============================================================================================================

void Area::DeleteAllChildren()
{
	if (mChildren.IsValid())
	{
		SetDirty();

		mChildren.Lock();
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
			{
				Area* area = mChildren[i];

				if (area != 0)
				{
					mRoot->RemoveAllReferencesTo(area);
				}
			}

			mChildren.Release();
		}
		mChildren.Unlock();
	}
}

//============================================================================================================
// Brings the area to the foreground
//============================================================================================================

void Area::BringToFront (Area* child)
{
	if (child != 0)
	{
		mChildren.Lock();
		{
			if (mChildren.Remove(child))
			{
				mChildren.Expand() = child;
			}
		}
		mChildren.Unlock();
		child->SetDirty();
	}

	if (mParent) mParent->BringToFront(this);
	else mRoot->_BringToFront(this);
}

//============================================================================================================
// Gives this area undivided attention
//============================================================================================================

void Area::SetFocus (bool focus)
{
	if (focus)
	{
		mRoot->_SetEventArea(this);
		mRoot->_SetFocusArea(this);

		BringToFront();
	}
	else
	{
		const Area* focus = mRoot->GetEventArea();
		const Area* input = mRoot->GetFocusArea();
		const Area* hover = mRoot->GetHoverArea();

		if (focus != 0 && focus->IsChildOf(this)) mRoot->_SetEventArea(0);
		if (input != 0 && input->IsChildOf(this)) mRoot->_SetFocusArea(0);
		if (hover != 0 && hover->IsChildOf(this)) mRoot->_SetHoverArea(0);
	}
}

//============================================================================================================
// Gives this area keyboard focus
//============================================================================================================

void Area::SetKeyboardFocus()
{
	if (mRoot) mRoot->_SetFocusArea(this);
}

//============================================================================================================
// Calls OnTextureChanged() and recurses through children
//============================================================================================================

void Area::_TextureChanged (const ITexture* ptr)
{
	OnTextureChanged (ptr);
	mChildren.Lock();
	{
		for (uint i = 0; i < mChildren.GetSize(); ++i)
			if (mChildren[i])
				mChildren[i]->_TextureChanged(ptr);
	}
	mChildren.Unlock();
}

//============================================================================================================
// Updates the region's dimensions and recurses through children
//============================================================================================================

bool Area::Update (const Vector2i& size, bool forceUpdate)
{
	bool childrenChanged (false);
	bool areaChanged (mRegion.Update(size, forceUpdate));

	areaChanged |= OnPreUpdate(areaChanged);
	areaChanged |= OnUpdate(areaChanged);

	if (areaChanged) SetDirty();

	mChildren.Lock();
	{
		const Region& region ( GetSubRegion() );
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			if (mChildren[i])
			{
				childrenChanged |= mChildren[i]->Update(region, areaChanged);
			}
		}
	}
	mChildren.Unlock();
	return areaChanged || childrenChanged;
}

//============================================================================================================
// Updates the region's dimensions and recurses through children
//============================================================================================================

bool Area::Update (const Region& parent, bool forceUpdate)
{
	bool childrenChanged (false);
	bool areaChanged (mRegion.Update(parent, forceUpdate));

	areaChanged |= OnPreUpdate(areaChanged);
	areaChanged |= OnUpdate(areaChanged);

	if (areaChanged) SetDirty();

	mChildren.Lock();
	{
		const Region& region ( GetSubRegion() );
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			if (mChildren[i])
			{
				childrenChanged |= mChildren[i]->Update(region, areaChanged);
			}
		}
	}
	mChildren.Unlock();
	return areaChanged || childrenChanged;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Area::SerializeFrom (const TreeNode& root)
{
	int layer (mLayer);

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if ( (value.IsValid() || node.HasChildren()) && !mRegion.Load(tag, value) )
		{
			if ( tag == "Receives Events" )
			{
				value >> mReceivesEvents;
			}
			else if ( tag == "Tooltip" )
			{
				value >> mTooltip;
			}
			else if ( tag == "Serializable" )
			{
				value >> mSerializable;
			}
			else if ( tag == "Layer" )
			{
				value >> layer;
			}
			else if (!CustomSerializeFrom(node))
			{
				// Try to find or add a child node
				Area* child = _AddChild( tag, value.IsString() ? value.AsString() : value.GetString() );
				if (child) child->SerializeFrom(node);
			}
		}
	}

	SetLayer(layer);
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Area::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild(GetClassID(), GetName());
		mRegion.SerializeTo(node);
		CustomSerializeTo(node);

		node.AddChild("Receives Events", mReceivesEvents);
		if (mTooltip) node.AddChild("Tooltip", mTooltip);
		node.AddChild("Layer", mLayer);

		mChildren.Lock();
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
			{
				if (mChildren[i])
				{
					mChildren[i]->SerializeTo(node);
				}
			}
		}
		mChildren.Unlock();
		return true;
	}
	return false;
}

//============================================================================================================
// Calls the virtual Area::OnRender() and recurses through children
//============================================================================================================

uint Area::Render()
{
	// OnRender() is overwritten by Frame class to actually render the queues
	uint val = OnRender();

	mChildren.Lock();
	{
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			if (mChildren[i])
			{
				val += mChildren[i]->Render();
			}
		}
	}
	mChildren.Unlock();
	return val;
}

//============================================================================================================
// Recurses through children after calling OnFill() on self
//============================================================================================================

void Area::Fill (Queue* queue)
{
	if (mRegion.IsVisible())
	{
		// OnFill() should be overwritten by Root elements to fill the buffer
		OnFill(queue);

		mChildren.Lock();
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
			{
				if (mChildren[i])
				{
					mChildren[i]->Fill(queue);
				}
			}
		}
		mChildren.Unlock();
	}
}