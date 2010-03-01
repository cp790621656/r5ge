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

UIWidget* _Create (const String& type)
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
// Convenience function
//============================================================================================================

UIWidget* UIWidget::GetRoot()
{
	return &mUI->mRoot;
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
	widget->SetDirty();
}

//============================================================================================================
// Removes the specified widget from the list of children.
// NOTE: Doing this does not release the widget. 'delete' the widget yourself, or call DestroySelf() instead.
//============================================================================================================

bool UIWidget::RemoveWidget (UIWidget* widget)
{
	if (mChildren.Remove(widget))
	{
		widget->SetDirty();
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

	if (mEventHandling != EventHandling::None && !mIgnoreEvents && mRegion.Contains(pos) && mRegion.IsVisible())
	{
		// Run through children first
		if ((mEventHandling & EventHandling::Children) != 0)
		{
			for (uint i = mChildren.GetSize(); i > 0; )
				if (mChildren[--i] && (ptr = mChildren[i]->_FindWidget(pos)))
					break;
		}

		// No child widget claims to own the position, but it's still inside this widget's
		if (ptr == 0 && (mEventHandling & EventHandling::Self) != 0)
		{
			ptr = this;
		}
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
		mChildren.Expand() = ptr;

		// Copy over the event listener
		USEventListener* listener = mUI->_GetListener(name);

		if (listener != 0)
		{
			ptr->mScripts.Expand() = listener;
			listener->mWidget = ptr;
		}

		// Call the virtual initialization
		ptr->OnInit();

		// Update the region
		if (mRegion.IsVisible()) ptr->Update(mRegion);
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
		ptr->OnInit();
		if (!mScripts.Contains(ptr)) ptr = 0;
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

		const char* events = "None";

		if		(mEventHandling == EventHandling::Children)	events = "Children";
		else if (mEventHandling == EventHandling::Self)		events = "Self";
		else if (mEventHandling == EventHandling::Normal)	events = "Normal";
		else if (mEventHandling == EventHandling::Full)		events = "Full";

		node.AddChild("Event Handling", events);

		if (mTooltip.IsValid()) node.AddChild("Tooltip", mTooltip);

		node.AddChild("Layer", mLayer);

		for (uint i = 0; i < mScripts.GetSize(); ++i)
		{
			const UIScript* script = mScripts[i];
			TreeNode& child = node.AddChild(UIScript::ClassID(), script->GetClassID());
			script->OnSerializeTo(child);
		}

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
	bool serializable = true;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if ( (value.IsValid() || node.HasChildren()) && !mRegion.OnSerializeFrom(node) )
		{
			if (tag == "Event Handling")
			{
				if (value.IsString())
				{
					const String& temp = value.AsString();

					if		(temp == "None")		mEventHandling = EventHandling::None;
					else if (temp == "Normal")		mEventHandling = EventHandling::Normal;
					else if (temp == "Children")	mEventHandling = EventHandling::Children;
					else if (temp == "Self")		mEventHandling = EventHandling::Self;
					else if (temp == "Full")		mEventHandling = EventHandling::Full;
				}
			}
			else if (tag == "Tooltip")
			{
				value >> mTooltip;
			}
			else if (tag == "Serializable")
			{
				if (value >> serializable)
				{
					mSerializable = serializable;
				}
			}
			else if (tag == "Layer")
			{
				value >> layer;
			}
			else if (tag == "Receives Events")
			{
				// LEGACY FUNCTIONALITY, WILL BE REMOVED
				bool receive = false;

				if (value >> receive)
				{
					mEventHandling = receive ? EventHandling::Normal : EventHandling::None;
				}
			}
			else if (tag == UIScript::ClassID())
			{
				UIScript* script = _AddScript(value.IsString() ? value.AsString() : value.GetString());
				if (script != 0) script->OnSerializeFrom(node);
			}
			else if (!OnSerializeFrom(node))
			{
				// Try to find or add a child node
				UIWidget* child = _AddWidget( tag, value.IsString() ? value.AsString() : value.GetString() );

				if (child != 0)
				{
					child->SerializeFrom(node);
					if (!serializable) child->SetSerializable(false);
				}
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
				child->Fill(queue);
			}
		}
	}
}

//============================================================================================================
// Immediately deletes all child widgets
//============================================================================================================

void UIWidget::DestroyAllWidgets()
{
	mDeletedWidgets.Release();

	if (mChildren.IsValid())
	{
		SetDirty();

		for (uint i = mChildren.GetSize(); i > 0; )
		{
			UIWidget* widget = mChildren[--i];

			if (widget != 0)
			{
				mUI->RemoveAllReferencesTo(widget);
			}
		}
		mChildren.Release();
	}
}

//============================================================================================================
// Immediately deletes all attached scripts
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

void UIWidget::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnMouseMove(pos, delta);
}

//============================================================================================================

void UIWidget::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnKeyPress(pos, key, isDown);
}

//============================================================================================================

void UIWidget::OnScroll (const Vector2i& pos, float delta)
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnScroll(pos, delta);
}

//============================================================================================================

void UIWidget::OnFocus (bool selected)
{
	for (uint i = mScripts.GetSize(); i > 0;)
		mScripts[--i]->OnFocus(selected);
}