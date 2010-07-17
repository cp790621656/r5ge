#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Adds a new light to the draw queue
//============================================================================================================

void DrawQueue::Add (LightSource* light, float distanceToCamera)
{
	LightEntry& ent = mLights.Expand();
	ent.mLight = light;
	ent.mDistance = distanceToCamera;
}

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

uint DrawQueue::Draw (TemporaryStorage& storage, IGraphics* graphics, const Techniques& techniques,
					  bool useLighting, bool insideOut)
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
			// Draw all deferred techniques first
			FOREACH(b, techniques)
			{
				const ITechnique* tech = techniques[b];

				// If the layer has something visible on the specified technique
				if (tech != 0 && (layer.mMask & tech->GetMask()) != 0)
				{
					// Call the optional callback
					if (mOnDraw) mOnDraw(tech);

					// Activate the technique
					graphics->SetActiveTechnique(tech, insideOut);

					// Activate lights
					if (useLighting && tech->GetLighting() != IGraphics::Lighting::None)
						ActivateLights(graphics);

					// Draw everything on this layer
					result += layer.Draw(storage, tech, insideOut);
				}
			}
		}
	}
	return result;
}

//============================================================================================================
// Activate all lights
//============================================================================================================

void DrawQueue::ActivateLights (IGraphics* graphics)
{
	uint last = mLights.GetSize();

	if (last > 0)
	{
		graphics->ResetModelViewMatrix();

		for (uint i = 0; i < last; ++i)
		{
			LightSource* light = mLights[i].mLight;
			graphics->SetActiveLight(i, (light == 0) ? 0 : &light->GetProperties());
		}

		for (uint i = last; i < 8; ++i)
		{
			graphics->SetActiveLight(i, 0);
		}
	}
}