#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Window is a complex widget based on Animated Frame, using several other widgets
//============================================================================================================

class UIWindow : public UIAnimatedFrame
{
	struct Movement
	{
		enum
		{
			None			= 0x0,
			Move			= 0x1,
			ResizeRight		= 0x2,
			ResizeBottom	= 0x4,
			ResizeBoth		= 0x6
		};
	};

protected:

	UISubPicture	mBackground;
	UISubPicture	mTitlebar;
	UILabel			mTitle;
	UIRegion		mContent;
	byte			mTitleHeight;
	byte			mMovement;
	bool			mResizable;

public:

	UIWindow();

	const UISkin*	GetSkin()			const		{ return mBackground.GetSkin();	}
	const Color3f&	GetColor()			const		{ return mTitle.GetColor();		}
	const String&	GetText()			const		{ return mTitle.GetText();		}
	const IFont*	GetFont()			const		{ return mTitle.GetFont();		}
	char			GetAlignment()		const		{ return mTitle.GetAlignment();	}
	byte			GetTitlebarHeight() const		{ return mTitleHeight;			}
	bool			IsResizable()		const		{ return mResizable;			}

	void SetSkin			(const UISkin* skin);
	void SetColor			(const Color3f& color)	{ mTitle.SetColor(color);		}
	void SetText			(const String& text)	{ mTitle.SetText(text);			}
	void SetFont			(const IFont* font)		{ mTitle.SetFont(font);			}
	void SetAlignment		(char val)				{ mTitle.SetAlignment(val);		}
	void SetTitlebarHeight	(byte val);
	void SetResizable		(bool val)				{ mResizable = val;				}

	// Resizes the window to fit the specified size
	void ResizeToFit (const Vector2i& size);

	// Returns the size the window has to be in order to have content area of specified size
	Vector2i GetSizeForContent (const Vector2i& size);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Window", UIWindow, UIAnimatedFrame, UIArea);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIArea* ptr);
	virtual void _SetRootPtr   (UIRoot* ptr);

public:

	// Window uses a content region which is smaller than the actual region. All children use this content region.
	virtual const UIRegion& GetSubRegion() const { return mContent; }

	// Marks this specific area as needing to be rebuilt
	virtual void SetDirty();

	// Called when something changes in the texture
	virtual void OnTextureChanged (const ITexture* ptr);

	// Any per-frame animation should go here
	virtual bool OnUpdate (bool dimensionsChanged);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root);
	virtual void OnSerializeTo (TreeNode& root) const;

protected:

	// Events
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
};