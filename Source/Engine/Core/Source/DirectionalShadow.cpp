#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// A matrix is needed in order to transform vertices into light's texture space
//============================================================================================================

Matrix44 g_shadowMat[4];

void SetShadowMatrix0 (const String& name, Uniform& uniform) { uniform = g_shadowMat[0]; }
void SetShadowMatrix1 (const String& name, Uniform& uniform) { uniform = g_shadowMat[1]; }
void SetShadowMatrix2 (const String& name, Uniform& uniform) { uniform = g_shadowMat[2]; }
void SetShadowMatrix3 (const String& name, Uniform& uniform) { uniform = g_shadowMat[3]; }

//============================================================================================================

Vector2f g_shadowOffset;

void SetShadowOffset (const String& name, Uniform& uniform) { uniform = g_shadowOffset; }

//============================================================================================================

Vector3f g_shadowStart;

void SetShadowStart (const String& name, Uniform& uniform) { uniform = g_shadowStart; }

//============================================================================================================

DirectionalShadow::DirectionalShadow() :
	mGraphics			(0),
	mTextureSize		(1024),
	mCascadeCount		(3),
	mCascadeBias		(0.5f),
	mBlurPasses			(2),
	mSoftness			(2.0f),
	mKernelSize			(1.0f),
	mDepthBias			(6.0f),
	mDummyColorTex		(0),
	mShadowTarget		(0),
	mBlurTarget0		(0),
	mBlurTarget1		(0),
	mShadowTex			(0),
	mBlurTex0			(0),
	mBlurH				(0),
	mBlurV				(0),
	mPost				(0)
{
	for (uint i = 0; i < 4; ++i)
	{
		mLightDepthTarget[i] = 0;
		mLightDepthTex[i] = 0;
		mShader[i] = 0;
	}
}

//============================================================================================================
// Initialization
//============================================================================================================

void DirectionalShadow::Initialize (IGraphics* graphics)
{
	mGraphics = graphics;

	mBlurH	= graphics->GetShader("[R5] Horizontal Depth-Respecting Blur");
	mBlurV	= graphics->GetShader("[R5] Vertical Depth-Respecting Blur");
	mPost	= mGraphics->GetTechnique("Post Process");
}

//============================================================================================================
// Release unused resources
//============================================================================================================

void DirectionalShadow::Release()
{
	if (mBlurTarget1 != 0)
	{
		mGraphics->DeleteRenderTarget(mBlurTarget1);
		mBlurTarget1 = 0;
	}

	if (mBlurTarget0 != 0)
	{
		mGraphics->DeleteRenderTarget(mBlurTarget0);
		mBlurTarget0 = 0;
	}

	if (mDummyColorTex != 0)
	{
		mGraphics->DeleteTexture(mDummyColorTex);
		mDummyColorTex = 0;
	}

	for (uint i = 0; i < 4; ++i)
	{
		if (mLightDepthTarget[i] != 0)
		{
			mGraphics->DeleteRenderTarget(mLightDepthTarget[i]);
			mLightDepthTarget[i] = 0;
		}

		if (mLightDepthTex[i] != 0)
		{
			mGraphics->DeleteTexture(mLightDepthTex[i]);
			mLightDepthTex[i] = 0;
		}
	}

	if (mShadowTarget != 0)
	{
		mGraphics->DeleteRenderTarget(mShadowTarget);
		mShadowTarget = 0;
	}

	if (mBlurTex0 != 0)
	{
		mGraphics->DeleteTexture(mBlurTex0);
		mBlurTex0 = 0;
	}

	if (mShadowTex != 0)
	{
		mGraphics->DeleteTexture(mShadowTex);
		mShadowTex = 0;
	}
}

//============================================================================================================
// Calculates the min/max values for bounds
//============================================================================================================

void GetMinMax (const Vector3f corners[8], const Quaternion& invRot, float from, float to, Vector3f& min, Vector3f& max)
{
	Bounds bounds;

	// Calculate the transformed bounds
	for (uint i = 0; i < 4; ++i)
	{
		Vector3f dir (corners[i+4] - corners[i]);
		bounds.Include((corners[i] + dir * from) * invRot);
		bounds.Include((corners[i] + dir * to)   * invRot);
	}

	// Figure out the minimum and maximum values for the bounding box.
	const Vector3f& projMin (bounds.GetMin());
	const Vector3f& projMax (bounds.GetMax());

	min.x = Float::Max(projMin.x, min.x);
	min.z = Float::Max(projMin.z, min.z);
	max.x = Float::Min(projMax.x, max.x);
	max.y = Float::Min(projMax.y, max.y);
	max.z = Float::Min(projMax.z, max.z);
}

