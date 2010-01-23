#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// List of registered object and script types
//============================================================================================================

Hash<UIWidget::CreateDelegate> gWidgetTypes;

//============================================================================================================
// INTERNAL: Registers a new widget type
//============================================================================================================

void UIWidget::_Register(const String& type, const CreateDelegate& callback)
{
	gWidgetTypes.Lock();
	gWidgetTypes[type] = callback;
	gWidgetTypes.Unlock();
}

//============================================================================================================
// Creates a new widget of the specified type
//============================================================================================================

UIWidget* _Create(const String& type)
{
	UIWidget* ptr (0);
	gWidgetTypes.Lock();
	{
		const UIWidget::CreateDelegate* callback = gWidgetTypes.GetIfExists(type);
		if (callback != 0) ptr = (*callback)();
	}
	gWidgetTypes.Unlock();
	return ptr;
}

//============================================================================================================
// Adds the specified widget as a child of this one. The widget will be removed from its current parent.
//============================================================================================================

void UIWidget::AddWidget (UIWidget* widget)
{
	UIWidget* parent = widget->GetParent();
	if (parent != 0) parent->RemoveWidget(widget);
	mChildren.Expand() = widget;
	widget->_SetParentPtr(this);
}

//============================================================================================================
// Removes the specified widget from the list of children.
// NOTE: Doing this does not release the widget. 'delete' the widget if you mean to destroy it.
//============================================================================================================

bool UIWidget::RemoveWidget (UIWidget* widget)
{
	if (mChildren.Remove(widget))
	{
		widget->_SetParentPtr(0);
		return true;
	}
	return false;
}

//============================================================================================================
// Find an widget by position that will respond to events
//============================================================================================================

UIWidget* UIWidget::_FindWidget (const Vector2i& pos)
{
	UIWidget* ptr (0);

	if ( mReceivesEvents && mRegion.Contains(pos) && mRegion.IsVisible() )
	{
		for (uint i = mChildren.GetSize(); i > 0; )
			if (mChildren[--i] && (ptr = mChildren[i]->_FindWidget(pos)))
				break;

		// No child widget claims to own the position, but it's still inside this widget's
		if (ptr == 0) ptr = this;
	}
	return ptr;
}

//============================================================================================================
// Simple search
//============================================================================================================

UIWidget* UIWidget::_FindWidget (const String& name, bool recursive)
{
	UIWidget* ptr (0);

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
			if (mChildren[i] && (ptr = mChildren[i]->_FindWidget(name)) )
			{
				break;
			}
		}
	}
	return ptr;
}

//============================================================================================================
// Adds a unique named child, or returns an existing widget if it is already present
//============================================================================================================

