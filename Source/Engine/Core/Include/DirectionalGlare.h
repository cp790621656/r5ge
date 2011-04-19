#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Distant screen-aligned glare
// Author: Michael Lyashenko
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
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);
};