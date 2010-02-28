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
	Bounds			 mCullBounds;	// Custom minimum bounds used for culling, set in local space
	mutable bool	 mRecalculate;	// Whether the matrix needs to be recalculated
	mutable Matrix43 mMatrix;		// Calculated world transformation matrix

public:

	ModelInstance() : mModel(0), mRecalculate(false) {}
	~ModelInstance() { SetModel(0); }

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Model Instance", ModelInstance, Object, Object);

public:

	Model*			GetModel()					{ return mModel;		}
	const Model*	GetModel()			const	{ return mModel;		}
	const Bounds&	GetCullBounds()		const	{ return mCullBounds;	}
	const Matrix43&	GetMatrix()			const;

	void SetModel		(Model* model);
	void SetCullBounds	(const Bounds& bounds)	{ mCullBounds = bounds;	mIsDirty = true; }

protected:

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draw the object using the specified technique
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);

	// Serialization to and from the scenegraph tree
	virtual void OnSerializeTo	 (TreeNode& node) const;
	virtual bool OnSerializeFrom (const TreeNode& root);
};