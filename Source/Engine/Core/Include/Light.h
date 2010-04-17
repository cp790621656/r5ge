#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Light source
//============================================================================================================

struct Light : ILight
{
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

	typedef FastDelegate<void (IGraphics* graphics, const List& lights, const ITexture* lightmap)> DrawCallback;

private:

	// Registers a new light source type: use the function template instead
	static void _Register (uint subType, const DrawCallback& callback);

public:

	// Registers a new light source type that will be used for deferred rendering
	template <typename T> static void Register() { _Register( HashKey(T::ClassID()), &T::_Draw ); }

	// Draw the lights
	static void Draw(IGraphics*			graphics,
					 const ITexture*	depth,
					 const ITexture*	normal,
					 const ITexture*	lightmap,
					 const List&		lights);
};