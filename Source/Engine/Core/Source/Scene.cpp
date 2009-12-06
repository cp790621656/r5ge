#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves active lights, sorting them front-to-back based on distance to the specified position
//============================================================================================================

const ILight::List& Scene::GetVisibleLights (const Vector3f& pos)
{
	uint size = mQueue.mLights.GetSize();

	if (size > 1)
	{
		// Run through all lights and calculate their distance to the position
		for (uint i = 0; i < size; ++i)
		{
			ILight::Entry& entry (mQueue.mLights[i]);

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
		mQueue.mLights.Sort();
	}
	return mQueue.mLights;
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
		IGraphics* graphics (mRoot->mCore->GetGraphics());
		Vector3f dir (rot.GetForward());

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
// Convenience function: Draws the scene using default forward rendering techniques
//============================================================================================================

uint Scene::DrawAllForward()
{
	if (mRoot != 0)
	{
		if (mForward.IsEmpty())
		{
			IGraphics* graphics = mRoot->mCore->GetGraphics();

			mForward.Expand() = graphics->GetTechnique("Opaque");
			mForward.Expand() = graphics->GetTechnique("Transparent");
			mForward.Expand() = graphics->GetTechnique("Particle");
			mForward.Expand() = graphics->GetTechnique("Glow");
			mForward.Expand() = graphics->GetTechnique("Glare");
		}
		return Draw(mForward);
	}
	return 0;
}

//============================================================================================================
// Convenience function: draws the scene using default deferred rendering techniques
//============================================================================================================

uint Scene::DrawAllDeferred (bool ssao, bool bloom)
{
	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		if (mDeferred.IsEmpty())
		{
			mDeferred.Expand() = graphics->GetTechnique("Deferred");
			mDeferred.Expand() = graphics->GetTechnique("Decal");
		}

		Deferred::DrawResult result = Deferred::DrawScene(graphics, mQueue.mLights, mDeferred,
			bind(&Scene::Draw, this), (ssao ? &SSAO::Low : 0));

		if (bloom)
		{
			PostProcess::Bloom(graphics, result.mColor, 1.0f);
		}
		else
		{
			PostProcess::None(graphics, result.mColor);
		}
		return result.mObjects;
	}
	return 0;
}

//============================================================================================================
// Draw the specified scene
//============================================================================================================

uint Scene::Draw (const Techniques& techniques, bool insideOut)
{
	uint result(0);

	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();
		result += mQueue.Draw(graphics, techniques, insideOut);

		// Restore the potentially changed properties
		graphics->ResetWorldMatrix();
		graphics->SetNormalize(false);
	}
	return result;
}

//============================================================================================================
// Selects the closest visible object to the specified position
//============================================================================================================

Object* Scene::Select (const Vector3f& pos)
{
	Object* ptr = 0;
	float radius = 65535.0f;

	//for (uint i = mObjects.GetSize(); i > 0; )
	//{
	//	mObjects[--i].mObject->_Select(pos, ptr, radius);
	//}
	return ptr;
}

//============================================================================================================
// Culls the scene using the specified frustum
//============================================================================================================

void Scene::_Cull (const Frustum& frustum, const Vector3f& pos, const Vector3f& dir)
{
	mQueue.Clear();
	mRoot->_Fill( Object::FillParams(mQueue, frustum, pos, dir) );
	mQueue.Sort();
}