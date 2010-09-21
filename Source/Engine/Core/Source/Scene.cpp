#include "../Include/_All.h"
using namespace R5;

void* g_lastScene = 0;
ModelInstance* g_lastModel = 0;

//============================================================================================================
// Sets the root of the scene
//============================================================================================================

void Scene::SetRoot (Object* root)
{
	mRoot = root;

	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->GetGraphics();

		// Don't allow the graphics pointer to change
		if (mGraphics != 0 && mGraphics != graphics && graphics != 0)
		{
			ASSERT(false, "You can't use the same scene with two different graphics managers!");
		}
		else
		{
			Initialize(graphics);
		}
	}
}

//============================================================================================================
// Retrieves active lights, sorting them front-to-back based on distance to the specified position
//============================================================================================================

const DrawQueue::Lights& Scene::GetVisibleLights (const Vector3f& pos)
{
	uint size = mQueue.mLights.GetSize();

	if (size > 1)
	{
		// Run through all lights and calculate their distance to the position
		for (uint i = 0; i < size; ++i)
		{
			DrawQueue::LightEntry& entry (mQueue.mLights[i]);

			if (entry.mLight->GetProperties().mType == ILight::Type::Directional)
			{
				// Directional light
				entry.mDistance = 0.0f;
			}
			else
			{
				// Point/spot light
				entry.mDistance = pos.GetDistanceTo(entry.mLight->GetAbsolutePosition());
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
			  cam->GetAbsoluteRange(), cam );
	}
}

//============================================================================================================
// Convenience function
//============================================================================================================

void Scene::Cull (const CameraController& cam)
{
	Cull(cam.GetPosition(), cam.GetRotation(), cam.GetRange(), cam.GetActiveCamera());
}

//============================================================================================================
// Culls the scene's objects, given the specified camera's perspective
//============================================================================================================

void Scene::Cull (const Vector3f& pos, const Quaternion& rot, const Vector3f& range, const Object* eye)
{
	if (mRoot != 0)
	{
		// Remember these values for ActivateMatrices() call
		mLastCamPos		= pos;
		mLastCamRot		= rot;
		mLastCamRange	= range;

		// Activate the matrices
		ActivateMatrices();

		// Save the projection matrix
		mLastProj = mGraphics->GetProjectionMatrix();

		// Update the frustum
		mFrustum.Update( mGraphics->GetModelViewProjMatrix() );

		// Raycast hits are no longer valid
		mHits.Clear();

		// Cull the scene
		_Cull(mFrustum, pos, mLastCamRot.GetForward(), eye);
	}
}

//============================================================================================================
// Culls the scene's objects given the specified camera position, rotation, and projection
//============================================================================================================

void Scene::Cull (const Vector3f& pos, const Quaternion& rot, const Matrix44& proj, const Object* eye)
{
	if (mRoot != 0)
	{
		// Save the specified values
		mLastProj	= proj;
		mLastCamPos = pos;
		mLastCamRot = rot;
		mLastCamRange.Set(0.0f, 0.0f, 0.0f);

		// Activate the matrices
		ActivateMatrices();

		// Update the frustum
		mFrustum.Update(mGraphics->GetModelViewProjMatrix());

		// Raycast hits are no longer valid
		mHits.Clear();

		// Cull the scene
		_Cull(mFrustum, mLastCamPos, mLastCamRot.GetForward(), eye);
	}
}

//============================================================================================================
// Re-activates the scene's matrices on the mGraphics controller
//============================================================================================================

void Scene::ActivateMatrices()
{
	g_lastModel = 0;

	mGraphics->SetScreenProjection(false);
	mGraphics->SetActiveRenderTarget(mTarget);
	mGraphics->SetCameraOrientation(mLastCamPos, mLastCamRot.GetForward(), mLastCamRot.GetUp());
	
	if (mLastCamRange.x > 0.0f)
	{
		mGraphics->SetCameraRange(mLastCamRange);
	}
	else
	{
		mGraphics->SetProjectionMatrix(mLastProj);
	}
	g_lastScene = this;
}

//============================================================================================================
// Casts a ray into the screen at the specified mouse position
//============================================================================================================

Scene::RayHits& Scene::Raycast (const Vector2i& screenPos)
{
	if (mRoot != 0 && (mHits.IsEmpty() || mLastRay != screenPos))
	{
		// Matrices must be activated prior to running a raycast
		if (g_lastScene != this) ActivateMatrices();

		mLastRay = screenPos;

		// Get the inverse modelview-projection matrix and the viewport size
		const Matrix44& imvp = mGraphics->GetInverseMVPMatrix();
		const Vector2i& size = mGraphics->GetViewport();

		const Vector2i& mousePos = mRoot->mCore->GetMousePos();
		Vector3f pos (mousePos.x, mousePos.y, 0.0f);
		pos.x /= size.x;
		pos.y /= size.y;
		pos.y = 1.0f - pos.y;

		// Calculate the closest point on the near clipping plane
		Vector3f near (imvp.Unproject(pos));

		// Populate the list
		mHits.Clear();
		mRoot->Raycast(near, Normalize(near - mGraphics->GetCameraPosition()), mHits);
		mHits.Sort();
	}
	return mHits;
}

//============================================================================================================
// Draws the scene using the specified technique
//============================================================================================================

uint Scene::DrawWithTechnique (const String& technique, bool clearColor, bool clearDepth, bool useLighting)
{
	if (mRoot != 0)
	{
		mTechs.Clear();
		mTechs.Expand() = mGraphics->GetTechnique(technique);
		return DrawWithTechniques(mTechs, clearColor, clearDepth, useLighting);
	}
	return 0;
}

//============================================================================================================
// Draws the scene using the specified technique
//============================================================================================================

uint Scene::DrawWithTechnique (const ITechnique* technique, bool clearColor, bool clearDepth, bool useLighting)
{
	mTechs.Clear();
	mTechs.Expand() = technique;
	return DrawWithTechniques(mTechs, clearColor, clearDepth, useLighting);
}

//============================================================================================================
// Draw the specified scene
//============================================================================================================

uint Scene::DrawWithTechniques (const Techniques& techniques, bool clearColor, bool clearDepth, bool useLighting)
{
	uint result(0);

	if (mRoot != 0)
	{
		// Clear the screen if needed
		if (clearColor || clearDepth) mGraphics->Clear(clearColor, clearDepth, clearDepth);

		if (mQueue.IsValid())
		{
			// Reset to perspective projection
			mGraphics->SetScreenProjection(false);

			// Draw the scene
			result += mQueue.Draw(*this, mGraphics, techniques, useLighting, false);

			// Restore the potentially altered default state
			mGraphics->SetNormalize(false);
		}
	}
	return result;
}

//============================================================================================================
// Culls the scene using the specified frustum
//============================================================================================================

void Scene::_Cull (const Frustum& frustum, const Vector3f& pos, const Vector3f& dir, const Object* eye)
{
	FillParams params (mQueue, frustum);
	params.mCamPos	= pos;
	params.mCamDir	= dir;
	params.mEye		= eye;

	mQueue.Clear();
	mRoot->Fill(params);
	mQueue.Sort();
}