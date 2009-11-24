#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Static (non-animated) model
//============================================================================================================

class Prop : public ModelTemplate
{
protected:

	uint			mMask;		// Material mask for quick comparison elimination
	mutable Bounds	mBounds;	// Local bounding volume

public:

	Prop (const String& name) : ModelTemplate(name), mMask(0) {}

	// Object creation
	R5_DECLARE_ABSTRACT_CLASS("Prop", ModelTemplate);

public:

	// Returns the material visibility mask
	uint GetMask() const { return mMask; }

	// Returns the number of triangles used by the model
	uint GetNumberOfTriangles();

	// Returns the bounds, recalculating if asked for
	const Bounds& GetBounds (bool recalculate = false) const { return mBounds; }

	// Helper functions that allows checking whether the model is using this material or texture
	bool IsUsingMaterial (const IMaterial* mat) const;
	bool IsUsingTexture  (const ITexture*  tex) const;

	// Recalculates the bounds and the visibility mask
	void Update();

protected:

	// Draw the object using the specified technique
	virtual uint _Draw (IGraphics* graphics, const ITechnique* tech);

	// Draw any special outline of the object
	virtual uint _DrawOutline (IGraphics* graphics, const ITechnique* tech) { return 0; }
};