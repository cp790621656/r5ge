#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
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