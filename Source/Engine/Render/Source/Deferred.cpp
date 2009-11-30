#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Add all directional light contribution
//============================================================================================================

uint AddDirectionalLights (IGraphics* graphics, const ILight::List& lights, const IShader* shader)
{
	uint triangles (0);

	if (lights.IsValid())
	{
		uint maxLights = 8;

		// No depth test as directional light has no volume
		graphics->SetDepthTest(false);
		graphics->SetActiveProjection( IGraphics::Projection::Orthographic );

		// Run through all point lights
		for (uint i = 0, count = 0; i < lights.GetSize(); ++i)
		{
			graphics->SetActiveLight( count++, lights[i].mLight );

			// If we're at the end of the array, there's additional work to be done
			bool theEnd = (i + 1) == lights.GetSize();

			// If we're at max number of lights, or at the end of the lights array
			if (count == maxLights || theEnd)
			{
				// Deactivate any trailing lights
				if (theEnd)
				{
					for (uint b = count; b < maxLights; ++b)
						graphics->SetActiveLight( b, 0 );
				}

				// Shader returns the true maximum number of lights it supports
				maxLights = graphics->SetActiveShader(shader);
				if (maxLights == 0) maxLights = 1;
				i -= count - maxLights;

				// Draw a full screen quad, effectively rendering the lights using the active shader
				triangles += graphics->Draw( IGraphics::Drawable::InvertedQuad );
				count = 0;
			}
		}
	}
	return triangles;
}

//============================================================================================================
// Add all point light contribution
//============================================================================================================

uint AddPointLights (IGraphics* graphics, const ILight::List& lights, const IShader* shader)
{
	uint triangles (0);

	if (lights.IsValid())
	{
		float nearClip = graphics->GetCameraRange().x;
		const Vector3f& camPos = graphics->GetCameraPosition();
		
		static IVBO* vbo = 0;
		static IVBO* ibo = 0;
		static uint indexCount = 0;

		if (vbo == 0)
		{
			vbo = graphics->CreateVBO();
			ibo = graphics->CreateVBO();

			Array<Vector3f> vertices;
			Array<ushort> indices;
			Shape::Icosahedron(vertices, indices, 1);
			indexCount = indices.GetSize();

			vbo->Set(vertices, IVBO::Type::Vertex);
			ibo->Set(indices,  IVBO::Type::Index);
		}

		// Enable depth testing as point lights have a definite volume
		graphics->SetDepthTest(true);
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, vbo, 0, IGraphics::DataType::Float, 3, 12 );

		// Disable all active lights except the first
		for (uint b = lights.GetSize(); b > 1; )
			graphics->SetActiveLight( --b, 0 );

		// Run through all point lights
		for (uint i = 0; i < lights.GetSize(); ++i)
		{
			const ILight::Entry& entry = lights[i];

			// Activate this light
			graphics->ResetWorldMatrix();
			graphics->SetActiveLight( 0, entry.mLight );

			// First light activates the shader
			if (i == 0) graphics->SetActiveShader(shader);

			// The range of the light is stored in the first attenuation parameter. The 6.5%
			// increase is there because the generated sphere goes up to (and never exceeds)
			// the radius of 1. However this means that the drawn triangles can actually be
			// closer as the sphere is never perfectly round. Thus we increase the radius by
			// this amount in order to avoid any visible edges when drawing the light. Note
			// that 6.5% is based on observation only. For icosahedrons of 2 iterations this
			// multiplier can be reduced down to 2%.

			float range (entry.mLight->GetAttenParams()->x * 1.065f);
			const Vector3f& lightPos (entry.mLight->GetPosition());

			if (lightPos.GetDistanceTo(camPos) > (range + nearClip * 2.0f))
			{
				// The camera is outside the sphere -- regular rendering approach
				graphics->SetCulling( IGraphics::Culling::Back );
				graphics->SetActiveDepthFunction( IGraphics::Condition::Less );
			}
			else
			{
				// The camera is inside the sphere -- draw the inner side, and only
				// on pixels that are closer to the camera than the light's range.

				graphics->SetCulling( IGraphics::Culling::Front );
				graphics->SetActiveDepthFunction( IGraphics::Condition::Greater );
			}

			// Draw the light's sphere
			graphics->SetWorldMatrix( Matrix43 (lightPos, range) );
			triangles += graphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, indexCount);
		}

		// Restore important states
		graphics->SetActiveDepthFunction( IGraphics::Condition::Less );
		graphics->SetCulling(IGraphics::Culling::Back);
		graphics->ResetWorldMatrix();
	}
	return triangles;
}

//============================================================================================================
// Draw all lights using depth, normal, and lightmap textures
//============================================================================================================

