#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Mesh made up of screen-facing billboards (tree canopy, for example)
//============================================================================================================

class Cloud : public Thread::Lockable
{
	String				mName;			// Every mesh needs a unique name
	Thread::Lockable	mLock;			// Thread-safe lock

	Vector3f			mOrigin;		// Origin all billboards are relative to, used to calculate normals
	Array<Vector4f>		mInstances;		// Serialized billboard instances (XYZ position + W size)
	Array<Vector3f>		mV;				// Vertices (calculated -- XYZ position, TexCoord UV, W size)

	IGraphics*			mGraphics;		// Pointer to the graphics controller managing the VBOs
	Bounds				mBounds;		// Bounding box and sphere
	IVBO*				mVbo;			// Vertex buffer object with interleaved vertex information
	uint				mVertices;		// Number of vertices in the VBO
	bool				mIsDirty;		// Whether the calculated buffers need to be rebuilt

public:

	Cloud(const String& name);
	~Cloud();

	// Static identifier, for consistency
	R5_DECLARE_SOLO_CLASS("Cloud");

	// Clears all memory used by the mesh, but does not release it
	void Clear() { ASSERT_IF_UNLOCKED; _Clear(); }

	// Releases all memory used by the mesh
	void Release();

private:

#ifdef _DEBUG
	void ASSERT_IF_UNLOCKED const { ASSERT(IsLocked(), "You must lock the mesh first!"); }
#endif

	// Clears all arrays and resets all flags
	void _Clear();

	// Recalculate min/max/center/radius bounds
	void _RecalculateBounds();

public: // Various functions to allow read access to private data

	void			SetName(const String& name)	{ mName = name;	}
	const String&	GetName()			const	{ return mName;	}
	const Bounds&	GetBounds()			const	{ return mBounds; }
	const Vector3f&	GetOrigin()			const	{ return mOrigin; }
	uint			GetSizeInMemory()	const	{ return mInstances.GetSize() * 96; }
	bool			IsValid()			const	{ return mInstances.IsValid(); }

public: // These functions should only be used after the mesh has been locked

	void SetOrigin(const Vector3f& origin) { ASSERT_IF_UNLOCKED; mOrigin = origin; }

	// Retrieves the array of billboard instances
	Array<Vector4f>& GetInstanceArray() { ASSERT_IF_UNLOCKED; return mInstances; }

	// Marks the mesh as needing to be rebuilt
	void SetDirty();

public:

	// Draws the mesh
	uint Draw (IGraphics* graphics);

	// Serialization
	bool SerializeTo (TreeNode& root) const;
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
};