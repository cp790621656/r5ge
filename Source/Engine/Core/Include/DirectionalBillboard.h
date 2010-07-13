#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Distant screen-aligned billboard -- always relative to the camera on the farthest edge of the horizon
//============================================================================================================

class DirectionalBillboard : public Billboard
{
protected:

	// Objects should never be created manually. Use the AddObject<> template instead.
	DirectionalBillboard() {}

public:

	R5_DECLARE_INHERITED_CLASS("Directional Billboard", DirectionalBillboard, Billboard, Object);

protected:

	virtual void OnUpdate() {}
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);
};