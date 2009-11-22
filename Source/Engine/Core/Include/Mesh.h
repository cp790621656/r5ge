#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Complete drawable mesh
//============================================================================================================

class Mesh
{
public:

	typedef Array<Vector3f>	Vertices;
	typedef Array<Vector3f>	Normals;
	typedef Array<Vector2f>	TexCoords;
	typedef Array<Vector3f>	Tangents;
	typedef Array<Color4f>	BoneWeights;
	typedef Array<Color4ub>	BoneIndices;
	typedef unsigned short	Index;
	typedef Array<Index>	Indices;

	struct VertexFormat
	{
		uint mFullSize;
		uint mTransSize;
		uint mVertex;
		uint mNormal;
		uint mTexCoord;
		uint mTangent;
		uint mBoneIndex;
		uint mBoneWeight;

		VertexFormat() { Clear(); }

		void Clear()
		{
			mFullSize	= 0;
			mTransSize	= 0;
			mVertex		= 0xFFFFFFFF;
			mNormal		= 0xFFFFFFFF;
			mTexCoord	= 0xFFFFFFFF;
			mTangent	= 0xFFFFFFFF;
			mBoneIndex	= 0xFFFFFFFF;
			mBoneWeight = 0xFFFFFFFF;
		}
	};

private:

	String				mName;				// Every mesh needs a unique name
	Thread::Lockable	mLock;				// Thread-safe lock
	mutable bool		mIsLocked;			// Whether the mesh is locked

	Vertices			mV;					// Vertex positions
	Normals				mN;					// Normals
	Tangents			mT;					// Tangents (calculated)
	TexCoords			mTc;				// Texture coordinates
	BoneIndices			mBi;				// Bone indices
	BoneWeights			mBw;				// Bone weights
	uint				mBones;				// Number of bones per vertex
	bool				mGeneratedNormals;	// Whether normals came from a file or were generated

	Vertices			mTv;				// Transformed vertices (software skinning)
	Normals				mTn;				// Transformed normals
	Tangents			mTt;				// Transformed tangents

	VertexFormat		mFormat;			// Current vertex format
	IVBO*				mVbo;				// Vertex buffer object with interleaved vertex information
	IVBO*				mTbo;				// Vertex buffer for transformed coordinates
	uint				mVboSize;			// Size of the VBO in bytes
	uint				mTboSize;			// Size of the transformed VBO in bytes

	Indices				mIndices;			// Primitive indices
	uint				mPrimitive;			// Primitive type for the indices
	IVBO*				mIbo;				// Index buffer object
	uint				mIboSize;			// Number of entries in the IBO
	
	IGraphics*			mGraphics;			// Pointer to the graphics controller managing the VBOs
	Bounds				mBounds;			// Bounding box and sphere

public:

	Mesh(const String& name);
	~Mesh();

	// Static identifier, for consistency
	R5_DECLARE_SOLO_CLASS("Mesh");

	// Thread-safe locking functionality
	void Lock()		const	{ mLock.Lock(); mIsLocked = true; }
	void Unlock()	const	{ mIsLocked = false; mLock.Unlock(); }

	// Clears all memory used by the mesh, but does not release it
	void Clear() { Lock(); _Clear(); Unlock(); }

	// Releases all memory used by the mesh
	void Release();

private:

	// Clears all arrays and resets all flags
	void _Clear();

	// Recalculate min/max/center/radius bounds
	void _RecalculateBounds();

	// Recalculates normals and tangents as requested
	void _CalculateNormalsAndTangents();

public: // Various functions to allow read access to private data

	void			SetName(const String& name)	{ mName = name;	}
	const String&	GetName()			const	{ return mName;	}
	const Bounds&	GetBounds()			const	{ return mBounds; }
	bool			IsValid()			const	{ return (mV.IsValid() && mIndices.IsValid()); }
	uint			GetSizeInMemory()	const;
	uint			GetPrimitive()		const	{ return mPrimitive; }

public: // These functions should only be used after the mesh has been locked

	Vertices&		GetVertexArray()		{ _AssertIfUnlocked(); return mV;		}
	Normals&		GetNormalArray()		{ _AssertIfUnlocked(); return mN;		}
	Tangents&		GetTangentArray()		{ _AssertIfUnlocked(); return mT;		}
	TexCoords&		GetTexCoordArray()		{ _AssertIfUnlocked(); return mTc;		}
	BoneWeights&	GetBoneWeightArray()	{ _AssertIfUnlocked(); return mBw;		}
	BoneIndices&	GetBoneIndexArray()		{ _AssertIfUnlocked(); return mBi;		}
	Indices&		GetIndexArray()			{ _AssertIfUnlocked(); return mIndices;	}

	// Rebuilds the mesh arrays -- use this function after changing the original values via functions above
	void Update (bool rebuildBuffers,		// Whether to rebuild VBOs
				 bool rebuildNormals,		// Whether to recalculate all normals (discards current normals)
				 bool rebuildTangents,		// Whether to recalculate all tangents (discards current tangents)
				 bool texCoordsChanged,		// Whether texture coordinates have changed
				 bool bonesChanged,			// Whether bone information has changed
				 bool indicesChanged);		// Whether indices have changed

	// Change the primitive type
	void SetPrimitive(uint primitive)		{ _AssertIfUnlocked(); mPrimitive = primitive;	}

	// You can manipulate transformed vertices directly -- if present, they will be used instead of the original ones
	// NOTE: You don't need to call 'Rebuild' function after changing the transformed values.
	Vertices&	GetTransformedVertices()	{ _AssertIfUnlocked(); return mTv; }
	Normals&	GetTransformedNormals()		{ _AssertIfUnlocked(); return mTn; }
	Tangents&	GetTransformedTangents()	{ _AssertIfUnlocked(); return mTt; }

private:

	void _AssertIfUnlocked() const { ASSERT(mIsLocked, "You must lock the mesh first!"); }
	bool _TransformToVBO (IGraphics* graphics, const Array<Matrix43>& transforms);
	void _TransformToVAs (const Array<Matrix43>& transforms);

public:

	// Flags field stores specific information accessible via these functions
	bool HasVertices()	const	{ return mV.IsValid() || mTv.IsValid(); }
	bool HasNormals()	const	{ return mN.IsValid() || mTn.IsValid(); }
	bool HasTangents()	const	{ return mT.IsValid() || mTt.IsValid(); }
	bool HasTexCoords()	const	{ return mTc.IsValid();	}
	bool HasBoneInfo()	const	{ return mBw.IsValid() && mBi.IsValid(); }
	bool HasIndices()	const	{ return mIndices.IsValid(); }

	// Returns the number of vertex entries
	uint GetNumberOfVertices() const;

	// Returns the size of the VBO vertex in bytes based on the data we have
	uint GetFinalVertexSize() const { return mFormat.mFullSize; }

	// Returns the number bone weights used by this mesh
	uint GetNumberOfWeights() const	{ return (mBw.IsValid() && (mBw.GetSize() == mBi.GetSize())) ? mBones : 0; }

	// Returns the number of triangles that would be rendered if Render() was called
	uint GetNumberOfTriangles() const;

	// Software skinning on the CPU -- recalculates transformed vertices, normals, and tangents
	bool ApplyTransforms (IGraphics* graphics, const Array<Matrix43>& transforms, uint instances);

	// Discards all current transformed arrays, returning to default values
	void DiscardTransforms() { Lock(); mTv.Clear(); mTn.Clear(); mTt.Clear(); mTboSize = 0; Unlock(); }

	// Renders the mesh
	uint Render (IGraphics* graphics);

	// Serialization
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& root) const;
};