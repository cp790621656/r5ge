#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple terrain implementation using QuadTree
//============================================================================================================

class Terrain : public QuadTree
{
public:

	struct Heightmap
	{
		const float*	mBufferData;	// Pointer to the heightmap's buffer
		uint			mBufferWidth;	// Width of the buffer
		uint			mBufferHeight;	// Height of the buffer
		Vector2i		mMeshSize;		// Dimensions of each individual node's generated mesh
		Vector3f		mTerrainScale;	// Scale to apply to the entire terrain (starts at 1,1,1)
		Vector3f		mTerrainOffset;	// Offset to apply to the entire terrain (starts at 0,0,0)

		Heightmap() : mBufferData(0), mBufferWidth(0), mBufferHeight(0), mTerrainScale(1.0f) {}

		Heightmap(const float* buffer, uint width, uint height) :
			mBufferData(buffer), mBufferWidth(width), mBufferHeight(height), mTerrainScale(1.0f) {}
	};

protected:

	const IMaterial* mMat;

	// Objects should never be created manually. Use the AddObject<> template instead.
	Terrain() : mMat(0) {}

public:

	R5_DECLARE_INHERITED_CLASS("Terrain", Terrain, QuadTree, Object);

	// The terrain is usually associated with a material
	const IMaterial* GetMaterial() const { return mMat; }
	void SetMaterial (const IMaterial* mat) { mMat = mat; }

protected:

	// Derived classes must override this function
	virtual QuadNode* _CreateNode() { return new TerrainNode(); }

	// Should return a unique identifier used by the draw queue
	virtual uint GetUID() const { return mMat == 0 ? 0 : mMat->GetUID(); }

	// Should retrieve the technique mask that the terrain can be rendered with (should not include children)
	virtual uint GetMask() const { return mMat != 0 ? mMat->GetTechniqueMask() : 0; }

	// Set up all render states and activate the material before moving down to QuadTree's OnDraw
	virtual uint OnDraw (const Deferred::Storage& storage, uint group, const ITechnique* tech);

	// Called when the object is being saved
	virtual void OnSerializeTo (TreeNode& root) const;

	// Called when the object is being loaded
	virtual bool OnSerializeFrom (const TreeNode& node);
};