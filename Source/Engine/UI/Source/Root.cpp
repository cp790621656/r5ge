#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Default function that fills the tooltip
//============================================================================================================

Root::Root() :	mSerializable	(true),
				mDimsChanged	(false),
				mIsDirty		(false),
				mHoverArea		(0),
				mFocusArea		(0),
				mSelectedArea	(0),
				mContext		(0),
				mDefaultSkin	(0),
				mDefaultFont	(0),
				mTtArea			(0),
				mTtTime			(100000.0f),
				mTtDelay		(1.0f),
				mTtQueued		(false),
				mTtShown		(false)
{
	memset(mKey, 0, 256);

	mTooltip._SetRootPtr(this);
	mTooltip.GetRegion().SetAlpha(0.0f);
	mTooltip.SetName("Tooltip");
	mTooltip.SetReceivesEvents(false);

	RegisterWidget<Area>			(this);
	RegisterWidget<Highlight>		(this);
	RegisterWidget<Picture>			(this);
	RegisterWidget<SubPicture>		(this);
	RegisterWidget<Label>			(this);
	RegisterWidget<TextArea>		(this);
	RegisterWidget<Input>			(this);
	RegisterWidget<Context>			(this);
	RegisterWidget<Menu>			(this);
	RegisterWidget<List>			(this);
	RegisterWidget<Window>			(this);
	RegisterWidget<AnimatedFrame>	(this);
	RegisterWidget<AnimatedSlider>	(this);
	RegisterWidget<AnimatedButton>	(this);
	RegisterWidget<AnimatedCheckbox>(this);
	RegisterWidget<ShadedArea>		(this);
}

//============================================================================================================
// If an area is being deleted, the root must be told so it removes all local references to it
//============================================================================================================

void Root::RemoveAllReferencesTo (const Area* area)
{
	if (mHoverArea		== area) mHoverArea		= 0;
	if (mFocusArea		== area) mFocusArea		= 0;
	if (mSelectedArea	== area) mSelectedArea	= 0;
	if (mTtArea			== area) mTtArea		= 0;
}

//============================================================================================================
// Retrieves the specified skin (creates if necessary)
//============================================================================================================

Skin* Root::GetSkin (const String& name)
{
	typedef Skin* SkinPtr;
	SkinPtr ptr (0);

	if (name.IsValid())
	{
		mSkins.Lock();
		{
			SkinPtr& skin = mSkins[name];
			if (skin == 0) skin = new Skin(this, name);
			ptr = skin;
		}
		mSkins.Unlock();
	}
	return ptr;
}

//============================================================================================================
// Registers a callback that would create a widget of specified type
//============================================================================================================

void Root::_RegisterWidget(const String& type, const CreateDelegate& callback)
{
	mCreators.Lock();
	mCreators[type] = callback;
	mCreators.Unlock();
}

//============================================================================================================
// Find an area by position that will respond to events
//============================================================================================================

Area* Root::_FindChild (const Vector2i& pos)
{
	Area* ptr (0);
	mChildren.Lock();
	{
		for (uint i = mChildren.GetSize(); i > 0; )
			if (mChildren[--i] && (ptr = mChildren[i]->_FindChild(pos)))
				break;
	}
	mChildren.Unlock();
	return ptr;
}

//============================================================================================================
// Finds an area with the specified name
//============================================================================================================

Area* Root::_FindChild (const String& name, bool recursive)
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
				if (mChildren[i] && (ptr = mChildren[i]->_FindChild(name)))
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
// Adds a top-level child of specified type (or returns a child with the same name, if found)
//============================================================================================================

Area* Root::_AddChild (const String& type, const String& name, bool unique)
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
			if ( ptr = _CreateArea(type, name, 0) )
			{
				if (mSize != 0) ptr->Update(mSize);
				mChildren.Expand() = ptr;
			}
		}
	}
	mChildren.Unlock();
	return ptr;
}

