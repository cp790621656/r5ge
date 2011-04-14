#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Light source -- must be derived from and extended
// Author: Michael Lyashenko
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