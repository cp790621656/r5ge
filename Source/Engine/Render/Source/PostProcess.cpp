#include "../Include/_All.h"
using namespace R5;

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
					 const ITexture*	color,
					 const ITexture*	depth,
					 const IShader*		replacement,
					 const IShader*		postprocess,
					 IRenderTarget*		target)
{
	static const ITechnique* technique  = graphics->GetTechnique("Post Process");
	static IShader*			blurH		= graphics->GetShader("[R5] Horizontal Blur");
	static IShader*			blurV		= graphics->GetShader("[R5] Vertical Blur");
	static IRenderTarget*	target00	= 0;
	static IRenderTarget*	target01	= 0;
	static IRenderTarget*	target10	= 0;
	static IRenderTarget*	target11	= 0;
	static IRenderTarget*	target20	= 0;
	static IRenderTarget*	target21	= 0;
	static ITexture*		texture00	= 0;
	static ITexture*		texture01	= 0;
	static ITexture*		texture10	= 0;
	static ITexture*		texture11	= 0;
	static ITexture*		texture20	= 0;
	static ITexture*		texture21	= 0;

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

		texture00 = graphics->GetTexture("[Generated] Half 0");
		texture01 = graphics->GetTexture("[Generated] Half 1");
		texture10 = graphics->GetTexture("[Generated] Quarter 0");
		texture11 = graphics->GetTexture("[Generated] Quarter 1");
		texture20 = graphics->GetTexture("[Generated] Eighth 0");
		texture21 = graphics->GetTexture("[Generated] Eighth 1");

		target00->AttachColorTexture(0, texture00, format);
		target01->AttachColorTexture(0, texture01, format);
		target10->AttachColorTexture(0, texture10, format);
		target11->AttachColorTexture(0, texture11, format);
		target20->AttachColorTexture(0, texture20, format);
		target21->AttachColorTexture(0, texture21, format);
	}

	// Always resize targets to match the viewport
	Vector2i half (graphics->GetViewport() / 2);
	Vector2i quarter (half / 2);
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
	graphics->SetActiveRenderTarget(target);
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

void PostProcess::None (IGraphics* graphics, const ITexture* color)
{
	static const ITechnique* technique = graphics->GetTechnique("Post Process");

	graphics->SetActiveRenderTarget(0);
	graphics->SetActiveProjection(IGraphics::Projection::Orthographic);
	graphics->SetActiveTechnique(technique);
	graphics->SetActiveMaterial(0);
	graphics->SetActiveTexture(0, color);
	graphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Bloom post-processing effect
//============================================================================================================

void PostProcess::Bloom (IGraphics* graphics, const ITexture* color, float threshold, IRenderTarget* target)
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

		BlurDownsample (graphics, color, 0, replacement, final, target);
	}
}

//============================================================================================================
// Depth of Field effect
//============================================================================================================

void PostProcess::DepthOfField (IGraphics*		graphics,
								const ITexture* color,
								const ITexture* depth,
								float			focalDistance,
								float			focalMin,
								float			focalMax,
								IRenderTarget*	target)
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

	BlurDownsample (graphics, color, depth, 0, dof, target);
}

//============================================================================================================
// Both depth-of-field as well as bloom effects
//============================================================================================================

void PostProcess::Both (IGraphics*		graphics,
						const ITexture*	color,
						const ITexture*	depth,
						float			threshold,
						float			focalDistance,
						float			focalMin,
						float			focalMax)
{
	uint format = color->GetFormat();
	if (format == ITexture::Format::Invalid) format = ITexture::Format::RGB16F;

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		// We want to reuse the specular light texture as it's no longer needed at this point, and it happens
		// to be the least useful texture in general. Reusing it saves the need to create another one.
		static ITexture* intrimColor = graphics->GetTexture("[Generated] Specular Light");
		static IRenderTarget* target = 0;

		if (target == 0)
		{
			target = graphics->CreateRenderTarget();
			target->AttachColorTexture(0, intrimColor, format);
			target->UseSkybox(false);
		}

		// The target's size should match the color texture's size
		target->SetSize(color->GetSize());

		// Apply depth of field first
		DepthOfField(graphics, color, depth, focalDistance, focalMin, focalMax, target);

		// Bloom follows afterwards
		Bloom(graphics, intrimColor, threshold, 0);
	}
	else
	{
		// Only apply depth of field
		DepthOfField(graphics, color, depth, focalDistance, focalMin, focalMax, 0);
	}
}