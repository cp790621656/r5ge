#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Initialize the scene
//============================================================================================================

void OSDrawForward::OnInit()
{
	mCam = R5_CAST(Camera, mObject);

	if (mCam == 0)
	{
		ASSERT(false, "You've attached OSDrawForward to something other than a camera, unable to proceed!");
		DestroySelf();
	}
	else
	{
		OSDraw::OnInit();
		mShadow.Initialize(mObject->GetCore());

		mShadowmap	= mGraphics->GetTexture("R5_Shadowmap");
		mOpaque		= mGraphics->GetTechnique("Opaque");
		mShadowed	= mGraphics->GetTechnique("Shadowed Opaque");
	}
}

//============================================================================================================
// Release the render target
//============================================================================================================

void OSDrawForward::OnDestroy()
{
	mShadow.Release();
	OSDraw::OnDestroy();

	if (mDepthTarget != 0)
	{
		mGraphics->DeleteRenderTarget(mDepthTarget);
		mDepthTarget = 0;
	}

	if (mDepthTexture != 0)
	{
		mGraphics->DeleteTexture(mDepthTexture);
		mDepthTexture = 0;
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

void OSDrawForward::OnDraw()
{
	// Cull the scene
	mScene.Cull(mCam);
	uint pass = 0;

	if (mShadows)
	{
		// Get all visible lights
		const Light::List& lights = mScene.GetVisibleLights();
		Matrix44 imvp;

		// Save the render target we're supposed to be using
		IRenderTarget* target = mScene.GetRenderTarget();

		// See if we have any directional lights
		FOREACH(i, lights)
		{
			// If we find a directional light, the scene should be drawn slightly different
			if (lights[i].mLight->mType == ILight::Type::Directional)
			{
				ILight* light = lights[i].mLight;

				// Create the depth texture of what the camera sees -- but only once
				if (pass == 0)
				{
					// Create the depth texture target
					if (mDepthTarget == 0)
					{
						mDepthTarget = mGraphics->CreateRenderTarget();
						mDepthTexture = mGraphics->CreateRenderTexture();
						mDepthTarget->AttachDepthTexture(mDepthTexture);
					}

					// The depth target's size should match the scene's target
					mDepthTarget->SetSize((target == 0) ? mCore->GetWindow()->GetSize() : target->GetSize());

					// Activate the depth target
					mGraphics->SetActiveRenderTarget(mDepthTarget);

					// Draw the scene into the depth render target
					mScene.DrawWithTechnique("Depth");

					// Save the inverse modelview-projection matrix
					imvp = mGraphics->GetInverseMVPMatrix();

					// Disable all lights but the first
					for (uint i = 1; i < 8; ++i) mGraphics->SetActiveLight(i, 0);
				}

				// Draw the shadows and associate the shadow texture with the shadowmap
				{
					const Vector3f& range = mCam->GetAbsoluteRange();
					mShadowmap->SetReplacement( mShadow.Draw(mScene.GetRoot(), light->mDir,
						imvp, mDepthTexture, range.x, range.y) );
				}

				// Draw the scene normally but with a shadow texture created above
				{
					// Adjust the technique's blending -- first pass should use normal blending, after that -- add
					mShadowed->SetBlending(pass == 0 ? IGraphics::Blending::Normal : IGraphics::Blending::Add);
					mShadowed->SetSerializable(false);

					// We'll now be drawing into the scene's render target
					mGraphics->SetActiveRenderTarget(target);

					// Activate the matrices (calculated in the Scene::Cull call at the top of the function)
					mScene.ActivateMatrices();

					// Activate the light and the depth offset
					mGraphics->SetDepthOffset(pass);
					mGraphics->SetActiveLight(0, light);

					// Draw the scene with the shadowed technique
					mScene.DrawWithTechnique(mShadowed, pass == 0, false);

					// Remove the shadowmap association
					mShadowmap->SetReplacement(0);
				}

				// Move on to the next pass
				++pass;
			}
		}

		// Now run through point lights and add them on top
		if (pass > 0)
		{
			uint index (0);

			FOREACH(i, lights)
			{
				if (lights[i].mLight->mType == ILight::Type::Point)
				{
					if (index == 0) mScene.ActivateMatrices();
					mGraphics->SetActiveLight(index++, lights[i].mLight);
				}
			}

			if (index > 0)
			{
				mGraphics->SetDepthOffset(pass++);
				mOpaque->SetBlending(IGraphics::Blending::Add);
				mScene.DrawWithTechnique(mOpaque, false, false);
				mOpaque->SetBlending(IGraphics::Blending::Normal);
				mOpaque->SetSerializable(false);
			}
		}
	}

	// Add particles
	if (pass == 0)
	{
		if (mComplete.IsEmpty())
		{
			mComplete.Expand() = mGraphics->GetTechnique("Opaque");
			mComplete.Expand() = mGraphics->GetTechnique("Wireframe");
			mComplete.Expand() = mGraphics->GetTechnique("Transparent");
			mComplete.Expand() = mGraphics->GetTechnique("Particle");
			mComplete.Expand() = mGraphics->GetTechnique("Glow");
			mComplete.Expand() = mGraphics->GetTechnique("Glare");
		}

		// Nothing has been drawn -- draw everything
		mScene.DrawWithTechniques(mComplete, true, true);
	}
	else
	{
		if (mAdditive.IsEmpty())
		{
			mAdditive.Expand() = mGraphics->GetTechnique("Wireframe");
			mAdditive.Expand() = mGraphics->GetTechnique("Transparent");
			mAdditive.Expand() = mGraphics->GetTechnique("Particle");
			mAdditive.Expand() = mGraphics->GetTechnique("Glow");
			mAdditive.Expand() = mGraphics->GetTechnique("Glare");
		}

		// Add additive objects after everything else has been drawn
		mScene.DrawWithTechniques(mAdditive, false, true);
	}
}