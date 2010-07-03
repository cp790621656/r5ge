#include "../Include/_All.h"
using namespace R5;

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
// Constructor just clears the pointers
//============================================================================================================

PostProcess::PostProcess() :
	mGraphics(0),
	mPostProcess(0),
	mBlurH(0),
	mBlurV(0),
	mDOF(0),
	mCombine(0),
	mBloomBlur(0),
	mTarget00(0),
	mTarget01(0),
	mTarget10(0),
	mTarget11(0),
	mTarget20(0),
	mTarget21(0),
	mTexture00(0),
	mTexture01(0),
	mTexture10(0),
	mTexture11(0),
	mTexture20(0),
	mTexture21(0) {}

//============================================================================================================
// Initialize local resources
//============================================================================================================

void PostProcess::Initialize (IGraphics* graphics)
{
	mGraphics		= graphics;
	mPostProcess	= mGraphics->GetTechnique("Post Process");
	mBlurH			= mGraphics->GetShader("[R5] Horizontal Blur");
	mBlurV			= mGraphics->GetShader("[R5] Vertical Blur");
	mDOF			= mGraphics->GetShader("[R5] Depth of Field");
	mCombine		= mGraphics->GetShader("[R5] Combine Bloom");
	mBloomBlur		= mGraphics->GetShader("[R5] Bloom Blur");

	mDOF->RegisterUniform("focusRange", &SetFocusRange);
	mBloomBlur->RegisterUniform("threshold", &SetThreshold);
}

//============================================================================================================
// No post-processing
//============================================================================================================

