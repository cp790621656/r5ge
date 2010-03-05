#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic light information
//============================================================================================================

struct Light
{
	// Light type is used to identify light groups
	struct Type
	{
		enum
		{
			Directional	= 1,
			Point		= 2,
		};
	};

	// Active light entry contains a pointer to the light and its calculated distance to the camera
	struct Entry
	{
		Light*	mLight;
		float	mDistance;

		Entry() : mLight(0), mDistance(0.0f) {}
		void operator = (Light* ptr) { mLight = ptr; }
		bool operator < (const Entry& light) const { return mDistance < light.mDistance; }
	};

	typedef Array<Entry> List;

public:

	byte		mType;		// Registered light type (directional, point, or spotlight)
	uint		mSubtype;	// Additional identifier that can be used for a more complex light type
	Vector3f	mPos;		// This light's position
	Vector3f	mDir;		// This light's direction
	Color4f		mAmbient;	// Ambient color
	Color4f		mDiffuse;	// Diffuse color
	Color4f		mSpecular;	// Specular color
	Vector3f	mAtten;		// Light's attenuation for point lights and spot lights
	Vector3f	mSpot;		// Additional spotlight-only parameters

	Light() : mType(Type::Directional), mSubtype(0) {}
};