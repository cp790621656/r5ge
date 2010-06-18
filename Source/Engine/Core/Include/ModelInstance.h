#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Instanced scene object
//============================================================================================================

class ModelInstance : public Object
{
protected:

	Model*			 mModel;		// Pointer to the model that was instanced
	mutable bool	 mRecalculate;	// Whether the matrix needs to be recalculated
	mutable Matrix43 mMatrix;		// Calculated world transformation matrix

	// Objects should never be created manually. Use the AddObject<> template instead.
	ModelInstance() : mModel(0), mRecalculate(false) {}

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Model Instance", ModelInstance, Object, Object);

	~ModelInstance() { SetModel(0); }

	Model*			GetModel()			{ return mModel; }
	const Model*	GetModel()	const	{ return mModel; }
	const Matrix43&	GetMatrix()	const;

	void SetModel (Model* model, bool runOnSerialize = true, bool threadSafe = true);

protected:

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draw the object using the specified technique
	virtual uint OnDraw (const Deferred::Storage& storage, uint group, const ITechnique* tech);

	// Serialization to and from the scenegraph tree
	virtual void OnSerializeTo	 (TreeNode& node) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};