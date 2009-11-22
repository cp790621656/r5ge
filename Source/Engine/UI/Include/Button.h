#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Button class
//============================================================================================================

class UIButton : public UIArea
{
public:

	struct State
	{
		enum
		{
			Enabled		= 1,
			Highlighted = 2,
			Pressed		= 4,
		};
	};

protected:

	UISubPicture	mImage;
	UILabel			mLabel;
	uint			mState;
	bool			mSticky;
	bool			mIgnoreMouseKey;

public:

	UIButton();

	const UISkin*	GetSkin()		const	{ return mImage.GetSkin();			}
	const Color3f&	GetColor()		const	{ return mLabel.GetColor();			}
	const String&	GetText()		const	{ return mLabel.GetText();			}
	const IFont*	GetFont()		const	{ return mLabel.GetFont();			}
	char			GetAlignment()	const	{ return mLabel.GetAlignment();		}
	bool			IsSticky()		const	{ return mSticky;					}
	uint			GetState()		const	{ return mState;					}

	void SetSkin	  (const UISkin* skin)	{ mImage.SetSkin(skin);				}
	void SetColor	  (const Color3f& color){ mLabel.SetColor(color);			}
	void SetText	  (const String& text)	{ mLabel.SetText(text);				}
	void SetFont	  (const IFont* font)	{ mLabel.SetFont(font);				}
	void SetAlignment (char alignment)		{ mLabel.SetAlignment(alignment);	}
	void SetSticky	  (bool val)			{ mSticky = val;					}

	// Changes the visible state of the button
	virtual bool SetState (uint state, bool val);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Button", UIButton, UIArea, UIArea);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIArea* ptr);
	virtual void _SetRootPtr   (UIRoot* ptr);

	// Area functions
	virtual void SetDirty()								{ mImage.SetDirty(); mLabel.SetDirty(); }
	virtual void OnTextureChanged (const ITexture* ptr)	{ mImage.OnTextureChanged(ptr); }
	virtual void OnLayerChanged()						{ mImage.SetLayer(mLayer, false); mLabel.SetLayer(mLayer+1, false); }
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

protected:

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
	virtual void CustomSerializeTo (TreeNode& root) const;

	// Events
	virtual bool OnMouseOver(bool inside);
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
};