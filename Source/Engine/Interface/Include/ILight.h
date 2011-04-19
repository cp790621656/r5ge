#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
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
			Invalid = 0,
			Directional,
			Point,
		};
	};

	byte		mType;		// Registered light type (directional, point, or spotlight)
	Vector3f	mPos;		// This light's position
	Vector3f	mDir;		// This light's direction
	Color4f		mAmbient;	// Ambient color
	Color4f		mDiffuse;	// Diffuse color
	Vector3f	mParams;	// X = range, Y = falloff, Z = unused
	Vector3f	mSpot;		// Additional spotlight-only parameters
	bool		mShadows;	// Whether the light casts shadows

	ILight() : mType(Type::Invalid), mShadows(false) {}
};