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

Vector4f g_depthRange;

void SetDepthRange (const String& name, Uniform& uniform) { uniform = g_depthRange; }

//============================================================================================================

DirectionalShadow::DirectionalShadow() :
	mCore				(0),
	mGraphics			(0),
	mTextureSize		(2048),
	mBlurPasses			(1),
	mLightDepthTarget	(0),
	mShadowTarget		(0),
	mBlurTarget0		(0),
	mBlurTarget1		(0),
	mLightDepthTex		(0),
	mShadowTex			(0),
	mBlurTex0			(0) {}

//============================================================================================================
// Initialization
//============================================================================================================

void DirectionalShadow::Initialize (Core* core)
{
	mCore = core;
	mGraphics = core->GetGraphics();
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
		mLightDepthTarget->SetSize( Vector2i(mTextureSize, mTextureSize) );
	}

	// Set up a temporary scene
	static Scene tempScene;
	tempScene.SetRoot(root);
	tempScene.SetRenderTarget(mLightDepthTarget);

	// Light's rotation
	Quaternion rot (dir);

	// Get the scene's calculated bounds and extents
	// TODO: Figure out a proper way of calculating the light frustum
	Bounds bounds (root->GetCompleteBounds());
	Vector3f extents ((bounds.GetMax() - bounds.GetMin()) * 0.5f);
	Vector3f center (bounds.GetCenter());

	// Transform the scene's bounds into light space
	bounds.Reset();
	bounds.Include(center + extents);
	bounds.Include(center - extents);
	bounds.Transform(Vector3f(), -rot, 1.0f);

	// Transformed size of the scene
	Vector3f size (bounds.GetMax() - bounds.GetMin());

	// Projection matrix should be an ortho box large enough to hold the entire transformed scene
	Matrix44 proj;
	proj.SetToBox(size.x, size.z, size.y);

	// Cull the light's scene
	tempScene.Cull(center, rot, proj);

	// Create a matrix that will transform the coordinates from camera to light space
	// Bias matrix transforming -1 to 1 range into 0 to 1
	static Matrix43 mvpToScreen (Vector3f(0.5f, 0.5f, 0.5f), 0.5f);
	static Matrix43 screenToMVP (Vector3f(-1.0f, -1.0f, -1.0f), 2.0f);

	// Tweak the projection matrix in order to remove z-fighting
	proj.Translate(Vector3f(0.0f, 0.0f, -(200.0f / mTextureSize) / size.y));

	// The shader variables are global so they can be used even if the script gets destroyed
	{
		// Matrix that will transform coordinates from camera space to light space
		g_shadowMat  = screenToMVP;
		g_shadowMat *= camIMVP;
		g_shadowMat *= mGraphics->GetModelViewMatrix();
		g_shadowMat *= proj;
		g_shadowMat *= mvpToScreen;

		// 1-pixel 30 degree rotated kernel (0.866, 0.5) multiplied by 2
		g_shadowOffset = mLightDepthTex->GetSize();
		g_shadowOffset.x = 1.732f / g_shadowOffset.x;
		g_shadowOffset.y = 1.0f / g_shadowOffset.y;
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
		static IShader* shader = 0;

		// Shader that will be used to create the screen-space shadow
		if (shader == 0)
		{
			shader = mGraphics->GetShader("Other/Shadow");
			shader->RegisterUniform("shadowMatrix", SetShadowMatrix);
			shader->RegisterUniform("shadowOffset", SetShadowOffset);
		}

		// Render target that will be used to create the shadow
		if (mShadowTarget == 0)
		{
			mShadowTarget	= mGraphics->CreateRenderTarget();
			mShadowTex		= mGraphics->GetTexture("Shadowmap");

			mShadowTarget->AttachColorTexture(0, mShadowTex, ITexture::Format::Alpha);
			mShadowTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// The shadow texture should have the same dimensions as the depth texture
		mShadowTarget->SetSize(camDepth->GetSize());

		static const ITechnique* technique = mGraphics->GetTechnique("Post Process");

		// Draw a full screen quad with the shader active, creating a full screen shadow texture
		mGraphics->SetActiveRenderTarget(mShadowTarget);
		mGraphics->SetScreenProjection(true);
		mGraphics->SetActiveTechnique(technique);
		mGraphics->SetActiveMaterial(0);
		mGraphics->SetActiveShader(shader);
		mGraphics->SetActiveTexture(0, camDepth);
		mGraphics->SetActiveTexture(1, mLightDepthTex);
		mGraphics->Clear();
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
}

//============================================================================================================
// Blur the shadow, creating a soft outline
//============================================================================================================

void DirectionalShadow::BlurShadows (const ITexture* camDepth, float near, float far)
{
	static IShader*	blurH = mGraphics->GetShader("Other/blurShadowH");
	static IShader* blurV = mGraphics->GetShader("Other/blurShadowV");
	static const ITechnique* technique = mGraphics->GetTechnique("Post Process");

	if (mBlurTarget0 == 0)
	{
		mBlurTex0 = mGraphics->CreateRenderTexture();

		mBlurTarget0 = mGraphics->CreateRenderTarget();
		mBlurTarget0->AttachColorTexture(0, mBlurTex0, mShadowTex->GetFormat());

		mBlurTarget1 = mGraphics->CreateRenderTarget();
		mBlurTarget1->AttachColorTexture(0, mShadowTex, mShadowTex->GetFormat());

		blurH->RegisterUniform("depthRange", SetDepthRange);
		blurV->RegisterUniform("depthRange", SetDepthRange);
	}

	// Update the depth range used by the shaders above
	g_depthRange.Set(near, far, near * far, far - near);

	// Use the same size as the shadow texture
	Vector2i size (mShadowTex->GetSize());
	mBlurTarget0->SetSize(size);
	mBlurTarget1->SetSize(size);

	// Camera's depth texture needs to be on texture channel 1
	mGraphics->SetScreenProjection(true);
	mGraphics->SetActiveTechnique(technique);
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveTexture(1, camDepth);

	// Blur the shadow texture using Gaussian blur
	for (uint i = 0; i < mBlurPasses; ++i)
	{
		mGraphics->SetActiveRenderTarget(mBlurTarget0);
		mGraphics->SetActiveShader(blurH);
		mGraphics->SetActiveTexture(0, mShadowTex);
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );

		mGraphics->SetActiveRenderTarget(mBlurTarget1);
		mGraphics->SetActiveShader(blurV);
		mGraphics->SetActiveTexture(0, mBlurTex0);
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
}

//============================================================================================================
// Draw the shadow
//============================================================================================================

ITexture* DirectionalShadow::Draw (Object* root, const Vector3f& dir, const Matrix44& imvp,
								   const ITexture* depth, float near, float far)
{
	// Draw the depth from the light's perspective
	DrawLightDepth(root, dir, imvp);

	// Create shadows by combining camera's depth with light's depth
	DrawShadows(depth);

	// Blur the shadow texture
	if (mBlurPasses > 0) BlurShadows(depth, near, far);

	// Shadow texture is the final result
	return mShadowTex;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void DirectionalShadow::SerializeTo (TreeNode& root) const
{
	TreeNode& node = root.AddChild("Shadows");
	node.AddChild("Texture Size", mTextureSize);
	node.AddChild("Blur Passes", mBlurPasses);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void DirectionalShadow::SerializeFrom (const TreeNode& root)
{
	FOREACH(i, root.mChildren)
	{
		const TreeNode& node = root.mChildren[i];
		if		(node.mTag == "Texture Size")	node.mValue >> mTextureSize;
		else if (node.mTag == "Blur Passes")	node.mValue >> mBlurPasses;
	}
}