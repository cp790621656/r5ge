#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic interface for a light source object
//============================================================================================================

struct ILight
{
	// Light type is used to identify light groups
	struct Type
	{
		enum
		{
			Directional	= 0,
			Point		= 1,
			Spot		= 2,
		};
	};

	// Active light entry contains a pointer to the light and its calculated distance to the camera
	struct Entry
	{
		ILight*		mLight;
		float		mDistance;

		Entry() : mLight(0), mDistance(0.0f) {}
		void operator = (ILight* ptr) { mLight = ptr; }
		bool operator < (const Entry& light) const { return mDistance < light.mDistance; }
	};

	typedef Array<Entry> List;

	virtual uint			GetLightType()		const=0;	// Light type, for custom identification
	virtual const Color4f&	GetAmbient()		const=0;	// Ambient light color
	virtual const Color4f&	GetDiffuse()		const=0;	// Diffuse color
	virtual const Color4f&	GetSpecular()		const=0;	// Specular color
	virtual const Vector3f&	GetPosition()		const=0;	// Position of the light for point/spotlights
	virtual const Vector3f&	GetDirection()		const=0;	// Direction, for directional/spotlights
	virtual const Vector3f*	GetAttenParams()	const=0;	// Attenuation parameters, if any
	virtual const Vector3f*	GetSpotParams()		const=0;	// Spot light parameters, if any
};