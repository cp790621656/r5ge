#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Screen Space Ambient Occlusion post process effect functions
//============================================================================================================

namespace SSAO
{
	typedef Scene::ITexturePtr		ITexturePtr;
	typedef Scene::IRenderTargetPtr	IRenderTargetPtr;

	// Parameters used by the SSAO shaders: blur consideration range and SSAO's strength
	void SetParams (float range, float strength);

	// Low quality approach
	const ITexture* Low (IGraphics* graphics, Deferred::Storage& storage);

	// High quality approach
	const ITexture* High (IGraphics* graphics, Deferred::Storage& storage);
};