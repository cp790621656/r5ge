#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Distant screen-aligned glare
//============================================================================================================

class DirectionalGlare : public Glare
{
protected:

	DirectionalGlare() {}

public:

	R5_DECLARE_INHERITED_CLASS("Directional Glare", DirectionalGlare, Glare, Object);

protected:

	virtual void OnUpdate();
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);
};