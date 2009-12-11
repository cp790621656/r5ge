#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// User interface root
//============================================================================================================

class UIRoot : public IUI
{
public:

	typedef FastDelegate<UIArea* (void)>  CreateDelegate;

private:

	typedef FastDelegate<bool (UIFrame&, UIArea*)> OnTooltipDelegate;

protected:

	bool						mSerializable;	// Whether the UI will be serialized when saved
	bool						mKey[256];		// Keymap with on/off states
	Vector2i					mMousePos;		// Current mouse position
	Vector2i					mSize;			// Size of the window
	Hash<CreateDelegate>		mCreators;		// Registered widget creator entries
	UIFrame						mRoot;			// Root widget of the user interface
	bool						mDimsChanged;	// Whether the regions need to be recalculated next frame
	bool						mIsDirty;		// Whether the UI needs to be redrawn next frame

	UIArea*						mHoverArea;		// Area the mouse is currently hovering over that will receive mouse events
	UIArea*						mFocusArea;		// Area that has been selected and will receive incoming key events (ex: input field)
	UIArea*						mSelectedArea;	// Mouse buttons give absolute focus to hovering areas while the button is held
	UIContext*					mContext;		// There is only one context menu per UI

	PointerHash<UIEventHandler>	mHandlers;		// Queued event handlers for areas that have not yet been created
	PointerHash<UISkin>			mSkins;			// Available UI skins
	UISkin*						mDefaultSkin;	// Default skin used for the tooltip background, and in the future -- mouse cursors
	IFont*						mDefaultFont;	// Default font used when there is no font specified

	UIAnimatedFrame				mTooltip;		// Tooltip gets its own frame that's rendered after everything else
	UIArea*						mTtArea;		// Area that the tooltip will be shown for
	float						mTtTime;		// Timestamp of the last mouse movement
	float						mTtDelay;		// Delay in seconds the mouse has to hover over an area before a tooltip is shown
	bool						mTtQueued;		// Whether tooltip will be shown when the time is reached
	bool						mTtShown;		// Whether the tooltip is currently shown
	OnTooltipDelegate			mTtDelegate;	// It's possible to overwrite the function that creates the tooltip

public:

	UIRoot();

	// Triggered by Frame::OnDirty, indicating the need for the UI to be redrawn
	bool IsDirty() const { return mIsDirty || mDimsChanged; }
	void SetDirty() { mIsDirty = true; }

	// It's useful to be able to retrieve the current size
	const Vector2i& GetSize() const { return mSize; }

	// Quick way to check if a key is being held down
	bool IsKeyDown (byte key) { return mKey[key]; }

	// It might be useful to remember where the mouse is currently located
	const Vector2i& GetMousePos() const { return mMousePos; }

	// Default values can be retrieved and changed
	UISkin*	GetDefaultSkin()				{ return mDefaultSkin; }
	IFont*	GetDefaultFont()				{ return mDefaultFont; }
	void	SetDefaultSkin(UISkin* skin)	{ mDefaultSkin = skin; }
	void	SetDefaultFont(IFont* font)		{ mDefaultFont = font; }

	// It's useful to know which areas are currently on the watch list
	const UIArea* GetHoverArea()	const { return mHoverArea; }
	const UIArea* GetFocusArea()	const { return mFocusArea; }
	const UIArea* GetEventArea()	const { return mSelectedArea; }

	// If an area is being deleted, the root must be told so it can remove all local references to it
	void RemoveAllReferencesTo (const UIArea* area);

	// Retrieves the specified skin (creates if necessary)
	UISkin* GetSkin (const String& name);

	// Retrieves a pointer to the context menu
	UIContext* GetContextMenu (bool createIfMissing = false);

	// Registers a callback that would create an area of specified type
	void _RegisterWidget (const String& type, const CreateDelegate& callback);

public:

	// Find a responsive area by position
	UIArea* _FindChild (const Vector2i& pos) { return mRoot._FindChild(pos); }

