#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Static (non-animated) model
//============================================================================================================

class Prop : public ModelTemplate
{
public:

	Prop (const String& name) : ModelTemplate(name) {}

	// Object creation
	R5_DECLARE_ABSTRACT_CLASS("Prop", ModelTemplate);

public:

	// Returns the number of triangles used by the model
	uint GetNumberOfTriangles();

	// Helper functions that allows checking whether the model is using this material or texture
	bool IsUsingMaterial (const IMaterial* mat) const;
	bool IsUsingTexture  (const ITexture*  tex) const;

protected:

	// Draw the object using the specified technique
	virtual uint _Draw (uint group, IGraphics* graphics, const ITechnique* tech, Limb* limb = 0);

	// Draw any special outline of the object
	virtual uint _DrawOutline (IGraphics* graphics, const ITechnique* tech) { return 0; }
};