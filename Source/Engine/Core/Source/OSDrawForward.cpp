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
	}
}

//============================================================================================================
// Release the render target
//============================================================================================================

void OSDrawForward::OnDestroy()
{
	OSDraw::OnDestroy();

	if (mTarget != 0)
	{
		mGraphics->DeleteRenderTarget(mTarget);
		mTarget = 0;
	}

	if (mDepth != 0)
	{
		mGraphics->DeleteTexture(mDepth);
		mDepth = 0;
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

void OSDrawForward::OnDraw()
{
	const OSSceneRoot::DrawShadows& shadows = mRoot->GetAllShadows();

	if (shadows.IsEmpty())
	{
		mScene.Cull(mCam);
		mScene.DrawAllForward();
	}
	else
	{
		// Create the depth texture of what the camera sees
		{
			if (mTarget == 0)
			{
				mTarget	= mGraphics->CreateRenderTarget();
				mDepth  = mGraphics->CreateRenderTexture();
				mTarget->AttachDepthTexture(mDepth);
			}

			mTarget->SetSize(mCore->GetWindow()->GetSize());
			mScene.SetRenderTarget(mTarget);
			mScene.Cull(mCam);
			mScene.DrawWithTechnique("Depth");
		}

		// Draw the shadows
		{
			// TODO: Add support for more than one light
			const Vector3f& range = mCam->GetAbsoluteRange();
			shadows[0](mGraphics->GetInverseMVPMatrix(), mDepth, range.x, range.y);
		}

		// Draw the scene normally but with a shadow texture created above
		{
			mScene.SetRenderTarget(0);
			mScene.ActivateMatrices();
			mScene.DrawWithTechnique("Shadowed Opaque");
		}

		// Add particles
		{
			static Deferred::Storage::Techniques additive;

			if (additive.IsEmpty())
			{
				additive.Expand() = mGraphics->GetTechnique("Wireframe");
				additive.Expand() = mGraphics->GetTechnique("Transparent");
				additive.Expand() = mGraphics->GetTechnique("Particle");
				additive.Expand() = mGraphics->GetTechnique("Glow");
				additive.Expand() = mGraphics->GetTechnique("Glare");
			}
			mScene.DrawWithTechniques(additive, false);
		}
	}
}