#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic light information
//============================================================================================================

struct ILight
{
	// ILight type is used to identify light groups
	struct Type
	{
		enum
		{
			Directional	= 1,
			Point		= 2,
		};
	};

	byte		mType;		// Registered light type (directional, point, or spotlight)
	Vector3f	mPos;		// This light's position
	Vector3f	mDir;		// This light's direction
	Color4f		mAmbient;	// Ambient color
	Color4f		mDiffuse;	// Diffuse color
	Color4f		mSpecular;	// Specular color
	Vector3f	mAtten;		// ILight's attenuation for point lights and spot lights
	Vector3f	mSpot;		// Additional spotlight-only parameters
	bool		mShadows;	// Whether the light casts shadows

	ILight() : mType(Type::Directional), mShadows(false) {}
};