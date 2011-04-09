#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Should create the node's topology and update 'mBounds'
//============================================================================================================

void TerrainNode::OnFill (void* ptr, float bboxPadding)
{
	IGraphics* graphics = mTree->GetCore()->GetGraphics();

	if (graphics != 0)
	{
		// Passed parameters used to generate the terrain
		const Terrain::Heightmap* hm	= (const Terrain::Heightmap*)ptr;
		const float*	buffer			= hm->mBufferData;
		const uint		bufferWidth		= hm->mBufferWidth;
		const uint		bufferHeight	= hm->mBufferHeight;
		const uint		meshWidth		= hm->mMeshSize.x < 2 ? 2 : hm->mMeshSize.x;
		const uint		meshHeight		= hm->mMeshSize.y < 2 ? 2 : hm->mMeshSize.y;
		const Vector3f& terrainScale	= hm->mTerrainScale;
		const Vector3f& terrainOffset	= hm->mTerrainOffset;

		// The number of indices is always one less than the number of vertices
		uint indexX = meshWidth  - 1;
		uint indexY = meshHeight - 1;

		Memory mem;

		// Generate all the vertices
		{
			// Scaled value that will convert vertex iterators below into
			// 0-1 range relative to the size of the terrain
			Vector2f iterToRelative (mSize.x / indexX, mSize.y / indexY);

			// Create a temporary buffer
			Vector3f* v = (Vector3f*)mem.Resize(meshWidth * meshHeight * sizeof(Vector3f));

			float fx, fy, wx, wy, wz;

			uint myWidth  = Float::RoundToUInt(mSize.x * bufferWidth);
			uint myHeight = Float::RoundToUInt(mSize.y * bufferHeight);
			bool upsample = (meshWidth > myWidth && meshHeight > myHeight);

			// Fill the buffer with vertices
			for (unsigned int y = 0; y < meshHeight; ++y)
			{
				fy = mOffset.y + iterToRelative.y * y;
				wy = terrainOffset.y + fy * terrainScale.y;

				for (unsigned int x = 0; x < meshWidth; ++x, ++v)
				{
					fx = mOffset.x + iterToRelative.x * x;
					wx = terrainOffset.x + fx * terrainScale.x;

					// Sample the buffer using hermite filtering if upsampling
					wz = upsample ?
						Interpolation::HermiteClamp (buffer, bufferWidth, bufferHeight, fx, fy) :
						Interpolation::BilinearClamp(buffer, bufferWidth, bufferHeight, fx, fy);

					// Set the vertex
					v->Set(wx, wy, wz * terrainScale.z + terrainOffset.z);

					// Include this vertex in the node's bounds
					mBounds.Include(*v);
				}
			}

			// Take padding into consideration
			if (bboxPadding > 0.0f)
			{
				Vector3f min (mBounds.GetMin());
				Vector3f max (mBounds.GetMax());

				min.y -= bboxPadding;
				max.y += bboxPadding;

				mBounds.Include(min);
				mBounds.Include(max);
			}

			if (mVBO == 0) mVBO = graphics->CreateVBO();
			
			// Fill the VBO
			mVBO->Lock();
			mVBO->Set(mem.GetBuffer(), mem.GetSize(), IVBO::Type::Vertex);
			mVBO->Unlock();
		}

		// Generate the indices
		{
			mIndices = indexX * indexY * 4;
			ushort* index = (ushort*)mem.Resize(mIndices * sizeof(short));

			// Fill the index array using quad primitives
			for (unsigned int y = 0; y < indexY; ++y)
			{
				unsigned int y0 = meshWidth * y;
				unsigned int y1 = meshWidth * (y + 1);

				for (unsigned int x = 0; x < indexX; ++x)
				{
					*index = y1 + x;		++index;
					*index = y0 + x;		++index;
					*index = y0 + x + 1;	++index;
					*index = y1 + x + 1;	++index;
				}
			}

			if (mIBO == 0) mIBO = graphics->CreateVBO();

			mIBO->Lock();
			mIBO->Set(mem.GetBuffer(), mem.GetSize(), IVBO::Type::Index);
			mIBO->Unlock();
		}
	}
}

//============================================================================================================
// Draw the object using the specified technique
//============================================================================================================

void TerrainNode::OnDraw (uint group, const ITechnique* tech, bool insideOut)
{
	IGraphics* graphics = mTree->GetCore()->GetGraphics();

	graphics->SetActiveVertexAttribute( IGraphics::Attribute::Vertex,
		mVBO, 0, IGraphics::DataType::Float, 3, 0 );

	graphics->DrawIndices(mIBO, IGraphics::Primitive::Quad, mIndices);
}