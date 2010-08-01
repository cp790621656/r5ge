//============================================================================================================
//  Defense of the Citadel, Copyright (c) 2010 Michael Lyashenko and Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Helper struct used below
//============================================================================================================

struct ModelInfo
{
	ModelInstance*	inst;
	Mesh*			mesh;
	IMaterial*		mat;

	bool IsValid() const { return (inst != 0) && (mesh != 0) && (mat != 0); }

	ModelInfo() : inst(0), mesh(0), mat(0) {}
};

//============================================================================================================

bool GetModelInfo (ModelInstance* inst, ModelInfo& info)
{
	Model* model = inst->GetModel();
	if (model == 0) return false;

	// The model is only eligible to be batched if it has 1 limb and no animation
	if (model->IsAnimated() || model->GetAllLimbs().GetSize() != 1) return false;

	// Get the first and only limb
	Limb* limb = model->GetAllLimbs()[0];
	if (limb == 0) return false;

	// Save the mesh and material
	info.mesh = limb->GetMesh();
	info.mat  = limb->GetMaterial();

	// Both mesh and material must be present
	return (info.mesh != 0 && info.mat != 0);
}

//============================================================================================================
// Helper function that retrieves the data associated with the model instance
//============================================================================================================

ModelInstanceGroup::TerrainData* ModelInstanceGroup::GetData (ModelInstance* inst, bool create)
{
	Octree::Node* node = (Octree::Node*)inst->GetSubParent();
	ASSERT(node != 0, "SubParent is set to null");
	if (create && node->mData == 0) node->mData = new TerrainData();
	return (TerrainData*)node->mData;
}

//============================================================================================================
// Function called just before the child gets removed
//============================================================================================================

void ModelInstanceGroup::OnRemoveChild (Object* obj)
{
	ModelInstance* inst = R5_CAST(ModelInstance, obj);

	if (inst != 0)
	{
		TerrainData* data = GetData(inst, false);

		if (data != 0)
		{
			ModelInfo info;

			if (GetModelInfo(inst, info))
			{
				FOREACH(i, data->mBatches)
				{
					Batch* batch = data->mBatches[i];

					if (batch->mMat == info.mat)
					{
						batch->mInstances.Remove(inst);
						batch->mIndexCount = 0;
						break;
					}
				}
			}
		}
	}
	Octree::OnRemoveChild(obj);
}

//============================================================================================================
// Called when a newly added object gets positioned somewhere in the octree
//============================================================================================================

void ModelInstanceGroup::OnNewlyAdded (Object* obj)
{
	ModelInfo info;

	// Get detailed information about the model, validating it for batching in the process
	ModelInstance* inst = R5_CAST(ModelInstance, obj);
	if (inst == 0 || !GetModelInfo(inst, info)) return;

	// Get the data struct associated with the instance
	TerrainData* data = GetData(inst, true);
	bool found = false;

	// Check to see if a batch for this material already exists
	FOREACH(b, data->mBatches)
	{
		Batch* batch = data->mBatches[b];

		if (batch->mMat == info.mat)
		{
			// Batch found -- add a new instance
			batch->mInstances.Expand() = inst;
			batch->mIndexCount = 0;
			found = true;
		}
	}

	if (!found)
	{
		// Batch was not found -- add a new one
		typedef Batch* BatchPtr;
		BatchPtr& batch = data->mBatches.Expand();
		batch = new Batch();
		batch->mMat = info.mat;
		batch->mInstances.Expand() = inst;
	}

	// Disable the model in-game so it doesn't show up or react to anything
	inst->SetFlag(Flag::Enabled, false);
}

//============================================================================================================
// We don't want to fill models that have been marked as batched
//============================================================================================================

void ModelInstanceGroup::OnFillNode (Node& node, FillParams& params)
{
	// Fill all non-batched objects
	Octree::OnFillNode(node, params);

	// If the node has associated data, let's work with it
	if (node.mData != 0)
	{
		TerrainData* data = (TerrainData*)node.mData;

		// Run through all batches
		FOREACH(i, data->mBatches)
		{
			Batch* batch = data->mBatches[i];

			// If the batch has instances to work with and is actually visible
			if (batch->mInstances.IsValid() &&
				(batch->mIndexCount == 0 ||
				 params.mFrustum.IsVisible(batch->mBounds)))
			{
				// Add this batch to the draw queue
				params.mDrawQueue.Add(mLayer, this, batch,
					batch->mMat->GetTechniqueMask(),
					batch->mMat->GetUID(), 0.0f);
			}
		}
	}
}

//============================================================================================================
// Draw the batched objects
//============================================================================================================

