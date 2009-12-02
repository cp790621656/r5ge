#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// The root of the scenegraph hierarchy
//============================================================================================================

class Scene
{
public:

	typedef Object::Drawables	Drawables;
	typedef ILight::List		Lights;

private:

	Object*		mRoot;		// Scene's root
	Frustum		mFrustum;	// Viewing frustum used for culling
	Drawables	mObjects;	// List of visible objects
	Lights		mLights;	// List of visible lights
	uint		mMask;		// Culling mask created on Scene::Cull

public:

	R5_DECLARE_SOLO_CLASS("Scene");

	Scene (Object* root = 0) : mRoot(root), mMask(0) {}

	// It should be possible to change the root of the scene if desired
	Object* GetRoot() { return mRoot; }
	void SetRoot (Object* root) { mRoot = root; }

	// These functions are valid after Cull() has been called
	const Frustum&		GetFrustum()		const { return mFrustum; }
	const Drawables&	GetVisibleObjects()	const { return mObjects; }
	const Lights&		GetVisibleLights()	const { return mLights;  }

	// Retrieves active lights, sorting them front-to-back based on distance to the specified position
	const Lights& GetVisibleLights (const Vector3f& pos);

	// Changes the camera's perspective to the specified values. All objects get culled.
	void Cull (const Camera* cam);
	void Cull (const CameraController& cam);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range);

	// Draws the scene using the specified technique
	uint Draw (const ITechnique* tech = 0, bool insideOut = false);

	// Selects the closest visible object to the specified position
	Object* Select (const Vector3f& pos);

private:

	// Culls the scene
	void _Cull (const Frustum& frustum, const Vector3f& pos, const Vector3f& dir);

	// Draws the objects collected in the 'mObjects' queue using the specified technique
	uint _Draw (const ITechnique* tech, bool insideOut);
};