#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Instanced scene object
// Author: Michael Lyashenko
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

	void SetModel (Model* model, bool runOnSerialize = true);

protected:

	// Try to set the model automatically
	virtual void OnInit();

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draw the object using the specified technique
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);

	// Serialization to and from the scenegraph tree
	virtual void OnSerializeTo	 (TreeNode& node) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};