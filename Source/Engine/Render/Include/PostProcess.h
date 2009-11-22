#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Various post-processing effects
//============================================================================================================

namespace PostProcess
{
	// No post-processing
	void None  (IGraphics* graphics, const ITexture* color);

	// Bloom post-processing effect
	void Bloom (IGraphics* graphics, const ITexture* color, float threshold);

	// Depth of field effect
	void DepthOfField ( IGraphics*		graphics,
						const ITexture* color,
						const ITexture* depth,
						float			focalDistance,	// Distance to the focal point
						float			focalMin,		// Range from focal distance that still gets 100% focus
						float			focalMax);		// Range from focal distance that gets 0% focus
};