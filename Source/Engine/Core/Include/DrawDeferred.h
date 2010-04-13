#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred rendering functionality
//============================================================================================================

namespace Deferred
{
	// Parameters used by the deferred and post-processing draw functions
	struct Storage
	{
		typedef Array<const ITechnique*>	Techniques;
		typedef Array<IRenderTarget*>		Targets;
		typedef Array<ITexture*>			Textures;

		// Delegate for the normal drawing callback. It expects a technique used to render the scene.
		typedef FastDelegate<uint (const Techniques& techniques, bool insideOut)> DrawCallback;

		IRenderTarget*	mRenderTarget;	// Render target we should be using
		DrawCallback	mDrawCallback;	// Actual draw callback -- will receive the list of techniques
		Techniques		mDrawTechniques;// List of techniques used to draw the scene
		byte			mAOLevel;		// Ambient Occlusion level: 0 = none, 1 = low, 2 = high
		bool			mInsideOut;		// Whether to draw inside out (useful for reflections)

		Textures		mTempTextures;	// Cached textures
		Targets			mTempTargets;	// Cached render targets

		const ITexture* mOutColor;		// Final color buffer texture
		const ITexture* mOutDepth;		// Final depth buffer texture
		const ITexture* mOutNormal;		// Final eye space normal texture
		const ITexture* mOutLightmap;	// Final eye space lightmap (SSAO)

		Deferred::Storage() : mRenderTarget(0), mAOLevel(0), mInsideOut(false),
			mOutColor(0), mOutDepth(0), mOutNormal(0), mOutLightmap(0) {}
	};

	// Draw the scene using the deferred rendering approach
	uint Draw (IGraphics* graphics, Deferred::Storage& storage, const Light::List& lights);
};