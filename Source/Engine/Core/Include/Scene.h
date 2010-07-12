#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// The root of the scenegraph hierarchy
//============================================================================================================

class Scene : public TemporaryStorage
{
public:

	typedef Array<const ITechnique*> Techniques;
	typedef Array<RaycastHit> RayHits;
	typedef DrawQueue::Lights Lights;

private:

	Object*			mRoot;			// Scene's root
	Frustum			mFrustum;		// Viewing frustum used for culling
	DrawQueue		mQueue;			// Draw queue created by the culling process
	Vector2i		mLastRay;		// Last mouse position used to cast a ray into the screen
	Vector3f		mLastCamPos;	// Last camera position
	Quaternion		mLastCamRot;	// Last camera rotation
	Vector3f		mLastCamRange;	// Last camera range
	Matrix44		mLastProj;		// Last projection matrix
	RayHits			mHits;			// List of raycast hits after running a raycast in the direction of the ray cast from the mouse
	Techniques		mTechs;			// Temporary techniques

public:

	R5_DECLARE_SOLO_CLASS("Scene");

	Scene (Object* root = 0) : mRoot(root) {}

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

	// It should be possible to change the root of the scene if desired
	Object* GetRoot() { return mRoot; }
	void SetRoot (Object* root);

	// Whether the scene has something to draw (scene must be culled first)
	bool HasSomethingToDraw() const { return mQueue.IsValid(); }

	// These functions are valid after Cull() has been called
	const Frustum&	GetFrustum()		const	{ return mFrustum; }
	const Lights&	GetVisibleLights()	const	{ return mQueue.mLights;  }

	// Retrieves active lights, sorting them front-to-back based on distance to the specified position
	const DrawQueue::Lights&	GetVisibleLights (const Vector3f& pos);

	// Changes the camera's perspective to the specified values. All objects get culled.
	void Cull (const Camera* cam);
	void Cull (const CameraController& cam);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range, const Object* eye = 0);
	void Cull (const Vector3f& pos, const Quaternion& rot, const Matrix44& proj, const Object* eye = 0);

	// Re-activates the scene's matrices on the graphics controller
	// NOTE: You don't need to call this if you're calling Cull()
	void ActivateMatrices();

	// Casts a ray into the screen at the specified mouse position
	RayHits& Raycast (const Vector2i& screenPos);

	// Advanced: Draws the scene using the specified technique
	uint DrawWithTechnique (const String& technique, bool clearScreen = true, bool useLighting = true);

	// Advanced: Draws the scene using the specified technique
	uint DrawWithTechnique (const ITechnique* technique, bool clearScreen = true, bool useLighting = true);

	// Advanced: Draws the scene using the specified techniques
	uint DrawWithTechniques (const Techniques& techniques, bool clearScreen = true, bool useLighting = true);

private:

	// Culls the scene
	void _Cull (const Frustum& frustum, const Vector3f& pos, const Vector3f& dir, const Object* eye);
};