#include "../Include/_All.h"
using namespace R5;
#define BLUR_PASSES 2

//============================================================================================================
// Callback triggered when updating the 'properties' uniform on the SSAO shader
//============================================================================================================

Vector2f g_params (1.0f, 4.0f);

void SetSSAOProperties (const String& name, Uniform& data)
{
	data = g_params;
}

//============================================================================================================
// Parameters used by the SSAO shaders: blur consideration range and SSAO's strength
//============================================================================================================

void SSAO::SetParams (float range, float strength)
{
	g_params.Set(range, strength);
}

//============================================================================================================
// Generates a random texture used by SSAO
//============================================================================================================

const ITexture* GetRandomizedNormalmap (IGraphics* graphics)
{
	static ITexture* random = graphics->GetTexture("[Generated] Random Offset");

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
// Low quality SSAO
//============================================================================================================

const ITexture* SSAO::Low (IGraphics* graphics, Deferred::Storage& storage)
{
	static ITechnique*		post	= graphics->GetTechnique("Post Process");
	static IShader*			ssao	= graphics->GetShader("[R5] SSAO/Sample");
	static IShader*			blurH	= graphics->GetShader("[R5] SSAO/Horizontal Blur");
	static IShader*			blurV	= graphics->GetShader("[R5] SSAO/Vertical Blur");
	static const ITexture*	random	= GetRandomizedNormalmap(graphics);

	IRenderTargetPtr& ssaoTarget	= storage.mTempTargets[10];
	IRenderTargetPtr& blurTarget0	= storage.mTempTargets[11];
	IRenderTargetPtr& blurTarget1	= storage.mTempTargets[12];

	ITexturePtr& lightmap = storage.mTempTextures[14];
	ITexturePtr& blurTex0 = storage.mTempTextures[15];
	ITexturePtr& blurTex1 = storage.mTempTextures[16];

	if (ssaoTarget == 0)
	{
		ssaoTarget	= graphics->CreateRenderTarget();
		lightmap	= graphics->CreateRenderTexture();

		// Screen space ambient occlusion target
		ssaoTarget->AttachColorTexture( 0, lightmap, ITexture::Format::Alpha );
		ssaoTarget->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

		ssao->RegisterUniform ("properties", &SetSSAOProperties);
		blurH->RegisterUniform("properties", &SetSSAOProperties);
		blurV->RegisterUniform("properties", &SetSSAOProperties);
	}

	if (blurTarget0 == 0)
	{
		blurTarget0	= graphics->CreateRenderTarget();
		blurTex0	= graphics->CreateRenderTexture();

		// Horizontal Blur texture target
		blurTarget0->AttachColorTexture( 0, blurTex0, ITexture::Format::Alpha );
		blurTarget0->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
	}

	if (blurTarget1 == 0)
	{
		blurTarget1	= graphics->CreateRenderTarget();
		blurTex1	= graphics->CreateRenderTexture();

		// Vertical Blur texture target
		blurTarget1->AttachColorTexture( 0, blurTex1, ITexture::Format::Alpha );
		blurTarget1->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
	}

	Vector2i size (storage.mRenderTarget == 0 ? graphics->GetViewport() :
		storage.mRenderTarget->GetSize());

	ssaoTarget->SetSize(size / 2);
	blurTarget0->SetSize(size);
	blurTarget1->SetSize(size);

	// Set up the active states
	graphics->SetActiveTechnique(post);
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		0 );
	graphics->SetActiveMaterial(0);
	graphics->Flush();

	// SSAO contribution pass
	{
		graphics->SetActiveRenderTarget( ssaoTarget );
		graphics->SetScreenProjection( true );
		graphics->SetActiveTexture( 0, storage.mOutDepth );
		graphics->SetActiveTexture( 1, storage.mOutNormal );
		graphics->SetActiveTexture( 2, random );
		graphics->SetActiveShader ( ssao );

		lightmap->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw(IGraphics::Drawable::InvertedQuad);
	}

	ITexture* result = lightmap;

	// SSAO blur passes
	for (uint i = 0; i < BLUR_PASSES; ++i)
	{
		// Blur the SSAO texture horizontally
		graphics->SetActiveRenderTarget( blurTarget0 );
		if (i == 0) graphics->SetScreenProjection( true );
		graphics->SetActiveTexture( 0, result );
		graphics->SetActiveTexture( 1, storage.mOutDepth );
		graphics->SetActiveShader( blurH );
		blurTex0->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
		result = blurTex0;

		// Blur the SSAO texture vertically
		graphics->SetActiveRenderTarget( blurTarget1 );
		graphics->SetActiveTexture( 0, result );
		graphics->SetActiveTexture( 1, storage.mOutDepth );
		graphics->SetActiveShader( blurV );
		blurTex1->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
		result = blurTex1;
	}
	return result;
}

