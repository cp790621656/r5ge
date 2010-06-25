#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// The root of the scenegraph hierarchy
//============================================================================================================

class Scene : protected Deferred::Storage
{
public:

	typedef Array<const ITechnique*>	Techniques;
	typedef Array<IRenderTarget*>		Targets;
	typedef Array<ITexture*>			Textures;

	typedef ITexture*			ITexturePtr;
	typedef IRenderTarget*		IRenderTargetPtr;
	typedef Array<RaycastHit>	RayHits;

private:

	Object*		mRoot;			// Scene's root
	Frustum		mFrustum;		// Viewing frustum used for culling
	DrawQueue	mQueue;			// Draw queue created by the culling process
	Techniques	mTechs;			// Cached list of techniques, for convenience reasons

	Vector2i	mLastRay;		// Last mouse position used to cast a ray into the screen
	Vector3f	mLastCamPos;	// Last camera position
	Quaternion	mLastCamRot;	// Last camera rotation
	Vector3f	mLastCamRange;	// Last camera range
	Matrix44	mLastProj;		// Last projection matrix

	RayHits		mHits;			// List of raycast hits after running a raycast in the direction of the ray cast from the mouse
	Techniques	mForward;		// Default techniques, only filled if used. Cached here for speed.

public:

	R5_DECLARE_SOLO_CLASS("Scene");

	Scene (Object* root = 0) : mRoot(root) {}

	// Since Deferred::Storage is inherited via 'protected', it's a good idea to allow access
	Deferred::Storage& GetDeferredStorage() { return *this; }

	// Finds a child object of the specified name and type
	template <typename Type> Type* FindObject (const String& name, bool recursive = true, bool threadSafe = true)
	{
		return mRoot->FindObject<Type>(name, recursive, threadSafe);
	}

	// Creates a new child of specified type and name
	template <typename Type> Type* AddObject (const String& name, bool threadSafe = true)
	{
		return mRoot->AddObject<Type>(name, threadSafe);
	}

public:

	// Forward the release event to deferred storage
	void Release() { if (mRoot != 0) Deferred::Storage::Release(mRoot->GetGraphics()); }

	// It should be possible to change the root of the scene if desired
	Object* GetRoot() { return mRoot; }
	void SetRoot (Object* root) { mRoot = root; }

	// Activating this function will mean that the scene will be rendered to the specified target
	void SetRenderTarget (IRenderTarget* target) { mRenderTarget = target; }

	// Whether to draw the scene culling front-faces instead of back (for reflections)
	void SetDrawInsideOut (bool val) { mInsideOut = val; }

	// Retrieves the default draw parameters for modification
	Techniques& GetDeferredTechniques()	{ return mDrawTechniques; }
	Techniques& GetForwardTehniques()	{ return mForward; }

	// These functions are valid after Cull() has been called
	const Frustum&		GetFrustum()		const	{ return mFrustum; }
	const Light::List&	GetVisibleLights()	const	{ return mQueue.mLights;  }
	IRenderTarget*		GetRenderTarget()			{ return mRenderTarget; }

	// Retrieves active lights, sorting them front-to-back based on distance to the specified position
	const Light::List& GetVisibleLights (const Vector3f& pos);

	// Changes the camera's perspective to the specified values. All objects get culled.
	void Cull (const Camera* cam);
	void Cull (const CameraController& cam);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Matrix44& proj);

	// Re-activates the scene's matrices on the graphics controller
	// NOTE: You don't need to call this if you're calling Cull()
	void ActivateMatrices();

	// Casts a ray into the screen at the specified mouse position
	RayHits& Raycast (const Vector2i& screenPos);

	// Advanced: Draws the scene using default forward rendering techniques
	uint DrawAllForward (bool clearScreen = true);

	// Advanced: Draws the scene using default deferred rendering techniques
	uint DrawAllDeferred (byte ssao = 0, byte postProcess = 1);

	// Draw the scene using the default combination of deferred rendering and forward rendering approaches.
	uint Draw (float bloom = 0.0f, const Vector3f& focalRange = Vector3f(), byte ssao = 0);

	// Advanced: Draws the scene using the specified technique
	uint DrawWithTechnique (const String& technique, bool clearScreen = true, bool useLighting = true);

	// Advanced: Draws the scene using the specified technique
	uint DrawWithTechnique (const ITechnique* technique, bool clearScreen = true, bool useLighting = true);

	// Advanced: Draws the scene using the specified techniques
	uint DrawWithTechniques (const Techniques& techniques, bool clearScreen = true, bool useLighting = true);

private:

	// Culls the scene
	void _Cull (IGraphics* graphics, const Frustum& frustum, const Vector3f& pos, const Vector3f& dir, bool camMoved);
};