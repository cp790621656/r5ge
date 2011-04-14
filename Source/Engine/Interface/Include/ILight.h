#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic light information
// Author: Michael Lyashenko
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