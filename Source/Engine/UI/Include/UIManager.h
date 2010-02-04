#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// User interface manager
//============================================================================================================

class UIManager : public IUI
{
	// Allow UIFrame class to Update and Draw queues, and the UIWidget class to access _GetListener()
	friend class UIFrame;
	friend class UIWidget;

private:

	typedef FastDelegate<bool (UIFrame&, UIWidget*)> OnTooltipDelegate;

	typedef PointerHash<USEventListener> Listeners;

protected:

	bool				mSerializable;	// Whether the UI will be serialized when saved
	bool				mKey[256];		// Keymap with on/off states
	Vector2i			mMousePos;		// Current mouse position
	Vector2i			mSize;			// Size of the window
	UIFrame				mRoot;			// Root widget of the user interface
	bool				mDimsChanged;	// Whether the regions need to be recalculated next frame
	bool				mIsDirty;		// Whether the UI needs to be redrawn next frame

	UIWidget*			mHover;			// Widget the mouse is currently hovering over that will receive mouse events
	UIWidget*			mFocus;			// Widget that has been selected and will receive incoming key events (ex: input field)
	UIWidget*			mSelected;		// Mouse buttons give absolute focus to the hovering widget while the button is held
	UIContext*			mContext;		// There is only one context menu per UI

	PointerHash<UISkin>	mSkins;			// Available UI skins
	UISkin*				mDefaultSkin;	// Default skin used for the tooltip background, and in the future -- mouse cursors
	IFont*				mDefaultFont;	// Default font used when there is no font specified

	UIAnimatedFrame		mTooltip;		// Tooltip gets its own frame that's rendered after everything else
	UIWidget*			mTtArea;		// Area that the tooltip will be shown for
	float				mTtTime;		// Timestamp of the last mouse movement
	float				mTtDelay;		// Delay in seconds the mouse has to hover over an widget before a tooltip is shown
	bool				mTtQueued;		// Whether tooltip will be shown when the time is reached
	bool				mTtShown;		// Whether the tooltip is currently shown
	OnTooltipDelegate	mTtDelegate;	// It's possible to overwrite the function that creates the tooltip
	Listeners			mListeners;		// List of listeners created prior to actual widgets

	mutable FrameStats	mGameStats;		// Statistics reported at the time the UI manager gets called
	mutable FrameStats	mUIStats;		// Statistics reported during the UI manager's execution

public:

	UIManager();

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
	const UIWidget* GetHoverArea()	const { return mHover; }
	const UIWidget* GetFocusArea()	const { return mFocus; }
	const UIWidget* GetEventArea()	const { return mSelected; }

	// If an widget is being deleted, the root must be told so it can remove all local references to it
	void RemoveAllReferencesTo (const UIWidget* widget);

	// Retrieves the specified skin (creates if necessary)
	UISkin* GetSkin (const String& name);

	// Retrieves a pointer to the context menu
	UIContext* GetContextMenu (bool createIfMissing = false);

	// Rendering statistics for the game and for the user interface
	const FrameStats& GetGameStats() const { return mGameStats; }
	const FrameStats& GetUIStats() const { return mUIStats; }

public:

	// Convenience function: Finds a widget child of specified type
	template <typename Type> Type* FindWidget (const String& name, bool recursive = true)
	{
		return mRoot.FindWidget<Type>(name, recursive);
	}

	// Convenience function: Finds a widget child of specified type
	template <typename Type> Type* FindWidget (const Vector2i& pos)
	{
		return mRoot.FindWidget<Type>(pos);
	}

	// Convenience function: Adds a new widget of specified type
	template <typename Type> Type* AddWidget (const String& name, bool unique = true)
	{
		return mRoot.AddWidget<Type>(name, unique);
	}

public:

	void _TextureChanged(const ITexture* ptr);	// Informs all areas that the texture has changed
	void _SetHoverArea	(UIWidget* ptr);		// This widget will receive mouse in/out events
	void _SetFocusArea	(UIWidget* ptr);		// This widget will receive keyboard events
	void _SetEventArea	(UIWidget* ptr);		// This widget will receive mouse movement events

public:

	// The delay in seconds between the time the mouse stops moving and when the tooltip is shown
	float GetTooltipDelay() const { return mTtDelay; }
	void SetTooltipDelay(float val) { mTtDelay = val; }

	// If desired, a custom tooltip creating behavior can be used to overwrite the default one
	void SetOnTooltip (const OnTooltipDelegate& fnct) { mTtDelegate = fnct; }

	// Creates a default tooltip (returns whether the tooltip is valid)
	bool CreateDefaultTooltip (UIWidget* widget);

	// Aligns the tooltip using default logic (returns whether the tooltip is valid)
	bool AlignDefaultTooltip();

public:

	typedef USEventListener::OnMouseOverDelegate	OnMouseOverDelegate;
	typedef USEventListener::OnMouseMoveDelegate	OnMouseMoveDelegate;
	typedef USEventListener::OnKeyDelegate			OnKeyDelegate;
	typedef USEventListener::OnScrollDelegate		OnScrollDelegate;
	typedef USEventListener::OnFocusDelegate		OnFocusDelegate;
	typedef USEventListener::OnStateDelegate		OnStateDelegate;
	typedef USEventListener::OnValueDelegate		OnValueDelegate;

	// R5 UI allows binding of callbacks to widgets even prior to those widgets being loaded
	void SetOnMouseOver		(const String& widgetName, const OnMouseOverDelegate&	fnct)	{ _AddListener(widgetName)->SetOnMouseOver(fnct); }
	void SetOnMouseMove		(const String& widgetName, const OnMouseMoveDelegate&	fnct)	{ _AddListener(widgetName)->SetOnMouseMove(fnct); }
	void SetOnKey			(const String& widgetName, const OnKeyDelegate&			fnct)	{ _AddListener(widgetName)->SetOnKey(fnct); }
	void SetOnScroll		(const String& widgetName, const OnScrollDelegate&		fnct)	{ _AddListener(widgetName)->SetOnScroll(fnct); }
	void SetOnFocus			(const String& widgetName, const OnFocusDelegate&		fnct)	{ _AddListener(widgetName)->SetOnFocus(fnct); }
	void SetOnStateChange	(const String& widgetName, const OnStateDelegate&		fnct)	{ _AddListener(widgetName)->SetOnStateChange(fnct); }
	void SetOnValueChange	(const String& widgetName, const OnValueDelegate&		fnct)	{ _AddListener(widgetName)->SetOnValueChange(fnct); }

private:

	// Retrieves an event listener script for the widget of specified name, creating a new one if necessary
	USEventListener* _AddListener (const String& name);

	// Retrieves a previously created USEventListener script, if any
	USEventListener* _GetListener (const String& name);

	// Hides the tooltip if it's currently visible
	void _HideTooltip();

	// Calls mOnFillTooltip function, and fills out the default tooltip if the return val is 'false'
	bool _FillTooltip (UIWidget* widget);

protected:

	// IEventReceiver functions
	virtual void OnResize	(const Vector2i& size);
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
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