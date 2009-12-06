#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Distant screen-aligned billboard -- always relative to the camera on the farthest edge of the horizon
//============================================================================================================

class DirectionalBillboard : public Billboard
{
protected:

	DirectionalBillboard() {}

public:

	R5_DECLARE_INHERITED_CLASS("Directional Billboard", DirectionalBillboard, Billboard, Object);

protected:

	virtual void OnUpdate() {}
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);
};