//============================================================================================================
// Draw the scene from the light's perspective
//============================================================================================================

void DirectionalShadow::DrawLightDepth (Object* root, const Object* eye, const Vector3f& dir, const Matrix44& camIMVP)
{
	Vector2f targetSize ((float)mTextureSize, (float)mTextureSize);
	static uint counter (0);

	// Create the PSSM render targets and textures
	for (uint i = 0; i < mCascadeCount; ++i)
	{
		if (mLightDepthTarget[i] == 0)
		{
			mLightDepthTarget[i] = mGraphics->CreateRenderTarget();
			mLightDepthTex[i]	 = mGraphics->CreateRenderTexture(String("Shadow Light Depth %u (%u)", i, counter++));

			// Texture comparison mode meant for shadows
			mLightDepthTex[i]->SetCompareMode(ITexture::CompareMode::Shadow);

			// Light target's depth texture doesn't change
			mLightDepthTarget[i]->AttachDepthTexture(mLightDepthTex[i]);

			// If the graphics card doesn't support depth-only attachments, add a shared dummy color texture.
			// Note that R5 is smart enough to automatically create a dummy RGBA texture if depth-only is not
			// supported, but using the same texture for all depth splits saves memory.

			if (!mGraphics->GetDeviceInfo().mDepthAttachments)
			{
				// Create a dummy texture if one hasn't been created
				if (mDummyColorTex == 0)
				{
					mDummyColorTex = mGraphics->CreateRenderTexture("Dummy Shadow Color");
				}

				// Attach it to the render target
				mLightDepthTarget[i]->AttachColorTexture(0, mDummyColorTex,
					mGraphics->GetDeviceInfo().mAlphaAttachments ?
					ITexture::Format::Alpha : ITexture::Format::RGBA);
			}
		}

		// Update the shadowmap size
		mLightDepthTarget[i]->SetSize(targetSize);
	}

	// Light's rotation
	Quaternion rot (dir);
	Quaternion invRot (-rot);
	Vector3f min, max;

	// Consider the entire scene at first
	{
		// Get the scene's calculated bounds and center
		Bounds bounds (root->GetCompleteBounds());
		Vector3f sceneCenter (bounds.GetCenter());

		// Transform the scene's bounds into light view
		bounds.Transform(Vector3f(), invRot, 1.0f);
		min = bounds.GetMin();
		max = bounds.GetMax();
	}

	// Start with 8 camera frustum points in world space
	Vector3f corners[8];
	camIMVP.GetCorners(corners);

	// Depth bias used to eliminate z-fighting
	float bias = -(mDepthBias * Float::Max(1.0f, mKernelSize)) / mTextureSize;

	// 30 degree rotated kernel
	g_shadowOffset.x = mKernelSize * 0.866f / mTextureSize;
	g_shadowOffset.y = mKernelSize * 0.5f / mTextureSize;

	// Run through all shadow splits
	for (uint i = 0; i < mCascadeCount; ++i)
	{
		Vector3f splitMin (min);
		Vector3f splitMax (max);

		float near = (float)i / mCascadeCount;
		float far  = (float)(i+1) / mCascadeCount;

		near = Interpolation::Linear(near, near * near, mCascadeBias);
		far  = Interpolation::Linear(far, far * far, mCascadeBias);

		if		(i == 0) g_shadowStart.x = far;
		else if (i == 1) g_shadowStart.y = far;
		else if (i == 2) g_shadowStart.z = far;

		// Narrow the selection down to only what's going to affect what the camera sees
		GetMinMax(corners, invRot, near, far, splitMin, splitMax);

		// Projection matrix should be an ortho box large enough to hold the entire transformed scene
		Matrix44 proj;
		Vector3f size (splitMax - splitMin);
		proj.SetToBox(size.x, size.z, size.y);

		// Set up a temporary scene
		static Scene tempScene;
		tempScene.SetRoot(root);
		tempScene.SetFinalTarget(mLightDepthTarget[i]);

		// Cull the light's scene (box's center gets transformed back to world space)
		tempScene.Cull( ((splitMax + splitMin) * 0.5f) * rot, rot, proj, eye );

		// Tweak the projection matrix in order to remove z-fighting
		proj.Translate(Vector3f(0.0f, 0.0f, bias));

		// Create a matrix that will transform the coordinates from camera to light space
		// Bias matrix transforming -1 to 1 range into 0 to 1
		static Matrix43 mvpToScreen (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);
		static Matrix43 screenToMVP (Vector3f(-1.0f, -1.0f, -1.0f), 2.0f);

		// Matrix that will transform coordinates from camera space to light space
		g_shadowMat[i]  = screenToMVP;
		g_shadowMat[i] *= camIMVP;
		g_shadowMat[i] *= mGraphics->GetModelViewMatrix();
		g_shadowMat[i] *= proj;
		g_shadowMat[i] *= mvpToScreen;

		// Draw the scene from the light's point of view, creating the "Light Depth" texture
		tempScene.DrawWithTechnique("Depth", true);
	}
}

