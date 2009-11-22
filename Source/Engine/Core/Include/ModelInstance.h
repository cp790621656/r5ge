#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Instanced scene object
//============================================================================================================

class ModelInstance : public Object
{
protected:

	Model*		mModel;			// Pointer to the model that was instanced
	Bounds		mCullBounds;	// Custom minimum bounds used for culling, set in local space
	Bounds		mBounds;		// Calculated world-space bounding volume
	Matrix43	mMatrix;		// Calculated world transformation matrix
	bool		mShowOutline;	// Whether to show the bounding outline -- useful for debugging

public:

	ModelInstance() : mModel(0), mShowOutline(false) {}
	~ModelInstance() { SetModel(0); }

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Model Instance", ModelInstance, Object, Object);

public:

	Model*			GetModel()					{ return mModel;		}
	const Bounds&	GetBounds()			const	{ return mBounds;		}
	const Bounds&	GetCullBounds()		const	{ return mCullBounds;	}
	const Matrix43&	GetMatrix()			const	{ return mMatrix;		}
	bool			IsShowingOutline()	const	{ return mShowOutline;  }

	void SetModel		(Model* model);
	void SetCullBounds	(const Bounds& bounds)	{ mCullBounds = bounds;	mIsDirty = true; }
	void SetShowOutline	(bool val)				{ mShowOutline = val;   }

protected:

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Cull the object based on the viewing frustum
	virtual bool OnCull (CullParams& params, bool isParentVisible, bool render);

	// Render the object using the specified technique
	virtual uint OnRender (IGraphics* graphics, const ITechnique* tech, bool insideOut);

	// Selects the closest object to the given position if the position is within the object's bounds
	virtual void OnSelect (const Vector3f& pos, ObjectPtr& ptr, float& radius);

private:

	// Draws the outline of the bounding box
	uint _DrawOutline (IGraphics* graphics, const ITechnique* tech);

protected:

	// Serialization to and from the scenegraph tree
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& root);
};