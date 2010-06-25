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

				// Draw the shadows
				{
					const Vector3f& range = mCam->GetAbsoluteRange();
					mShadow.Draw(mScene.GetRoot(), light->mDir, imvp, mDepthTexture, range.x, range.y);
				}

				// Draw the scene normally but with a shadow texture created above
				{
					static ITechnique* tech = mGraphics->GetTechnique("Shadowed Opaque");

					// Adjust the technique's blending -- first pass should use normal blending, after that -- add
					tech->SetBlending(pass == 0 ? IGraphics::Blending::Normal : IGraphics::Blending::Add);
					tech->SetSerializable(false);

					// We'll now be drawing into the scene's render target
					mGraphics->SetActiveRenderTarget(target);

					// Activate the matrices (calculated in the Scene::Cull call at the top of the function)
					mScene.ActivateMatrices();

					// Activate the light and the depth offset
					mGraphics->SetDepthOffset(pass);
					mGraphics->SetActiveLight(0, light);

					// Draw the scene with the shadowed technique
					mScene.DrawWithTechnique(tech, pass == 0, false);
				}

				// Move on to the next pass
				++pass;
			}
		}
	}

	// Add particles
	if (pass == 0)
	{
		// Nothing has been drawn -- draw everything
		static Deferred::Storage::Techniques complete;

		if (complete.IsEmpty())
		{
			complete.Expand() = mGraphics->GetTechnique("Opaque");
			complete.Expand() = mGraphics->GetTechnique("Wireframe");
			complete.Expand() = mGraphics->GetTechnique("Transparent");
			complete.Expand() = mGraphics->GetTechnique("Particle");
			complete.Expand() = mGraphics->GetTechnique("Glow");
			complete.Expand() = mGraphics->GetTechnique("Glare");
		}
		mScene.DrawWithTechniques(complete, true, true);
	}
	else
	{
		// Add additive objects after everything else has been drawn
		static Deferred::Storage::Techniques additive;

		if (additive.IsEmpty())
		{
			additive.Expand() = mGraphics->GetTechnique("Wireframe");
			additive.Expand() = mGraphics->GetTechnique("Transparent");
			additive.Expand() = mGraphics->GetTechnique("Particle");
			additive.Expand() = mGraphics->GetTechnique("Glow");
			additive.Expand() = mGraphics->GetTechnique("Glare");
		}
		mScene.DrawWithTechniques(additive, false, true);
	}
}