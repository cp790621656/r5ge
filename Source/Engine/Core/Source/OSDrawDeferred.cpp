#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

OSDrawDeferred::OSDrawDeferred() :
	mCam(0),
	mBloom(1.0f),
	mAOQuality(0),
	mAOPasses(2),
	mCombine(0),
	mDepth(0),
	mNormal(0),
	mMatDiff(0),
	mMatSpec(0),
	mLightDiff(0),
	mLightSpec(0),
	mFinal(0),
	mMaterialTarget(0),
	mLightTarget(0),
	mFinalTarget(0) {}

//============================================================================================================
// Initialize the scene
//============================================================================================================

void OSDrawDeferred::OnInit()
{
	mCam = R5_CAST(Camera, mObject);

	if (mCam == 0)
	{
		ASSERT(false, "You've attached OSDrawDeferred to something other than a camera, unable to proceed!");
		DestroySelf();
	}
	else
	{
		OSDraw::OnInit();
		mScene.Initialize(mGraphics);

		// Initial AO settings
		mAOParams.Set(1.0f, 4.0f, 1.0f);

		// Default deferred draw techniques
		mDeferred.Expand() = mGraphics->GetTechnique("Deferred");
		mDeferred.Expand() = mGraphics->GetTechnique("Decal");

		// Forward rendering techniques are missing 'opaque' when used with deferred rendering
		mForward.Expand() = mGraphics->GetTechnique("Wireframe");
		mForward.Expand() = mGraphics->GetTechnique("Transparent");
		mForward.Expand() = mGraphics->GetTechnique("Particle");
		mForward.Expand() = mGraphics->GetTechnique("Glow");
		mForward.Expand() = mGraphics->GetTechnique("Glare");

		// Post-processing techniques
		mPostProcess.Initialize(mGraphics);
	}
}

//============================================================================================================
// Material encoding stage
//============================================================================================================

void OSDrawDeferred::MaterialStage()
{
	// Render target's background color
	IRenderTarget* target = mScene.GetFinalTarget();
	Vector2i size (mScene.GetFinalTargetSize());
	Color4f color (target == 0 ? mGraphics->GetBackgroundColor() : target->GetBackgroundColor());

	// Set up the material render target
	if (mMaterialTarget == 0)
	{
		const uint HDRFormat = (color.a == 1.0f) ? ITexture::Format::RGB16F : ITexture::Format::RGBA16F;

		mMaterialTarget = mScene.GetRenderTarget(0);
		mDepth			= mScene.GetRenderTexture(0);
		mNormal			= mScene.GetRenderTexture(1);
		mMatDiff		= mScene.GetRenderTexture(2);
		mMatSpec		= mScene.GetRenderTexture(3);

		// Set up the material render target
		mMaterialTarget->AttachDepthTexture(mDepth);
		mMaterialTarget->AttachStencilTexture(mDepth);
		mMaterialTarget->AttachColorTexture(0, mMatDiff, HDRFormat);
		mMaterialTarget->AttachColorTexture(1, mMatSpec, ITexture::Format::RGBA);
		mMaterialTarget->AttachColorTexture(2, mNormal,  ITexture::Format::RGBA);
	}

	// Update changing target properties
	mMaterialTarget->SetSize(size);
	mMaterialTarget->SetBackgroundColor(color);
	mMaterialTarget->UseSkybox(target == 0 || target->IsUsingSkybox());

	// Set up the graphics states and clear the render target
	mGraphics->SetCulling(IGraphics::Culling::Back);
	mGraphics->SetActiveDepthFunction(IGraphics::Condition::Less);
	mGraphics->SetStencilTest(false);
	mGraphics->SetActiveRenderTarget(mMaterialTarget);
	mGraphics->Clear(true, true, true);

	// Set up the stencil test
	mGraphics->SetStencilTest(true);
	mGraphics->SetActiveStencilFunction (IGraphics::Condition::Always, 0x1, 0x1);
	mGraphics->SetActiveStencilOperation(IGraphics::Operation::Keep,
										 IGraphics::Operation::Keep,
										 IGraphics::Operation::Replace);

	// Draw the scene
	mScene.DrawWithTechniques(mDeferred, false, false);

	// Our depth and final buffers can now be used by future draw processes
	mScene.SetDepth(mDepth);
	mScene.SetNormal(mNormal);
}

