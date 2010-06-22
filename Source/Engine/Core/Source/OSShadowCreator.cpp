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
// Add this light to the draw script
//============================================================================================================

void OSShadowCreator::OnInit()
{
	// Register with the draw script
	mRoot = OSSceneRoot::FindRootOf(mObject);
	mRoot->AddShadow(bind(&OSShadowCreator::DrawShadow, this));

	// For convenience
	mCore		= mObject->GetCore();
	mGraphics	= mObject->GetGraphics();

	// Set the root of the scene
	mScene.SetRoot(mRoot->GetOwner());
}

//============================================================================================================
// Remove this light from the draw script
//============================================================================================================

void OSShadowCreator::OnDestroy()
{
	if (mRoot != 0)
	{
		mRoot->RemoveShadow(bind(&OSShadowCreator::DrawShadow, this));
		mRoot = 0;
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

void OSShadowCreator::DrawLightDepth (const Matrix44& camIMVP)
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

		// Draw the scene into our render target
		mScene.SetRenderTarget(mLightDepthTarget);
	}

	// Light's rotation
	Quaternion rot (mObject->GetAbsoluteRotation());

	// Get the scene's calculated bounds and extents
	Bounds bounds (mScene.GetRoot()->GetCompleteBounds());
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
	mScene.Cull(center, rot, proj);

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
	mScene.DrawWithTechnique("Depth", true);
}

//============================================================================================================
// Combine the camera's depth with the light's depth to create a shadow texture
//============================================================================================================

void OSShadowCreator::DrawShadows (const ITexture* camDepth)
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
			// TODO: Remove the "Shadowmap" name requirement here
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

void OSShadowCreator::BlurShadows (const ITexture* camDepth, float near, float far)
{
	static IRenderTarget* blurTarget0 = 0;
	static IRenderTarget* blurTarget1 = 0;

	static ITexture* blurTex0 = mGraphics->CreateRenderTexture();

	static IShader*	blurH = mGraphics->GetShader("Other/blurShadowH");
	static IShader* blurV = mGraphics->GetShader("Other/blurShadowV");

	static const ITechnique* technique = mGraphics->GetTechnique("Post Process");

	if (blurTarget0 == 0)
	{
		blurTarget0 = mGraphics->CreateRenderTarget();
		blurTarget0->AttachColorTexture(0, blurTex0, mShadowTex->GetFormat());

		blurTarget1 = mGraphics->CreateRenderTarget();
		blurTarget1->AttachColorTexture(0, mShadowTex, mShadowTex->GetFormat());

		blurH->RegisterUniform("depthRange", SetDepthRange);
		blurV->RegisterUniform("depthRange", SetDepthRange);
	}

	// Update the depth range used by the shaders above
	g_depthRange.Set(near, far, near * far, far - near);

	// Use the same size as the shadow texture
	Vector2i size (mShadowTex->GetSize());
	blurTarget0->SetSize(size);
	blurTarget1->SetSize(size);

	// Camera's depth texture needs to be on texture channel 1
	mGraphics->SetScreenProjection(true);
	mGraphics->SetActiveTechnique(technique);
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveTexture(1, camDepth);

	// Blur the shadow texture using Gaussian blur
	for (uint i = 0; i < mBlurPasses; ++i)
	{
		mGraphics->SetActiveRenderTarget(blurTarget0);
		mGraphics->SetActiveShader(blurH);
		mGraphics->SetActiveTexture(0, mShadowTex);
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );

		mGraphics->SetActiveRenderTarget(blurTarget1);
		mGraphics->SetActiveShader(blurV);
		mGraphics->SetActiveTexture(0, blurTex0);
		mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
}

//============================================================================================================
// Draw the shadow
//============================================================================================================

ITexture* OSShadowCreator::DrawShadow (const Matrix44& imvp, const ITexture* depth, float near, float far)
{
	// Draw the depth from the light's perspective
	DrawLightDepth(imvp);

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

void OSShadowCreator::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Texture Size", mTextureSize);
	node.AddChild("Blur Passes", mBlurPasses);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSShadowCreator::OnSerializeFrom (const TreeNode& node)
{
	if		(node.mTag == "Texture Size")	node.mValue >> mTextureSize;
	else if (node.mTag == "Blur Passes")	node.mValue >> mBlurPasses;
}