//============================================================================================================
// Combine the camera's depth with the light's depth to create a shadow texture
//============================================================================================================

void DirectionalShadow::DrawShadows (const ITexture* camDepth)
{
	if (camDepth != 0)
	{
		// Render target that will be used to create the shadow
		if (mShadowTarget == 0)
		{
			mShadowTarget	= mGraphics->CreateRenderTarget();
			mShadowTex		= mGraphics->CreateRenderTexture();
			mShadowTarget->AttachColorTexture(0, mShadowTex, ITexture::Format::Alpha);
		}

		// The shadow texture should have the same dimensions as the depth texture
		mShadowTarget->SetSize(camDepth->GetSize());

		// Draw a full screen quad with the shader active, creating a full screen shadow texture
		mGraphics->SetActiveRenderTarget(mShadowTarget);
		mGraphics->SetScreenProjection(true);
		mGraphics->SetActiveTechnique(mPost);
		mGraphics->SetActiveMaterial(0);
		mGraphics->SetActiveTexture(0, camDepth);
		mGraphics->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));

		// Activate the proper shader and associated textures
		if (mCascadeCount < 2)
		{
			// Shader that will be used to create the screen-space shadow
			if (mShader[0] == 0)
			{
				mShader[0] = mGraphics->GetShader("[R5] Shadow 1");
				mShader[0]->RegisterUniform("shadowMatrix0", SetShadowMatrix0);
				mShader[0]->RegisterUniform("shadowOffset", SetShadowOffset);
			}

			mGraphics->SetActiveShader(mShader[0]);
			mGraphics->SetActiveTexture(1, mLightDepthTex[0]);
		}
		else if (mCascadeCount == 2)
		{
			if (mShader[1] == 0)
			{
				mShader[1] = mGraphics->GetShader("[R5] Shadow 2");
				mShader[1]->RegisterUniform("shadowMatrix0", SetShadowMatrix0);
				mShader[1]->RegisterUniform("shadowMatrix1", SetShadowMatrix1);
				mShader[1]->RegisterUniform("shadowOffset", SetShadowOffset);
				mShader[1]->RegisterUniform("shadowStart", SetShadowStart);
			}

			mGraphics->SetActiveShader(mShader[1]);
			mGraphics->SetActiveTexture(1, mLightDepthTex[0]);
			mGraphics->SetActiveTexture(2, mLightDepthTex[1]);
		}
		else if (mCascadeCount == 3)
		{
			if (mShader[2] == 0)
			{
				mShader[2] = mGraphics->GetShader("[R5] Shadow 3");
				mShader[2]->RegisterUniform("shadowMatrix0", SetShadowMatrix0);
				mShader[2]->RegisterUniform("shadowMatrix1", SetShadowMatrix1);
				mShader[2]->RegisterUniform("shadowMatrix2", SetShadowMatrix2);
				mShader[2]->RegisterUniform("shadowOffset", SetShadowOffset);
				mShader[2]->RegisterUniform("shadowStart", SetShadowStart);
			}

			mGraphics->SetActiveShader(mShader[2]);
			mGraphics->SetActiveTexture(1, mLightDepthTex[0]);
			mGraphics->SetActiveTexture(2, mLightDepthTex[1]);
			mGraphics->SetActiveTexture(3, mLightDepthTex[2]);
		}
		else
		{
			if (mShader[3] == 0)
			{
				mShader[3] = mGraphics->GetShader("[R5] Shadow 4");
				mShader[3]->RegisterUniform("shadowMatrix0", SetShadowMatrix0);
				mShader[3]->RegisterUniform("shadowMatrix1", SetShadowMatrix1);
				mShader[3]->RegisterUniform("shadowMatrix2", SetShadowMatrix2);
				mShader[3]->RegisterUniform("shadowMatrix3", SetShadowMatrix3);
				mShader[3]->RegisterUniform("shadowOffset", SetShadowOffset);
				mShader[3]->RegisterUniform("shadowStart", SetShadowStart);
			}

			mGraphics->SetActiveShader(mShader[3]);
			mGraphics->SetActiveTexture(1, mLightDepthTex[0]);
			mGraphics->SetActiveTexture(2, mLightDepthTex[1]);
			mGraphics->SetActiveTexture(3, mLightDepthTex[2]);
			mGraphics->SetActiveTexture(4, mLightDepthTex[3]);
		}

		// Clear the render target and draw a full-screen quad with the shader active
		mGraphics->Clear();
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
}

