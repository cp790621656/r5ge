#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Light types get stored in a global array
//============================================================================================================

Hash<Deferred::DrawLightsDelegate> g_lightTypes;

//============================================================================================================
// Registers a new light source subType
//============================================================================================================

void Deferred::RegisterLight (uint subType, const Deferred::DrawLightsDelegate& func)
{
	g_lightTypes.Lock();
	g_lightTypes[subType] = func;
	g_lightTypes.Unlock();
}

//============================================================================================================
// Add all directional light contribution
//============================================================================================================

void Deferred::DrawDirectionalLights (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap)
{
	static IShader* dirShader0	 = graphics->GetShader("[R5] Light/Directional");
	static IShader* dirShader1	 = graphics->GetShader("[R5] Light/DirectionalAO");

	IShader* shader = (lightmap != 0) ? dirShader1 : dirShader0;

	// No depth test as directional light has no volume
	graphics->SetActiveTexture(2, lightmap);
	graphics->SetDepthTest(false);
	graphics->SetActiveProjection( IGraphics::Projection::Orthographic );
	graphics->ResetModelViewMatrix();
	graphics->SetActiveShader(shader);

	// Run through all directional lights
	for (uint i = 0; i < lights.GetSize(); ++i)
	{
		graphics->SetActiveLight(i, lights[i].mLight);
		graphics->Draw( IGraphics::Drawable::InvertedQuad );
	}
}

//============================================================================================================
// Add all point light contribution
//============================================================================================================

void Deferred::DrawPointLights (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap)
{
	static IShader* dirShader0	 = graphics->GetShader("[R5] Light/Point");
	static IShader* dirShader1	 = graphics->GetShader("[R5] Light/PointAO");

	IShader* shader = (lightmap != 0) ? dirShader1 : dirShader0;

	graphics->SetActiveTexture(2, lightmap);
	graphics->SetActiveProjection( IGraphics::Projection::Perspective );

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
		graphics->SetActiveLight(--b, 0);

	// Save the view matrix as it won't be changing
	const Matrix43& view = graphics->GetViewMatrix();
	Matrix43 mat;

	// Run through all point lights
	for (uint i = 0; i < lights.GetSize(); ++i)
	{
		const Light::Entry& entry = lights[i];

		// Copy the light information as we'll be modifying it
		Light light (*entry.mLight);

		// The range of the light is stored in the first attenuation parameter. The 6.5%
		// increase is there because the generated sphere goes up to (and never exceeds)
		// the radius of 1. However this means that the drawn triangles can actually be
		// closer as the sphere is never perfectly round. Thus we increase the radius by
		// this amount in order to avoid any visible edges when drawing the light. Note
		// that 6.5% is based on observation only. For icosahedrons of 2 iterations this
		// multiplier can be reduced down to 2%.

		float range (light.mAtten.x * 1.065f);

		// Distance to the light source
		float dist (light.mPos.GetDistanceTo(camPos) > (range + nearClip * 2.0f));

		// Start with the view matrix and apply the light's world transforms
		mat = view;
		mat.PreTranslate(light.mPos);
		mat.PreScale(range);

		// Set the matrix that will be used to transform this light and to draw it at the correct position
		graphics->SetModelViewMatrix(mat);

		// Reset the light's position as it will be transformed by the matrix we set above.
		// This is done in order to avoid an extra matrix switch, taking advantage of the
		// fact that OpenGL transforms light coordinates by the current ModelView matrix.
		light.mPos = Vector3f();

		// First light activates the shader
		if (i == 0) graphics->SetActiveShader(shader);

		// Activate the light at the matrix-transformed origin
		graphics->SetActiveLight(0, &light);

		if (dist)
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

		// Draw the light's sphere at the matrix-transformed position
		graphics->DrawIndices(ibo, IGraphics::Primitive::Triangle, indexCount);
	}

	// Restore important states
	graphics->SetActiveDepthFunction( IGraphics::Condition::Less );
	graphics->SetCulling(IGraphics::Culling::Back);
	graphics->ResetModelViewMatrix();
}

//============================================================================================================
// Draw all lights using depth, normal, and lightmap textures
//============================================================================================================

