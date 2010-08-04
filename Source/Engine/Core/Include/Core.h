#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// R5 Core -- central nexus for everything
//============================================================================================================

class Core : public IEventReceiver, public EventDispatcher
{
public:

	typedef ResourceArray<Resource>			Resources;
	typedef ResourceArray<Mesh>				Meshes;
	typedef ResourceArray<Cloud>			Clouds;
	typedef ResourceArray<Skeleton>			Skeletons;
	typedef ResourceArray<ModelTemplate>	ModelTemplates;
	typedef ResourceArray<Model>			Models;

protected:

	IWindow*		mWin;				// Pointer to the window created by the application
	IGraphics*		mGraphics;			// Graphics manager and controller
	IUI*			mUI;				// User interface, if available
	IAudio*			mAudio;				// Audio, if available
	Object			mRoot;				// Root of the entire scene
	bool			mIsDirty;			// Whether to update the scene next update, regardless of time delta
	bool			mIsKeyDown[256];	// Provides a quick way of checking whether some key is held down
	Vector2i		mMousePos;			// Saved current mouse position
	Vector2i		mUpdatedSize;		// If the window size gets updated, appropriate components must be notified
	Resources		mResources;			// Managed array of resources
	Meshes			mMeshes;			// Managed array of regular meshes
	Clouds			mClouds;			// Managed array of billboard clouds
	Skeletons		mSkeletons;			// Managed array of skeletons
	ModelTemplates	mModelTemplates;	// Managed array of model templates that can be used to create models
	Models			mModels;			// Managed array of instantiable models
	Array<String>	mExecuted;			// Executed resources, for serialization purposes
	uint			mSleepDelay;		// How long the graphics thread will sleep for after each frame
	uint			mFullDraw;			// How many times in a row the scene has been drawn fully (up to 10)
	Thread::IDType	mThreadID;			// ID of the thread the Core was created in

	// Thread safety
	Thread::Lockable mLock;

public:

	// Default constructor
	Core (IWindow* window, IGraphics* graphics, IUI* gui = 0, IAudio* audio = 0);

	// Convenience constructor -- sets the scene's root to the root of the game object tree
	Core (IWindow* window, IGraphics* graphics, IUI* gui, IAudio* audio, Scene& scene);

	// Destructor will wait for all threads to finish before exiting
	~Core();

private:

	// Default initialization function used by the constructors
	void Init();

public:

	R5_DECLARE_SOLO_CLASS("Core");

	// It should be possible to retrieve values passed in the constructor
	IWindow*	GetWindow()		{ return mWin;		}
	IGraphics*	GetGraphics()	{ return mGraphics; }
	IUI*		GetUI()			{ return mUI;		}
	IAudio*		GetAudio()		{ return mAudio;	}

	// Convenience, quick serialization to and from the specified file
	bool operator << (const char* file);
	bool operator >> (const char* file) const;

	void Release();		// Releases the meshes and the scene graph
	bool Update();		// Updates all components
	void Shutdown();	// Shuts down the app

	// Thread safety -- the core is locked during updates and serialization
	void Lock()		const { mLock.Lock(); }
	void Unlock()	const { mLock.Unlock(); }
	bool IsLocked() const { return mLock.IsLocked(); }

	// ID of the thread the Core was created in
	const Thread::IDType& GetThreadID() const { return mThreadID; }

	// If we know we've changed something in the scene, we want to trigger an update next frame
	void SetDirty() { mIsDirty = true; }

	// Useful to have direct access to these components
	Object*			GetRoot()				{ return &mRoot;			}
	Meshes&			GetAllMeshes()			{ return mMeshes;			}
	Clouds&			GetAllClouds()			{ return mClouds;			}
	Models&			GetAllModels()			{ return mModels;			}
	Resources&		GetAllResources()		{ return mResources;		}
	Skeletons&		GetAllSkeletons()		{ return mSkeletons;		}
	ModelTemplates&	GetAllModelTemplates()	{ return mModelTemplates;	}

	// Resource retrieval and creation
	Mesh*			GetMesh			(const String& name, bool createIfMissing = true);
	Cloud*			GetCloud		(const String& name, bool createIfMissing = true);
	Model*			GetModel		(const String& name, bool createIfMissing = true);
	Resource*		GetResource		(const String& name, bool createIfMissing = true);
	Skeleton*		GetSkeleton		(const String& name, bool createIfMissing = true);
	ModelTemplate*	GetModelTemplate(const String& name, bool createIfMissing = true);

	// Number of threads currently being executed in the background
	uint CountExecutingThreads() const;

	// Whether we're in a UI-only mode where the scene is not being processed
	bool IsInUIOnlyMode() const { return mFullDraw < 3 || !mRoot.GetFlag(Object::Flag::Enabled); }

	// UI-only mode can be used to draw the UI while the scene is loading in the background
	void SetUIOnlyMode (bool val) { mRoot.SetFlag(Object::Flag::Enabled, !val); }

	// It's useful to know the current key states
	bool IsKeyDown (byte key) { return mIsKeyDown[key]; }

	// It's also very useful to know the current mouse position
	const Vector2i&	GetMousePos() const	{ return mMousePos; }

	// Sleep delay is used to put the graphics thread to sleep after drawing the frame
	uint GetSleepDelay() const		{ return mSleepDelay; }
	void SetSleepDelay (uint delay) { mSleepDelay = delay; }

public:

	// IEventReceiver functions
	virtual bool OnChar		(byte key);
	virtual bool OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnScroll	(const Vector2i& pos, float delta);
	virtual void OnResize	(const Vector2i& size);

public:

	// Save everything to the specified TreeNode
	bool SerializeTo (TreeNode& root, bool window = true, bool graphics = true, bool ui = true) const;

	// Serialization functions -- 'false' is returned only if the application should exit immediately
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);

	// Serializes from the specified file (the file will be kept in memory as a Resource)
	bool SerializeFrom (const String& file, bool separateThread = true);

	// Executes an existing (loaded) resource in a different thread
	void SerializeFrom (Resource* ptr);
};