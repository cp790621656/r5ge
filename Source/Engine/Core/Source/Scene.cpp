#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves active lights, sorting them front-to-back based on distance to the specified position
//============================================================================================================

const Light::List& Scene::GetVisibleLights (const Vector3f& pos)
{
	uint size = mQueue.mLights.GetSize();

	if (size > 1)
	{
		// Run through all lights and calculate their distance to the position
		for (uint i = 0; i < size; ++i)
		{
			Light::Entry& entry (mQueue.mLights[i]);

			if (entry.mLight->mType == Light::Type::Directional)
			{
				// Directional light
				entry.mDistance = 0.0f;
			}
			else
			{
				// Point/spot light
				entry.mDistance = pos.GetDistanceTo(entry.mLight->mPos);
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
		Cull( cam->GetAbsolutePosition(),
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

		// Set up the view and projection matrices
		graphics->ResetModelViewMatrix();
		graphics->SetCameraRange(range);
		graphics->SetCameraOrientation( pos, dir, rot.GetUp() );

		// Update the frustum
		mFrustum.Update( graphics->GetModelViewProjMatrix() );

		// Raycast hits are no longer valid
		mHits.Clear();

		// Cull the scene
		_Cull(graphics, mFrustum, pos, dir);
	}
}

//============================================================================================================
// Casts a ray into the screen at the specified mouse position
//============================================================================================================

Array<RaycastHit>& Scene::Raycast (const Vector2i& screenPos)
{
	if (mRoot != 0 && (mHits.IsEmpty() || mLastRay != screenPos))
	{
		mLastRay = screenPos;

		IGraphics* graphics (mRoot->mCore->GetGraphics());

		// Get the inverse modelview-projection matrix and the viewport size
		const Matrix44& imvp = graphics->GetInverseMVPMatrix();
		const Vector2i& size = graphics->GetViewport();

		Vector2f pos2 (mRoot->mCore->GetMousePos());
		pos2 /= size;
		pos2.y = 1.0f - pos2.y;

		// Calculate the closest point on the near clippling plane
		Vector3f near (imvp.Unproject(pos2, 0.0f));

		// Populate the list
		mHits.Clear();
		mRoot->Raycast(near, Normalize(near - graphics->GetCameraPosition()), mHits);
		mHits.Sort();
	}
	return mHits;
}

//============================================================================================================
// Convenience function: Draws the scene using default forward rendering techniques
//============================================================================================================

uint Scene::DrawAllForward (bool clearScreen)
{
	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		if (mForward.IsEmpty())
		{
			mForward.Expand() = graphics->GetTechnique("Opaque");
			mForward.Expand() = graphics->GetTechnique("Wireframe");
			mForward.Expand() = graphics->GetTechnique("Transparent");
			mForward.Expand() = graphics->GetTechnique("Particle");
			mForward.Expand() = graphics->GetTechnique("Glow");
			mForward.Expand() = graphics->GetTechnique("Glare");
		}

		if (clearScreen) graphics->Clear();
		return Draw(mForward);
	}
	return 0;
}

//============================================================================================================
// Convenience function: draws the scene using default deferred rendering techniques
//============================================================================================================

Deferred::DrawResult Scene::DrawAllDeferred (byte ssao, byte postProcess)
{
	Deferred::DrawResult result;

	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		// Set the draw callback
		if (!mParams.mDrawCallback)
		{
			mParams.mDrawCallback = bind(&Scene::Draw, this);
		}

		// Set the list of techniques used to draw the scene
		if (mParams.mDrawTechniques.IsEmpty())
		{
			mParams.mDrawTechniques.Expand() = graphics->GetTechnique("Deferred");
			mParams.mDrawTechniques.Expand() = graphics->GetTechnique("Decal");
		}

		// Update the potentially changed parameters
		mParams.mAOLevel = ssao;

		// Draw the scene
		result = Deferred::DrawScene(graphics, mQueue.mLights, mParams);

		// Post-process step
		if (postProcess == 2)
		{
			PostProcess::Bloom(graphics, result.mColor, 1.0f);
		}
		else if (postProcess == 1)
		{
			PostProcess::None(graphics, result.mColor);
		}
		return result;
	}
	return result;
}

//============================================================================================================
// Draws the scene using the specified technique
//============================================================================================================

uint Scene::Draw (const String& technique)
{
	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();
		return Draw(graphics->GetTechnique(technique));
	}
	return 0;
}

//============================================================================================================
// Draws the scene using the specified technique
//============================================================================================================

uint Scene::Draw (const ITechnique* technique)
{
	mTechs.Clear();
	mTechs.Expand() = technique;
	return Draw(mTechs);
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

		// Reset to perspective projection
		graphics->SetActiveProjection( IGraphics::Projection::Perspective );

		// Draw the scene
		result += mQueue.Draw(graphics, techniques, insideOut);

		// Restore the potentially altered default state
		graphics->SetNormalize(false);
	}
	return result;
}

//============================================================================================================
// Culls the scene using the specified frustum
//============================================================================================================

void Scene::_Cull (IGraphics* graphics, const Frustum& frustum, const Vector3f& pos, const Vector3f& dir)
{
	FillParams params (mQueue, frustum);
	params.mCamPos = pos;
	params.mCamDir = dir;

	mQueue.Clear();
	mRoot->Fill(params);
	mQueue.Sort();
}