//============================================================================================================
// Retrieves a pointer to the context menu
//============================================================================================================

Context* Root::GetContextMenu (bool createIfMissing)
{
	if (mContext == 0 && createIfMissing)
	{
		mContext = AddWidget<Context>(this, "_Context Menu_");
		mContext->SetSerializable(false);
	}
	return mContext;
}

//============================================================================================================
// Informs all areas that the texture has changed
//============================================================================================================

void Root::_TextureChanged (const ITexture* ptr)
{
	mChildren.Lock();
	{
		for (uint i = 0; i < mChildren.GetSize(); ++i)
			if (mChildren[i])
				mChildren[i]->_TextureChanged(ptr);
	}
	mTooltip._TextureChanged(ptr);
	mChildren.Unlock();
}

//============================================================================================================
// Changes the hovering area
//============================================================================================================

void Root::_SetHoverArea (Area* ptr)
{
	if (mHoverArea != ptr)
	{
		// Update the hover area
		if (mHoverArea) mHoverArea->OnMouseOver(false);
		mHoverArea = ptr;
		if (mHoverArea) mHoverArea->OnMouseOver(true);
	}
}

//============================================================================================================
// Manual way of changing the focus area
//============================================================================================================

void Root::_SetFocusArea (Area* ptr)
{
	if (mFocusArea != ptr)
	{
		Area* oldInput = mFocusArea;
		mFocusArea = ptr;
		
		if (oldInput) oldInput->OnFocus(false);
		if (mFocusArea) mFocusArea->OnFocus(true);
		if (mFocusArea) mFocusArea->BringToFront();
	}
}

//============================================================================================================
// Manual way of changing the selected area
//============================================================================================================

void Root::_SetEventArea (Area* ptr)
{
	mSelectedArea = ptr;
}

//============================================================================================================
// Sets event handlers for the specified area
//============================================================================================================

void Root::SetOnMouseOver (const String& areaName, const EventHandler::OnMouseOverDelegate& fnct)
{
	mHandlers.Lock();
	_GetHandler(areaName)->SetOnMouseOver(fnct);
	mHandlers.Unlock();
}

//============================================================================================================

void Root::SetOnMouseMove (const String& areaName, const EventHandler::OnMouseMoveDelegate& fnct)
{
	mHandlers.Lock();
	_GetHandler(areaName)->SetOnMouseMove(fnct);
	mHandlers.Unlock();
}

//============================================================================================================

void Root::SetOnKey (const String& areaName, const EventHandler::OnKeyDelegate& fnct)
{
	mHandlers.Lock();
	{
		EventHandler* handler = _GetHandler(areaName);
		handler->SetOnKey(fnct);
	}
	mHandlers.Unlock();
}

//============================================================================================================

void Root::SetSetOnScroll (const String& areaName, const EventHandler::OnScrollDelegate& fnct)
{
	mHandlers.Lock();
	_GetHandler(areaName)->SetOnScroll(fnct);
	mHandlers.Unlock();
}

//============================================================================================================

void Root::SetOnStateChange (const String& areaName, const EventHandler::OnChangeDelegate& fnct)
{
	mHandlers.Lock();
	_GetHandler(areaName)->SetOnStateChange(fnct);
	mHandlers.Unlock();
}

//============================================================================================================

void Root::SetOnValueChange (const String& areaName, const EventHandler::OnChangeDelegate& fnct)
{
	mHandlers.Lock();
	_GetHandler(areaName)->SetOnValueChange(fnct);
	mHandlers.Unlock();
}

//============================================================================================================
// Creates a default tooltip (returns whether the tooltip is valid)
//============================================================================================================

