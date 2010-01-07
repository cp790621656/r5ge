#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
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

	// Deferred draw parameters. Saved for convenience and speed.
	Deferred::DrawParams mParams;

	// Default techniques, only filled if used. Cached here for speed.
	Array<const ITechnique*> mForward;

public:

	R5_DECLARE_SOLO_CLASS("Scene");

	Scene (Object* root = 0) : mRoot(root) {}

	// It should be possible to change the root of the scene if desired
	Object* GetRoot() { return mRoot; }
	void SetRoot (Object* root) { mRoot = root; }

	// These functions are valid after Cull() has been called
	const Frustum&		GetFrustum()		const { return mFrustum; }
	const Light::List&	GetVisibleLights()	const { return mQueue.mLights;  }

	// Retrieves active lights, sorting them front-to-back based on distance to the specified position
	const Light::List& GetVisibleLights (const Vector3f& pos);

	// Changes the camera's perspective to the specified values. All objects get culled.
	void Cull (const Camera* cam);
	void Cull (const CameraController& cam);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range);

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

	// Selects the closest visible object to the specified position
	Object* Select (const Vector3f& pos);

private:

	// Culls the scene
	void _Cull (const Frustum& frustum, const Vector3f& pos, const Vector3f& dir);
};