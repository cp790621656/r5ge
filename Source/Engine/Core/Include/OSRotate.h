#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Script that rotates the owner
//============================================================================================================

class OSRotate : public Script
{
protected:

	Vector3f mAxis;
	float mRate;

	// Use the AddScript<> template to add new scripts
	OSRotate() : mAxis(0.0f, 0.0f, 1.0f), mRate(1.0f) {}

public:

	R5_DECLARE_INHERITED_CLASS("OSRotate", OSRotate, Script, Script);

	const Vector3f& GetAxis() const { return mAxis; }
	float GetRate() const { return mRate; }

	void SetAxis (const Vector3f& axis) { mAxis = axis; }
	void SetRate (float rate) { mRate = rate; }

	// Rotate the owner
	virtual void OnPreUpdate();

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const;
	virtual void OnSerializeFrom(const TreeNode& node);
};