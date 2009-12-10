#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Deferred rendering approach functions
//============================================================================================================

namespace Deferred
{
	typedef Array<const ITechnique*> TechniqueList;

	// Delegate for the normal drawing callback. It expects a technique used to render the scene.
	typedef FastDelegate<uint (const TechniqueList& techniques, bool insideOut)> DrawCallback;

	// Optional Ambient Occlusion callback -- requires depth and normal buffers
	typedef FastDelegate<const ITexture* (IGraphics* graphics,
		const ITexture* depth,
		const ITexture* normal)> AOCallback;

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

	// Deferred rendering draw function -- does all the setup and rendering into off-screen buffers
	DrawResult DrawScene (
		IGraphics*			 graphics,
		const Light::List&	 lights,		// List of all visible lights
		const TechniqueList& techniques,	// Techniques to draw the scene with
		const DrawCallback&	 drawCallback,	// Actual scene drawing callback
		const AOCallback&	 aoCallback		= 0,
		bool				 insideOut		= false);
};