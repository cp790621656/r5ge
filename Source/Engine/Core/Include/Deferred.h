#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Deferred rendering approach functions
//============================================================================================================

namespace Deferred
{
	typedef Object::ILight::List Lights;

	// Draw all lights using depth, normal, and lightmap textures
	uint DrawLights (	IGraphics*		graphics,
						const ITexture*	depth,
						const ITexture*	normal,
						const ITexture*	lightmap,
						const Lights&	lights );

	// Final deferred approach function -- combines material and light textures together
	uint Combine (	IGraphics*		graphics,
					const ITexture*	matDiff,
					const ITexture*	matSpec,
					const ITexture*	lightDiff,
					const ITexture*	lightSpec );
};