UIWidget* UIWidget::_AddWidget (const String& type, const String& name, bool unique)
{
	UIWidget* ptr (0);

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

	// If the entry was not found, create a new one
	if (ptr == 0 && 0 != (ptr = _Create(type)))
	{
		ptr->SetName(name);
		ptr->_SetRootPtr(mUI);
		ptr->_SetParentPtr(this);
		if (mRegion.IsVisible()) ptr->Update(mRegion);
		mChildren.Expand() = ptr;

		// Copy over the event listener
		USEventListener* listener = mUI->_GetListener(name);

		if (listener != 0)
		{
			ptr->mScripts.Expand() = listener;
			listener->mWidget = ptr;
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Retrieve an existing script of the specified type
//============================================================================================================

UIScript* UIWidget::_GetScript (const String& type)
{
	UIScript* ptr (0);

	if (mScripts.IsValid())
	{
		for (uint i = mScripts.GetSize(); i > 0; )
		{
			UIScript* script = mScripts[--i];

			if (script != 0 && type == script->GetClassID())
			{
				ptr = script;
				break;
			}
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Adds a new script of the specified type or retrieves an existing one
//============================================================================================================

UIScript* UIWidget::_AddScript (const String& type)
{
	UIScript* ptr (0);

	for (uint i = mScripts.GetSize(); i > 0; )
	{
		UIScript* script = mScripts[--i];

		if (script != 0 && type == script->GetClassID())
		{
			ptr = script;
			break;
		}
	}

	// If the entry was not found, create a new one
	if (ptr == 0 && 0 != (ptr = UIScript::_Create(type)))
	{
		ptr->mWidget = this;
		mScripts.Expand() = ptr;
		ptr->Init();
	}
	return ptr;
}

//============================================================================================================
// Layer manipulation
//============================================================================================================

void UIWidget::SetLayer (int layer, bool setDirty)
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
// Destroys this widget. The widget is only deleted immediately if it has no parent.
// If it does have a parent, as in the case of most widgets, it gets scheduled for deletion instead.
//============================================================================================================

void UIWidget::DestroySelf()
{
	if (mParent != 0)
	{
		// Ensure that the parent knows that the draw queues should be updated
		SetDirty();

		// Schedule this widget for deletion
		mParent->_DelayedDelete(this);
		mParent = 0;
	}
}

//============================================================================================================
// Deletes all child widgets
//============================================================================================================

void UIWidget::DestroyAllWidgets()
{
	mDeletedWidgets.Release();

	if (mChildren.IsValid())
	{
		SetDirty();

		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			UIWidget* widget = mChildren[i];

			if (widget != 0)
			{
				mUI->RemoveAllReferencesTo(widget);
			}
		}
		mChildren.Release();
	}
}

//============================================================================================================
// Deletes all attached scripts
//============================================================================================================

void UIWidget::DestroyAllScripts()
{
	mDeletedScripts.Release();

	if (mScripts.IsValid())
	{
		// Run through all scripts
		for (uint i = mScripts.GetSize(); i > 0; )
		{
			// Remove the reference to this widget
			if (mScripts[--i] != 0)
			{
				mScripts[i]->mWidget = 0;
			}
		}
		mScripts.Release();
	}
}

//============================================================================================================
// Brings the widget to the foreground
//============================================================================================================

void UIWidget::BringToFront (UIWidget* child)
{
	if (child != 0)
	{
		if (mChildren.Remove(child)) mChildren.Expand() = child;
		child->SetDirty();
	}
	if (mParent != 0) mParent->BringToFront(this);
}

//============================================================================================================
// Gives this widget undivided attention
//============================================================================================================

void UIWidget::SetFocus (bool focus)
{
	if (focus)
	{
		mUI->_SetEventArea(this);
		mUI->_SetFocusArea(this);

		BringToFront();
	}
	else
	{
		const UIWidget* focus = mUI->GetEventArea();
		const UIWidget* input = mUI->GetFocusArea();
		const UIWidget* hover = mUI->GetHoverArea();

		if (focus != 0 && focus->IsChildOf(this)) mUI->_SetEventArea(0);
		if (input != 0 && input->IsChildOf(this)) mUI->_SetFocusArea(0);
		if (hover != 0 && hover->IsChildOf(this)) mUI->_SetHoverArea(0);
	}
}

//============================================================================================================
// Gives this widget keyboard focus
//============================================================================================================

void UIWidget::SetKeyboardFocus()
{
	if (mUI) mUI->_SetFocusArea(this);
}

//============================================================================================================
// Calls OnTextureChanged() and recurses through children
//============================================================================================================

void UIWidget::_TextureChanged (const ITexture* ptr)
{
	OnTextureChanged (ptr);
	for (uint i = 0; i < mChildren.GetSize(); ++i)
		if (mChildren[i])
			mChildren[i]->_TextureChanged(ptr);
}

//============================================================================================================
// Updates the region's dimensions and recurses through children
//============================================================================================================

bool UIWidget::_Update (bool areaChanged)
{
	bool childrenChanged (false);

	mDeletedWidgets.Release();
	mDeletedScripts.Release();

	areaChanged |= OnPreUpdate(areaChanged);
	areaChanged |= OnUpdate(areaChanged);

	if (areaChanged) SetDirty();

	if (mScripts.IsValid())
	{
		for (uint i = mScripts.GetSize(); i > 0; )
			mScripts[--i]->OnUpdate();
	}

	const UIRegion& region ( GetSubRegion() );

	for (uint i = 0; i < mChildren.GetSize(); ++i)
	{
		if (mChildren[i])
		{
			childrenChanged |= mChildren[i]->Update(region, areaChanged);
		}
	}
	return areaChanged || childrenChanged;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool UIWidget::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild(GetClassID(), GetName());
		mRegion.OnSerializeTo(node);
		OnSerializeTo(node);

		node.AddChild("Receives Events", mReceivesEvents);
		if (mTooltip) node.AddChild("Tooltip", mTooltip);
		node.AddChild("Layer", mLayer);

		for (uint i = 0; i < mScripts.GetSize();  ++i) mScripts [i]->SerializeTo(node);
		for (uint i = 0; i < mChildren.GetSize(); ++i) mChildren[i]->SerializeTo(node);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIWidget::SerializeFrom (const TreeNode& root)
{
	int layer (mLayer);

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if ( (value.IsValid() || node.HasChildren()) && !mRegion.OnSerializeFrom(node) )
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
			else if ( tag == UIScript::ClassID() )
			{
				UIScript* script = _AddScript(value.IsString() ? value.AsString() : value.GetString());
				if (script != 0) script->SerializeFrom(node);
			}
			else if (!OnSerializeFrom(node))
			{
				// Try to find or add a child node
				UIWidget* child = _AddWidget( tag, value.IsString() ? value.AsString() : value.GetString() );
				if (child != 0) child->SerializeFrom(node);
			}
		}
	}

	SetLayer(layer);
	return true;
}

//============================================================================================================
// Calls the virtual Area::OnDraw() and recurses through children
//============================================================================================================

uint UIWidget::Draw()
{
	// OnDraw() is overwritten by Frame class to actually render the queues
	uint val = OnDraw();

	for (uint i = 0; i < mChildren.GetSize(); ++i)
	{
		if (mChildren[i])
		{
			val += mChildren[i]->Draw();
		}
	}
	return val;
}

//============================================================================================================
// Recurses through children after calling OnFill() on self
//============================================================================================================

void UIWidget::Fill (UIQueue* queue)
{
	if (mRegion.IsVisible())
	{
		// OnFill() should be overwritten by Root elements to fill the buffer
		OnFill(queue);

		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			UIWidget* child = mChildren[i];

			if (child != 0 && !child->IsOfClass(UIFrame::ClassID()))
			{
				mChildren[i]->Fill(queue);
			}
		}
	}
}

//============================================================================================================
// Should notify the listeners of state changes
//============================================================================================================

void UIWidget::OnStateChange(uint state, bool isSet)
{
	for (uint i = mScripts.GetSize(); i > 0; )
		mScripts[--i]->OnStateChange(state, isSet);
}

//============================================================================================================
// Should notify the listeners of value changes
//============================================================================================================

void UIWidget::OnValueChange()
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnValueChange();
}

//============================================================================================================
// Standard event virtual functions that inform all scripts
//============================================================================================================

void UIWidget::OnMouseOver (bool inside)
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnMouseOver(inside);
}

//============================================================================================================

bool UIWidget::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	bool handled = false;
	for (uint i = mScripts.GetSize(); i > 0;)
		handled |= mScripts[--i]->OnMouseMove(pos, delta);
	return handled;
}

//============================================================================================================

bool UIWidget::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	bool handled = false;
	for (uint i = mScripts.GetSize(); i > 0;)
		handled |= mScripts[--i]->OnKeyPress(pos, key, isDown);
	return handled;
}

//============================================================================================================

bool UIWidget::OnScroll (const Vector2i& pos, float delta)
{
	bool handled = false;
	for (uint i = mScripts.GetSize(); i > 0;)
		handled |= mScripts[--i]->OnScroll(pos, delta);
	return handled;
}

//============================================================================================================

void UIWidget::OnFocus (bool selected)
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnFocus(selected);
}