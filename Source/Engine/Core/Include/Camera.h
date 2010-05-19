#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Base camera class
//============================================================================================================

class Camera : public Object
{
protected:

	Vector3f mRelativeRange;	// X = Near, Y = Far, Z = FOV
	Vector3f mAbsoluteRange;

public:

	Camera() : mRelativeRange(0.3f, 100.0f, 90.0f) {}

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Camera", Camera, Object, Object);

	const Vector3f& GetRelativeRange() const { return mRelativeRange; }
	const Vector3f& GetAbsoluteRange() const { return mAbsoluteRange; }

	void SetRelativeRange (const Vector3f& range) { mRelativeRange = range; }
	void SetAbsoluteRange (const Vector3f& range);

protected:

	// Custom overrideable function called after the node's absolute coordinates have been calculated
	virtual void OnUpdate();

public:

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};