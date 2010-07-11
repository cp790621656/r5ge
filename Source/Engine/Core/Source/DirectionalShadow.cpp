#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// A matrix is needed in order to transform vertices into light's texture space
//============================================================================================================

Matrix44 g_shadowMat;

void SetShadowMatrix (const String& name, Uniform& uniform) { uniform = g_shadowMat; }

//============================================================================================================

Vector2f g_shadowOffset;

void SetShadowOffset (const String& name, Uniform& uniform) { uniform = g_shadowOffset; }

//============================================================================================================

DirectionalShadow::DirectionalShadow() :
	mGraphics			(0),
	mTextureSize		(2048),
	mBlurPasses			(1),
	mSoftness			(1.0f),
	mKernelSize			(2.0f),
	mDepthBias			(3.0f),
	mLightDepthTarget	(0),
	mShadowTarget		(0),
	mBlurTarget0		(0),
	mBlurTarget1		(0),
	mLightDepthTex		(0),
	mShadowTex			(0),
	mBlurTex0			(0),
	mShadow				(0),
	mBlurH				(0),
	mBlurV				(0),
	mPost				(0) {}

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

	if (mLightDepthTarget != 0)
	{
		mGraphics->DeleteRenderTarget(mLightDepthTarget);
		mLightDepthTarget = 0;
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

	if (mLightDepthTex != 0)
	{
		mGraphics->DeleteTexture(mLightDepthTex);
		mLightDepthTex = 0;
	}

	if (mShadowTex != 0)
	{
		mGraphics->DeleteTexture(mShadowTex);
		mShadowTex = 0;
	}
}

//============================================================================================================
// Draw the scene from the light's perspective
//============================================================================================================

void DirectionalShadow::DrawLightDepth (Object* root, const Vector3f& dir, const Matrix44& camIMVP)
{
	if (mLightDepthTarget == 0)
	{
		mLightDepthTarget = mGraphics->CreateRenderTarget();
		mLightDepthTex	  = mGraphics->CreateRenderTexture();

		// Texture comparison mode meant for shadows
		mLightDepthTex->SetCompareMode(ITexture::CompareMode::Shadow);

		// Light target's depth texture doesn't change
		mLightDepthTarget->AttachDepthTexture(mLightDepthTex);
	}

	// Update the shadowmap size
	mLightDepthTarget->SetSize( Vector2i(mTextureSize, mTextureSize) );

	// Set up a temporary scene
	static Scene tempScene;
	tempScene.SetRoot(root);
	tempScene.SetFinalTarget(mLightDepthTarget);

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

	// Narrow the selection down to only what's going to affect what the camera sees
	{
		// Start with 8 camera frustum points in world space
		Vector3f corners[8];
		camIMVP.GetCorners(corners);

		// Rotate all points into light view
		for (uint i = 0; i < 8; ++i) corners[i] *= invRot;

		// Figure out the minimum and maximum values for the bounding box.
		Bounds bounds;
		for (uint i = 0; i < 8; ++i) bounds.Include(corners[i]);
		Vector3f projMin (bounds.GetMin());
		Vector3f projMax (bounds.GetMax());

		// Choose the minimum values for everything but 'minY' (directional lights have no 'near' plane)
		min.x = Float::Max(projMin.x, min.x);
		min.z = Float::Max(projMin.z, min.z);
		max.x = Float::Min(projMax.x, max.x);
		max.y = Float::Min(projMax.y, max.y);
		max.z = Float::Min(projMax.z, max.z);
	}

	// Projection matrix should be an ortho box large enough to hold the entire transformed scene
	Matrix44 proj;
	Vector3f size (max - min);
	proj.SetToBox(size.x, size.z, size.y);

	// Cull the light's scene (box's center gets transformed back to world space)
	tempScene.Cull( ((max + min) * 0.5f) * rot, rot, proj );

	// Create a matrix that will transform the coordinates from camera to light space
	// Bias matrix transforming -1 to 1 range into 0 to 1
	static Matrix43 mvpToScreen (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);
	static Matrix43 screenToMVP (Vector3f(-1.0f, -1.0f, -1.0f), 2.0f);

	// Tweak the projection matrix in order to remove z-fighting
	float bias = -(mDepthBias * Float::Max(1.0f, mKernelSize)) / mTextureSize;
	proj.Translate(Vector3f(0.0f, 0.0f, bias));

	// The shader variables are global so they can be used even if the script gets destroyed
	{
		// Matrix that will transform coordinates from camera space to light space
		g_shadowMat  = screenToMVP;
		g_shadowMat *= camIMVP;
		g_shadowMat *= mGraphics->GetModelViewMatrix();
		g_shadowMat *= proj;
		g_shadowMat *= mvpToScreen;

		// 30 degree rotated kernel
		g_shadowOffset = mLightDepthTex->GetSize();
		g_shadowOffset.x = mKernelSize * 0.866f / g_shadowOffset.x;
		g_shadowOffset.y = mKernelSize * 0.5f / g_shadowOffset.y;
	}

	// Draw the scene from the light's point of view, creating the "Light Depth" texture
	tempScene.DrawWithTechnique("Depth", true);
}

//============================================================================================================
// Combine the camera's depth with the light's depth to create a shadow texture
//============================================================================================================

void DirectionalShadow::DrawShadows (const ITexture* camDepth)
{
	if (camDepth != 0)
	{
		// Shader that will be used to create the screen-space shadow
		if (mShadow == 0)
		{
			mShadow = mGraphics->GetShader("[R5] Shadow");
			mShadow->RegisterUniform("shadowMatrix", SetShadowMatrix);
			mShadow->RegisterUniform("shadowOffset", SetShadowOffset);
		}

		// Render target that will be used to create the shadow
		if (mShadowTarget == 0)
		{
			mShadowTarget	= mGraphics->CreateRenderTarget();
			mShadowTex		= mGraphics->CreateRenderTexture();

			mShadowTarget->AttachColorTexture(0, mShadowTex, ITexture::Format::Alpha);
			mShadowTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// The shadow texture should have the same dimensions as the depth texture
		mShadowTarget->SetSize(camDepth->GetSize());

		// Draw a full screen quad with the shader active, creating a full screen shadow texture
		mGraphics->SetActiveRenderTarget(mShadowTarget);
		mGraphics->SetScreenProjection(true);
		mGraphics->SetActiveTechnique(mPost);
		mGraphics->SetActiveMaterial(0);
		mGraphics->SetActiveShader(mShadow);
		mGraphics->SetActiveTexture(0, camDepth);
		mGraphics->SetActiveTexture(1, mLightDepthTex);
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

ITexture* DirectionalShadow::Draw (Object* root, const Vector3f& dir, const Matrix44& imvp, const ITexture* depth)
{
	// Draw the depth from the light's perspective
	DrawLightDepth(root, dir, imvp);

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
	node.AddChild("Texture Size", mTextureSize);
	node.AddChild("Blur Passes", mBlurPasses);
	node.AddChild("Softness", mSoftness);
	node.AddChild("Kernel Size", mKernelSize);
	node.AddChild("Depth Bias", mDepthBias);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void DirectionalShadow::OnSerializeFrom (const TreeNode& node)
{
	if		(node.mTag == "Texture Size")	node.mValue >> mTextureSize;
	else if (node.mTag == "Blur Passes")	node.mValue >> mBlurPasses;
	else if (node.mTag == "Softness")		node.mValue >> mSoftness;
	else if (node.mTag == "Kernel Size")	node.mValue >> mKernelSize;
	else if (node.mTag == "Depth Bias")		node.mValue >> mDepthBias;
}