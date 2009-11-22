#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Input field
//============================================================================================================

class Context;
class Input : public Area
{
public:

	typedef Array<String>	HistoryList;

protected:

	SubPicture		mImage;
	EditableLabel	mLabel;
	uint			mMaxHistorySize;
	HistoryList		mHistory;
	bool			mShowHistory;

public:

	Input();

	const Skin*			GetSkin()			const	{ return mImage.GetSkin();	}
	const Face*			GetFace()			const	{ return mImage.GetFace(); }
	const Color3f&		GetColor()			const	{ return mLabel.GetColor();}
	const String&		GetText()			const	{ return mLabel.GetText();	}
	const IFont*		GetFont()			const	{ return mLabel.GetFont();	}
	uint				GetMaxHistorySize()	const	{ return mMaxHistorySize;	}
	const HistoryList&	GetHistory()		const	{ return mHistory;			}

	void SetSkin			(const Skin*	skin)	{ mImage.SetSkin(skin);	}
	void SetFace			(const String&	face)	{ mImage.SetFace(face);	}
	void SetColor			(const Color3f& color)	{ mLabel.SetColor(color);	}
	void SetText			(const String&	text)	{ mLabel.SetText(text);	}
	void SetFont			(const IFont*	font)	{ mLabel.SetFont(font);	}
	void SetMaxHistorySize	(uint	lines);

	void ClearHistory()								{ mHistory.Lock(); mHistory.Clear(); mHistory.Unlock(); }
	void AddToHistory (const String& text);

private:

	// Private callbacks
	bool _OnLabelKey		(Area* area, const Vector2i& pos, byte key, bool isDown);
	bool _OnLabelFocus		(Area* area, bool hasFocus);
	bool _OnLabelValue		(Area* area);
	bool _OnContextValue	(Area* area);

	Context* _ShowHistory();
	Context* _HideHistory();

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Input", Input, Area, Area);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (Area* ptr);
	virtual void _SetRootPtr   (Root* ptr);

	// Area functions
	virtual void SetDirty()								{ mImage.SetDirty(); mLabel.SetDirty(); }
	virtual void OnTextureChanged (const ITexture* ptr)	{ mImage.OnTextureChanged(ptr); }
	virtual void OnLayerChanged()						{ mImage.SetLayer(mLayer, false); mLabel.SetLayer(mLayer+1, false); }
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (Queue* queue)					{ mImage.OnFill(queue); mLabel.OnFill(queue); }

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
	virtual void CustomSerializeTo (TreeNode& root) const;

	// Events
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ mLabel.OnMouseMove(pos, delta);	Area::OnMouseMove(pos, delta);	return true; }
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown)	{ mLabel.OnKey(pos, key, isDown);	Area::OnKey(pos, key, isDown);	return true; }
	virtual bool OnFocus	(bool selected)									{ mLabel.OnFocus(selected);			Area::OnFocus(selected);		return true; }
	virtual bool OnChar		(byte character)								{ mLabel.OnChar(character);			Area::OnChar(character);		return true; }
};