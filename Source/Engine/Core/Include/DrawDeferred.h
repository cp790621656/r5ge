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
		typedef FastDelegate<uint (const Techniques& techniques, bool clearScreen)> DrawCallback;

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

		Storage() : mRenderTarget(0), mAOLevel(0), mInsideOut(false),
			mOutColor(0), mOutDepth(0), mOutNormal(0), mOutLightmap(0)
		{
			mTempTextures.ExpandTo(32, true);
			mTempTargets.ExpandTo(32, true);
		}

		// Releases all associated resources
		void Release (IGraphics* graphics);

		// ADVANCED FUNCTIONALITY:
		// The following textures are available during deferred OnDraw callbacks, but Graphics::Flush()
		// must be called first. Be careful using these -- keep in mind that the engine is writing to
		// the very same textures while you're accessing them, so don't read/write on more than 1 pixel
		// at a time, or you will get unpredictable results. Ideal usage would be for soft particles:
		// depth write is off during their process, making it safe to use for sampling purposes.

		const ITexture* GetDepth()				const { return mTempTextures[0]; }
		const ITexture* GetNormal()				const { return mTempTextures[1]; }
		const ITexture* GetMaterialDiffuse()	const { return mTempTextures[2]; }
		const ITexture* GetMaterialSpecular()	const { return mTempTextures[3]; }
	};

	// Draw the scene using the deferred rendering approach
	uint Draw (IGraphics* graphics, Deferred::Storage& storage, const Light::List& lights);
};