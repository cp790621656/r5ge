#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// ILight types get stored in a global array
//============================================================================================================

Hash<Light::DrawCallback> g_lightTypes;

//============================================================================================================
// Registers a new light source type: use the function template instead
//============================================================================================================

void Light::_Register (uint subType, const DrawCallback& callback)
{
	g_lightTypes.Lock();
	g_lightTypes[subType] = callback;
	g_lightTypes.Unlock();
}

//============================================================================================================
// Draw all lights using depth, normal, and lightmap textures
//============================================================================================================

void Light::Draw (
	IGraphics*			graphics,
	const ITexture*		depth,
	const ITexture*		normal,
	const ITexture*		lightmap,
	const List&			lights )
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

	// We are using 2 textures
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
				DrawCallback* dlg = g_lightTypes.GetIfExists(subType);
				if (dlg != 0) (*dlg)(graphics, group, lightmap);
			}

			// Once we've processed all the lights, we're done
			if (!keepGoing) break;
		}
	}
	g_lightTypes.Unlock();
}