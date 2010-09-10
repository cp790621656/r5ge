#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

OSDrawDeferred::OSDrawDeferred() :
	mBloom(1.0f),
	mAOQuality(0),
	mAOPasses(2),
	mCombine(0),
	mDepth(0),
	mNormal(0),
	mMatDiff(0),
	mMatParams(0),
	mLightDiff(0),
	mLightSpec(0),
	mFinal(0),
	mMaterialTarget(0),
	mProjTarget(0),
	mLightTarget(0),
	mFinalTarget(0),
	mProjTechnique(0),
	mProjTargetActive(false) {}

//============================================================================================================
// Initialize the scene
//============================================================================================================

void OSDrawDeferred::OnInit()
{
	OSDraw::OnInit();

	if (mCam != 0)
	{
		mScene.SetOnDraw(bind(&OSDrawDeferred::OnDrawWithTechnique, this));

		// Initial AO settings
		mAOParams.Set(1.0f, 4.0f, 1.0f);

		// Projected textures will need to be handled separately 
		mProjTechnique = mGraphics->GetTechnique("Projected Texture");

		// Default deferred draw techniques
		mDeferred.Expand() = mGraphics->GetTechnique("Deferred");
		mDeferred.Expand() = mGraphics->GetTechnique("Decal");
		mDeferred.Expand() = mProjTechnique;

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
	Vector2i size (mScene.GetFinalTargetSize());

	// Set up the material render target
	if (mMaterialTarget == 0)
	{
		const uint HDRFormat = (mBackground.a == 1.0f) ? ITexture::Format::RGB16F : ITexture::Format::RGBA16F;

		mMaterialTarget = mScene.GetRenderTarget(0);
		mDepth			= mScene.GetRenderTexture(0);
		mNormal			= mScene.GetRenderTexture(1);
		mMatDiff		= mScene.GetRenderTexture(2);
		mMatParams		= mScene.GetRenderTexture(3);

		// Set up the material render target
		mMaterialTarget->AttachDepthTexture(mDepth);
		mMaterialTarget->AttachStencilTexture(mDepth);
		mMaterialTarget->AttachColorTexture(0, mMatDiff, HDRFormat);
		mMaterialTarget->AttachColorTexture(1, mMatParams, ITexture::Format::RGBA);
		mMaterialTarget->AttachColorTexture(2, mNormal, ITexture::Format::RGBA16F);
	}

	// Update changing target properties
	mMaterialTarget->SetSize(size);
	mMaterialTarget->UseSkybox(false);

	// Set up the graphics states and clear the render target
	mGraphics->SetCulling(IGraphics::Culling::Back);
	mGraphics->SetActiveDepthFunction(IGraphics::Condition::Less);
	mGraphics->SetStencilTest(false);
	mGraphics->SetActiveRenderTarget(mMaterialTarget);
	mGraphics->SetBackgroundColor(mBackground);
	mGraphics->SetFogRange(mFogRange);
	mGraphics->Clear(true, true, true);

	// Set up the stencil test
	mGraphics->SetStencilTest(true);
	mGraphics->SetActiveStencilFunction (IGraphics::Condition::Always, 0x1, 0x1);
	mGraphics->SetActiveStencilOperation(IGraphics::Operation::Keep,
										 IGraphics::Operation::Keep,
										 IGraphics::Operation::Replace);
	// Draw the scene
	mScene.SetDepth(mDepth);
	mScene.SetNormal(mNormal);
	mScene.SetAO(mMatParams);
	mScene.DrawWithTechniques(mDeferred, false, false);
}

//============================================================================================================
// Light encoding stage
//============================================================================================================

void OSDrawDeferred::LightStage()
{
	// If we have no lights to draw, just exit
	const DrawQueue::Lights& lights = mScene.GetVisibleLights();
	if (lights.IsEmpty()) return;

	// Create AO
	if (mAOQuality > 0)
	{
		mScene.SetAO(mSSAO.Create(mScene, (mAOQuality > 1),
			mAOPasses, mAOParams.x, mAOParams.y, mAOParams.z));
	}

	// Disable all active lights except the first
	for (uint b = lights.GetSize(); b > 1; ) mGraphics->SetActiveLight(--b, 0);

	// Create the light target
	if (mLightTarget == 0)
	{
		mLightTarget = mScene.GetRenderTarget (1);
		mLightDiff	 = mScene.GetRenderTexture(4);
		mLightSpec	 = mScene.GetRenderTexture(5);

		// Additive blending doesn't write to the alpha channel
		uint format	= mMatDiff->GetFormat() & (~ITexture::Format::Alpha);

		mLightTarget->AttachDepthTexture(mDepth);
		mLightTarget->AttachStencilTexture(mDepth);
		mLightTarget->AttachColorTexture(0, mLightDiff, format);
		mLightTarget->AttachColorTexture(1, mLightSpec, format);
		mLightTarget->UseSkybox(false);
	}

	// Clear the render target
	mLightTarget->SetSize(mMaterialTarget->GetSize());

	// We'll need to keep track of the lights we've already processed
	mProcessedLight.ExpandTo(lights.GetSize());
	mProcessedLight.MemsetZero();
	uint offset (0);
	bool cleared (false);

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
				bool resetStates = false;

				// If we haven't chosen a new class ID, use this one
				if (classID == 0)
				{
					resetStates = true;
					offset = i + 1;
					classID = entry.mLight->GetClassID();
				}

				// If the class ID matches, draw this light
				if (classID == entry.mLight->GetClassID())
				{
					const ILight& light = lights[i].mLight->GetProperties();
					ITexture* shadow = 0;

					// If the light is supposed to be casting shadows, let's draw them
					if (light.mShadows)
					{
						if (entry.mLight->IsOfClass(DirectionalLight::ClassID()))
						{
							// Draw the shadow
							shadow = mShadow.Draw(mScene.GetRoot(), lights[i].mLight, light.mDir, mIMVP, mDepth);

							// Restore the matrices
							mScene.ActivateMatrices();
						}
					}

					// Update the shadow texture
					mScene.SetShadow(shadow);

					// If we just drew a shadow then we need to reset all graphics states back to what we need
					if (shadow != 0) resetStates = true;

					// Activate the light target
					mGraphics->SetActiveRenderTarget(mLightTarget);
					mGraphics->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));

					// Clear the target if it's the first time
					if (!cleared)
					{
						cleared = true;
						mGraphics->Clear(true, false, false);
					}

					// Reset all states if asked for
					if (resetStates)
					{
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

						// Set up the stencil buffer to allow rendering only where pixels are '1'
						mGraphics->SetActiveStencilFunction (IGraphics::Condition::Equal, 0x1, 0x1);
						mGraphics->SetActiveStencilOperation(IGraphics::Operation::Keep,
															 IGraphics::Operation::Keep,
															 IGraphics::Operation::Keep);
					}

					// Draw the light
					entry.mLight->OnDrawLight(mScene, resetStates);
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
	IRenderTarget* target = mScene.GetFinalTarget();

	if (mFinalTarget == 0)
	{
		mCombine	 = mGraphics->GetShader("[R5] Combine Deferred");
		mFinalTarget = mScene.GetRenderTarget(2);
		mFinal		 = mScene.GetRenderTexture(6);

		mFinalTarget->AttachDepthTexture(mDepth);
		mFinalTarget->AttachStencilTexture(mDepth);
		mFinalTarget->AttachColorTexture(0, mFinal, mMatDiff->GetFormat());
		mFinalTarget->UseSkybox(target == 0 || target->IsUsingSkybox());
	}

	mFinalTarget->SetSize(mMaterialTarget->GetSize());

	mGraphics->SetDepthWrite(false);
	mGraphics->SetDepthTest(false);
	mGraphics->SetStencilTest(false);
	mGraphics->SetFog(false);
	mGraphics->SetActiveRenderTarget(mFinalTarget);
	mGraphics->SetScreenProjection(false);
	mGraphics->SetBackgroundColor(mBackground);
	mGraphics->Clear(true, false, false);

	mGraphics->SetFogRange(mFogRange);
	mGraphics->SetCulling(IGraphics::Culling::Back);
	mGraphics->SetBlending(IGraphics::Blending::Replace);
	mGraphics->SetScreenProjection(true);
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveShader(mCombine);
	mGraphics->SetActiveTexture(0, mDepth);
	mGraphics->SetActiveTexture(1, mMatDiff);
	mGraphics->SetActiveTexture(2, mMatParams);
	mGraphics->SetActiveTexture(3, mLightDiff);
	mGraphics->SetActiveTexture(4, mLightSpec);
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

	// Update the fog color
	mGraphics->SetBackgroundColor(mBackground);

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

	// If we have something to draw, let's draw it
	if (mScene.HasSomethingToDraw())
	{
		// Clear the "final" texture references
		mScene.ClearFinalTextures();

		// Save the inverse MVP for shadows
		mIMVP = mGraphics->GetInverseMVPMatrix();

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
		mGraphics->SetBackgroundColor(mBackground);
		mGraphics->Clear();
	}
}

//============================================================================================================
// Callback triggered before the technique is activated and objects get drawn
//============================================================================================================

void OSDrawDeferred::OnDrawWithTechnique (const ITechnique* tech)
{
	if (tech == mProjTechnique)
	{
		// Activate the projected target if it's not yet active
		if (!mProjTargetActive && mMatDiff != 0)
		{
			mProjTargetActive = true;

			// Create the simplified forward rendering target
			if (mProjTarget == 0)
			{
				mProjTarget = mScene.GetRenderTarget(14);
				mProjTarget->AttachDepthTexture(mDepth);
				mProjTarget->AttachStencilTexture(mDepth);
				mProjTarget->AttachColorTexture(0, mMatDiff, mMatDiff->GetFormat());
			}

			// Activate the simplified forward rendering target
			mProjTarget->SetSize(mMaterialTarget->GetSize());
			mGraphics->SetActiveRenderTarget(mProjTarget);
		}
	}
	else if (mProjTargetActive)
	{
		// Turn off the projected target
		mProjTargetActive = false;
		mGraphics->SetActiveRenderTarget(mMaterialTarget);
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