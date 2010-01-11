#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Draw the scene
//============================================================================================================

uint DrawQueue::Draw (IGraphics* graphics, const Techniques& techniques, bool insideOut)
{
	uint result(0);
	uint mask = 0;

	// Run through all specified techniques and collect the combined mask
	for (uint i = techniques.GetSize(); i > 0; )
	{
		const ITechnique* tech = techniques[--i];
		if (tech != 0) mask |= tech->GetMask();
	}

	// Run through all layers
	for (uint i = 0; i < 32; ++i)
	{
		DrawLayer& layer = mLayers[i];

		// Ensure this layer is visible with our mask
		if ((layer.mMask & mask) != 0)
		{
			// Run through all specified techniques
			for (uint b = 0; b < techniques.GetSize(); ++b)
			{
				const ITechnique* tech = techniques[b];

				// If the layer has something visible on the specified technique
				if ( tech != 0 && (layer.mMask & tech->GetMask()) != 0 )
				{
					// Activate the technique
					graphics->SetActiveTechnique(tech, insideOut);

					// Activate all lights
					if (tech->GetLighting() != IGraphics::Lighting::None)
					{
						uint last = mLights.GetSize();

						if (last > 0)
						{
							graphics->ResetModelViewMatrix();

							for (uint i = 0; i < last; ++i)
								graphics->SetActiveLight(i, mLights[i].mLight);

							for (uint i = last; i < 8; ++i)
								graphics->SetActiveLight(i, 0);
						}
					}

					// Draw everything on this layer using this technique
					result += layer.Draw(tech, insideOut);
				}
			}
		}
	}
	return result;
}