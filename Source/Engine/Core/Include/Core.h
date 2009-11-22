#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// R5 Core -- central nexus for everything
//============================================================================================================

class Core : public IEventReceiver, public EventDispatcher
{
public:

	typedef ResourceArray<Script>			Scripts;
	typedef ResourceArray<Mesh>				Meshes;
	typedef ResourceArray<Skeleton>			Skeletons;
	typedef ResourceArray<ModelTemplate>	ModelTemplates;
	typedef ResourceArray<Model>			Models;

protected:

	IWindow*		mWin;				// Pointer to the window created by the application
	IGraphics*		mGraphics;			// Graphics manager and controller
	IUI*			mUI;				// User interface, if available
	Scene*			mScene;				// Scene root
	bool			mIsKeyDown[256];	// Provides a quick way of checking whether some key is held down
	Vector2i		mMousePos;			// Saved current mouse position
	Vector2i		mUpdatedSize;		// If the window size gets updated, appropriate components must be notified
	Frustum			mFrustum;			// Viewing frustum for visibility culling
	Scripts			mScripts;			// Managed array of scripts
	Meshes			mMeshes;			// Managed array of meshes
	Skeletons		mSkeletons;			// Managed array of skeletons
	ModelTemplates	mModelTemplates;	// Managed array of model templates that can be used to create models
	Models			mModels;			// Managed array of instantiable models
	Array<String>	mExecuted;			// Executed scripts, for serialization purposes

public:

	Core (IWindow* window, IGraphics* graphics, IUI* gui);
	~Core();

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

	// Begins the rendering process, locking all the necessary resources
	void BeginFrame();

	// Prepares the scene for rendering onto the specified target
	void PrepareScene (IRenderTarget* target = 0);

	// Culls the scene's objects, given the specified camera's perspective
	void CullScene (const Vector3f& pos, const Quaternion& rot, const Vector3f& range);

	// Convenience function
	void CullScene (const Camera* cam);

	// Convenience function
	void CullScene (const CameraController& cam);

	// Draw the entire scene with the specified technique
	uint DrawScene (const ITechnique* tech = 0, bool insideOut = false);

	// Draw the user interface
	uint DrawUI();

	// Finish drawing, refreshing the screen
	void EndFrame();

	// Useful to have direct access to these components
	Scene*			GetScene()				{ return mScene;			}
	Meshes&			GetAllMeshes()			{ return mMeshes;			}
	Models&			GetAllModels()			{ return mModels;			}
	Scripts&		GetAllScripts()			{ return mScripts;			}
	Skeletons&		GetAllSkeletons()		{ return mSkeletons;		}
	ModelTemplates&	GetAllModelTemplates()	{ return mModelTemplates;	}

	// Resource retrieval and creation
	Mesh*			GetMesh			(const String& name, bool createIfMissing = true);
	Model*			GetModel		(const String& name, bool createIfMissing = true);
	Script*			GetScript		(const String& name, bool createIfMissing = true);
	Skeleton*		GetSkeleton		(const String& name, bool createIfMissing = true);
	ModelTemplate*	GetModelTemplate(const String& name, bool createIfMissing = true);

	// It's useful to know the current key states
	bool IsKeyDown (byte key) { return mIsKeyDown[key]; }

	// It's also very useful to know the current mouse position
	const Vector2i&	GetMousePos() const	{ return mMousePos; }

public:

	// IEventReceiver functions
	virtual bool OnChar		(byte key);
	virtual bool OnKey		(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnScroll	(const Vector2i& pos, float delta);
	virtual void OnResize	(const Vector2i& size);

private:

	// Serialization for scripts
	void ParseScripts(const TreeNode& root);

public:

	// Serialization functions -- 'false' is returned only if the application should exit immediately
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& root) const;

	// Executes an existing (loaded) script in a different thread
	void SerializeFrom (Script* ptr);
};