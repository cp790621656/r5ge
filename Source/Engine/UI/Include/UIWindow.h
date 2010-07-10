#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
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
	String			mPrefix;
	byte			mTitleHeight;
	byte			mMovement;
	bool			mResizable;

public:

	UIWindow();

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIWindow", UIWindow, UIAnimatedFrame, UIWidget);

	const UISkin*	GetSkin()			const		{ return mBackground.GetSkin();	}
	const String&	GetPrefix()			const		{ return mPrefix;				}
	const Color3f&	GetColor()			const		{ return mTitle.GetColor();		}
	const String&	GetText()			const		{ return mTitle.GetText();		}
	const IFont*	GetFont()			const		{ return mTitle.GetFont();		}
	char			GetAlignment()		const		{ return mTitle.GetAlignment();	}
	byte			GetTitlebarHeight() const		{ return mTitleHeight;			}
	bool			IsResizable()		const		{ return mResizable;			}

	void SetSkin			(const UISkin* skin, bool setDirty = true);
	void SetPrefix			(const String& prefix, bool setDirty = true);
	void SetColor			(const Color3f& color)	{ mTitle.SetColor(color);		}
	void SetText			(const String& text)	{ mTitle.SetText(text);			}
	void SetFont			(const IFont* font)		{ mTitle.SetFont(font);			}
	void SetAlignment		(char val)				{ mTitle.SetAlignment(val);		}
	void SetTitlebarHeight	(byte val);
	void SetResizable		(bool val)				{ mResizable = val;				}

	// Resizes the window to fit the specified size
	void ResizeToFit (const Vector2i& size);

	// Returns the size the window has to be in order to have content widget of specified size
	Vector2i GetSizeForContent (const Vector2i& size);

public:

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIWidget* ptr);
	virtual void _SetRootPtr   (UIManager* ptr);

public:

	// Window uses a content region which is smaller than the actual region. All children use this content region.
	virtual const UIRegion& GetSubRegion() const { return mContent; }

	// Called when something changes in the texture
	virtual void OnTextureChanged (const ITexture* ptr);

	// Any per-frame animation should go here
	virtual bool OnUpdate (bool dimensionsChanged);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;

protected:

	// Events
	virtual void OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual void OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
};