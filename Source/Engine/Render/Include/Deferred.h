#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Deferred rendering approach functions
//============================================================================================================

namespace Deferred
{
	// Delegate for the normal drawing callback. It expects a technique used to render the scene.
	typedef FastDelegate<uint (const ITechnique*, bool insideOut)> DrawCallback;

	// Optional Ambient Occlusion callback -- requires depth and normal buffers
	typedef FastDelegate<const ITexture* (IGraphics* graphics,
		const ITexture* depth,
		const ITexture* normal)> AOCallback;

	// Deferred::DrawScene should return more than just the number of triangles drawn
	struct DrawResult
	{
		uint mTriangles;			// Number of rendered triangles
		const ITexture* mColor;		// Color buffer texture
		const ITexture* mDepth;		// Depth buffer texture
		const ITexture* mNormal;	// Eye space normal texture
		const ITexture* mLightmap;	// Eye space lightmap (SSAO)

		DrawResult()			: mTriangles(0),	 mColor(0), mDepth(0), mNormal(0), mLightmap(0) {}
		DrawResult(uint count)	: mTriangles(count), mColor(0), mDepth(0), mNormal(0), mLightmap(0) {}

		operator uint() const { return mTriangles; }
	};

	// Deferred rendering draw function -- does all the setup and rendering into off-screen buffers
	DrawResult DrawScene (	IGraphics*			 graphics,
							const ILight::List&	 lights,		// List of all visible lights
							const DrawCallback&	 drawCallback,	// Actual scene drawing callback
							const AOCallback&	 aoCallback		= 0,
							bool				 insideOut		= false);
};