#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Expanded region, holding a pointer to an associated Widget, parent, and children
//============================================================================================================

class UIRoot;
class UIArea : public UIEventHandler
{
	friend class UIRoot;
	friend class UIFrame;

public:

	typedef PointerArray<UIArea> Children;

protected:

	String		mName;				// Every area needs a name
	UIRegion	mRegion;			// As well as a region
	String		mTooltip;			// All areas can have a tooltip
	bool		mReceivesEvents;	// Whether the area will receive events
	bool		mIsFading;			// Whether the area is fading (will not respond to events)
	UIRoot*		mRoot;				// Pointer to the ui that controls this area
    UIArea*		mParent;			// Pointer to the parent area
	Children	mChildren;			// Array of children nodes
	int			mLayer;				// Layer used by this area
	bool		mSerializable;		// Whether the area is saved along with everything else

public:

	UIArea() :	mReceivesEvents (true),
				mIsFading		(false),
				mRoot			(0),
				mParent			(0),
				mLayer			(0),
				mSerializable	(true) {}

	virtual ~UIArea() {}

public:

	const String& GetName() const		{ return mName;		}
	const String& GetTooltip() const	{ return mTooltip;	}
	void SetName(const String& name)	{ mName = name;		}
	void SetTooltip(const String& text)	{ mTooltip = text;	}
	UIArea* GetParent()					{ return mParent;	}
	const Children& GetChildren() const { return mChildren;	}

	// Returns whether the specified parent is reachable by going up the hierarchy tree
	bool IsChildOf (const UIArea* parent) const { return (parent == this || (mParent == 0 ? false : mParent->IsChildOf(parent))); }

public:

	// Find existing areas or add new ones -- should not be used directly.
	// Use this template instead:  Find<ClassType>(parent, "Name");
	UIArea* _FindChild (const Vector2i& pos);
	UIArea* _FindChild (const String& name, bool recursive = true);
	UIArea* _AddChild  (const String& type, const String& name, bool unique = true);

public:

	// Whether the area is actually saved with everything else is subject to change
	bool IsSerializable() const { return mSerializable; }
	void SetSerializable(bool val) { mSerializable = val; }

	// Layer manipulation
	int  GetLayer() const { return mLayer; }
	void SetLayer (int layer, bool setDirty = true);

	// Deletes all child areas
	void DeleteAllChildren();

	// Brings the area to the foreground
	void BringToFront (UIArea* child = 0);

	// Gives this area undivided attention
	void SetFocus (bool focus);

public:

	// Gives this area keyboard focus
	void SetKeyboardFocus();

	// Areas that don't receive events will not respond to anything
	bool ReceivesEvents() const			{ return mReceivesEvents && !mIsFading; }
	void SetReceivesEvents(bool val)	{ mReceivesEvents = val; }

	// Having access to the area's dimensions can come in handy
	UIRegion& GetRegion() { return mRegion; }
	void SetRegion (float left, float top, float width, float height) { mRegion.SetRect(left, top, width, height); }

	// Calls OnTextureChanged() and recurses through children
	void _TextureChanged (const ITexture* ptr);

	// Updates the region's dimensions, calls Area::OnUpdate(), and recurses through children
	bool Update  (const Vector2i& size, bool forceUpdate = false);
	bool Update  (const UIRegion& parent, bool forceUpdate = false);

	// Calls the virtual Area::OnDraw() and recurses through children
	uint Draw();

	// Serialization
	bool SerializeFrom	(const TreeNode& root);
	bool SerializeTo	(TreeNode& root) const;

protected:

	// Convenience function, seeing as most areas will use the built-in layer
	void OnDirty (const ITexture* tex) { OnDirty(tex, mLayer, 0); }

	// Should notify the listeners of state changes
	void OnStateChange() { if (mOnStateChange) mOnStateChange(this); }

	// Should notify the listeners of value changes
	void OnValueChange() { if (mOnValueChange) mOnValueChange(this); }

	// Calls OnFill() on itself then recurses through all non-UIFrame children.
	void Fill (UIQueue* queue);

public:

	// Area creation
	R5_DECLARE_BASE_CLASS("Area", UIArea);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (UIArea* ptr) { mParent = ptr; }
	virtual void _SetRootPtr   (UIRoot* ptr) { mRoot   = ptr; }

protected:

	// Functions overwritten by the Frame class
	virtual void OnDirty (const ITexture* tex, int layer, const UIArea* area = 0) { if (mParent != 0) mParent->OnDirty(tex, layer, area); }
	virtual uint OnDraw() { return 0; }

public:

	// Can be overwritten for additional functionality (see the Animated Frame class)
	virtual float GetAlpha() const { return mRegion.GetCalculatedAlpha(); }
	virtual void  SetAlpha (float val, float animTime = 0.0f) { mRegion.SetAlpha(val); }

	// UIArea::Update() function gets the sub-region, and that gets passed to all children
	// See UIWindow class for an example
	virtual const UIRegion& GetSubRegion() const { return mRegion; }

	// Marks this specific area as needing to be rebuilt
	virtual void SetDirty() {}

	// Called when something changes in the texture
	virtual void OnTextureChanged (const ITexture* ptr) {}

	// Called when the layer changes
	virtual void OnLayerChanged() {}

	// Called before OnUpdate(); can be used to override the area's alpha or dimensions
	virtual bool OnPreUpdate (bool dimensionsChanged) { return false; }

	// Any per-frame animation should go here
	virtual bool OnUpdate (bool dimensionsChanged) { return false; }

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue) {}

	// Called before and after rendering the queue, respectively
	virtual void OnPreDraw (IGraphics* graphics) const {}
	virtual void OnPostDraw(IGraphics* graphics) const {}

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& root) { return false; }
	virtual void OnSerializeTo (TreeNode& root) const {}

protected:

	// Events
	virtual bool OnMouseOver(bool inside)									{ return mOnMouseOver	? mOnMouseOver	(this, inside)			 : false; }
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ return mOnMouseMove	? mOnMouseMove	(this, pos, delta)		 : false; }
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown)	{ return mOnKey			? mOnKey		(this, pos, key, isDown) : false; }
	virtual bool OnScroll	(const Vector2i& pos, float delta)				{ return mOnScroll		? mOnScroll		(this, pos, delta)		 : false; }
	virtual bool OnFocus	(bool selected)									{ return mOnSelect		? mOnSelect		(this, selected)		 : false; }
	virtual bool OnChar		(byte character)								{ return false; }
};