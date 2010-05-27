#include "../Include/_All.h"
using namespace R5;

typedef ITexture*			ITexturePtr;
typedef IRenderTarget*		IRenderTargetPtr;

//============================================================================================================
// Depth of field shader callback function
//============================================================================================================

Quaternion g_focus;

void SetFocusRange (const String& name, Uniform& uniform) { uniform = g_focus; }

//============================================================================================================
// Bloom power callback function
//============================================================================================================

float g_threshold = 0.75f;

void SetThreshold (const String& name, Uniform& uniform) { uniform = g_threshold; }

//============================================================================================================
// Blur sharpness callback function
//============================================================================================================

float g_sharpness = 1.0f;

void SetSharpness (const String& name, Uniform& uniform) { uniform = g_sharpness; }

//============================================================================================================
// Blurring effect -- common part
//============================================================================================================

void BlurCommon ( IGraphics*		graphics,
				  IRenderTarget*	target0,
				  IRenderTarget*	target1,
				  const ITexture*	texture0,
				  const ITexture*	texture1,
				  const IShader*	shader0,
				  const IShader*	shader1 )
{
	graphics->SetActiveRenderTarget( target0 );
	graphics->SetScreenProjection( true );
	graphics->SetActiveTexture( 0, texture0 );
	graphics->SetActiveShader( shader0 );
	graphics->Draw( IGraphics::Drawable::InvertedQuad );

	graphics->SetActiveRenderTarget( target1 );
	graphics->SetActiveTexture( 0, texture1 );
	graphics->SetActiveShader( shader1 );
	graphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Blurring effect -- Half size
//============================================================================================================
// The reason for these functions is being able to store appropriate render targets and textures as static
//============================================================================================================

void BlurDownsample (IGraphics*			graphics,
					 Deferred::Storage&	storage,
					 const ITexture*	color,
					 const ITexture*	depth,
					 const IShader*		replacement,
					 const IShader*		postprocess)
{
	static const ITechnique* technique  = graphics->GetTechnique("Post Process");
	static IShader*			blurH		= graphics->GetShader("[R5] Horizontal Blur");
	static IShader*			blurV		= graphics->GetShader("[R5] Vertical Blur");

	storage.mTempTargets.ExpandTo(32, true);
	storage.mTempTextures.ExpandTo(32, true);

	IRenderTargetPtr&	target00	= storage.mTempTargets[3];
	IRenderTargetPtr&	target01	= storage.mTempTargets[4];
	IRenderTargetPtr&	target10	= storage.mTempTargets[5];
	IRenderTargetPtr&	target11	= storage.mTempTargets[6];
	IRenderTargetPtr&	target20	= storage.mTempTargets[7];
	IRenderTargetPtr&	target21	= storage.mTempTargets[8];
	ITexturePtr&		texture00	= storage.mTempTextures[7];
	ITexturePtr&		texture01	= storage.mTempTextures[8];
	ITexturePtr&		texture10	= storage.mTempTextures[9];
	ITexturePtr&		texture11	= storage.mTempTextures[10];
	ITexturePtr&		texture20	= storage.mTempTextures[11];
	ITexturePtr&		texture21	= storage.mTempTextures[12];

	// Initialize the common render targets and textures the first time this function is executed
	if (target00 == 0)
	{
		uint format = color->GetFormat();

		target00  = graphics->CreateRenderTarget();
		target01  = graphics->CreateRenderTarget();
		target10  = graphics->CreateRenderTarget();
		target11  = graphics->CreateRenderTarget();
		target20  = graphics->CreateRenderTarget();
		target21  = graphics->CreateRenderTarget();

		texture00 = graphics->CreateRenderTexture();
		texture01 = graphics->CreateRenderTexture();
		texture10 = graphics->CreateRenderTexture();
		texture11 = graphics->CreateRenderTexture();
		texture20 = graphics->CreateRenderTexture();
		texture21 = graphics->CreateRenderTexture();

		target00->AttachColorTexture(0, texture00, format);
		target01->AttachColorTexture(0, texture01, format);
		target10->AttachColorTexture(0, texture10, format);
		target11->AttachColorTexture(0, texture11, format);
		target20->AttachColorTexture(0, texture20, format);
		target21->AttachColorTexture(0, texture21, format);
	}

	// Always resize targets to match the render target
	Vector2i target (storage.mRenderTarget == 0 ? graphics->GetViewport() : storage.mRenderTarget->GetSize());
	Vector2i half	(target  / 2);
	Vector2i quarter(half	 / 2);
	Vector2i eighth (quarter / 2);

	// Target 0
	{
		target00->SetSize(half);
		target01->SetSize(half);

		texture00->SetFiltering( ITexture::Filter::Linear );
		texture01->SetFiltering( ITexture::Filter::Linear );
	}

	// Target 1
	{
		target10->SetSize(quarter);
		target11->SetSize(quarter);

		texture10->SetFiltering( ITexture::Filter::Linear );
		texture11->SetFiltering( ITexture::Filter::Linear );
	}

	// Target 2
	{
		target20->SetSize(eighth);
		target21->SetSize(eighth);

		texture20->SetFiltering( ITexture::Filter::Linear );
		texture21->SetFiltering( ITexture::Filter::Linear );
	}

	// Activate the proper states
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);

	// Run several blur passes
	BlurCommon(graphics, target00, target01, color, texture00, (replacement != 0) ? replacement : blurH, blurV);
	BlurCommon(graphics, target10, target11, texture01, texture10, blurH, blurV);
	BlurCommon(graphics, target20, target21, texture11, texture20, blurH, blurV);

	// Draw to the render target
	graphics->SetActiveRenderTarget(storage.mRenderTarget);
	graphics->SetScreenProjection( true );

	// Clear the target
	//if (target != 0) graphics->Clear(true, false, false);

	// Activate all textures
	graphics->SetActiveTexture(0, color);
	graphics->SetActiveTexture(1, (depth != 0) ? depth : texture01);
	graphics->SetActiveTexture(2, texture11);
	graphics->SetActiveTexture(3, texture21);

	// Use the pre-process shader to combine all textures
	graphics->SetActiveShader( postprocess );
	graphics->Draw( IGraphics::Drawable::InvertedQuad );

	// Update the final color texture
	storage.mOutColor = (storage.mRenderTarget == 0) ? 0 : storage.mRenderTarget->GetColorTexture(0);
}

