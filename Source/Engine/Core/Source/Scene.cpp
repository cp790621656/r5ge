#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Register known node types
//============================================================================================================

Scene::Scene (Core* ptr)
{
	ASSERT(ptr != 0, "Core pointer must not be empty!");

	_SetCore(ptr);

	RegisterObject<Object>			(this);
	RegisterObject<Camera>			(this);
	RegisterObject<DebugCamera>		(this);
	RegisterObject<AnimatedCamera>	(this);
	RegisterObject<ModelInstance>	(this);
	RegisterObject<DirectionalLight>(this);
	RegisterObject<PointLight>		(this);
	RegisterObject<Glow>			(this);
}

//============================================================================================================
// Retrieves active lights, sorting them front-to-back based on distance to the specified position
//============================================================================================================

const ILight::List& Scene::GetVisibleLights (const Vector3f& pos)
{
	uint size = mLights.GetSize();

	if (size > 1)
	{
		// Run through all lights and calculate their distance to the position
		for (uint i = 0; i < size; ++i)
		{
			ILight::Entry& entry (mLights[i]);

			if (entry.mLight->GetAttenParams() == 0)
			{
				// Directional light
				entry.mDistance = 0.0f;
			}
			else
			{
				// Point/spot light
				entry.mDistance = (entry.mLight->GetPosition() - pos).Dot();
			}
		}
		mLights.Sort();
	}
	return mLights;
}

//============================================================================================================
// Updates the entire scene
//============================================================================================================

void Scene::Update()
{
	if (GetFlag(Flag::Enabled))
	{
		Object::_Update(mAbsolutePos, mAbsoluteRot, mAbsoluteScale, false);
	}
}

//============================================================================================================
// Culls the scene
//============================================================================================================

void Scene::Cull (const Frustum& frustum)
{
	mRenderables = 0;

	// Clear all visible objects and lights
	mRenderables.Clear();
	mLights.Clear();

	IGraphics* graphics = mCore->GetGraphics();

	// Culling parameters for this run
	CullParams params ( frustum,
						graphics->GetCameraPosition(),
						graphics->GetCameraDirection(),
						mRenderables,
						mLights );

	// Cull the entire scene, filling the arrays in the process
	Object::_Cull(params, true, true);

	// Sort all objects by layer, group, and distance to the camera
	mRenderables.Sort();
}

//============================================================================================================
// Renders all queues
//============================================================================================================

uint Scene::Render (IGraphics* graphics, const ITechnique* tech, bool insideOut)
{
	uint triangles (0);

	// Only proceed if there is something to render with
	if (mRenderables.IsValid() && tech != 0)
	{
		// Activate the rendering technique
		graphics->SetActiveTechnique(tech, insideOut);

		// Activate lights if the technique supports lighting
		if (tech->GetLighting() != IGraphics::Lighting::None)
		{
			const ILight::List& lights = GetVisibleLights();
			uint last = lights.GetSize();

			for (uint i = 0; i < last; ++i)
				graphics->SetActiveLight( i, lights[i].mLight );

			for (uint i = last; i < 8; ++i)
				graphics->SetActiveLight( i, 0 );
		}

		if (tech->GetSorting() == ITechnique::Sorting::BackToFront)
		{
			// Objects are automatically sorted front to back in order to minimize overdraw,
			// but if we are here then we want to render them back-to-front instead. This is
			// the best rendering method for transparent objects, but it has the most overdraw.
			for (uint i = 0, imax = mRenderables.GetSize(); i < imax; )
			{
				int layer = mRenderables[i].mLayer;
				uint last = i;

				// Keep going until we stumble upon either the end or a different layer
				while (++last < imax && mRenderables[last].mLayer == layer);

				// Run through all renderables on this layer, rendering them back to front
				for (uint b = last; b > i; )
				{
					triangles += mRenderables[--b].mObject->OnRender(graphics, tech, insideOut);
				}

				// Continue from the next layer
				i = last;
			}
		}
		else
		{
			// Run through all objects, rendering them front-to-back
			for (uint i = 0, imax = mRenderables.GetSize(); i < imax; ++i)
			{
				triangles += mRenderables[i].mObject->OnRender(graphics, tech, insideOut);
			}
		}
	}
	return triangles;
}

//============================================================================================================
// Registers a new object creator
//============================================================================================================

void Scene::_RegisterObject (const String& type, const CreateDelegate& callback)
{
	mEntries.Lock();
	mEntries[type] = callback;
	mEntries.Unlock();
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Scene::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild( GetClassID() );

		if (mChildren.IsValid())
		{
			mChildren.Lock();
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
					mChildren[i]->SerializeTo(node);
			}
			mChildren.Unlock();
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Scene::SerializeFrom ( const TreeNode& root, bool forceUpdate )
{
	bool serializable = mSerializable;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if ( tag == "Serializable" )
		{
			// Only possible use for "Serializable" flag inside resource files at Scene level is to set
			// "Serializable" flag to 'false' on all children that follow. Scene itself cannot be set to not
			// be serializable via resource files -- only children can be. The reason behind this is simple:
			// if something is not serializable, it will never be saved to resource files to begin with.

			if (serializable) value >> serializable;
		}
		else
		{
			Object* ptr = _AddObject(tag, value.IsString() ? value.AsString() : value.GetString());
			
			if (ptr != 0)
			{
				ptr->SerializeFrom(node, forceUpdate);
				if (!serializable) ptr->SetSerializable(false);
			}
		}
	}
	return true;
}

//============================================================================================================
// Creates a new node of specified type
//============================================================================================================

Object* Scene::_CreateNode (const String& type)
{
	Object* node (0);
	mEntries.Lock();
	{
		const CreateDelegate* callback = mEntries.GetIfExists(type);

		if (callback != 0)
		{
			node = (*callback)();
			node->mCore = mCore;
		}
	}
	mEntries.Unlock();
	return node;
}