void PostProcess::None (TemporaryStorage& storage)
{
	mGraphics->SetActiveRenderTarget(storage.GetFinalTarget());
	mGraphics->SetScreenProjection(true);
	mGraphics->SetActiveTechnique(mPostProcess);
	mGraphics->SetActiveMaterial(0);
	mGraphics->SetActiveTexture(0, storage.GetColor());
	mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Blurring effect -- Half size
//============================================================================================================
// The reason for these functions is being able to store appropriate render targets and textures as static
//============================================================================================================

void PostProcess::BlurDownsample (
	TemporaryStorage&	storage,
	const ITexture*		depth,
	const IShader*		replacement,
	const IShader*		postProcess)
{
	ITexture* color = storage.GetColor();

	if (mTarget00 == 0)
	{
		mTarget00	= storage.GetRenderTarget(3);
		mTarget01	= storage.GetRenderTarget(4);
		mTarget10	= storage.GetRenderTarget(5);
		mTarget11	= storage.GetRenderTarget(6);
		mTarget20	= storage.GetRenderTarget(7);
		mTarget21	= storage.GetRenderTarget(8);

		mTexture00	= storage.GetRenderTexture(7);
		mTexture01	= storage.GetRenderTexture(8);
		mTexture10	= storage.GetRenderTexture(9);
		mTexture11	= storage.GetRenderTexture(10);
		mTexture20	= storage.GetRenderTexture(11);
		mTexture21	= storage.GetRenderTexture(12);

		uint format = color->GetFormat();

		mTarget00->AttachColorTexture(0, mTexture00, format);
		mTarget01->AttachColorTexture(0, mTexture01, format);
		mTarget10->AttachColorTexture(0, mTexture10, format);
		mTarget11->AttachColorTexture(0, mTexture11, format);
		mTarget20->AttachColorTexture(0, mTexture20, format);
		mTarget21->AttachColorTexture(0, mTexture21, format);
	}

	// Always resize targets to match the render target
	Vector2i target (storage.GetFinalTargetSize());
	Vector2i half	(target  / 2);
	Vector2i quarter(half	 / 2);
	Vector2i eighth (quarter / 2);

	// Activate the proper states
	mGraphics->SetActiveTechnique(mPostProcess);
	mGraphics->SetActiveMaterial(0);

	// First blur pass (half size)
	{
		mTarget00->SetSize(half);
		mTarget01->SetSize(half);

		mTexture00->SetFiltering( ITexture::Filter::Linear );
		mTexture01->SetFiltering( ITexture::Filter::Linear );

		mGraphics->SetActiveRenderTarget(mTarget00);
		mGraphics->SetScreenProjection(true);
		mGraphics->SetActiveTexture(0, color);
		mGraphics->SetActiveShader((replacement != 0) ? replacement : mBlurH);
		mGraphics->Draw(IGraphics::Drawable::InvertedQuad);

		mGraphics->SetActiveRenderTarget(mTarget01);
		mGraphics->SetActiveTexture(0, mTexture00);
		mGraphics->SetActiveShader(mBlurV);
		mGraphics->Draw(IGraphics::Drawable::InvertedQuad);
	}

	// Second blur pass (quarter size)
	{
		mTarget10->SetSize(quarter);
		mTarget11->SetSize(quarter);

		mTexture10->SetFiltering( ITexture::Filter::Linear );
		mTexture11->SetFiltering( ITexture::Filter::Linear );

		mGraphics->SetActiveRenderTarget(mTarget10);
		mGraphics->SetScreenProjection(true);
		mGraphics->SetActiveTexture(0, mTexture01);
		mGraphics->SetActiveShader(mBlurH);
		mGraphics->Draw(IGraphics::Drawable::InvertedQuad);

		mGraphics->SetActiveRenderTarget(mTarget11);
		mGraphics->SetActiveTexture(0, mTexture10);
		mGraphics->SetActiveShader(mBlurV);
		mGraphics->Draw(IGraphics::Drawable::InvertedQuad);
	}

	// Third blur pass (1/8th of the original texture)
	{
		mTarget20->SetSize(eighth);
		mTarget21->SetSize(eighth);

		mTexture20->SetFiltering( ITexture::Filter::Linear );
		mTexture21->SetFiltering( ITexture::Filter::Linear );

		mGraphics->SetActiveRenderTarget(mTarget20);
		mGraphics->SetScreenProjection(true);
		mGraphics->SetActiveTexture(0, mTexture11);
		mGraphics->SetActiveShader(mBlurH);
		mGraphics->Draw(IGraphics::Drawable::InvertedQuad);

		mGraphics->SetActiveRenderTarget(mTarget21);
		mGraphics->SetActiveTexture(0, mTexture20);
		mGraphics->SetActiveShader(mBlurV);
		mGraphics->Draw(IGraphics::Drawable::InvertedQuad);
	}

	// Draw to the final render target
	mGraphics->SetActiveRenderTarget(storage.GetFinalTarget());
	mGraphics->SetScreenProjection(true);

	// Clear the target
	//if (target != 0) mGraphics->Clear(true, false, false);

	// Activate all textures
	mGraphics->SetActiveTexture(0, color);
	mGraphics->SetActiveTexture(1, (depth != 0) ? depth : mTexture01);
	mGraphics->SetActiveTexture(2, mTexture11);
	mGraphics->SetActiveTexture(3, mTexture21);

	// Use the pre-process shader to combine all textures
	mGraphics->SetActiveShader( postProcess );
	mGraphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Bloom post-processing effect
//============================================================================================================

void PostProcess::Bloom (TemporaryStorage& storage, float threshold)
{
	uint format = storage.GetColor()->GetFormat();

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		g_threshold = threshold;
		BlurDownsample(storage, 0, mBloomBlur, mCombine);
	}
	else
	{
		None(storage);
	}
}

//============================================================================================================
// Depth of Field effect
//============================================================================================================

void PostProcess::DepthOfField (TemporaryStorage& storage, const Vector3f& focalRange)
{
	g_focus.x = focalRange.x;
	g_focus.y = focalRange.y;
	g_focus.z = Interpolation::Linear(focalRange.y, focalRange.z, 0.35f);
	g_focus.w = focalRange.z;

	BlurDownsample(storage, storage.GetDepth(), 0, mDOF);
}

//============================================================================================================
// Both depth-of-field as well as bloom effects
//============================================================================================================

void PostProcess::Both (TemporaryStorage& storage, float threshold, const Vector3f&	focalRange)
{
	uint format = storage.GetColor()->GetFormat();
	if (format == ITexture::Format::Invalid) format = ITexture::Format::RGB16F;

	// Only apply bloom if the format is HDR or the threshold has been set below 1
	if ((format & ITexture::Format::HDR) != 0 || threshold < 0.999f)
	{
		// We want to reuse the specular light texture as it's no longer needed at this point, and it happens
		// to be the least useful texture in general. Reusing it saves the need to create another one.

		ITexture*		intrimColor = storage.GetRenderTexture(5);
		IRenderTarget*	target		= storage.GetRenderTarget(13);

		if (target->GetColorTexture(0) == 0)
		{
			target->AttachColorTexture(0, intrimColor, format);
			target->UseSkybox(false);
		}

		// Save the final render target
		IRenderTarget* previous = storage.GetFinalTarget();

		// The target's size should match the color texture's size
		target->SetSize(storage.GetColor()->GetSize());

		// Replace the render target
		storage.SetFinalTarget(target);

		// Apply depth of field first
		DepthOfField(storage, focalRange);

		// Replace the final color texture
		storage.SetColor(intrimColor);

		// Restore the render target
		storage.SetFinalTarget(previous);

		// Bloom everything
		Bloom(storage, threshold);

		// Restore the color texture
		storage.SetColor((previous == 0) ? 0 : (ITexture*)previous->GetColorTexture(0));
	}
	else
	{
		// Only apply depth of field
		DepthOfField(storage, focalRange);
	}
}