	// Finds an area with the specified name
	UIArea* _FindChild (const String& name, bool recursive = true) { return mRoot._FindChild(name, recursive); }

	// Adds a top-level child of specified type (or returns a child with the same name, if found)
	UIArea* _AddChild (const String& type, const String& name, bool unique = true) { return mRoot._AddChild(type, name, unique); }

public:

	void _TextureChanged(const ITexture* ptr);	// Informs all areas that the texture has changed
	void _SetHoverArea	(UIArea* ptr);			// This area will receive mouse in/out events
	void _SetFocusArea	(UIArea* ptr);			// This area will receive keyboard events
	void _SetEventArea	(UIArea* ptr);			// This area will receive mouse movement events

public:

	// Sets event handlers for the specified area
	void SetOnMouseOver		(const String& areaName, const UIEventHandler::OnMouseOverDelegate&	fnct);
	void SetOnMouseMove		(const String& areaName, const UIEventHandler::OnMouseMoveDelegate&	fnct);
	void SetOnKey			(const String& areaName, const UIEventHandler::OnKeyDelegate&		fnct);
	void SetSetOnScroll		(const String& areaName, const UIEventHandler::OnScrollDelegate&	fnct);
	void SetOnStateChange	(const String& areaName, const UIEventHandler::OnChangeDelegate&	fnct);
	void SetOnValueChange	(const String& areaName, const UIEventHandler::OnChangeDelegate&	fnct);

	// The delay in seconds between the time the mouse stops moving and when the tooltip is shown
	float GetTooltipDelay() const { return mTtDelay; }
	void SetTooltipDelay(float val) { mTtDelay = val; }

	// If desired, a custom tooltip creating behavior can be used to overwrite the default one
	void SetOnTooltip (const OnTooltipDelegate& fnct) { mTtDelegate = fnct; }

	// Creates a default tooltip (returns whether the tooltip is valid)
	bool CreateDefaultTooltip (UIArea* area);

	// Aligns the tooltip using default logic (returns whether the tooltip is valid)
	bool AlignDefaultTooltip();

private:

	// Hides the tooltip if it's currently visible
	void _HideTooltip();

	// Calls mOnFillTooltip function, and fills out the default tooltip if the return val is 'false'
	bool _FillTooltip (UIArea* area);

	// Internal function that retrieves an event handler associated with the specified area
	UIEventHandler* _GetHandler (const String& areaName);

protected:

	friend class UIArea;	// Allow Area class to create areas
	friend class UIFrame;	// Allow Frame class to Update and Draw queues

	// Create a widget of specified type
	UIArea* _CreateArea (const String& type, const String& name, UIArea* parent);

public:

	// IEventReceiver functions
	virtual void OnResize	(const Vector2i& size);
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnScroll	(const Vector2i& pos, float delta);
	virtual bool OnChar		(byte character);

	// Serialization
	virtual bool IsSerializable() const { return mSerializable; }
	virtual void SetSerializable(bool val) { mSerializable = val; }
	virtual bool SerializeFrom (const TreeNode& root);
	virtual bool SerializeTo (TreeNode& root) const;

	// IUI Functions
	virtual bool Update();
	virtual uint Draw();

public:

	// Functions that must be overwritten
	virtual float		GetCurrentTime() const=0;			// Retrieves the current time in seconds
	virtual ITexture*	GetTexture	(const String& name)=0;	// Retrieves a texture of the specified name
	virtual IFont*		GetFont		(const String& name)=0;	// Retrieves a font of the specified name
	virtual IShader*	GetShader	(const String& name)=0;	// Retrieves a shader of the specified name

protected:

	virtual UIQueue*	CreateQueue ()=0;				// Create a single rendering queue
	virtual void		UpdateBuffer(UIQueue* queue)=0;	// Updates the buffer associated with the rendering queue
	virtual void		OnPreDraw () const=0;			// Prepares to render
	virtual uint		DrawQueue (UIQueue* queue)=0;	// Draws a single queue, returning the number of triangles drawn
	virtual void		OnPostDraw() const=0;			// Post-render cleanup
};