bool Root::CreateDefaultTooltip (Area* area)
{
	// If there is no area to work with, or there is no default font, no need to do anything
	if (area == 0 || mDefaultFont == 0) return false;

	// If the area has no tooltip text, no need to do anything
	const String& text (area->GetTooltip());
	if (text.IsEmpty()) return false;

	byte textSize   = mDefaultFont->GetSize();
	uint  textWidth = mDefaultFont->GetLength(text, 0, 0xFFFFFFFF, IFont::Tags::Skip);

	// If the printable text has no width, no point in showing an empty tooltip
	if (textWidth == 0) return false;

	Area* parent (0);

	if (mDefaultSkin == 0 || mDefaultSkin->GetFace("Tooltip")->GetSize() == 0)
	{
		// No skin available -- use a simple highlight
		Highlight* hl = AddWidget<Highlight>(&mTooltip, "Tooltip Backdrop");
		hl->SetColor( Color4f(0.0f, 0.0f, 0.0f, 1.0f) );

		Region& hlrgn (hl->GetRegion());
		hlrgn.GetRelativeLeft().Set		(0.0f, -3.0f);
		hlrgn.GetRelativeRight().Set	(1.0f,  3.0f);
		hlrgn.GetRelativeTop().Set		(0.0f, -3.0f);
		hlrgn.GetRelativeBottom().Set	(1.0f,  3.0f);

		Region& rgn = mTooltip.GetRegion();
		rgn.GetRelativeRight().Set (0.0f, (float)textWidth);
		rgn.GetRelativeBottom().Set(0.0f, (float)textSize);

		parent = hl;
	}
	else
	{
		SubPicture* img = AddWidget<SubPicture>(&mTooltip, "Tooltip Background");
		img->Set(mDefaultSkin, "Tooltip");

		short border = img->GetFace()->GetBorder();
		if (border < 0) border = 0;

		Region& rgn = mTooltip.GetRegion();
		rgn.GetRelativeRight().Set (0.0f, (float)(textWidth + border * 2));
		rgn.GetRelativeBottom().Set(0.0f, (float)(textSize  + border * 2));

		parent = img;
	}

	if (parent != 0)
	{
		Label* lbl = AddWidget<Label>(parent, "Tooltip Label");
		lbl->SetLayer(1, true);
		lbl->SetFont(mDefaultFont);
		lbl->SetText(text);
	}
	return true;
}

//============================================================================================================
// Aligns the tooltip using default logic (returns whether the tooltip is valid)
//============================================================================================================

bool Root::AlignDefaultTooltip()
{
	// Update
	Region& rgn  = mTooltip.GetRegion();
	float left   = (float)mMousePos.x;
	float top    = (float)mMousePos.y;
	float topOff = top + 25.0f;
	float width  = rgn.GetRelativeRight().mAbsolute - rgn.GetRelativeLeft().mAbsolute;
	float half	 = width * 0.5f;
	float height = rgn.GetRelativeBottom().mAbsolute - rgn.GetRelativeTop().mAbsolute;

	if (width > 0.0f && height > 0.0f)
	{
		// The tooltip should be centered horizontally
		if		(left - half < 0)			left = 0;
		else if (left + half > mSize.x)	left = mSize.x - width;
		else								left = left - half;

		// Vertically it should be either some space below, or directly above the mouse
		top = (topOff + height < mSize.y) ? topOff : top - height - 5;

		// Set the tooltip's region
		rgn.SetRect(left, top, width, height);
		return true;
	}
	return false;
}

//============================================================================================================
// Hides the tooltip if it's currently visible
//============================================================================================================

void Root::_HideTooltip()
{
	mTtQueued = false;
	mTtTime = GetCurrentTime();

	if (mTtShown)
	{
		mTtShown = false;
		mTooltip.Hide();
	}
}

//============================================================================================================
// Calls mOnFillTooltip function, and fills out the default tooltip if the return val is 'false'
//============================================================================================================

bool Root::_FillTooltip (Area* area)
{
	if (area)
	{
		mTooltip.DeleteAllChildren();
		Region& rgn = mTooltip.GetRegion();
		rgn.SetRect(0, 0, 0, 0);

		return (mTtDelegate ? mTtDelegate(mTooltip, area) :
			CreateDefaultTooltip(area) && AlignDefaultTooltip());
	}
	return false;
}

