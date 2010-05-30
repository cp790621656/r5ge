#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Draw the scene using the deferred rendering techniques
//============================================================================================================

uint Deferred::Draw (IGraphics* graphics, Deferred::Storage& storage, const Light::List& lights)
{
	storage.mTempTargets.ExpandTo(32, true);
	storage.mTempTextures.ExpandTo(32, true);

	typedef ITexture*		ITexturePtr;
	typedef IRenderTarget*	IRenderTargetPtr;

	ITexturePtr& normal		= storage.mTempTextures[0];
	ITexturePtr& depth		= storage.mTempTextures[1];
	ITexturePtr& matDiff	= storage.mTempTextures[2];
	ITexturePtr& matSpec	= storage.mTempTextures[3];
	ITexturePtr& lightDiff	= storage.mTempTextures[4];
	ITexturePtr& lightSpec	= storage.mTempTextures[5];
	ITexturePtr& final		= storage.mTempTextures[6];

	IRenderTargetPtr& diffuseTarget = storage.mTempTargets[0];
	IRenderTargetPtr& lightTarget	= storage.mTempTargets[1];
	IRenderTargetPtr& finalTarget	= storage.mTempTargets[2];

	if (normal == 0)
	{
		if (storage.mRenderTarget == 0)
		{
			// Rendering to screen should reuse the same textures
			normal		= graphics->GetTexture("[Generated] Normal");
			depth		= graphics->GetTexture("[Generated] Depth");
			matDiff		= graphics->GetTexture("[Generated] Diffuse Material");
			matSpec		= graphics->GetTexture("[Generated] Specular Material");
			lightDiff	= graphics->GetTexture("[Generated] Light Diffuse");
			lightSpec	= graphics->GetTexture("[Generated] Light Specular");
			final		= graphics->GetTexture("[Generated] Final");
		}
		else
		{
			// Rendering to an off-screen buffer should use temporary textures
			normal		= graphics->CreateRenderTexture();
			depth		= graphics->CreateRenderTexture();
			matDiff		= graphics->CreateRenderTexture();
			matSpec		= graphics->CreateRenderTexture();
			lightDiff	= graphics->CreateRenderTexture();
			lightSpec	= graphics->CreateRenderTexture();
			final		= graphics->CreateRenderTexture();
		}
	}

	// Use the specified size if possible, viewport size otherwise
	Vector2i size (storage.mRenderTarget == 0 ? graphics->GetActiveViewport() :
		storage.mRenderTarget->GetSize());

	// Background color
	Color4f color (storage.mRenderTarget == 0 ? graphics->GetBackgroundColor() :
		storage.mRenderTarget->GetBackgroundColor());

	// Made constant so it can be quickly changed for testing purposes
	const uint HDRFormat = (color.a == 1.0f) ? ITexture::Format::RGB16F : ITexture::Format::RGBA16F;
	uint count (0);

	// Deferred rendering target
	{
		if (diffuseTarget == 0)
		{
			diffuseTarget = graphics->CreateRenderTarget();
			diffuseTarget->AttachDepthTexture(depth);
			diffuseTarget->AttachStencilTexture(depth);
			diffuseTarget->AttachColorTexture(0, matDiff, HDRFormat);
			diffuseTarget->AttachColorTexture(1, matSpec, ITexture::Format::RGBA);
			diffuseTarget->AttachColorTexture(2, normal,  ITexture::Format::RGBA);
			diffuseTarget->UseSkybox(storage.mRenderTarget == 0 || storage.mRenderTarget->IsUsingSkybox());
		}

		// Setting size only changes it if it's different
		diffuseTarget->SetSize(size);
		diffuseTarget->SetBackgroundColor(color);

		// Deferred rendering -- encoding pass
		if (storage.mDrawCallback && storage.mDrawTechniques.IsValid())
		{
			graphics->SetCulling(IGraphics::Culling::Back);
			graphics->SetActiveDepthFunction(IGraphics::Condition::Less);

			graphics->SetStencilTest(false);
			graphics->SetActiveRenderTarget(diffuseTarget);
			graphics->Clear(true, true, true);

			// Set up the stencil test
			graphics->SetStencilTest(true);
			graphics->SetActiveStencilFunction ( IGraphics::Condition::Always, 0x1, 0x1 );
			graphics->SetActiveStencilOperation( IGraphics::Operation::Keep,
												 IGraphics::Operation::Keep,
												 IGraphics::Operation::Replace );

			// Draw the scene using the deferred approach
			count += storage.mDrawCallback(storage.mDrawTechniques, storage.mInsideOut);
		}

		// Update the textures for the SSAO functions below
		storage.mOutColor	 = 0;
		storage.mOutDepth	 = depth;
		storage.mOutNormal	 = normal;
		storage.mOutLightmap = 0;

		// Screen-space ambient occlusion pass
		if (storage.mAOLevel > 0)
		{
			graphics->SetStencilTest(true);
			graphics->SetActiveStencilFunction ( IGraphics::Condition::Equal, 0x1, 0x1 );
			graphics->SetActiveStencilOperation( IGraphics::Operation::Keep,
												 IGraphics::Operation::Keep,
												 IGraphics::Operation::Keep );
			if (storage.mAOLevel == 1)
			{
				storage.mOutLightmap = SSAO::Low(graphics, storage);
			}
			else
			{
				storage.mOutLightmap = SSAO::High(graphics, storage);
			}
		}
	}

	// Light contribution
	{
		// Scene Light contribution lightTarget
		if (lightTarget == 0)
		{
			lightTarget = graphics->CreateRenderTarget();
			lightTarget->AttachDepthTexture(depth);
			lightTarget->AttachStencilTexture(depth);
			lightTarget->AttachColorTexture(0, lightDiff, HDRFormat);
			lightTarget->AttachColorTexture(1, lightSpec, HDRFormat);
			lightTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
			lightTarget->UseSkybox(false);
		}
		
		lightTarget->SetSize(size);
		graphics->SetActiveRenderTarget(lightTarget);
		graphics->Clear(true, false, false);

		Light::Draw(graphics, storage.mOutDepth, storage.mOutNormal, storage.mOutLightmap, lights);
		count += lights.GetSize();
	}

	// Combine the light contribution with material
	{
		// Final color finalTarget
		if (finalTarget == 0)
		{
			finalTarget = graphics->CreateRenderTarget();
			finalTarget->AttachDepthTexture(depth);
			finalTarget->AttachStencilTexture(depth);
			finalTarget->AttachColorTexture(0, final, HDRFormat);
			finalTarget->SetBackgroundColor(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
			finalTarget->UseSkybox(false);
		}

		static IShader* shader = graphics->GetShader("[R5] Combine Deferred");

		finalTarget->SetSize(size);
		graphics->SetActiveRenderTarget(finalTarget);

		graphics->SetDepthWrite(false);
		graphics->SetDepthTest(false);
		graphics->SetStencilTest(false);
		graphics->SetBlending(IGraphics::Blending::None);
		graphics->SetCulling(IGraphics::Culling::Back);

		graphics->SetScreenProjection( true );
		graphics->SetActiveMaterial(0);
		graphics->SetActiveShader(shader);
		graphics->SetActiveTexture( 0, matDiff );
		graphics->SetActiveTexture( 1, matSpec );
		graphics->SetActiveTexture( 2, lightDiff );
		graphics->SetActiveTexture( 3, lightSpec );
		graphics->Draw( IGraphics::Drawable::InvertedQuad );

		// 'final' texture now contains our final color
		storage.mOutColor = final;
	}
	return count;
}