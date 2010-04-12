#include "../Include/_All.h"
using namespace R5;

typedef ITexture*				ITexturePtr;
typedef IRenderTarget*			IRenderTargetPtr;
typedef Deferred::DrawParams	DrawParams;

//============================================================================================================
// Depth of field shader callback function
//============================================================================================================

Quaternion g_focus;

void SetFocusRange (const String& name, Uniform& uniform)
{
	uniform = g_focus;
}

//============================================================================================================
// Bloom power callback function
//============================================================================================================

float g_threshold = 0.75f;

void SetThreshold (const String& name, Uniform& uniform)
{
	uniform = g_threshold;
}

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
	graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
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
					 DrawParams&		params,
					 const ITexture*	color,
					 const ITexture*	depth,
					 const IShader*		replacement,
					 const IShader*		postprocess)
{
	static const ITechnique* technique  = graphics->GetTechnique("Post Process");
	static IShader*			blurH		= graphics->GetShader("[R5] Horizontal Blur");
	static IShader*			blurV		= graphics->GetShader("[R5] Vertical Blur");

	params.mTargets.ExpandTo(9, true);
	params.mTextures.ExpandTo(13, true);

	IRenderTargetPtr&	target00	= params.mTargets[3];
	IRenderTargetPtr&	target01	= params.mTargets[4];
	IRenderTargetPtr&	target10	= params.mTargets[5];
	IRenderTargetPtr&	target11	= params.mTargets[6];
	IRenderTargetPtr&	target20	= params.mTargets[7];
	IRenderTargetPtr&	target21	= params.mTargets[8];
	ITexturePtr&		texture00	= params.mTextures[7];
	ITexturePtr&		texture01	= params.mTextures[8];
	ITexturePtr&		texture10	= params.mTextures[9];
	ITexturePtr&		texture11	= params.mTextures[10];
	ITexturePtr&		texture20	= params.mTextures[11];
	ITexturePtr&		texture21	= params.mTextures[12];

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
	Vector2i target (params.mRenderTarget == 0 ? graphics->GetViewport() : params.mRenderTarget->GetSize());
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
	graphics->SetActiveRenderTarget(params.mRenderTarget);
	graphics->SetActiveProjection( IGraphics::Projection::Orthographic );

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
}

//============================================================================================================
// No post-processing
//============================================================================================================

void PostProcess::None (IGraphics* graphics, DrawParams& params, const ITexture* color)
{
	static const ITechnique* technique = graphics->GetTechnique("Post Process");

	graphics->SetActiveRenderTarget(params.mRenderTarget);
	graphics->SetActiveProjection(IGraphics::Projection::Orthographic);
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);
	graphics->SetActiveTexture(0, color);
	graphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Bloom post-processing effect
//============================================================================================================

void PostProcess::Bloom (IGraphics* graphics, DrawParams& params, const ITexture* color, float threshold)
{
	uint format = color->GetFormat();

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		g_threshold = threshold;

		static IShader*	final = 0;
		static IShader*	replacement	= 0;

		if (final == 0)
		{
			final = graphics->GetShader("[R5] Bloom/Apply");
			replacement = graphics->GetShader("[R5] Bloom/Blur");
			replacement->RegisterUniform("threshold", &SetThreshold);
		}

		BlurDownsample(graphics, params, color, 0, replacement, final);
	}
	else
	{
		PostProcess::None(graphics, params, color);
	}
}

//============================================================================================================
// Depth of Field effect
//============================================================================================================

void PostProcess::DepthOfField (IGraphics*			graphics,
								DrawParams&			params,
								const ITexture*		color,
								const ITexture*		depth,
								float				focalDistance,
								float				focalMin,
								float				focalMax)
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

	BlurDownsample(graphics, params, color, depth, 0, dof);
}

//============================================================================================================
// Both depth-of-field as well as bloom effects
//============================================================================================================

void PostProcess::Both (IGraphics*			graphics,
						DrawParams&			params,
						const ITexture*		color,
						const ITexture*		depth,
						float				threshold,
						float				focalDistance,
						float				focalMin,
						float				focalMax)
{
	uint format = color->GetFormat();
	if (format == ITexture::Format::Invalid) format = ITexture::Format::RGB16F;

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		// We want to reuse the specular light texture as it's no longer needed at this point, and it happens
		// to be the least useful texture in general. Reusing it saves the need to create another one.
		ITexturePtr&		intrimColor = params.mTextures.Get(5);
		IRenderTargetPtr&	target		= params.mTargets.Get(13);

		if (target == 0)
		{
			target = graphics->CreateRenderTarget();
			target->AttachColorTexture(0, intrimColor, format);
			target->UseSkybox(false);
		}

		// The target's size should match the color texture's size
		target->SetSize(color->GetSize());

		// Replace
		IRenderTarget* saved = params.mRenderTarget;
		params.mRenderTarget = target;

		// Apply depth of field first
		DepthOfField(graphics, params, color, depth, focalDistance, focalMin, focalMax);

		// Restore
		params.mRenderTarget = saved;

		// Bloom follows afterwards
		Bloom(graphics, params, intrimColor, threshold);
	}
	else
	{
		// Only apply depth of field
		DepthOfField(graphics, params, color, depth, focalDistance, focalMin, focalMax);
	}
}