//============================================================================================================
// No post-processing
//============================================================================================================

void PostProcess::None (IGraphics* graphics, Deferred::Storage& storage)
{
	static const ITechnique* technique = graphics->GetTechnique("Post Process");

	graphics->SetActiveRenderTarget(storage.mRenderTarget);
	graphics->SetScreenProjection(true);
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);
	graphics->SetActiveTexture(0, storage.mOutColor);
	graphics->Draw( IGraphics::Drawable::InvertedQuad );

	// Update the final color texture
	storage.mOutColor = (storage.mRenderTarget == 0) ? 0 : storage.mRenderTarget->GetColorTexture(0);
}

//============================================================================================================
// Gaussian blur
//============================================================================================================

void PostProcess::Blur (IGraphics* graphics, Deferred::Storage& storage, float sharpness)
{
	static IShader* combineBlur = graphics->GetShader("[R5] Combine Blur");
	static IShader* combineMonoBlur = 0;

	g_sharpness = sharpness;

	if (combineMonoBlur == 0)
	{
		combineMonoBlur = graphics->GetShader("[R5] Combine Mono Blur");
		combineMonoBlur->RegisterUniform("sharpness", &SetSharpness);
	}

	BlurDownsample(graphics, storage, storage.mOutColor, 0, 0, (sharpness == 1.0f) ? combineBlur : combineMonoBlur);
}

//============================================================================================================
// Bloom post-processing effect
//============================================================================================================

void PostProcess::Bloom (IGraphics* graphics, Deferred::Storage& storage, float threshold)
{
	uint format = storage.mOutColor->GetFormat();

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		g_threshold = threshold;

		static IShader*	final = graphics->GetShader("[R5] Combine Bloom");
		static IShader*	replacement	= 0;

		if (replacement == 0)
		{
			replacement = graphics->GetShader("[R5] Bloom Blur");
			replacement->RegisterUniform("threshold", &SetThreshold);
		}

		BlurDownsample(graphics, storage, storage.mOutColor, 0, replacement, final);
	}
	else
	{
		PostProcess::None(graphics, storage);
	}
}

//============================================================================================================
// Depth of Field effect
//============================================================================================================

void PostProcess::DepthOfField (IGraphics*	graphics,
								Deferred::Storage&	storage,
								float		focalDistance,
								float		focalMin,
								float		focalMax)
{
	static IShader* dof	= 0;

	g_focus.x = focalDistance;
	g_focus.y = focalMin;
	g_focus.z = (focalMax + focalMin) * 0.5f;
	g_focus.w = focalMax;

	if (dof == 0)
	{
		dof = graphics->GetShader("[R5] Depth of Field");
		dof->RegisterUniform("focusRange", &SetFocusRange);
	}

	BlurDownsample(graphics, storage, storage.mOutColor, storage.mOutDepth, 0, dof);
}

//============================================================================================================
// Both depth-of-field as well as bloom effects
//============================================================================================================

void PostProcess::Both (IGraphics*	graphics,
						Deferred::Storage&	storage,
						float		threshold,
						float		focalDistance,
						float		focalMin,
						float		focalMax)
{
	uint format = storage.mOutColor->GetFormat();
	if (format == ITexture::Format::Invalid) format = ITexture::Format::RGB16F;

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		// We want to reuse the specular light texture as it's no longer needed at this point, and it happens
		// to be the least useful texture in general. Reusing it saves the need to create another one.
		ITexturePtr&		intrimColor = storage.mTempTextures.Get(5);
		IRenderTargetPtr&	target		= storage.mTempTargets.Get(13);

		if (target == 0)
		{
			target = graphics->CreateRenderTarget();
			target->AttachColorTexture(0, intrimColor, format);
			target->UseSkybox(false);
		}

		// The target's size should match the color texture's size
		target->SetSize(storage.mOutColor->GetSize());

		// Replace the render target
		IRenderTarget* final = storage.mRenderTarget;
		storage.mRenderTarget = target;

		// Apply depth of field first
		DepthOfField(graphics, storage, focalDistance, focalMin, focalMax);

		// Restore the render target
		storage.mRenderTarget = final;

		// Bloom everything
		Bloom(graphics, storage, threshold);
	}
	else
	{
		// Only apply depth of field
		DepthOfField(graphics, storage, focalDistance, focalMin, focalMax);
	}
}