#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Similar static model instance children of this object will be drawn in fewer draw calls
// Author: Michael Lyashenko
//============================================================================================================

class ModelInstanceGroup : public Octree
{
protected:

	// All meshes using the same material will be drawn into a single set of VBOs
	struct Batch
	{
		typedef Array<ModelInstance*> Instances;

		IMaterial*		mMat;		// Material used by this batch
		Instances		mInstances;	// List of all instances that will be rendered
		Memory			mVertices;	// Temporary memory used by the VBO
		Array<ushort>	mIndices;	// Temporary memory used by the IBO
		Bounds			mBounds;	// Bounds used to cull the batch

		IVBO*	mVBO;
		IVBO*	mIBO;
		uint	mStride;
		uint	mIndexCount;
		uint	mPrimitive;
		uint	mNormalOffset;
		uint	mTanOffset;
		uint	mTexOffset;
		uint	mColorOffset;

		Batch() : mMat(0), mVBO(0), mIBO(0), mStride(0), mIndexCount(0), mPrimitive(0),
			mNormalOffset(0), mTanOffset(0), mTexOffset(0), mColorOffset(0) {}
	};

	// Custom data struct containing batched mesh data
	struct TerrainData : Octree::Data
	{
		PointerArray<Batch> mBatches;
		virtual ~TerrainData() {}
	};

public:

	R5_DECLARE_INHERITED_CLASS(ModelInstanceGroup, Octree, Object);

protected:

	// Helper function that retrieves the data associated with the model instance
	TerrainData* GetData (ModelInstance* inst, bool create);

	// Called when a newly added object gets positioned somewhere in the octree
	virtual void OnNewlyAdded (Object* obj);

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj);

	// We don't want to fill models that have been marked as batched
	virtual void OnFillNode (Node& node, FillParams& params);

	// Draw the batched objects
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);
};