#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Callback triggered when updating the 'properties' uniform on the SSAO shader
//============================================================================================================

Vector2f g_params (1.0f, 4.0f);

void SetSSAOProperties (const String& name, Uniform& data) { data = g_params; }

//============================================================================================================
// Generates a random texture used by SSAO
//============================================================================================================

ITexture* GetRandomizedNormalmap (IGraphics* graphics)
{
	ITexture* random = graphics->GetTexture("[Generated] Random Offset");

	if (random->GetFormat() == ITexture::Format::Invalid)
	{
		uint width = 32;
		uint size = width * width;

		R5::Random rand (52346453);
		Array<byte> buffer;
		buffer.Reserve(size * 4);

		String testOut;

		for (uint i = 0; i < size; ++i)
		{
			Vector3f v (rand.GenerateRangeFloat(),
						rand.GenerateRangeFloat(),
						rand.GenerateRangeFloat());

			v.Normalize();
			float mag = rand.GenerateFloat();
			mag = 0.1f + mag * 0.9f;

			v *= 0.5f;
			v += 0.5f;

			buffer.Expand() = Float::ToRangeByte(v.x);
			buffer.Expand() = Float::ToRangeByte(v.y);
			buffer.Expand() = Float::ToRangeByte(v.z);
			buffer.Expand() = Float::ToRangeByte(mag);
		}

		random->Set(buffer, width, width, 1, ITexture::Format::RGBA, ITexture::Format::RGBA);
		random->SetFiltering(ITexture::Filter::Nearest);
		random->SetWrapMode(ITexture::WrapMode::Repeat);
	}
	return random;
}

//============================================================================================================
// Creates resources used by the SSAO process
//============================================================================================================

void SSAO::CreateResources (TemporaryStorage& storage)
{
	IGraphics* graphics = storage.GetGraphics();

	mPost	= graphics->GetTechnique("Post Process");
	mSSAO	= graphics->GetShader("[R5] Sample SSAO");
	mBlurH	= graphics->GetShader("[R5] Horizontal SSAO Blur");
	mBlurV	= graphics->GetShader("[R5] Vertical SSAO Blur");

	mRandom = GetRandomizedNormalmap(graphics);

	mSSAOTarget	 = storage.GetRenderTarget(10);
	mBlurTarget0 = storage.GetRenderTarget(11);
	mBlurTarget1 = storage.GetRenderTarget(12);

	mLightmap = storage.GetRenderTexture(14);
	mBlurTex0 = storage.GetRenderTexture(15);
	mBlurTex1 = storage.GetRenderTexture(16);

	mLightmap->SetFiltering(ITexture::Filter::Linear);
	mBlurTex0->SetFiltering(ITexture::Filter::Linear);
	mBlurTex1->SetFiltering(ITexture::Filter::Linear);
	
	mSSAO->RegisterUniform("properties", &SetSSAOProperties);

	mSSAOTarget->AttachColorTexture( 0, mLightmap, ITexture::Format::Alpha );
	mBlurTarget0->AttachColorTexture( 0, mBlurTex0, ITexture::Format::Alpha );
	mBlurTarget1->AttachColorTexture( 0, mBlurTex1, ITexture::Format::Alpha );
}

//============================================================================================================
// High quality SSAO
//============================================================================================================

ITexture* SSAO::Create (TemporaryStorage& storage, bool highQuality, uint passes, float range, float strength, float sharpness)
{
	IGraphics*		graphics = storage.GetGraphics();
	const ITexture* depth	 = storage.GetDepth();
	const ITexture* normal	 = storage.GetNormal();

	// Create necessary resources
	if (mSSAOTarget == 0) CreateResources(storage);

	// Update the shader values
	g_params.Set(range, strength);

	// Use the target size for dimensions
	Vector2i size (storage.GetFinalTargetSize());

	if (highQuality)
	{
		mSSAOTarget->SetSize(size);
		mBlurTarget0->SetSize(size);
	}
	else
	{
		mBlurTarget0->SetSize(size);
		mBlurTarget1->SetSize(size);
		mSSAOTarget->SetSize(size / 2);
	}

	// Set up the active states
	graphics->SetActiveMaterial(0);
	graphics->SetActiveTechnique(mPost);
	graphics->Flush();

	// SSAO contribution pass
	graphics->SetActiveRenderTarget(mSSAOTarget);
	graphics->SetScreenProjection(true);
	graphics->SetActiveTexture(0, depth);
	graphics->SetActiveTexture(1, normal);
	graphics->SetActiveTexture(2, mRandom);
	graphics->SetActiveShader(mSSAO);
	graphics->Draw(IGraphics::Drawable::InvertedQuad);

	// Start with the lightmap texture
	ITexture* result = mLightmap;
	graphics->SetActiveTexture(2, 0);

	// SSAO blur passes
	if (highQuality)
	{
		for (uint i = 0; i < passes; ++i)
		{
			// Blur the SSAO texture horizontally
			graphics->SetActiveRenderTarget(mBlurTarget0);
			graphics->SetActiveTexture(1, result);
			graphics->SetActiveShader(mBlurH);
			if (i == 0) mBlurH->SetUniform("threshold", Vector2f(1.0f / size.x, g_params.x / sharpness));
			graphics->Draw(IGraphics::Drawable::InvertedQuad);
			result = mBlurTex0;

			// Blur the SSAO texture vertically
			graphics->SetActiveRenderTarget(mSSAOTarget);
			graphics->SetActiveTexture(1, result);
			graphics->SetActiveShader(mBlurV);
			if (i == 0) mBlurV->SetUniform("threshold", Vector2f(1.0f / size.y, g_params.x / sharpness));
			graphics->Draw(IGraphics::Drawable::InvertedQuad);
			result = mLightmap;
		}
	}
	else
	{
		for (uint i = 0; i < passes; ++i)
		{
			// Blur the SSAO texture horizontally
			graphics->SetActiveRenderTarget(mBlurTarget0);
			graphics->SetActiveTexture(1, result);
			graphics->SetActiveShader(mBlurH);
			if (i == 0) mBlurH->SetUniform("threshold", Vector2f(1.0f / size.x, g_params.x / sharpness));
			graphics->Draw(IGraphics::Drawable::InvertedQuad);
			result = mBlurTex0;

			// Blur the SSAO texture vertically
			graphics->SetActiveRenderTarget(mBlurTarget1);
			graphics->SetActiveTexture(1, result);
			graphics->SetActiveShader(mBlurV);
			if (i == 0) mBlurV->SetUniform("threshold", Vector2f(1.0f / size.y, g_params.x / sharpness));
			graphics->Draw(IGraphics::Drawable::InvertedQuad);
			result = mBlurTex1;
		}
	}
	return result;
}