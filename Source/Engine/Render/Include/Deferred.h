#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Deferred rendering approach functions
//============================================================================================================

namespace Deferred
{
	typedef ITexture* ITexturePtr;
	typedef IRenderTarget* IRenderTargetPtr;
	typedef Array<const ITechnique*> TechniqueList;

	// Delegate for the normal drawing callback. It expects a technique used to render the scene.
	typedef FastDelegate<uint (const TechniqueList& techniques, bool insideOut)> DrawCallback;

	// Optional Ambient Occlusion callback -- requires depth and normal buffers
	typedef FastDelegate<const ITexture* (IGraphics* graphics,
		const ITexture* depth,
		const ITexture* normal)> AOCallback;

	// Delegate function for lights
	typedef FastDelegate<void (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap)> DrawLightsDelegate;

	// Deferred::DrawScene should return more than just the number of triangles drawn
	struct DrawResult
	{
		uint mObjects;
		const ITexture* mColor;		// Color buffer texture
		const ITexture* mDepth;		// Depth buffer texture
		const ITexture* mNormal;	// Eye space normal texture
		const ITexture* mLightmap;	// Eye space lightmap (SSAO)

		DrawResult()			: mObjects(0), mColor(0), mDepth(0), mNormal(0), mLightmap(0) {}
		DrawResult(uint count)	: mObjects(0), mColor(0), mDepth(0), mNormal(0), mLightmap(0) {}
	};

	// Parameters used by the DrawScene functions
	struct DrawParams
	{
		DrawCallback	mDrawCallback;		// Actual draw callback -- will receive the list of techniques
		TechniqueList	mDrawTechniques;	// List of techniques used to draw the scene
		byte			mAOLevel;			// Ambient Occlusion level: 0 = none, 1 = low, 2 = high
		bool			mInsideOut;			// Whether to draw inside out (useful for reflections)
		Color4ub		mColor;				// Background color
		bool			mUseColor;			// Whether to use the background color (overrides the skybox)

		IRenderTarget*			mRenderTarget;	// Render target we should be using
		Array<ITexture*>		mTextures;		// Cached textures
		Array<IRenderTarget*>	mTargets;		// Cached render targets

		DrawParams() : mAOLevel(0), mInsideOut(false), mUseColor(false), mRenderTarget(0) {}
	};

	// Registers a new light source type
	void RegisterLight (uint subType, const DrawLightsDelegate& func);

	// Callback function that will draw directional lights
	// NOTE: By default this function is registered inside Core's DirectionalLight class constructor
	void DrawDirectionalLights (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap);

	// Callback function that will draw point lights
	// NOTE: By default this function is registered inside Core's PointLight class constructor
	void DrawPointLights (IGraphics* graphics, const Light::List& lights, const ITexture* lightmap);

	// Deferred rendering draw function -- does all the setup and rendering into off-screen buffers
	DrawResult DrawScene (IGraphics* graphics, DrawParams& params, const Light::List& lights);
};