//============================================================================================================
// Light encoding stage
//============================================================================================================

void OSDrawDeferred::LightStage()
{
	// Create AO
	mScene.SetAO( (mAOQuality > 0) ? mSSAO.Create(mScene, (mAOQuality > 1), mAOPasses,
		mAOParams.x, mAOParams.y, mAOParams.z) : 0 );

	// Create the light target
	if (mLightTarget == 0)
	{
		mLightTarget = mScene.GetRenderTarget (1);
		mLightDiff	 = mScene.GetRenderTexture(4);
		mLightSpec	 = mScene.GetRenderTexture(5);

		mLightTarget->AttachDepthTexture(mDepth);
		mLightTarget->AttachStencilTexture(mDepth);
		mLightTarget->AttachColorTexture(0, mLightDiff, mMatDiff->GetFormat());
		mLightTarget->AttachColorTexture(1, mLightSpec, mMatDiff->GetFormat());
		mLightTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
		mLightTarget->UseSkybox(false);
	}

	// Clear the render target
	mLightTarget->SetSize(mMaterialTarget->GetSize());
	mGraphics->SetActiveRenderTarget(mLightTarget);
	mGraphics->Clear(true, false, false);

	// If we have no lights to draw, just exit
	const DrawQueue::Lights& lights = mScene.GetVisibleLights();
	if (lights.IsEmpty()) return;

	// Disable all active lights except the first
	for (uint b = lights.GetSize(); b > 1; ) mGraphics->SetActiveLight(--b, 0);

	// Set up appropriate states
	mGraphics->SetFog(false);
	mGraphics->SetStencilTest(true);
	mGraphics->SetDepthWrite(false);
	mGraphics->SetColorWrite(true);
	mGraphics->SetAlphaTest(false);
	mGraphics->SetWireframe(false);
	mGraphics->SetLighting(IGraphics::Lighting::None);
	mGraphics->SetBlending(IGraphics::Blending::Add);

	// Disable active material and clear any active buffers
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,		0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,		0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex,	0 );
	mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight,	0 );

	// We are using 2 textures
	mGraphics->SetActiveTexture(0, mDepth);
	mGraphics->SetActiveTexture(1, mNormal);

	// Set up the stencil buffer to allow rendering only where pixels are '1'
	mGraphics->SetActiveStencilFunction (IGraphics::Condition::Equal, 0x1, 0x1);
	mGraphics->SetActiveStencilOperation(IGraphics::Operation::Keep,
										 IGraphics::Operation::Keep,
										 IGraphics::Operation::Keep);

	// We'll need to keep track of the lights we've already processed
	mProcessedLight.ExpandTo(lights.GetSize());
	mProcessedLight.MemsetZero();
	uint offset (0);

	// Keep looping until done
	for (;;)
	{
		const char* classID = 0;
		bool keepGoing = false;

		// Run through all lights
		for (uint i = offset, imax = lights.GetSize(); i < imax; ++i)
		{
			// Once we find a light we haven't processed...
			if (!mProcessedLight[i])
			{
				const DrawQueue::LightEntry& entry (lights[i]);
				bool first = false;

				// If we haven't chosen a new class ID, use this one
				if (classID == 0)
				{
					first = true;
					offset = i + 1;
					classID = entry.mLight->GetClassID();
				}

				// If the class ID matches, draw this light
				if (classID == entry.mLight->GetClassID())
				{
					entry.mLight->OnDrawLight(mScene, first);
					mProcessedLight[i] = true;
				}
				else
				{
					// We encountered a light type we have not yet processed
					keepGoing = true;
				}
			}
		}

		// If we're done, break out
		if (!keepGoing) break;
	}

	// Reset the matrices that may have been changed
	mGraphics->ResetModelViewMatrix();
}

//============================================================================================================
// Combine the light contribution with material
//============================================================================================================

