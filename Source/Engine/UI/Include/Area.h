#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Expanded region, holding a pointer to an associated Widget, parent, and children
//============================================================================================================

class Root;
class Area : public EventHandler
{
	friend class Root;
	friend class Frame;

protected:

	String				mName;				// Every area needs a name
	Region				mRegion;			// As well as a region
	String				mTooltip;			// All areas can have a tooltip
	bool				mReceivesEvents;	// Whether the area will receive events
	bool				mIsFading;			// Whether the area is fading (will not respond to events)
	Root*				mRoot;				// Pointer to the ui that controls this area
    Area*				mParent;			// Pointer to the parent area
	PointerArray<Area>	mChildren;			// Array of children nodes
	int					mLayer;				// Layer used by this area
	bool				mSerializable;		// Whether the area is saved along with everything else

public:

	Area() :	mReceivesEvents (true),
				mIsFading		(false),
				mRoot			(0),
				mParent			(0),
				mLayer			(0),
				mSerializable	(true) {}

	virtual ~Area() {}

public:

	const String& GetName() const		{ return mName;    }
	const String& GetTooltip() const	{ return mTooltip; }
	void SetName(const String& name)	{ mName = name;    }
	void SetTooltip(const String& text)	{ mTooltip = text; }
	Area* GetParent()					{ return mParent;  }

	// Returns whether the specified parent is reachable by going up the hierarchy tree
	bool IsChildOf (const Area* parent) const { return (parent == this || (mParent == 0 ? false : mParent->IsChildOf(parent))); }

public:

	// Find existing areas or add new ones -- should not be used directly.
	// Use this template instead:  Find<ClassType>(parent, "Name");
	Area* _FindChild (const Vector2i& pos);
	Area* _FindChild (const String& name, bool recursive = true);
	Area* _AddChild  (const String& type, const String& name, bool unique = true);

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
	void BringToFront (Area* child = 0);

	// Gives this area undivided attention
	void SetFocus (bool focus);

public:

	// Gives this area keyboard focus
	void SetKeyboardFocus();

	// Areas that don't receive events will not respond to anything
	bool ReceivesEvents() const			{ return mReceivesEvents && !mIsFading; }
	void SetReceivesEvents(bool val)	{ mReceivesEvents = val; }

	// Having access to the area's dimensions can come in handy
	Region& GetRegion() { return mRegion; }
	void	SetRegion (float left, float top, float width, float height) { mRegion.SetRect(left, top, width, height); }

	// Calls OnTextureChanged() and recurses through children
	void _TextureChanged (const ITexture* ptr);

	// Updates the region's dimensions, calls Area::OnUpdate(), and recurses through children
	bool Update  (const Vector2i& size, bool forceUpdate = false);
	bool Update  (const Region& parent, bool forceUpdate = false);

	// Calls the virtual Area::OnRender() and recurses through children
	uint Render();

	// Serialization
	bool SerializeFrom	(const TreeNode& root);
	bool SerializeTo	(TreeNode& root) const;

protected:

	// Convenience function, seeing as most areas will sue the built-in layer
	void OnDirty (const ITexture* tex) { OnDirty(tex, mLayer, 0); }

	// Functions overwritten by Frame class
	virtual void OnDirty (const ITexture* tex, int layer, const Area* area = 0) { if (mParent != 0) mParent->OnDirty(tex, layer, area); }
	virtual void Fill (Queue* queue);
	virtual uint OnRender() { return 0; }

public:

	// Area creation
	R5_DECLARE_BASE_CLASS("Area", Area);

	// Internal functions. These values are normally set by Root::CreateArea
	virtual void _SetParentPtr (Area* ptr) { mParent = ptr; }
	virtual void _SetRootPtr   (Root* ptr) { mRoot   = ptr; }

public:

	// Can be overwritten for additional functionality (see the Animated Frame class)
	virtual float GetAlpha() const								{ return mRegion.GetAlpha();	}
	virtual void  SetAlpha (float val, float animTime = 0.0f)	{ mRegion.SetAlpha(val);		}

	// Area::Update() function gets the sub-region, and that gets passed to all children (see Window class for an example)
	virtual const Region& GetSubRegion() const { return mRegion; }

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
	virtual void OnFill (Queue* queue) {}

	// Called before and after rendering the queue, respectively
	virtual void OnPreRender (IGraphics* graphics) const {}
	virtual void OnPostRender(IGraphics* graphics) const {}

	// Serialization
	virtual bool CustomSerializeFrom(const TreeNode& root) { return false; }
	virtual void CustomSerializeTo(TreeNode& root) const {}

protected:

	// Events
	virtual bool OnMouseOver(bool inside)									{ return mOnMouseOver	? mOnMouseOver	(this, inside)			 : false; }
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)	{ return mOnMouseMove	? mOnMouseMove	(this, pos, delta)		 : false; }
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown)	{ return mOnKey			? mOnKey		(this, pos, key, isDown) : false; }
	virtual bool OnScroll	(const Vector2i& pos, float delta)				{ return mOnScroll		? mOnScroll		(this, pos, delta)		 : false; }
	virtual bool OnFocus	(bool selected)									{ return mOnSelect		? mOnSelect		(this, selected)		 : false; }
	virtual bool OnChar		(byte character)								{ return false; }
};