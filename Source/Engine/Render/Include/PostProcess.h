#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Various post-processing effects
//============================================================================================================

namespace PostProcess
{
	// No post-processing
	void None  (IGraphics* graphics, const ITexture* color);

	// Bloom post-processing effect
	void Bloom (IGraphics* graphics, const ITexture* color, float threshold, IRenderTarget* target = 0);

	// Depth of field effect
	void DepthOfField ( IGraphics*		graphics,
						const ITexture* color,
						const ITexture* depth,
						float			focalDistance,	// Distance to the focal point
						float			focalMin,		// Distance from focal point that still gets 100% focus
						float			focalMax,		// Distance from focal point that gets 0% focus
						IRenderTarget*	target = 0);

	// Both depth of field as well as bloom effects
	void Both (	IGraphics*		graphics,
				const ITexture*	color,
				const ITexture*	depth,
				float			threshold,			// Depth of field threshold (1.0 if using HDR)
				float			focalDistance,		// Distance to the 100% focal point
				float			focalMin,			// Distance from focal point that still gets 100% focus
				float			focalMax);			// Distance from focal distance that gets 0% focus
};