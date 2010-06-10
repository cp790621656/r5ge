#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Distant screen-aligned glare
//============================================================================================================

class DirectionalGlare : public Glare
{
protected:

	// Objects should never be created manually. Use the AddObject<> template instead.
	DirectionalGlare() {}

public:

	R5_DECLARE_INHERITED_CLASS("Directional Glare", DirectionalGlare, Glare, Object);

protected:

	virtual void OnUpdate();
	virtual uint OnDraw (uint group, const ITechnique* tech, bool insideOut);
};