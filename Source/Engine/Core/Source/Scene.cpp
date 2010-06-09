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
		bool camChanged = (mLastCamPos != pos || mLastCamRot != rot || mLastCamRange != range);

		if (camChanged)
		{
			mLastCamPos		= pos;
			mLastCamRot		= rot;
			mLastCamRange	= range;
		}

		// Activate the matrices
		ActivateMatrices();

		// Save the projection matrix
		IGraphics* graphics (mRoot->mCore->GetGraphics());
		mLastProj = graphics->GetProjectionMatrix();

		// Update the frustum
		mFrustum.Update( graphics->GetModelViewProjMatrix() );

		// Raycast hits are no longer valid
		mHits.Clear();

		// Cull the scene
		_Cull(graphics, mFrustum, pos, mLastCamRot.GetForward(), camChanged);
	}
}

//============================================================================================================
// Culls the scene's objects given the specified camera position, rotation, and projection
//============================================================================================================

void Scene::Cull (const Vector3f& pos, const Quaternion& rot, const Matrix44& proj)
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
		IGraphics* graphics (mRoot->mCore->GetGraphics());
		mFrustum.Update(graphics->GetModelViewProjMatrix());

		// Raycast hits are no longer valid
		mHits.Clear();

		// Cull the scene
		_Cull(graphics, mFrustum, mLastCamPos, mLastCamRot.GetForward(), true);
	}
}

//============================================================================================================
// Re-activates the scene's matrices on the graphics controller
//============================================================================================================

void Scene::ActivateMatrices()
{
	IGraphics* graphics (mRoot->mCore->GetGraphics());

	graphics->SetScreenProjection(false);
	graphics->SetActiveRenderTarget(mRenderTarget);
	graphics->SetCameraOrientation(mLastCamPos, mLastCamRot.GetForward(), mLastCamRot.GetUp());
	
	if (mLastCamRange.x > 0.0f)
	{
		graphics->SetCameraRange(mLastCamRange);
	}
	else
	{
		graphics->SetProjectionMatrix(mLastProj);
	}
}

//============================================================================================================
// Casts a ray into the screen at the specified mouse position
//============================================================================================================

Scene::RayHits& Scene::Raycast (const Vector2i& screenPos)
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
		return Draw(mForward, clearScreen);
	}
	return 0;
}

//============================================================================================================
// Convenience function: draws the scene using default deferred rendering techniques
//============================================================================================================

uint Scene::DrawAllDeferred (byte ssao, byte postProcess)
{
	uint count (0);

	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		// Set the draw callback
		if (!mDrawCallback)
		{
			mDrawCallback = bind(&Scene::Draw, this);
		}

		// Set the list of techniques used to draw the scene
		if (mDrawTechniques.IsEmpty())
		{
			mDrawTechniques.Expand() = graphics->GetTechnique("Deferred");
			mDrawTechniques.Expand() = graphics->GetTechnique("Decal");
		}

		// Update the potentially changed parameters
		mAOLevel = ssao;

		// Draw the scene
		count = Deferred::Draw(graphics, *this, mQueue.mLights);

		// Post-process step
		if (postProcess == 2)
		{
			PostProcess::Bloom(graphics, *this, 1.0f);
		}
		else if (postProcess == 1)
		{
			PostProcess::None(graphics, *this);
		}
	}
	return count;
}

//============================================================================================================
// Draw the scene using the default combination of deferred rendering and forward rendering approaches.
//============================================================================================================

uint Scene::Draw (float bloom, const Vector3f& focalRange)
{
	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		if (mDrawTechniques.IsEmpty())
		{
			mDrawTechniques.Expand() = graphics->GetTechnique("Deferred");
			mDrawTechniques.Expand() = graphics->GetTechnique("Decal");
		}

		if (mForward.IsEmpty())
		{
			mForward.Expand() = graphics->GetTechnique("Wireframe");
			mForward.Expand() = graphics->GetTechnique("Transparent");
			mForward.Expand() = graphics->GetTechnique("Particle");
			mForward.Expand() = graphics->GetTechnique("Glow");
			mForward.Expand() = graphics->GetTechnique("Glare");
		}

		// Set the draw callback
		if (!mDrawCallback)
		{
			mDrawCallback = bind(&Scene::Draw, this);
		}

		// Draw the scene using the deferred techniques
		uint count = Deferred::Draw(graphics, *this, mQueue.mLights);

		// Draw the scene using forward rendering techniques
		count += DrawAllForward(false);

		// Post-processing
		if (bloom != 0.0f)
		{
			if (focalRange.IsZero())
			{
				// Only bloom
				PostProcess::Bloom(graphics, *this, bloom);
			}
			else
			{
				// Bloom and depth-of-field
				PostProcess::Both(graphics, *this, bloom, focalRange.x, focalRange.y, focalRange.z);
			}
		}
		else if (!focalRange.IsZero())
		{
			// Only depth-of-field
			PostProcess::DepthOfField(graphics, *this, focalRange.x, focalRange.y, focalRange.z);
		}
		else
		{
			// No post-processing
			PostProcess::None(graphics, *this);
		}

		// Return the number of rendered objects
		return count;
	}
	return 0;
}

//============================================================================================================
// Draws the scene using the specified technique
//============================================================================================================

uint Scene::Draw (const String& technique, bool clearScreen)
{
	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();
		return Draw(graphics->GetTechnique(technique), clearScreen);
	}
	return 0;
}

//============================================================================================================
// Draws the scene using the specified technique
//============================================================================================================

uint Scene::Draw (const ITechnique* technique, bool clearScreen)
{
	mTechs.Clear();
	mTechs.Expand() = technique;
	return Draw(mTechs, clearScreen);
}

//============================================================================================================
// Draw the specified scene
//============================================================================================================

uint Scene::Draw (const Techniques& techniques, bool clearScreen, bool insideOut)
{
	uint result(0);

	if (mRoot != 0)
	{
		IGraphics* graphics = mRoot->mCore->GetGraphics();

		// Clear the screen if needed
		if (clearScreen) graphics->Clear();

		// Reset to perspective projection
		graphics->SetScreenProjection( false );

		// Draw the scene
		result += mQueue.Draw(graphics, techniques, insideOut);

		// Restore the potentially altered default state
		graphics->SetNormalize(false);

		// Automatically set output depth and color textures
		const IRenderTarget* target = graphics->GetActiveRenderTarget();
		mOutDepth = (target == 0) ? 0 : target->GetDepthTexture();
		mOutColor = (target == 0) ? 0 : target->GetColorTexture(0);
	}
	return result;
}

//============================================================================================================
// Culls the scene using the specified frustum
//============================================================================================================

void Scene::_Cull (IGraphics* graphics, const Frustum& frustum, const Vector3f& pos, const Vector3f& dir, bool camMoved)
{
	FillParams params (mQueue, frustum);
	params.mCamPos		= pos;
	params.mCamDir		= dir;
	params.mCamChanged	= camMoved;

	mQueue.Clear();
	mRoot->Fill(params);
	mQueue.Sort();
}