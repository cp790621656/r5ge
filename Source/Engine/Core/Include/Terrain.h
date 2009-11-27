#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Simple terrain implementation using QuadTree
//============================================================================================================

class Terrain : public QuadTree
{
public:

	struct Heightmap
	{
		const float*	mBufferData;	// Pointer to the heightmap's buffer
		Vector2i		mBufferSize;	// Width and height of the buffer
		Vector2i		mMeshSize;		// Dimensions of each individual node's generated mesh
		Vector3f		mTerrainScale;	// Scale to apply to the entire terrain (starts at 1,1,1)
		Vector3f		mTerrainOffset;	// Offset to apply to the entire terrain (starts at 0,0,0)

		Heightmap() : mBufferData(0), mTerrainScale(1.0f) {}

		Heightmap(const float* buffer, const Vector2i& size) : mTerrainScale(1.0f)
		{
			mBufferData = buffer;
			mBufferSize = size;
		}
	};

protected:

	// Material used by the terrain
	const IMaterial* mMat;

public:

	R5_DECLARE_INHERITED_CLASS("Terrain", Terrain, QuadTree, Object);

	// The terrain is usually associated with a material
	const IMaterial* GetMaterial() const { return mMat; }
	void SetMaterial (const IMaterial* mat) { mMat = mat; }

protected:

	// Derived classes must override this function
	virtual QuadNode* _CreateNode() { return new TerrainNode(); }
};