#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Add all directional light contribution
//============================================================================================================

uint AddDirectionalLights (IGraphics* graphics, const Deferred::Lights& lights, const IShader* shader)
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

uint AddPointLights (IGraphics* graphics, const Deferred::Lights& lights, const IShader* shader)
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
			Shape::Icosahedron(1, vertices, indices);
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
			const Object::ILight::Entry& entry = lights[i];

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

			if (lightPos.GetDistanceTo(camPos) > (range + nearClip * 2))
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

uint Deferred::DrawLights (	IGraphics*		graphics,
							const ITexture*	depth,
							const ITexture*	normal,
							const ITexture*	lightmap,
							const Lights&	lights )
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

	// We are using 3 textures -- depth, view space normal (with shininess in alpha), and the optional lightmap
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
	static Object::ILight::List directional;
	static Object::ILight::List point;

	directional.Clear();
	point.Clear();

	for (uint i = 0; i < lights.GetSize(); ++i)
	{
		const Object::ILight::Entry& entry (lights[i]);

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

uint Deferred::Combine (IGraphics*		graphics,
						const ITexture*	matDiff,
						const ITexture*	matSpec,
						const ITexture*	lightDiff,
						const ITexture*	lightSpec)
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