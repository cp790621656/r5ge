#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Light source -- must be derived from and extended
//============================================================================================================

class LightSource : public Object
{
protected:

	ILight mProperties;

public:

	R5_DECLARE_ABSTRACT_CLASS("LightSource", Object);

	ILight& GetProperties() { return mProperties; }
	const ILight& GetProperties() const { return mProperties; }

	// Draw the light
	virtual void OnDrawLight (TemporaryStorage& storage, bool setStates)=0;
};