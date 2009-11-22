#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// The root of the scenegraph hierarchy
//============================================================================================================

class Scene : public Object
{
	typedef FastDelegate<Object* (void)>	CreateDelegate;

	struct RenderGroup
	{
		const ITechnique*	mTechnique;
		Array<Object*>		mList;

		RenderGroup() : mTechnique(0) {}
	};

protected:

	Hash<CreateDelegate>	mEntries;		// List of registered creatable scene objects
	Renderables				mRenderables;	// List of visible objects
	ILight::List			mLights;		// List of visible lights

public:

	Scene (Core* ptr);

	// Object creation
	R5_DECLARE_ABSTRACT_CLASS("Scene", Object);

	// Allow direct access to visible lights and objects
	Renderables&		GetVisibleObjects()			{ return mRenderables; }
	ILight::List&		GetVisibleLights()			{ return mLights;  }
	const Renderables&	GetVisibleObjects() const	{ return mRenderables; }
	const ILight::List&	GetVisibleLights()  const	{ return mLights;  }

	// Retrieves active lights, sorting them front-to-back based on distance to the specified position
	const ILight::List& GetVisibleLights(const Vector3f& pos);

	// Updates the entire scene
	void Update();

	// Culls the scene
	void Cull (const Frustum& frustum);

	// Renders all queues
	uint Render (IGraphics* graphics, const ITechnique* tech, bool insideOut);

	// Deletion is straightforward
	void DeleteObject (Object* ptr) { delete ptr; }

public:

	// Registers a new object type
	void _RegisterObject (const String& type, const CreateDelegate& callback);

	// Serialization is slightly different
	bool SerializeTo (TreeNode& root) const;
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);

protected:

	friend class Object;

	// Creates a new node of specified type
	Object* _CreateNode (const String& type);
};