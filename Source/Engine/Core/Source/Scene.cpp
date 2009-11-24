#include "../Include/_All.h"
using namespace R5;

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
// Convenience function
//============================================================================================================

void Scene::Cull (const Camera* cam)
{
	if (cam != 0)
	{
		Cull(
			cam->GetAbsolutePosition(),
			cam->GetAbsoluteRotation(),
			cam->GetAbsoluteRange() );
	}
}

//============================================================================================================
// Convenience function
//============================================================================================================

void Scene::Cull (const CameraController& cam)
{
	Cull(cam.GetPosition(), cam.GetRotation(), cam.GetRange());
}

//============================================================================================================
// Culls the scene's objects, given the specified camera's perspective
//============================================================================================================

void Scene::Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range)
{
	if (mRoot != 0)
	{
		Core* core (mRoot->mCore);
		IGraphics* graphics (core->GetGraphics());
		Vector3f dir (rot.GetDirection());

		graphics->SetCameraRange(range);
		graphics->SetCameraOrientation( pos, dir, rot.GetUp() );
		graphics->SetActiveProjection( IGraphics::Projection::Perspective );

		// Update the frustum
		mFrustum.Update( graphics->GetViewProjMatrix() ); // If world matrix is used: (world * ViewProj)

		// Cull the scene
		_Cull(mFrustum, pos, dir);
	}
}

//============================================================================================================
// Draw the specified scene
//============================================================================================================

uint Scene::Draw (const ITechnique* tech, bool insideOut)
{
	uint count (0);

	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		if (tech != 0)
		{
			// Draw all objects from the specified technique
			count += _Draw(tech, insideOut);
		}
		else
		{
			// Draw the scene using all default forward rendering techniques
			ITechnique* opaque = graphics->GetTechnique("Opaque");
			ITechnique* trans  = graphics->GetTechnique("Transparent");
			ITechnique* part   = graphics->GetTechnique("Particle");
			ITechnique* glow   = graphics->GetTechnique("Glow");
			ITechnique* glare  = graphics->GetTechnique("Glare");

			count += _Draw(opaque, insideOut);
			count += _Draw(trans,  insideOut);
			count += _Draw(part,   insideOut);
			count += _Draw(glow,   insideOut);
			count += _Draw(glare,  insideOut);
		}

		// Restore the potentially changed properties
		graphics->ResetWorldMatrix();
		graphics->SetNormalize(false);
	}
	return count;
}


//============================================================================================================
// Culls the scene using the specified frustum
//============================================================================================================

void Scene::_Cull (const Frustum& frustum, const Vector3f& pos, const Vector3f& dir)
{
	// Clear all visible objects and lights
	mObjects.Clear();
	mLights.Clear();

	// Culling parameters for this run
	Object::CullParams params (frustum, pos, dir, mObjects, mLights);

	// Cull the entire scene, filling the arrays in the process
	mRoot->_Cull(params, true, true);

	// Sort all objects by layer, group, and distance to the camera
	mObjects.Sort();
}

//============================================================================================================
// Draws all queues
//============================================================================================================

uint Scene::_Draw (const ITechnique* tech, bool insideOut)
{
	uint triangles (0);
	IGraphics* graphics = mRoot->mCore->GetGraphics();

	// Only proceed if there is something to render with
	if (mObjects.IsValid() && tech != 0)
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
			for (uint i = 0, imax = mObjects.GetSize(); i < imax; )
			{
				int layer = mObjects[i].mLayer;
				uint last = i;

				// Keep going until we stumble upon either the end or a different layer
				while (++last < imax && mObjects[last].mLayer == layer);

				// Run through all renderables on this layer, rendering them back to front
				for (uint b = last; b > i; )
				{
					triangles += mObjects[--b].mObject->OnDraw(graphics, tech, insideOut);
				}

				// Continue from the next layer
				i = last;
			}
		}
		else
		{
			// Run through all objects, rendering them front-to-back
			for (uint i = 0, imax = mObjects.GetSize(); i < imax; ++i)
			{
				triangles += mObjects[i].mObject->OnDraw(graphics, tech, insideOut);
			}
		}
	}
	return triangles;
}