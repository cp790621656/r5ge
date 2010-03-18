#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// The root of the scenegraph hierarchy
//============================================================================================================

class Scene
{
public:

	typedef Array<const ITechnique*> Techniques;

private:

	Object*		mRoot;		// Scene's root
	Frustum		mFrustum;	// Viewing frustum used for culling
	DrawQueue	mQueue;		// Draw queue created by the culling process
	Techniques	mTechs;		// Cached list of techniques, for convenience reasons
	Vector2i	mLastRay;	// Last mouse position used to cast a ray into the screen

	Vector3f	mLastCamPos;
	Quaternion	mLastCamRot;
	Vector3f	mLastCamRange;

	// List of raycast hits after running a raycast in the direction of the ray cast from the mouse
	Array<RaycastHit> mHits;

	// Deferred draw parameters. Saved for convenience and speed.
	Deferred::DrawParams mParams;

	// Default techniques, only filled if used. Cached here for speed.
	Array<const ITechnique*> mForward;

public:

	R5_DECLARE_SOLO_CLASS("Scene");

	Scene (Object* root = 0) : mRoot(root) {}

	// Finds a child object of the specified name and type
	template <typename Type> Type* FindObject (const String& name, bool recursive = true)
	{
		return mRoot->FindObject<Type>(name, recursive);
	}

	// Creates a new child of specified type and name
	template <typename Type> Type* AddObject (const String& name)
	{
		return mRoot->AddObject<Type>(name);
	}

	// It should be possible to change the root of the scene if desired
	Object* GetRoot() { return mRoot; }
	void SetRoot (Object* root) { mRoot = root; }

	// Retrieves the default draw parameters for modification
	Techniques& GetDeferredTechniques()	{ return mParams.mDrawTechniques; }
	Techniques& GetForwardTechniques()	{ return mForward; }

	// These functions are valid after Cull() has been called
	const Frustum&		GetFrustum()		const { return mFrustum; }
	const Light::List&	GetVisibleLights()	const { return mQueue.mLights;  }

	// Retrieves active lights, sorting them front-to-back based on distance to the specified position
	const Light::List& GetVisibleLights (const Vector3f& pos);

	// Changes the camera's perspective to the specified values. All objects get culled.
	void Cull (const Camera* cam);
	void Cull (const CameraController& cam);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range);

	// Casts a ray into the screen at the specified mouse position
	Array<RaycastHit>& Raycast (const Vector2i& screenPos);

	// Convenience function: draws the scene using default forward rendering techniques
	uint DrawAllForward (bool clearScreen = true);

	// Convenience function: draws the scene using default deferred rendering techniques
	// PostProcess: 0 (do nothing), 1 = draw to screen as-is, 2 = bloom
	Deferred::DrawResult DrawAllDeferred (byte ssao = 0, byte postProcess = 1);

	// Draws the scene using the specified technique
	uint Draw (const String& technique);

	// Draws the scene using the specified technique
	uint Draw (const ITechnique* technique);

	// Draws the scene using the specified techniques
	uint Draw (const Techniques& techniques, bool insideOut = false);

private:

	// Culls the scene
	void _Cull (IGraphics* graphics, const Frustum& frustum, const Vector3f& pos, const Vector3f& dir, bool camMoved);
};