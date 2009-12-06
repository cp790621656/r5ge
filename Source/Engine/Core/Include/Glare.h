#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Screen-aligned billboard that fades out if it's obscured
//============================================================================================================

class Glare : public Billboard
{
protected:

	Vector3f	mAlpha;			// Alpha factors for smooth interpolation (start, current, target)
	float		mTimeStamp;		// Timestamp used for gradual alpha changes

	Glare() : mTimeStamp(0.0f) {}

public:

	R5_DECLARE_INHERITED_CLASS("Glare", Glare, Billboard, Object);

protected:

	// Changes the alpha value that the glare animates to
	void _SetTargetAlpha (float target);

	// If the alpha is changing, interpolate the value
	virtual void OnUpdate();

	// Draw the glare
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);
};