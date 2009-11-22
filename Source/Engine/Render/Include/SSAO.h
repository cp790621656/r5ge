#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Screen Space Ambient Occlusion post process effect functions
//============================================================================================================

namespace SSAO
{
	// Parameters used by the SSAO shaders: blur consideration range and SSAO's strength
	void SetParams (float range, float strength);

	// Low quality approach
	const ITexture* Low (	IGraphics*		graphics,
							const ITexture*	depth,
							const ITexture*	normal );

	// High quality approach
	const ITexture* High (	IGraphics*		graphics,
							const ITexture*	depth,
							const ITexture*	normal );
};