void DrawLights (
	IGraphics*			graphics,
	const ITexture*		depth,
	const ITexture*		normal,
	const ITexture*		lightmap,
	const Light::List&	lights )
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
	graphics->SetActiveTexture(0, depth);
	graphics->SetActiveTexture(1, normal);

	// Set up the stencil buffer to allow rendering only where pixels are '1'
	graphics->SetActiveStencilFunction( IGraphics::Condition::Equal, 0x1, 0x1 );
	graphics->SetActiveStencilOperation(IGraphics::Operation::Keep,
										IGraphics::Operation::Keep,
										IGraphics::Operation::Keep);
	
	// We want to process lights in batches of similar lights. All directional lights
	// should be processed at once. So should point lights, and so on. In order to do this
	// we collect lights into groups, then process those lights as separate individual groups.

	static Light::List group;
	static Array<bool> processed;

	processed.Reserve(lights.GetSize());
	processed.MemsetZero();

	g_lightTypes.Lock();
	{
		uint offset = 0;

		// Keep going until all lights have been processed
		for (;;)
		{
			uint subType = 0xFFFFFFFF;
			bool keepGoing = false;
			group.Clear();

			// Run through all lights
			for (uint i = offset, imax = lights.GetSize(); i < imax; ++i)
			{
				const Light::Entry& entry (lights[i]);

				// Once we find a light we haven't processed...
				if (!processed[i])
				{
					// If we haven't chosen a new subtype, use this one
					if (subType == 0xFFFFFFFF)
					{
						offset = i + 1;
						subType = entry.mLight->mSubtype;
						ASSERT(subType != 0xFFFFFFFF, "Subtype should never be set to '-1'!");
					}

					// Skip lights of different subtypes
					if (subType != entry.mLight->mSubtype)
					{
						keepGoing = true;
						continue;
					}
					
					// Add this light to the list and consider it to be processed
					group.Expand() = entry;
					processed[i] = true;
				}
			}

			// Retrieve the registered callback and execute it
			if (group.IsValid())
			{
				Deferred::DrawLightsDelegate* dlg = g_lightTypes.GetIfExists(subType);
				if (dlg != 0) (*dlg)(graphics, group, lightmap);
			}

			// Once we've processed all the lights, we're done
			if (!keepGoing) break;
		}
	}
	g_lightTypes.Unlock();
}

//============================================================================================================
// Final deferred approach function -- combine everything together
//============================================================================================================

void Combine (
	IGraphics*		graphics,
	const ITexture*	matDiff,
	const ITexture*	matSpec,
	const ITexture*	lightDiff,
	const ITexture*	lightSpec )
{
	static IShader* shader = graphics->GetShader("[R5] Deferred/Combine");

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
	graphics->Draw( IGraphics::Drawable::InvertedQuad );
}

//============================================================================================================
// Deferred rendering draw function -- does all the setup and renders into off-screen buffers
//============================================================================================================

Deferred::DrawResult Deferred::DrawScene (IGraphics* graphics, DrawParams& params, const Light::List& lights)
{
	params.mTargets.ExpandTo(3, true);
	params.mTextures.ExpandTo(7, true);

	ITexturePtr& normal		= params.mTextures[0];
	ITexturePtr& depth		= params.mTextures[1];
	ITexturePtr& matDiff	= params.mTextures[2];
	ITexturePtr& matSpec	= params.mTextures[3];
	ITexturePtr& lightDiff	= params.mTextures[4];
	ITexturePtr& lightSpec	= params.mTextures[5];
	ITexturePtr& final		= params.mTextures[6];

	IRenderTargetPtr& diffuseTarget = params.mTargets[0];
	IRenderTargetPtr& lightTarget	= params.mTargets[1];
	IRenderTargetPtr& finalTarget	= params.mTargets[2];

	if (normal == 0)
	{
		normal		= graphics->CreateRenderTexture();
		depth		= graphics->CreateRenderTexture();
		matDiff		= graphics->CreateRenderTexture();
		matSpec		= graphics->CreateRenderTexture();
		lightDiff	= graphics->CreateRenderTexture();
		lightSpec	= graphics->CreateRenderTexture();
		final		= graphics->CreateRenderTexture();
	}

	// Use the specified size if possible, viewport size otherwise
	Vector2i size (params.mRenderTarget == 0 ? graphics->GetActiveViewport() :
		params.mRenderTarget->GetSize());

	const ITexture* lightmap (0);
	DrawResult result;

	// Background color
	Color4f color (params.mRenderTarget == 0 ? graphics->GetBackgroundColor() :
				params.mRenderTarget->GetBackgroundColor());

	// Made constant so it can be quickly changed for testing purposes
	const uint HDRFormat = (color.a == 1.0f) ? ITexture::Format::RGB16F : ITexture::Format::RGBA16F;

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
			diffuseTarget->UseSkybox(params.mRenderTarget == 0 || params.mRenderTarget->IsUsingSkybox());
		}

		// Setting size only changes it if it's different
		diffuseTarget->SetSize(size);
		diffuseTarget->SetBackgroundColor(color);

		// Deferred rendering -- encoding pass
		if (params.mDrawCallback && params.mDrawTechniques.IsValid())
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
			result.mObjects += params.mDrawCallback(params.mDrawTechniques, params.mInsideOut);
		}

		// Screen-space ambient occlusion pass
		if (params.mAOLevel > 0)
		{
			graphics->SetStencilTest(true);
			graphics->SetActiveStencilFunction ( IGraphics::Condition::Equal, 0x1, 0x1 );
			graphics->SetActiveStencilOperation( IGraphics::Operation::Keep,
												 IGraphics::Operation::Keep,
												 IGraphics::Operation::Keep );
			if (params.mAOLevel == 1)
			{
				lightmap = SSAO::Low(graphics, depth, normal);
			}
			else
			{
				lightmap = SSAO::High(graphics, depth, normal);
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
		DrawLights(graphics, depth, normal, lightmap, lights);
		result.mObjects += lights.GetSize();
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

		finalTarget->SetSize(size);
		graphics->SetActiveRenderTarget(finalTarget);
		Combine(graphics, matDiff, matSpec, lightDiff, lightSpec);
	}

	// Return some useful information
	result.mColor		= final;
	result.mDepth		= depth;
	result.mNormal		= normal;
	result.mLightmap	= lightmap;
	return result;
}