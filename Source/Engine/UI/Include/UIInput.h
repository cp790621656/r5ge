#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Input field
//============================================================================================================

class UIContext;
class UIInput : public UIWidget
{
public:

	typedef Array<String>	HistoryList;

protected:

	UISubPicture	mImage;
	UIEditableLabel	mLabel;
	uint			mMaxHistorySize;
	HistoryList		mHistory;
	bool			mShowHistory;

public:

	UIInput();

	const UISkin*	GetSkin()			const	{ return mImage.GetSkin();		}
	const UIFace*	GetFace()			const	{ return mImage.GetFace();		}
	const String&	GetText()			const	{ return mLabel.GetText();		}
	const IFont*	GetFont()			const	{ return mLabel.GetFont();		}
	const Color4ub&	GetTextColor()		const	{ return mLabel.GetTextColor(); }
	const Color4ub&	GetBackColor()		const	{ return mImage.GetBackColor(); }
	uint			GetMaxHistorySize()	const	{ return mMaxHistorySize;		}

	// Entire history list
	const HistoryList&	GetHistory() const	{ return mHistory; }

	// Common properties
	void SetSkin (const UISkin* skin)	{ mImage.SetSkin(skin);	}
	void SetFace (const String& face)	{ mImage.SetFace(face);	}
	void SetText (const String& text)	{ mLabel.SetText(text);	}
	void SetFont (const IFont* font)	{ mLabel.SetFont(font);	}

	// Text and background colors
	void SetTextColor (const Color4ub& c) { mLabel.SetTextColor(c);	}
	void SetBackColor (const Color4ub& c) { mImage.SetBackColor(c);	}

	// Maximum number of lines kept in the input's history
	void SetMaxHistorySize (uint lines);
	void ClearHistory() { mHistory.Lock(); mHistory.Clear(); mHistory.Unlock(); }
	void AddToHistory (const String& text);

	// Selects the entire text
	void SelectAll() { mLabel.SelectAll(); }

private:

	// Private callbacks
	void _OnLabelKey		(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	void _OnLabelFocus		(UIWidget* widget, bool hasFocus);
	void _OnLabelValue		(UIWidget* widget) { OnValueChange(); }
	void _OnContextValue	(UIWidget* widget);

	UIContext* _ShowHistory();
	UIContext* _HideHistory();

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIInput", UIInput, UIWidget, UIWidget);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIWidget* ptr);
	virtual void _SetRootPtr   (UIManager* ptr);

	// Area functions
	virtual void SetDirty()								{ mImage.SetDirty(); mLabel.SetDirty(); }
	virtual void OnTextureChanged (const ITexture* ptr)	{ mImage.OnTextureChanged(ptr); }
	virtual void OnLayerChanged()						{ mImage.SetLayer(mLayer, false); mLabel.SetLayer(mLayer+1, false); }
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue)				{ mImage.OnFill(queue); mLabel.OnFill(queue); }

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;

	// Events
	virtual void OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ mLabel.OnMouseMove(pos, delta);		UIWidget::OnMouseMove(pos, delta); }
	virtual void OnKeyPress	(const Vector2i& pos, byte key, bool isDown)	{ mLabel.OnKeyPress(pos, key, isDown);	UIWidget::OnKeyPress(pos, key, isDown); }
	virtual void OnFocus	(bool selected)									{ mLabel.OnFocus(selected);				UIWidget::OnFocus(selected); }
	virtual void OnChar		(byte character)								{ mLabel.OnChar(character);				UIWidget::OnChar(character); }
};