void OSDrawDeferred::CombineStage()
{
	if (mFinalTarget == 0)
	{
		mCombine	 = mGraphics->GetShader("[R5] Combine Deferred");
		mFinalTarget = mScene.GetRenderTarget(2);
		mFinal		 = mScene.GetRenderTexture(6);

		mFinalTarget->AttachDepthTexture(mDepth);
		mFinalTarget->AttachStencilTexture(mDepth);
		mFinalTarget->AttachColorTexture(0, mFinal, mMatDiff->GetFormat());
		mFinalTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
		mFinalTarget->UseSkybox(false);
	}

	mFinalTarget->SetSize(mMaterialTarget->GetSize());

	mGraphics->SetDepthWrite(false);
	mGraphics->SetDepthTest(false);
	mGraphics->SetStencilTest(false);
	mGraphics->SetBlending(IGraphics::Blending::None);
	mGraphics->SetCulling(IGraphics::Culling::Back);
	mGraphics->SetActiveRenderTarget(mFinalTarget);
	mGraphics->SetScreenProjection( true );
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveShader(mCombine);
	mGraphics->SetActiveTexture( 0, mMatDiff );
	mGraphics->SetActiveTexture( 1, mMatSpec );
	mGraphics->SetActiveTexture( 2, mLightDiff );
	mGraphics->SetActiveTexture( 3, mLightSpec );
	mGraphics->Draw( IGraphics::Drawable::InvertedQuad );

	// Our final texture is the new color texture
	mScene.SetColor(mFinal);
}

//============================================================================================================
// Final stage -- add forward-rendered objects and post-process effects
//============================================================================================================

void OSDrawDeferred::PostProcessStage()
{
	// Add forward rendering
	mGraphics->ResetModelViewMatrix();
	mGraphics->SetScreenProjection(false);

	// Draw the grid if requested
	if (mGrid) mGraphics->Draw( IGraphics::Drawable::Grid );

	// Draw all objects with forward rendering techniques
	mScene.DrawWithTechniques(mForward, false, true);

	// Apply a post-processing effect
	if (mBloom != 0.0f && mFocalRange)	mPostProcess.Both(mScene, mBloom, mFocalRange);
	else if (mBloom != 0.0f)			mPostProcess.Bloom(mScene, mBloom);
	else if (mFocalRange)				mPostProcess.DepthOfField(mScene, mFocalRange);
	else								mPostProcess.None(mScene);
}

//============================================================================================================
// Draw callback
//============================================================================================================

void OSDrawDeferred::OnDraw()
{
	// Cull the scene with our camera
	mScene.Cull(mCam);

	// Clear the "final" texture references
	mScene.ClearFinalTextures();

	// If we have something to draw, let's draw it
	if (mScene.HasSomethingToDraw())
	{
		// First stage -- encode the entire scene's materials into the material render target
		MaterialStage();

		// Second stage -- encode all lights into the light render target
		LightStage();

		// Third stage -- combine the material with the light
		CombineStage();

		// Final stage -- add forward-rendered objects and post-process effects
		PostProcessStage();
	}
	else
	{
		mGraphics->Clear();
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSDrawDeferred::OnSerializeTo (TreeNode& root) const
{
	OSDraw::OnSerializeTo(root);
	root.AddChild("Bloom",			mBloom);
	root.AddChild("Focal Range",	mFocalRange);
	root.AddChild("AO Quality",		mAOQuality);
	root.AddChild("AO Blur Passes",	mAOPasses);
	root.AddChild("AO Parameters",	mAOParams);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSDrawDeferred::OnSerializeFrom (const TreeNode& node)
{
	if		(node.mTag == "Bloom")			node.mValue >> mBloom;
	else if (node.mTag == "Focal Range")	node.mValue >> mFocalRange;
	else if (node.mTag == "AO Quality")		node.mValue >> mAOQuality;
	else if (node.mTag == "AO Blur Passes")	node.mValue >> mAOPasses;
	else if (node.mTag == "AO Parameters")	node.mValue >> mAOParams;
	else OSDraw::OnSerializeFrom(node);
}