uint DrawLights (	IGraphics*			graphics,
					const ITexture*		depth,
					const ITexture*		normal,
					const ITexture*		lightmap,
					const ILight::List&	lights )
{
	// Set up appropriate states
	graphics->SetFog(false);
	graphics->SetStencilTest(true);
	graphics->SetDepthWrite(false);
	graphics->SetColorWrite(true);
	graphics->SetAlphaTest(false);
	graphics->SetWireframe(false);
	graphics->SetLighting(IGraphics::Lighting::None);
	graphics->SetBlending(IGraphics::Blending::Add);

	// Disable active material and clear any active buffers
	graphics->SetActiveMaterial(0);
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex,	0 );
	graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight,	0 );

	// We are using 3 textures -- depth, view space normal (with shininess in alpha), and the ao lightmap
	graphics->SetActiveTexture( 0, depth );
	graphics->SetActiveTexture( 1, normal );
	graphics->SetActiveTexture( 2, lightmap );

	// Set up the stencil buffer to allow rendering only where pixels are '1'
	graphics->SetActiveProjection( IGraphics::Projection::Perspective );
	graphics->SetActiveStencilFunction( IGraphics::Condition::Equal, 0x1, 0x1 );
	graphics->SetActiveStencilOperation(IGraphics::Operation::Keep,
										IGraphics::Operation::Keep,
										IGraphics::Operation::Keep);

	// Collect all lights
	static ILight::List directional;
	static ILight::List point;

	directional.Clear();
	point.Clear();

	for (uint i = 0; i < lights.GetSize(); ++i)
	{
		const ILight::Entry& entry (lights[i]);

		if (entry.mLight != 0)
		{
			uint type = entry.mLight->GetLightType();

			if (type == ILight::Type::Directional)
			{
				directional.Expand() = entry;
			}
			else if (type == ILight::Type::Point)
			{
				point.Expand() = entry;
			}
		}
	}

	static IShader* dirShader0	 = graphics->GetShader("Deferred/Light/Directional");
	static IShader* pointShader0 = graphics->GetShader("Deferred/Light/Point");
	static IShader* dirShader1	 = graphics->GetShader("SSAO/Light/Directional");
	static IShader* pointShader1 = graphics->GetShader("SSAO/Light/Point");

	// Draw actual light contribution
	return	AddDirectionalLights(graphics, directional, (lightmap != 0) ? dirShader1   : dirShader0  ) + 
			AddPointLights		(graphics, point,		(lightmap != 0) ? pointShader1 : pointShader0);
}

//============================================================================================================
// Final deferred approach function -- combine everything together
//============================================================================================================

