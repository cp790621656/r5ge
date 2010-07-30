#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIContext::UIContext() :	mSkin		(0),
							mFont		(0),
							mTextColor	(0xFFFFFFFF),
							mBackColor	(0xFFFFFFFF),
							mShadow		(true),
							mAlignment	(UILabel::Alignment::Left),
							mIsDirty	(false),
							mMinWidth	(0.0f),
							mHighlight	(0)
{
	mSerializable = false;
	mRegion.SetAlpha(0.0f);
}

//============================================================================================================
// Clear all entries
//============================================================================================================

void UIContext::ClearAllEntries()
{
	mText.Clear();
	mEntries.Clear();
	mIsDirty = true;
}

//============================================================================================================
// Add a new entry to the selection
//============================================================================================================

void UIContext::AddEntry (const String& entry)
{
	mEntries.Expand() = entry;
	mIsDirty = true;
}

//============================================================================================================
// Rebuilds the entire widget
//============================================================================================================

void UIContext::_Rebuild()
{
	if (mFont == 0 || mSkin == 0) return;

	bool mouseKeyDown = mUI->IsKeyDown( Key::MouseLeft ) ||
						mUI->IsKeyDown( Key::MouseRight );

	// Ensure that this frame receives mouse movement events
	mUI->_SetEventArea(mouseKeyDown ? this : 0);

	// Delete all children as we want to recreate them here
	DestroyAllWidgets();

	// Figure out the border sizes
	const UIFace* background	= mSkin->GetFace(mFace);
	const UIFace* highlight		= mSkin->GetFace("Generic Highlight");
	short backgroundBorder		= (background == 0) ? 0 : background->GetBorder();
	short highlightBorder		= (highlight  == 0) ? 0 : -highlight->GetBorder();

	// Limit the border to positive range
	if (backgroundBorder < 0) backgroundBorder = 0;
	if (highlightBorder  < 0) highlightBorder  = 0;

	uint maxBorder = backgroundBorder * 2 + highlightBorder * 2;

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
	float halfBorder = Float::Round(highlightBorder * 0.5f, 1.0f);

	if (subWidth > 0.0f)
	{
		// Add the background SubPicture
		UISubPicture* img = AddWidget<UISubPicture>("_Background_");

		if (img != 0)
		{
			img->Set(mSkin, mFace);
			img->SetBackColor(mBackColor);

			USEventListener* listener = img->AddScript<USEventListener>();
			listener->SetOnKey		( bind(&UIContext::_OnKeyPress,	 this) );
			listener->SetOnMouseMove( bind(&UIContext::_OnMouseMove, this) );
			listener->SetOnFocus	( bind(&UIContext::_OnFocusChange, this) );

			// Run through all entries and add them to the frame
			for (uint i = 0; i < mEntries.GetSize(); ++i)
			{
				UILabel* lbl = img->AddWidget<UILabel>(mEntries[i]);

				if (lbl != 0)
				{
					lbl->GetRegion().SetRect(highlightBorder, offset - halfBorder, subWidth,
						(float)textSize + highlightBorder);
					lbl->SetLayer			( 1, false			  );
					lbl->SetShadow			( mShadow			  );
					lbl->SetAlignment		( mAlignment		  );
					lbl->SetTextColor		( mTextColor		  );
					lbl->SetFont			( mFont				  );
					lbl->SetText			( mEntries[i]		  );
					lbl->SetEventHandling	( EventHandling::None );
				}

				// Advance the offset by the selection border + the text's height
				offset += highlightBorder + textSize;
			}
			img->SetFocus(true);
			img->AddScript<USEventListener>()->SetOnFocus( bind(&UIContext::_OnFocusChange, this) );
		}
	}
}

//============================================================================================================
// Event callback for a visual highlight
//============================================================================================================

void UIContext::_OnMouseMove (UIWidget* widget, const Vector2i& pos, const Vector2i& delta)
{
	UISubPicture* bg = R5_CAST(UISubPicture, widget);

	if (bg != 0 && bg->GetSkin() != 0)
	{
		const Children& children = bg->GetAllChildren();			

		FOREACH (i, children)
		{
			UIWidget* child = children[i];

			if (child != mHighlight && child->GetRegion().Contains(pos))
			{
				const UIRegion& areaRegion (child->GetRegion());
				const UIRegion& parentRegion (bg->GetSubRegion());
				float top = areaRegion.GetCalculatedTop() - parentRegion.GetCalculatedTop();

				mHighlight = bg->AddWidget<UISubPicture>("_Highlight_");

				if (mHighlight != 0)
				{
					mHighlight->SetEventHandling(EventHandling::None);
					mHighlight->SetSkin(bg->GetSkin());
					mHighlight->SetFace("Generic Highlight");
					mHighlight->GetRegion().SetTop(0, top);
					mHighlight->GetRegion().SetBottom(0, top + areaRegion.GetCalculatedHeight());
				}
				return;
			}
		}
	}
}

//============================================================================================================
// When the left mouse button is released find the label to select
//============================================================================================================

void UIContext::_OnKeyPress (UIWidget* widget, const Vector2i& pos, byte key, bool isDown)
{	
	// Left mouse key should select objects	
	if (key == Key::MouseLeft)
	{
		if (!isDown)
		{
			const Children& children = widget->GetAllChildren();

			FOREACH (i, children)
			{
				if (children[i] != mHighlight && children[i]->GetRegion().Contains(pos))
				{
					const UILabel* lbl = R5_CAST(UILabel, children[i]);

					if (lbl != 0)
					{
						mText = lbl->GetText();
						OnValueChange();
					}
					return;
				}
			}			
		}
	}
}

//============================================================================================================
// If alpha is set to '1', rebuild the entry list
//============================================================================================================

void UIContext::SetAlpha (float val, float animTime)
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
		const Vector2i& screenSize = mUI->GetSize();

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
		SetEventHandling(EventHandling::Normal);
	}
	else if (val == 0.0f && alpha > 0.0f)
	{
		SetFocus(false);
	}
	UIAnimatedFrame::SetAlpha(val, animTime);
}

//============================================================================================================
// OnFocus callback should take menu items into account
//============================================================================================================

void UIContext::OnFocus (bool hasFocus)
{
	if (!hasFocus)
	{
		const UIWidget* focus = mUI->GetFocusArea();
		hasFocus = (focus != 0 && focus->IsChildOf(this));
	}
	UIAnimatedFrame::OnFocus(hasFocus);
}