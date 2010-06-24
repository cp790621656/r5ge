#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Whether we have something to draw
//============================================================================================================

bool DrawQueue::IsValid() const
{
	for (uint i = 0; i < 32; ++i)
	{
		if (mLayers[i].mList.IsValid())
		{
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Draw the scene
//============================================================================================================

uint DrawQueue::Draw (const Deferred::Storage& storage, IGraphics* graphics, const Techniques& techniques)
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
					graphics->SetActiveTechnique(tech, storage.mInsideOut);

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
					result += layer.Draw(storage, tech);
				}
			}
		}
	}
	return result;
}