uint Combine (	IGraphics*		graphics,
				const ITexture*	matDiff,
				const ITexture*	matSpec,
				const ITexture*	lightDiff,
				const ITexture*	lightSpec )
{
	static IShader* shader = graphics->GetShader("Deferred/Process/Combine");

	graphics->SetDepthWrite(false);
	graphics->SetDepthTest(false);
	graphics->SetStencilTest(false);
	graphics->SetBlending(IGraphics::Blending::None);
	graphics->SetCulling(IGraphics::Culling::Back);

	graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
	graphics->SetActiveMaterial(0);
	graphics->SetActiveShader(shader);
	graphics->SetActiveTexture( 0, matDiff );
	graphics->SetActiveTexture( 1, matSpec );
	graphics->SetActiveTexture( 2, lightDiff );
	graphics->SetActiveTexture( 3, lightSpec );
	
	return graphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Deferred rendering draw function -- does all the setup and renders into off-screen buffers
//============================================================================================================

Deferred::DrawResult Deferred::DrawScene (
	IGraphics*				graphics,
	const ILight::List&		lights,
	const DrawCallback&		drawCallback,
	const DrawCallback&		decalCallback,
	const AOCallback&		aoCallback,
	bool					insideOut)
{
	static bool				firstTime	= true;
	static ITechnique*		deferred	= graphics->GetTechnique("Deferred");
	static ITechnique*		decal		= graphics->GetTechnique("Decal");
	static IRenderTarget*	target0		= graphics->CreateRenderTarget();
	static IRenderTarget*	target1		= graphics->CreateRenderTarget();
	static IRenderTarget*	target2		= graphics->CreateRenderTarget();
	static ITexture*		normal		= graphics->GetTexture("[Generated] Normal");
	static ITexture*		depth		= graphics->GetTexture("[Generated] Depth");
	static ITexture*		matDiff		= graphics->GetTexture("[Generated] Diffuse Material");
	static ITexture*		matSpec		= graphics->GetTexture("[Generated] Specular Material");
	static ITexture*		lightDiff	= graphics->GetTexture("[Generated] Diffuse Light");
	static ITexture*		lightSpec	= graphics->GetTexture("[Generated] Specular Light");
	static ITexture*		final		= graphics->GetTexture("[Generated] Final");

	uint triangles (0);
	const ITexture* lightmap (0);

	if (firstTime)
	{
		firstTime = false;

		// Deferred rendering target
		target0->AttachDepthTexture( depth );
		target0->AttachStencilTexture( depth );
		target0->AttachColorTexture( 0, matDiff, ITexture::Format::RGB16F );
		target0->AttachColorTexture( 1, matSpec, ITexture::Format::RGBA );
		target0->AttachColorTexture( 2, normal,  ITexture::Format::RGB30A2 );
		target0->UseSkybox(true);

		// Scene Light contribution target
		target1->AttachDepthTexture( depth );
		target1->AttachStencilTexture( depth );
		target1->AttachColorTexture( 0, lightDiff, ITexture::Format::RGB16F );
		target1->AttachColorTexture( 1, lightSpec, ITexture::Format::RGB16F );
		target1->SetBackgroundColor( Color4f(0.0f, 0.0f, 0.0f, 1.0f) );
		target1->UseSkybox(false);

		// Final color target
		target2->AttachDepthTexture( depth );
		target2->AttachStencilTexture( depth );
		target2->AttachColorTexture( 0, final, ITexture::Format::RGB16F );
		target2->SetBackgroundColor( Color4f(0.0f, 0.0f, 0.0f, 1.0f) );
		target2->UseSkybox(false);
	}

	// Setting size only changes it if it's different
	Vector2i size (graphics->GetActiveViewport());
	target0->SetSize( size );
	target1->SetSize( size );
	target2->SetSize( size );

	// Active background color
	Color4f color (graphics->GetBackgroundColor());
	color.a = 1.0f;
	target0->SetBackgroundColor(color);

	// Deferred rendering -- encoding pass
	if (drawCallback)
	{
		graphics->SetCulling( IGraphics::Culling::Back );
		graphics->SetActiveDepthFunction( IGraphics::Condition::Less );

		graphics->SetStencilTest(false);
		graphics->SetActiveRenderTarget( target0 );
		graphics->SetActiveProjection( IGraphics::Projection::Perspective );
		graphics->Clear(true, true, true);

		// Set up the stencil test
		graphics->SetStencilTest(true);
		graphics->SetActiveStencilFunction ( IGraphics::Condition::Always, 0x1, 0x1 );
		graphics->SetActiveStencilOperation( IGraphics::Operation::Keep,
											 IGraphics::Operation::Keep,
											 IGraphics::Operation::Replace );

		// Draw the scene using the deferred approach
		triangles += drawCallback(deferred, insideOut);
	}

	// Decal pass
	if (decalCallback)
	{
		// Switching the render targets back and forth effectively force-finishes all draw operations
		graphics->SetActiveRenderTarget( 0 );
		graphics->SetActiveRenderTarget( target0 );

		// Affect only pixels we wrote to and don't write to depth
		graphics->SetStencilTest(true);
		graphics->SetDepthWrite(false);
		graphics->SetActiveStencilFunction ( IGraphics::Condition::Equal, 0x1, 0x1 );
		graphics->SetActiveStencilOperation( IGraphics::Operation::Keep,
											 IGraphics::Operation::Keep,
											 IGraphics::Operation::Keep );

		// Trigger the decal callback
		triangles += decalCallback(decal, insideOut);
	}

	// Screen-space ambient occlusion pass
	if (aoCallback)
	{
		graphics->SetStencilTest(true);
		graphics->SetActiveStencilFunction ( IGraphics::Condition::Equal, 0x1, 0x1 );
		graphics->SetActiveStencilOperation( IGraphics::Operation::Keep,
											 IGraphics::Operation::Keep,
											 IGraphics::Operation::Keep );

		// Trigger the ambient occlusion callback
		lightmap = aoCallback(graphics, depth, normal);
	}

	// Light contribution
	{
		graphics->SetActiveRenderTarget( target1 );
		graphics->Clear(true, false, false);
		triangles += DrawLights(graphics, depth, normal, lightmap, lights);
	}

	// Combine the light contribution with material
	{
		graphics->SetActiveRenderTarget( target2 );
		triangles += Combine(graphics, matDiff, matSpec, lightDiff, lightSpec);
	}

	// Return some useful information
	DrawResult result;
	result.mTriangles	= triangles;
	result.mColor		= final;
	result.mDepth		= depth;
	result.mNormal		= normal;
	result.mLightmap	= lightmap;
	return result;
}