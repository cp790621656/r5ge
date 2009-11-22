#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// User interface root
//============================================================================================================

class Root : public IUI
{
public:

	typedef FastDelegate<Area* (void)>  CreateDelegate;

private:

	typedef FastDelegate<bool (Frame&, Area*)> OnTooltipDelegate;

protected:

	bool						mSerializable;	// Whether the UI will be serialized when saved
	bool						mKey[256];		// Keymap with on/off states
	Vector2i					mMousePos;		// Current mouse position
	Vector2i					mSize;			// Size of the window
	Hash<CreateDelegate>		mCreators;		// Registered widget creator entries
	PointerArray<Area>			mChildren;		// Array of areas that make up the Root hierarchy
	bool						mDimsChanged;	// Whether the regions need to be recalculated next frame
	bool						mIsDirty;		// Whether the UI needs to be redrawn next frame

	Area*						mHoverArea;		// Area the mouse is currently hovering over that will receive mouse events
	Area*						mFocusArea;		// Area that has been selected and will receive incoming key events (ex: input field)
	Area*						mSelectedArea;	// Mouse buttons give absolute focus to hovering areas while the button is held
	Context*					mContext;		// There is only one context menu per UI

	PointerHash<EventHandler>	mHandlers;		// Queued event handlers for areas that have not yet been created
	PointerHash<Skin>			mSkins;			// Available UI skins
	Skin*						mDefaultSkin;	// Default skin used for the tooltip background, and in the future -- mouse cursors
	IFont*						mDefaultFont;	// Default font used when there is no font specified

	AnimatedFrame				mTooltip;		// Tooltip gets its own frame that's rendered after everything else
	Area*						mTtArea;		// Area that the tooltip will be shown for
	float						mTtTime;		// Timestamp of the last mouse movement
	float						mTtDelay;		// Delay in seconds the mouse has to hover over an area before a tooltip is shown
	bool						mTtQueued;		// Whether tooltip will be shown when the time is reached
	bool						mTtShown;		// Whether the tooltip is currently shown
	OnTooltipDelegate			mTtDelegate;	// It's possible to overwrite the function that creates the tooltip

public:

	Root();

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
	Skin*	GetDefaultSkin()			{ return mDefaultSkin; }
	IFont*	GetDefaultFont()			{ return mDefaultFont; }
	void	SetDefaultSkin(Skin* skin)	{ mDefaultSkin = skin; }
	void	SetDefaultFont(IFont* font)	{ mDefaultFont = font; }

	// It's useful to know which areas are currently on the watch list
	const Area* GetHoverArea()	const { return mHoverArea; }
	const Area* GetFocusArea()	const { return mFocusArea; }
	const Area* GetEventArea()	const { return mSelectedArea; }

	// If an area is being deleted, the root must be told so it can remove all local references to it
	void RemoveAllReferencesTo (const Area* area);

	// Retrieves the specified skin (creates if necessary)
	Skin* GetSkin (const String& name);

	// Retrieves a pointer to the context menu
	Context* GetContextMenu (bool createIfMissing = false);

	// Registers a callback that would create an area of specified type
	void _RegisterWidget (const String& type, const CreateDelegate& callback);

public:

	// Find a responsive area by position
	Area* _FindChild (const Vector2i& pos);

	// Finds an area with the specified name
	Area* _FindChild (const String& name, bool recursive = true);

	// Adds a top-level child of specified type (or returns a child with the same name, if found)
	Area* _AddChild (const String& type, const String& name, bool unique = true);

public:

	void _TextureChanged(const ITexture* ptr);	// Informs all areas that the texture has changed
	void _SetHoverArea	(Area* ptr);			// This area will receive mouse in/out events
	void _SetFocusArea	(Area* ptr);			// This area will receive keyboard events
	void _SetEventArea	(Area* ptr);			// This area will receive mouse movement events

public:

	// Sets event handlers for the specified area
	void SetOnMouseOver		(const String& areaName, const EventHandler::OnMouseOverDelegate&	fnct);
	void SetOnMouseMove		(const String& areaName, const EventHandler::OnMouseMoveDelegate&	fnct);
	void SetOnKey			(const String& areaName, const EventHandler::OnKeyDelegate&			fnct);
	void SetSetOnScroll		(const String& areaName, const EventHandler::OnScrollDelegate&		fnct);
	void SetOnStateChange	(const String& areaName, const EventHandler::OnChangeDelegate&		fnct);
	void SetOnValueChange	(const String& areaName, const EventHandler::OnChangeDelegate&		fnct);
	void SetOnTooltip		(const OnTooltipDelegate& fnct) { mTtDelegate = fnct; }

public:

	// The delay in seconds between the time the mouse stops moving and when the tooltip is shown
	float GetTooltipDelay() const { return mTtDelay; }
	void SetTooltipDelay(float val) { mTtDelay = val; }

	// The following two functions can be used from inside the "mTtDelegate" to create default
	// tooltips if no custom tooltip is desired. Look inside Root::_FillTooltip() for an example.

	// Creates a default tooltip (returns whether the tooltip is valid)
	bool CreateDefaultTooltip(Area* area);

	// Aligns the tooltip using default logic (returns whether the tooltip is valid)
	bool AlignDefaultTooltip();

private:

	// Hides the tooltip if it's currently visible
	void _HideTooltip();

	// Calls mOnFillTooltip function, and fills out the default tooltip if the return val is 'false'
	bool _FillTooltip (Area* area);

	// Internal function that retrieves an event handler associated with the specified area
	EventHandler* _GetHandler (const String& areaName);

	// Brings the specified area to foreground
	void _BringToFront (Area* ptr);

protected:

	friend class Area;	// Allow Area class to create areas
	friend class Frame;	// Allow Frame class to Update and Render queues

	// Create a widget of specified type
	Area* _CreateArea (const String& type, const String& name, Area* parent);

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
	virtual uint Render();

public:

	// Functions that must be overwritten
	virtual float		GetCurrentTime() const=0;			// Retrieves the current time in seconds
	virtual ITexture*	GetTexture	(const String& name)=0;	// Retrieves a texture of the specified name
	virtual IFont*		GetFont		(const String& name)=0;	// Retrieves a font of the specified name
	virtual IShader*	GetShader	(const String& name)=0;	// Retrieves a shader of the specified name

protected:

	virtual Queue*	CreateQueue ()=0;				// Create a single rendering queue
	virtual void	UpdateBuffer(Queue* queue)=0;	// Updates the buffer associated with the rendering queue
	virtual void	OnPreRender () const=0;			// Prepares to render
	virtual uint	RenderQueue (Queue* queue)=0;	// Renders a single queue, returning the number of triangles drawn
	virtual void	OnPostRender() const=0;			// Post-render cleanup
};