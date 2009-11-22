#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Window is a complex widget based on Animated Frame, using several other widgets
//============================================================================================================

class Window : public AnimatedFrame
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

	SubPicture	mBackground;
	SubPicture	mTitlebar;
	Label		mTitle;
	Region		mContent;
	byte		mTitleHeight;
	byte		mMovement;
	bool		mResizable;

public:

	Window();

	const Skin*		GetSkin()			const		{ return mBackground.GetSkin();	}
	const Color3f&	GetColor()			const		{ return mTitle.GetColor();		}
	const String&	GetText()			const		{ return mTitle.GetText();		}
	const IFont*	GetFont()			const		{ return mTitle.GetFont();		}
	char			GetAlignment()		const		{ return mTitle.GetAlignment();	}
	byte			GetTitlebarHeight() const		{ return mTitleHeight;			}
	bool			IsResizable()		const		{ return mResizable;			}

	void SetSkin			(const Skin* skin);
	void SetColor			(const Color3f& color)	{ mTitle.SetColor(color);		}
	void SetText			(const String& text)	{ mTitle.SetText(text);			}
	void SetFont			(const IFont* font)		{ mTitle.SetFont(font);			}
	void SetAlignment		(char val)				{ mTitle.SetAlignment(val);		}
	void SetTitlebarHeight	(byte val);
	void SetResizable		(bool val)				{ mResizable = val;				}

	// Returns the size the window has to be in order to have content area of specified size
	Vector2f GetSizeForContent (float x, float y);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Window", Window, AnimatedFrame, Area);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (Area* ptr);
	virtual void _SetRootPtr   (Root* ptr);

public:

	// Window uses a content region which is smaller than the actual region. All children use this content region.
	virtual const Region& GetSubRegion() const { return mContent; }

	// Marks this specific area as needing to be rebuilt
	virtual void SetDirty();

	// Called when something changes in the texture
	virtual void OnTextureChanged (const ITexture* ptr);

	// Any per-frame animation should go here
	virtual bool OnUpdate (bool dimensionsChanged);

	// Called when a queue is being rebuilt
	virtual void OnFill (Queue* queue);

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
	virtual void CustomSerializeTo (TreeNode& root) const;

protected:

	// Events
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
};