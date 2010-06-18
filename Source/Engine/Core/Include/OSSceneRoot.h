#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Represents the root of the scene that's used by other scripts
//============================================================================================================

class OSSceneRoot : public Script
{
public:

	typedef FastDelegate<ITexture* (const Matrix44& imvp, const ITexture* depth,
		float near, float far)> DrawShadowCallback;

	typedef Array<DrawShadowCallback> DrawShadows;

protected:

	// Shadow-creating lights
	DrawShadows mShadows;

public:

	R5_DECLARE_INHERITED_CLASS("OSSceneRoot", OSSceneRoot, Script, Script);

	// Find the root of the specified parent object, creating a new one if necessary
	static OSSceneRoot* FindRootOf (Object* parent);

	// Add/remove shadow creating callbacks
	void AddShadow	  (const DrawShadowCallback& callback) { mShadows.AddUnique(callback); }
	void RemoveShadow (const DrawShadowCallback& callback) { mShadows.Remove(callback); }

	// Read-only access to shadow callbacks
	const Array<DrawShadowCallback>& GetAllShadows() const { return mShadows; }
};