//============================================================================================================
// Internal function that retrieves an event handler associated with the specified area
//============================================================================================================

EventHandler* Root::_GetHandler (const String& areaName)
{
	EventHandler* ptr = _FindChild(areaName);

	if (ptr == 0)
	{
		typedef EventHandler* HandlerPtr;
		HandlerPtr& h = mHandlers[areaName];
		if (h == 0) h = new EventHandler();
		ptr = h;
	}
	return ptr;
}

//============================================================================================================
// Brings the specified area to foreground
//============================================================================================================

void Root::_BringToFront (Area* ptr)
{
	if (ptr != 0)
	{
		mChildren.Lock();
		{
			if (mChildren.Remove(ptr))
			{
				mChildren.Expand() = ptr;
				ptr->SetDirty();
			}
		}
		mChildren.Unlock();
	}
}

//============================================================================================================
// Create an area of specified type
//============================================================================================================

Area* Root::_CreateArea (const String& type, const String& name, Area* parent)
{
	Area* ptr (0);
	mCreators.Lock();
	{
		const CreateDelegate* callback = mCreators.GetIfExists(type);

		if (callback != 0 && (ptr = (*callback)()) != 0)
		{
			ptr->SetName(name);
			ptr->_SetParentPtr(parent);
			ptr->_SetRootPtr(this);

			// See if this area has a queued event handler
			mHandlers.Lock();
			{
				uint	key		= HashKey(name);
				EventHandler*	handler = mHandlers.GetIfExists(key);

				if (handler != 0)
				{
					ptr->CopyEvents( *handler );
					mHandlers.Delete(key);
				}
			}
			mHandlers.Unlock();
		}
	}
	mCreators.Unlock();
	return ptr;
}

//============================================================================================================
// Mark the entire Root as dirty so it's rebuilt on the next frame
//============================================================================================================

void Root::OnResize(const Vector2i& size)
{
	if ( mSize != size )
	{
		mSize = size;
		mChildren.Lock();
		mDimsChanged = true;
		mChildren.Unlock();
	}
}

//============================================================================================================
// Run through all areas and ask them to update their regions, and associated widgets
//============================================================================================================

bool Root::Update()
{
	// Run through all children and update them
	mChildren.Lock();
	{
		for (uint i = 0; i < mChildren.GetSize(); ++i)
		{
			if (mChildren[i])
			{
				mChildren[i]->Update(mSize, mDimsChanged);
			}
		}
		mDimsChanged = false;
	}
	mChildren.Unlock();

	// Show the tooltip if the tooltip area is valid, and the time has been reached
	if (mTtQueued && (mTtTime + mTtDelay < GetCurrentTime()))
	{
		mTtQueued = false;

		// Only actually show it if there is text to show
		if (_FillTooltip(mTtArea))
		{
			mTtShown = true;
			mTooltip.Show();
		}
	}

	// Update the tooltip
	mTooltip.Update(mSize, mDimsChanged);
	return mIsDirty;
}

//============================================================================================================
// Handle mouse movement
//============================================================================================================

bool Root::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	_HideTooltip();
	mMousePos = pos;

	if (mSelectedArea)
	{
		// If we have an area that has focus (mouse key was held down on it), inform it of mouse movement
		return mSelectedArea->OnMouseMove(pos, delta);
	}
	else
	{
		// If some mouse key is held down, ignore mouse movement altogether
		for (uint i = Key::MouseFirst + 1; i < Key::MouseLast; ++i)
			if (mKey[i]) return false;

		// Hover over the area
		_SetHoverArea( _FindChild(pos) );

		// Queue the tooltip
		mTtArea	= mHoverArea;
		mTtTime	= GetCurrentTime();
		mTtQueued	= true;
	}

	// Inform the hovering area of mouse movement
	return (mHoverArea != 0) ? mHoverArea->OnMouseMove(pos, delta) : false;
}