//============================================================================================================
// Blur the shadow, creating a soft outline
//============================================================================================================

void DirectionalShadow::BlurShadows (const ITexture* camDepth)
{
	if (mBlurTarget0 == 0)
	{
		mBlurTex0 = mGraphics->CreateRenderTexture();

		mBlurTarget0 = mGraphics->CreateRenderTarget();
		mBlurTarget0->AttachColorTexture(0, mBlurTex0, mShadowTex->GetFormat());

		mBlurTarget1 = mGraphics->CreateRenderTarget();
		mBlurTarget1->AttachColorTexture(0, mShadowTex, mShadowTex->GetFormat());
	}

	// Use the same size as the shadow texture
	Vector2i size (mShadowTex->GetSize());
	mBlurTarget0->SetSize(size);
	mBlurTarget1->SetSize(size);

	// Camera's depth texture needs to be on texture channel 1
	mGraphics->SetScreenProjection(true);
	mGraphics->SetActiveTechnique(mPost);
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveTexture(0, camDepth);

	// Blur the shadow texture using Gaussian blur
	for (uint i = 0; i < mBlurPasses; ++i)
	{
		mGraphics->SetActiveRenderTarget(mBlurTarget0);
		mGraphics->SetActiveShader(mBlurH);
		if (i == 0) mBlurH->SetUniform("threshold", Vector2f(1.0f / 1920.0f, mSoftness * 80.0f / 1920.0f));
		mGraphics->SetActiveTexture(1, mShadowTex);
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );

		mGraphics->SetActiveRenderTarget(mBlurTarget1);
		mGraphics->SetActiveShader(mBlurV);
		if (i == 0) mBlurV->SetUniform("threshold", Vector2f(1.0f / 1200.0f, mSoftness * 80.0f / 1200.0f));
		mGraphics->SetActiveTexture(1, mBlurTex0);
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
}

//============================================================================================================
// Draw the shadow
//============================================================================================================

ITexture* DirectionalShadow::Draw (Object* root, const Object* eye, const Vector3f& dir,
								   const Matrix44& imvp, const ITexture* depth)
{
	// Sanity check
	if (mCascadeCount < 1) mCascadeCount = 1;
	if (mCascadeCount > 4) mCascadeCount = 4;

	// Draw the depth from the light's perspective
	DrawLightDepth(root, eye, dir, imvp);

	// Create shadows by combining camera's depth with light's depth
	DrawShadows(depth);

	// Blur the shadow texture
	if (mBlurPasses > 0) BlurShadows(depth);

	// Shadow texture is the final result
	return mShadowTex;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void DirectionalShadow::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Texture Size",	mTextureSize);
	node.AddChild("Cascade Count",	mCascadeCount);
	node.AddChild("Cascade Bias",	mCascadeBias);
	node.AddChild("Blur Passes",	mBlurPasses);
	node.AddChild("Softness",		mSoftness);
	node.AddChild("Kernel Size",	mKernelSize);
	node.AddChild("Depth Bias",		mDepthBias);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void DirectionalShadow::OnSerializeFrom (const TreeNode& node)
{
	if		(node.mTag == "Texture Size")	node.mValue >> mTextureSize;
	else if (node.mTag == "Cascades")		node.mValue >> mCascadeCount;
	else if (node.mTag == "Cascade Count")	node.mValue >> mCascadeCount;
	else if (node.mTag == "Cascade Bias")	node.mValue >> mCascadeBias;
	else if (node.mTag == "Blur Passes")	node.mValue >> mBlurPasses;
	else if (node.mTag == "Softness")		node.mValue >> mSoftness;
	else if (node.mTag == "Kernel Size")	node.mValue >> mKernelSize;
	else if (node.mTag == "Depth Bias")		node.mValue >> mDepthBias;
}