uint ModelInstanceGroup::OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut)
{
	Batch* batch = (Batch*)param;

	// If the index count was reset, we need to rebuild the VBOs
	if (batch->mIndexCount == 0)
	{
		batch->mBounds.Clear();
		ushort vertexCount = 0;

		Memory& mem = batch->mVertices;
		batch->mStride		 = 0;
		batch->mNormalOffset = 0;
		batch->mTanOffset	 = 0;
		batch->mTexOffset	 = 0;
		batch->mColorOffset  = 0;

		// Run through all instances
		for (uint i = 0, imax = batch->mInstances.GetSize(); i < imax; ++i)
		{
			ModelInstance* inst = batch->mInstances[i];
			const Matrix43& mat = inst->GetMatrix();
			Mesh* mesh = inst->GetModel()->GetAllLimbs()[0]->GetMesh();

			mesh->Lock();
			{
				Mesh::Vertices&		verts	= mesh->GetVertexArray();
				Mesh::Normals&		norms	= mesh->GetNormalArray();
				Mesh::Tangents&		tangs	= mesh->GetTangentArray();
				Mesh::TexCoords&	texs	= mesh->GetTexCoordArray();
				Mesh::Colors&		colors	= mesh->GetColorArray();
				Mesh::Indices&		indices	= mesh->GetIndexArray();

				if (verts.IsValid())
				{
					if (i == 0)
					{
						batch->mStride = 12;
						batch->mPrimitive = mesh->GetPrimitive();

						if (verts.GetSize() == norms.GetSize())
						{
							batch->mNormalOffset = batch->mStride;
							batch->mStride += 12;
						}

						if (verts.GetSize() == tangs.GetSize())
						{
							batch->mTanOffset = batch->mStride;
							batch->mStride += 12;
						}

						if (verts.GetSize() == texs.GetSize())
						{
							batch->mTexOffset = batch->mStride;
							batch->mStride += 8;
						}

						if (verts.GetSize() == colors.GetSize())
						{
							batch->mColorOffset = batch->mStride;
							batch->mStride += 4;
						}
					}

					// Append all of this meshes' vertices to the memory buffer
					for (uint b = 0, bmax = verts.GetSize(); b < bmax; ++b)
					{
						Vector3f vertex (verts[b] * mat);
						mem.Append(vertex);

						if (batch->mNormalOffset != 0) mem.Append(norms[b] % mat);
						if (batch->mTanOffset	 != 0) mem.Append(tangs[b] % mat);
						if (batch->mTexOffset	 != 0) mem.Append(texs[b]);
						if (batch->mColorOffset  != 0) mem.Append(colors[b]);

						batch->mBounds.Include(vertex);
					}

					// Append all of the indices
					for (uint b = 0, bmax = indices.GetSize(); b < bmax; ++b)
					{
						batch->mIndices.Expand() = vertexCount + indices[b];
					}

					ASSERT((vertexCount + verts.GetSize()) < 65535, "Time to implement uint indices...");

					// The next index buffer will start at this offset
					vertexCount += (ushort)verts.GetSize();
					batch->mIndexCount += indices.GetSize();
				}
			}
			mesh->Unlock();
		}

		// Create the VBOs
		if (batch->mIndexCount > 0)
		{
			if (batch->mVBO == 0) batch->mVBO = mGraphics->CreateVBO();
			if (batch->mIBO == 0) batch->mIBO = mGraphics->CreateVBO();

			batch->mVBO->Set(batch->mVertices.GetBuffer(), batch->mVertices.GetSize());
			batch->mIBO->Set(batch->mIndices.GetBuffer(), batch->mIndices.GetSizeInMemory(), IVBO::Type::Index);
		}

		// Clear the temporary arrays
		batch->mVertices.Clear();
		batch->mIndices.Clear();
	}

	// If the batch has nothing to draw or isn't visible with this technique, do nothing
	if (batch->mIndexCount == 0 || !batch->mMat->IsVisibleWith(tech)) return 0;

	// Reset the matrix, set the active material and set up VBOs
	mGraphics->ResetModelMatrix();
	mGraphics->SetActiveMaterial(batch->mMat);
	mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::BoneIndex,  0);
	mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::BoneWeight, 0);

	// Color
	if (batch->mColorOffset == 0)
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Color, 0);
	}
	else
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Color,
			batch->mVBO, batch->mColorOffset, IGraphics::DataType::Byte, 4, batch->mStride);
	}

	// Texture coordinates
	if (batch->mTexOffset == 0)
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord0, 0);
	}
	else
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord0,
			batch->mVBO, batch->mTexOffset, IGraphics::DataType::Float, 2, batch->mStride);
	}

	// Tangents
	if (batch->mTanOffset == 0)
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent, 0);
	}
	else
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent,
			batch->mVBO, batch->mTanOffset, IGraphics::DataType::Float, 3, batch->mStride);
	}

	// Tangents
	if (batch->mTanOffset == 0)
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent, 0);
	}
	else
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent,
			batch->mVBO, batch->mTanOffset, IGraphics::DataType::Float, 3, batch->mStride);
	}

	// Normals
	if (batch->mNormalOffset == 0)
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Normal, 0);
	}
	else
	{
		mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Normal,
			batch->mVBO, batch->mNormalOffset, IGraphics::DataType::Float, 3, batch->mStride);
	}

	// Position
	mGraphics->SetActiveVertexAttribute(IGraphics::Attribute::Position,
		batch->mVBO, 0, IGraphics::DataType::Float, 3, batch->mStride);

	// Draw the instance
	return mGraphics->DrawIndices( batch->mIBO, batch->mPrimitive, batch->mIndexCount );
}