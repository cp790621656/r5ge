#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Button class
// Author: Michael Lyashenko
//============================================================================================================

class UIButton : public UIWidget
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
	String			mPrefix;
	uint			mState;
	bool			mSticky;
	bool			mIgnoreMouseKey;
	int				mPadding;

public:

	UIButton();

	const UISkin*	GetSkin()		 const	{ return mImage.GetSkin();			}
	const String&	GetPrefix()		 const	{ return mPrefix;					}
	const Color4ub&	GetTextColor()	 const	{ return mLabel.GetTextColor();		}
	const Color4ub&	GetShadowColor() const	{ return mLabel.GetShadowColor();	}
	const Color4ub&	GetBackColor()	 const	{ return mImage.GetBackColor();		}
	const String&	GetText()		 const	{ return mLabel.GetText();			}
	const IFont*	GetFont()		 const	{ return mLabel.GetFont();			}
	char			GetAlignment()	 const	{ return mLabel.GetAlignment();		}
	bool			IsSticky()		 const	{ return mSticky;					}
	uint			GetState()		 const	{ return mState;					}
	int				GetTextPadding() const	{ return mPadding;					}

	void SetSkin		(const UISkin* skin)	{ mImage.SetSkin(skin);				}
	void SetPrefix		(const String& pref)	{ mPrefix = pref; mImage.SetDirty();}
	void SetTextColor	(const Color4ub& c)		{ mLabel.SetTextColor(c);			}
	void SetShadowColor	(const Color4ub& c)		{ mLabel.SetShadowColor(c);			}
	void SetBackColor	(const Color4ub& c)		{ mImage.SetBackColor(c);			}
	void SetText		(const String& text)	{ mLabel.SetText(text);				}
	void SetFont		(const IFont* font)		{ mLabel.SetFont(font);				}
	void SetAlignment	(char alignment)		{ mLabel.SetAlignment(alignment);	}
	void SetSticky		(bool val)				{ mSticky = val; if(!val) SetState(State::Pressed, false); }
	void SetTextPadding	(int padding);

	// Convenience function
	bool GetState (uint state) const { return (mState & state) != 0; }

	// Changes the visible state of the button
	virtual bool SetState (uint state, bool val);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS(UIButton, UIWidget, UIWidget);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIWidget* ptr);
	virtual void _SetRootPtr   (UIManager* ptr);

	// Area functions
	virtual void SetDirty();
	virtual void OnTextureChanged (const ITexture* ptr)	{ mImage.OnTextureChanged(ptr); }
	virtual void OnLayerChanged()						{ mImage.SetLayer(mLayer, false); mLabel.SetLayer(mLayer+1, false); }
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

protected:

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;

	// Events
	virtual void OnMouseOver(bool inside);
	virtual void OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual void OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
};