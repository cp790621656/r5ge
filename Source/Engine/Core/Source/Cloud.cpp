#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Cloud::Cloud(const String& name) :
	mName		(name),
	mGraphics	(0),
	mVbo		(0),
	mVertices	(0),
	mIsDirty	(false) {}

//============================================================================================================

Cloud::~Cloud()
{
	if (mGraphics != 0 && mVbo != 0) mGraphics->DeleteVBO(mVbo);
}

//============================================================================================================
// Releases all memory used by the mesh
//============================================================================================================

void Cloud::Release()
{
	Lock();
	{
		_Clear();
		if (mVbo != 0) mVbo->Release();
	}
	Unlock();
}

//============================================================================================================
// Clears all arrays and resets all flags
//============================================================================================================

void Cloud::_Clear()
{
	mInstances.Clear();
	mV.Clear();
	mBounds.Reset();
}

//============================================================================================================
// Recalculate min/max/center/radius bounds
//============================================================================================================

void Cloud::_RecalculateBounds()
{
	mBounds.Reset();

	for (uint i = 0; i < mInstances.GetSize(); ++i)
	{
		mBounds.Include(mInstances[i].xyz(), mInstances[i].w);
	}
}

//============================================================================================================
// Marks the mesh as needing to be rebuilt
//============================================================================================================

void Cloud::SetDirty()
{
	ASSERT_IF_UNLOCKED;
	_RecalculateBounds();
	mIsDirty = true;
}

//============================================================================================================
// Draws the mesh. Ideally all of this should be done in a geometry shader...
//============================================================================================================

uint Cloud::Draw (IGraphics* graphics)
{
	uint triangles = 0;

	// Clouds are only drawn via a shader for efficiency and simplicity purposes
	const IShader* shader = graphics->GetActiveShader();

	// Update the active shader's uniform for the origin of the mesh
	if (shader != 0 && shader->SetUniform("R5_origin", mOrigin))
	{
		Lock();
		{
			if (mIsDirty)
			{
				mIsDirty = false;
				mV.Clear();

				// 4 vertices per instance
				mVertices = mInstances.GetSize() * 4;

				// Each vertex also has UV texCoords + W size
				mV.Reserve(mVertices * 2);

				// Run through all instances
				for (uint i = 0; i < mInstances.GetSize(); ++i)
				{
					const Vector4f& q = mInstances[i];
					const Vector3f& v = q.xyz();

					mV.Expand() = v;
					mV.Expand().Set(0.0f, 1.0f, q.w);

					mV.Expand() = v;
					mV.Expand().Set(0.0f, 0.0f, q.w);

					mV.Expand() = v;
					mV.Expand().Set(1.0f, 0.0f, q.w);

					mV.Expand() = v;
					mV.Expand().Set(1.0f, 1.0f, q.w);
				}

				// Create the vertex buffer object
				if (mVbo == 0)
				{
					mGraphics = graphics;
					mVbo = graphics->CreateVBO();
				}

				// If we have a VBO, let's use it
				if (mVbo != 0)
				{
					mVbo->Lock();
					mVbo->Set(mV);
					mVbo->Unlock();
					mV.Clear();
				}
			}

			// Unused attributes
			graphics->SetActiveVertexAttribute(IGraphics::Attribute::Tangent,	 0);
			graphics->SetActiveVertexAttribute(IGraphics::Attribute::Normal,	 0);
			graphics->SetActiveVertexAttribute(IGraphics::Attribute::Color,		 0);
			graphics->SetActiveVertexAttribute(IGraphics::Attribute::BoneWeight, 0);
			graphics->SetActiveVertexAttribute(IGraphics::Attribute::BoneIndex,	 0);
			graphics->SetActiveVertexAttribute(IGraphics::Attribute::TexCoord1,	 0);

			// Use the VBO if we were able to create one
			if (mVbo != 0)
			{
				// Texture coordinates
				graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,
					mVbo, 12, IGraphics::DataType::Float, 3, 24 );

				// Vertex positions
				graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,
					mVbo, 0, IGraphics::DataType::Float, 3, 24 );
			}
			else
			{
				float* ptr = &mV[0].x;

				graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, ptr + 3,
					IGraphics::DataType::Float, 3, 24 );

				graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, ptr,
					IGraphics::DataType::Float, 3, 24 );
			}

			// Draw all the billboards
			triangles += graphics->DrawVertices( IGraphics::Primitive::Quad, mVertices );
		}
	}
	Unlock();
	return 0;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Cloud::SerializeTo (TreeNode& root) const
{
	if (IsValid())
	{
		TreeNode& node = root.AddChild(ClassID(), mName);
		node.AddChild("Origin", mOrigin);
		node.AddChild("Instances", mInstances);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Cloud::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];

		if (node.mTag == "Origin")
		{
			if (node.mValue >> mOrigin)
			{
				mIsDirty = true;
			}
		}
		else if (node.mTag == "Instances")
		{
			if (node.mValue >> mInstances)
			{
				mIsDirty = true;
			}
		}
	}
	return true;
}