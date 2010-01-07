#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// R5 Core -- central nexus for everything
//============================================================================================================

class Core : public IEventReceiver, public EventDispatcher
{
public:

	typedef ResourceArray<Resource>			Resources;
	typedef ResourceArray<Mesh>				Meshes;
	typedef ResourceArray<BillboardMesh>	BBMeshes;
	typedef ResourceArray<Skeleton>			Skeletons;
	typedef ResourceArray<ModelTemplate>	ModelTemplates;
	typedef ResourceArray<Model>			Models;

protected:

	IWindow*		mWin;				// Pointer to the window created by the application
	IGraphics*		mGraphics;			// Graphics manager and controller
	IUI*			mUI;				// User interface, if available
	Object			mRoot;				// Root of the entire scene
	bool			mIsDirty;			// Whether to update the scene next update, regardless of time delta
	bool			mIsKeyDown[256];	// Provides a quick way of checking whether some key is held down
	Vector2i		mMousePos;			// Saved current mouse position
	Vector2i		mUpdatedSize;		// If the window size gets updated, appropriate components must be notified
	Resources		mResources;			// Managed array of resources
	Meshes			mMeshes;			// Managed array of regular meshes
	BBMeshes	mBBMeshes;			// Managed array of billboard meshes
	Skeletons		mSkeletons;			// Managed array of skeletons
	ModelTemplates	mModelTemplates;	// Managed array of model templates that can be used to create models
	Models			mModels;			// Managed array of instantiable models
	Array<String>	mExecuted;			// Executed resources, for serialization purposes
	uint			mSleepDelay;		// How long the graphics thread will sleep for after each frame

private:

	// Default initialization function used by the constructors
	void Init();

public:

	// Default constructor
	Core (IWindow* window, IGraphics* graphics, IUI* gui);

	// Convenience constructor -- sets the scene's root to the root of the game object tree
	Core (IWindow* window, IGraphics* graphics, IUI* gui, Scene& scene);

	// Destructor will wait for all threads to finish before exiting
	~Core();

public:

	R5_DECLARE_SOLO_CLASS("Core");

	// It should be possible to retrieve values passed in the constructor
	IWindow*	GetWindow()		{ return mWin;		}
	IGraphics*	GetGraphics()	{ return mGraphics; }
	IUI*		GetUI()			{ return mUI;		}

	// Convenience, quick serialization to and from the specified file
	bool operator << (const char* file);
	bool operator >> (const char* file) const;

	void Release();		// Releases the meshes and the scene graph
	bool Update();		// Updates all components
	void Shutdown();	// Shuts down the app

	// If we know we've changed something in the scene, we want to trigger an update next frame
	void SetDirty() { mIsDirty = true; }

	// Useful to have direct access to these components
	Object*			GetRoot()				{ return &mRoot;			}
	Meshes&			GetAllMeshes()			{ return mMeshes;			}
	BBMeshes&		GetAllBillboardMeshes() { return mBBMeshes;			}
	Models&			GetAllModels()			{ return mModels;			}
	Resources&		GetAllResources()		{ return mResources;		}
	Skeletons&		GetAllSkeletons()		{ return mSkeletons;		}
	ModelTemplates&	GetAllModelTemplates()	{ return mModelTemplates;	}

	// Resource retrieval and creation
	Mesh*			GetMesh			(const String& name, bool createIfMissing = true);
	BillboardMesh*	GetBillboardMesh(const String& name, bool createIfMissing = true);
	Model*			GetModel		(const String& name, bool createIfMissing = true);
	Resource*		GetResource		(const String& name, bool createIfMissing = true);
	Skeleton*		GetSkeleton		(const String& name, bool createIfMissing = true);
	ModelTemplate*	GetModelTemplate(const String& name, bool createIfMissing = true);

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
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnScroll	(const Vector2i& pos, float delta);
	virtual void OnResize	(const Vector2i& size);

private:

	// Serialization for resources
	void ParseResources (const TreeNode& root);

public:

	// Serialization functions -- 'false' is returned only if the application should exit immediately
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& root) const;

	// Executes an existing (loaded) resource in a different thread
	void SerializeFrom (Resource* ptr);
};