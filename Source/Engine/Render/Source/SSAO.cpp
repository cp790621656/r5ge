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

const ITexture* SSAO::Low ( IGraphics*		graphics,
							const ITexture*	depth,
							const ITexture*	normal )
{
	static ITechnique*		post		= graphics->GetTechnique("Post Process");
	static IShader*			ssao		= graphics->GetShader("_BuiltIn/Deferred/SSAO");
	static IShader*			blurH		= graphics->GetShader("_BuiltIn/Blur/depthH");
	static IShader*			blurV		= graphics->GetShader("_BuiltIn/Blur/depthV");
	static IRenderTarget*	ssaoTarget	= graphics->CreateRenderTarget();
	static IRenderTarget*	blurTarget0	= graphics->CreateRenderTarget();
	static IRenderTarget*	blurTarget1	= graphics->CreateRenderTarget();
	static ITexture*		lightmap	= graphics->GetTexture("[Generated] Lightmap Low");
	static ITexture*		blurTex0	= graphics->GetTexture("[Generated] SSAO Blur 0 Low");
	static ITexture*		blurTex1	= graphics->GetTexture("[Generated] SSAO Blur 1 Low");
	static const ITexture*	random		= GetRandomizedNormalmap(graphics);

	if (!ssaoTarget->HasColor())
	{
		ssao->RegisterUniform("properties", &SetSSAOProperties);
		blurH->RegisterUniform("properties", &SetSSAOProperties);
		blurV->RegisterUniform("properties", &SetSSAOProperties);

		// Screen space ambient occlusion target
		ssaoTarget->AttachColorTexture( 0, lightmap, ITexture::Format::Alpha );
		ssaoTarget->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

		// Horizontal Blur texture target
		blurTarget0->AttachColorTexture( 0, blurTex0, ITexture::Format::Alpha );
		blurTarget0->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

		// Vertical Blur texture target
		blurTarget1->AttachColorTexture( 0, blurTex1, ITexture::Format::Alpha );
		blurTarget1->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
	}

	Vector2i size (graphics->GetViewport());

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

	// SSAO contribution pass
	{
		graphics->SetActiveRenderTarget( ssaoTarget );
		graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
		graphics->SetActiveTexture( 0, depth );
		graphics->SetActiveTexture( 1, normal );
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
		if (i == 0) graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
		graphics->SetActiveTexture( 0, result );
		graphics->SetActiveTexture( 1, depth );
		graphics->SetActiveShader( blurH );
		blurTex0->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
		result = blurTex0;

		// Blur the SSAO texture vertically
		graphics->SetActiveRenderTarget( blurTarget1 );
		graphics->SetActiveTexture( 0, result );
		graphics->SetActiveTexture( 1, depth );
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

const ITexture* SSAO::High ( IGraphics*			graphics,
							 const ITexture*	depth,
							 const ITexture*	normal )
{
	static ITechnique*		post		= graphics->GetTechnique("Post Process");
	static IShader*			ssao		= graphics->GetShader("_BuiltIn/Deferred/SSAO");
	static IShader*			blurH		= graphics->GetShader("_BuiltIn/Blur/depthH");
	static IShader*			blurV		= graphics->GetShader("_BuiltIn/Blur/depthV");
	static IRenderTarget*	ssaoTarget	= graphics->CreateRenderTarget();
	static IRenderTarget*	blurTarget	= graphics->CreateRenderTarget();
	static ITexture*		blurTex		= graphics->GetTexture("[Generated] SSAO Blur High");
	static const ITexture*	random		= GetRandomizedNormalmap(graphics);
	static ITexture*		lightmap	= graphics->GetTexture("[Generated] Lightmap High");

	if (!ssaoTarget->HasColor())
	{
		ssao->RegisterUniform("properties", &SetSSAOProperties);
		blurH->RegisterUniform("properties", &SetSSAOProperties);
		blurV->RegisterUniform("properties", &SetSSAOProperties);

		// Screen space ambient occlusion target
		ssaoTarget->AttachColorTexture( 0, lightmap, ITexture::Format::Alpha );
		ssaoTarget->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

		// Blur texture target
		blurTarget->AttachColorTexture( 0, blurTex, ITexture::Format::Alpha );
		blurTarget->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
	}

	Vector2i size (graphics->GetViewport());

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

	// SSAO contribution pass
	{
		graphics->SetActiveRenderTarget( ssaoTarget );
		graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
		graphics->SetActiveTexture( 0, depth );
		graphics->SetActiveTexture( 1, normal );
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
		graphics->SetActiveTexture( 1, depth );
		graphics->SetActiveShader( blurH );
		blurTex->SetFiltering(ITexture::Filter::Linear);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );

		// Blur the SSAO texture vertically
		graphics->SetActiveRenderTarget( ssaoTarget );
		graphics->SetActiveTexture( 0, blurTex );
		graphics->SetActiveTexture( 1, depth );
		graphics->SetActiveShader( blurV );
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
	return lightmap;
}