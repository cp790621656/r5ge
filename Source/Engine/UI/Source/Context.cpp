#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Context::Context() :	mSkin		(0),
						mFont		(0),
						mColor		(1.0f),
						mShadow		(true),
						mAlignment	(Label::Alignment::Left),
						mIsDirty	(false),
						mMinWidth	(0.0f)
{
	mSerializable = false;
	mRegion.SetAlpha(0.0f);
}

//============================================================================================================
// Clear all entries
//============================================================================================================

void Context::ClearAllEntries()
{
	mLast.Clear();
	mEntries.Lock();
	mEntries.Clear();
	mEntries.Unlock();
	mIsDirty = true;
}

//============================================================================================================
// Add a new entry to the selection
//============================================================================================================

void Context::AddEntry (const String& entry)
{
	mEntries.Lock();
	mEntries.Expand() = entry;
	mEntries.Unlock();
	mIsDirty = true;
}

//============================================================================================================
// Rebuilds the entire area
//============================================================================================================

void Context::_Rebuild()
{
	if (mFont == 0 || mSkin == 0) return;

	bool mouseKeyDown = mRoot->IsKeyDown( Key::MouseLeft ) ||
						mRoot->IsKeyDown( Key::MouseRight );

	// Ensure that this frame receives mouse movement events
	mRoot->_SetEventArea(mouseKeyDown ? this : 0);

	// Delete all children as we want to recreate them here
	DeleteAllChildren();

	// Figure out the border sizes
	const Face* background	= mSkin->GetFace(mFace);
	const Face* highlight	= mSkin->GetFace("Generic Highlight");

	short backgroundBorder	= background->GetBorder();
	short highlightBorder	= -highlight->GetBorder();

	// Limit the border to positive range
	if (backgroundBorder < 0) backgroundBorder = 0;
	if (highlightBorder  < 0) highlightBorder  = 0;

	uint maxBorder = backgroundBorder * 2 + highlightBorder * 2;

	mEntries.Lock();
	{
		// Get the height of the stacked labels, taking border into account
		short textSize	= mFont->GetSize();
		float height	= (float)(mEntries.GetSize() * (textSize + highlightBorder) + backgroundBorder * 2 + highlightBorder);
		float width		= mMinWidth;

		// Choose the optimal width that would fit all text entries without cutting them off
		{
			uint minWidth = Float::RoundToInt(width);

			for (uint i = 0; i < mEntries.GetSize(); ++i)
			{
				uint length = mFont->GetLength(mEntries[i], 0, 0xFFFFFFFF, true) + maxBorder;
				if (minWidth < length) minWidth = length;
			}
			width = (float)minWidth;
		}

		// Remember the required dimensions
		mSize.Set(width, height);

		// Start at the top, offset by the size of the background border + selection border
		float offset = (float)backgroundBorder;
		float subWidth = (float)(width - backgroundBorder * 2 - highlightBorder * 2);

		if (subWidth > 0.0f)
		{
			// Add the background SubPicture
			SubPicture* img = AddWidget<SubPicture>(this, "_Background_");

			if (img != 0)
			{
				img->Set(mSkin, mFace);

				// Run through all entries and add them to the frame
				for (uint i = 0; i < mEntries.GetSize(); ++i)
				{
					Label* lbl = AddWidget<Label>(img, mEntries[i]);

					if (lbl != 0)
					{
						lbl->GetRegion().SetRect(highlightBorder, offset, subWidth, textSize);
						lbl->SetLayer		( 1, false		);
						lbl->SetShadow		( mShadow		);
						lbl->SetAlignment	( mAlignment	);
						lbl->SetColor		( mColor		);
						lbl->SetFont		( mFont		);
						lbl->SetText		( mEntries[i]	);
						lbl->SetOnMouseOver	( bind(&Context::_OnMouseOverItem,	this) );
						lbl->SetOnFocus		( bind(&Context::_OnItemFocus,		this) );
					}

					// Advance the offset by the selection border + the text's height
					offset += highlightBorder + textSize;
				}
			}
		}
	}
	mEntries.Unlock();
}

//============================================================================================================
// Event callback for a visual highlight
//============================================================================================================

bool Context::_OnMouseOverItem (Area* area, bool inside)
{
	if (inside)
	{
		SubPicture* parent = R5_CAST(SubPicture, area->GetParent());

		if (parent != 0 && parent->GetSkin() != 0)
		{
			SubPicture* img = AddWidget<SubPicture>(parent, "_Highlight_");

			if (img != 0)
			{
				const Region& areaRegion (area->GetSubRegion());
				const Region& parentRegion (parent->GetSubRegion());
				float top = areaRegion.GetTop() - parentRegion.GetTop();

				img->SetSkin(parent->GetSkin());
				img->SetFace("Generic Highlight");
				img->GetRegion().SetTop(0, top);
				img->GetRegion().SetBottom(0, top + areaRegion.GetHeight());
				img->SetReceivesEvents(false);
			}
		}
	}
	return true;
}

//============================================================================================================
// Triggered when an item is selected from the list
//============================================================================================================

bool Context::_OnItemFocus (Area* area, bool hasFocus)
{
	if (hasFocus)
	{
		Label* lbl = R5_CAST(Label, area);

		if (lbl != 0)
		{
			mLast = lbl->GetText();
			if (mOnValueChange) mOnValueChange(this);
		}
	}
	else
	{
		OnFocus(false);
	}
	return true;
}

//============================================================================================================
// If alpha is set to '1', rebuild the entry list
//============================================================================================================

void Context::SetAlpha (float val, float animTime)
{
	float alpha = GetAlpha();

	if (val == 1.0f)
	{
		// If the frame was marked as dirty, rebuild all entries
		if (mIsDirty)
		{
			mIsDirty = false;
			_Rebuild();
		}

		// Reposition the frame
		const Vector2i& screenSize = mRoot->GetSize();

		// Context menu's default position is at the specified XY coordinates, and going down from there.
		// If the menu can't fit due to screen bounds, the menu will be positioned at X(Y-Z) and up.
		if ( (mAnchor.y + mSize.y > (float)screenSize.y) && (mAnchor.y - mAnchor.z - mSize.y > 0) )
		{
			// If the frame won't fit below the list box, but will above; start it above the listbox
			mRegion.SetRect(mAnchor.x, mAnchor.y - mAnchor.z - mSize.y, mSize.x, mSize.y);
		}
		else
		{
			// Otherwise the frame will always start below the list box
			mRegion.SetRect(mAnchor.x, mAnchor.y, mSize.x, mSize.y);
		}

		// We want the frame to be responsive from here on
		SetReceivesEvents(true);
	}
	else if (val == 0.0f && alpha > 0.0f)
	{
		SetFocus(false);
	}
	AnimatedFrame::SetAlpha(val, animTime);
}

//============================================================================================================
// OnFocus callback should take menu items into account
//============================================================================================================

bool Context::OnFocus (bool hasFocus)
{
	if (!hasFocus)
	{
		const Area* focus = mRoot->GetFocusArea();
		hasFocus = (focus != 0 && focus->IsChildOf(this));
	}
	return AnimatedFrame::OnFocus(hasFocus);
}