//============================================================================================================
// High quality SSAO
//============================================================================================================

const ITexture* SSAO::High (IGraphics* graphics, Deferred::Storage& storage)
{
	static ITechnique*		post	= graphics->GetTechnique("Post Process");
	static IShader*			ssao	= graphics->GetShader("[R5] SSAO/Sample");
	static IShader*			blurH	= graphics->GetShader("[R5] SSAO/Horizontal Blur");
	static IShader*			blurV	= graphics->GetShader("[R5] SSAO/Vertical Blur");
	static const ITexture*	random	= GetRandomizedNormalmap(graphics);

	IRenderTargetPtr&	ssaoTarget	= storage.mTempTargets[10];
	IRenderTargetPtr&	blurTarget	= storage.mTempTargets[11];
	ITexturePtr&		lightmap	= storage.mTempTextures[14];
	ITexturePtr&		blurTex		= storage.mTempTextures[15];

	if (ssaoTarget == 0)
	{
		ssaoTarget	= graphics->CreateRenderTarget();
		blurTarget	= graphics->CreateRenderTarget();
		lightmap	= graphics->CreateRenderTexture();
		blurTex		= graphics->CreateRenderTexture();
	}

	if (!ssaoTarget->HasColor())
	{
		ssao->RegisterUniform ("properties", &SetSSAOProperties);
		blurH->RegisterUniform("properties", &SetSSAOProperties);
		blurV->RegisterUniform("properties", &SetSSAOProperties);

		// Screen space ambient occlusion target
		ssaoTarget->AttachColorTexture( 0, lightmap, ITexture::Format::Alpha );
		ssaoTarget->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

		// Blur texture target
		blurTarget->AttachColorTexture( 0, blurTex, ITexture::Format::Alpha );
		blurTarget->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
	}

	Vector2i size (storage.mRenderTarget == 0 ? graphics->GetViewport() :
		storage.mRenderTarget->GetSize());

	ssaoTarget->SetSize(size);
	blurTarget->SetSize(size);

	// Set up the active states
	graphics->SetActiveTechnique(post);
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		0 );
	graphics->SetActiveMaterial(0);
	graphics->Flush();

	// SSAO contribution pass
	{
		graphics->SetActiveRenderTarget( ssaoTarget );
		graphics->SetScreenProjection( true );
		graphics->SetActiveTexture( 0, storage.mOutDepth );
		graphics->SetActiveTexture( 1, storage.mOutNormal );
		graphics->SetActiveTexture( 2, random );
		graphics->SetActiveShader ( ssao );

		lightmap->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw(IGraphics::Drawable::InvertedQuad);
	}

	// SSAO blur passes
	for (uint i = 0; i < BLUR_PASSES; ++i)
	{
		// Blur the SSAO texture horizontally
		graphics->SetActiveRenderTarget( blurTarget );
		graphics->SetActiveTexture( 0, lightmap );
		graphics->SetActiveTexture( 1, storage.mOutDepth );
		graphics->SetActiveShader( blurH );
		blurTex->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );

		// Blur the SSAO texture vertically
		graphics->SetActiveRenderTarget( ssaoTarget );
		graphics->SetActiveTexture( 0, blurTex );
		graphics->SetActiveTexture( 1, storage.mOutDepth );
		graphics->SetActiveShader( blurV );
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
	return lightmap;
}