//============================================================================================================
// Handle keys
//============================================================================================================

bool Root::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	_HideTooltip();
	mKey[key] = isDown;

	if (key > Key::MouseFirst && key < Key::MouseLast)
	{
		if (isDown)
		{
			mSelectedArea = _FindChild(pos);
			_SetFocusArea(mSelectedArea);
		}
		else
		{
			mSelectedArea = 0;
			_SetHoverArea( _FindChild(pos) );
		}
	}

	return (mFocusArea != 0) ? mFocusArea->OnKey(pos, key, isDown) : false;
}

//============================================================================================================
// Respond to the mouse wheel event
//============================================================================================================

bool Root::OnScroll (const Vector2i& pos, float delta)
{
	_HideTooltip();
	return (mHoverArea != 0) ? mHoverArea->OnScroll(pos, delta) : false;
}

//============================================================================================================
// Respond to printable characters
//============================================================================================================

bool Root::OnChar (byte character)
{
	_HideTooltip();
	return (mFocusArea != 0) ? mFocusArea->OnChar(character) : false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Root::SerializeFrom (const TreeNode& root)
{
	bool localSerializable (true);

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if (tag == "UI")
		{
			return SerializeFrom(node);
		}
		else if (tag == "Serializable")
		{
			value >> localSerializable;
		}
		else if (tag == "Default Skin")
		{
			mDefaultSkin = GetSkin( value.IsString() ? value.AsString() : value.GetString() );
		}
		else if (tag == "Default Font")
		{
			mDefaultFont = GetFont( value.IsString() ? value.AsString() : value.GetString() );
		}
		else if (tag == "Tooltip Delay")
		{
			value >> mTtDelay;
		}
		else if (tag == "Skin")
		{
			Skin* skin = GetSkin( value.IsString() ? value.AsString() : value.GetString() );
			skin->SerializeFrom(node);
			if (!localSerializable) skin->SetSerializable(false);
		}
		else if (tag == "Layout")
		{
			for (uint b = 0; b < node.mChildren.GetSize(); ++b)
			{
				const TreeNode& area = node.mChildren[b];

				Area* ptr = _AddChild(area.mTag,  area.mValue.IsString() ?
					area.mValue.AsString() : area.mValue.GetString() );

				if (ptr != 0)
				{
					ptr->SerializeFrom(area);
					if (!localSerializable) ptr->SetSerializable(false);
				}
			}
		}
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Root::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild("UI");
		TreeNode& skin = node.AddChild("Default Skin");
		TreeNode& font = node.AddChild("Default Font");

		if (mDefaultSkin != 0) skin.mValue = mDefaultSkin->GetName();
		if (mDefaultFont != 0) font.mValue = mDefaultFont->GetName();
		node.AddChild("Tooltip Delay", mTtDelay);

		mSkins.Lock();
		{
			const PointerArray<Skin>& allSkins = mSkins.GetAllValues();

			if (allSkins.IsValid())
			{
				for (uint i = 0; i < allSkins.GetSize(); ++i)
				{
					if (allSkins[i] != 0)
					{
						allSkins[i]->SerializeTo(node);
					}
				}
			}
		}
		mSkins.Unlock();

		if (mChildren.IsValid())
		{
			TreeNode& layout = node.AddChild("Layout");

			mChildren.Lock();
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
				{
					if (mChildren[i] != 0)
					{
						mChildren[i]->SerializeTo(layout);
					}
				}
			}
			mChildren.Unlock();
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Render everything
//============================================================================================================

uint Root::Render()
{
	mIsDirty = false;
	uint triangles (0);

	if (mChildren.IsValid())
	{
		OnPreRender();
		{
			// Render all children
			mChildren.Lock();
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
					triangles += mChildren[i]->Render();
			}
			mChildren.Unlock();

			// Render the tooltip
			triangles += mTooltip.Render();
		}
		OnPostRender();
	}
	return triangles;
}