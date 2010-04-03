#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Skeleton class used for skinning animation
//============================================================================================================

class Bone
{
public:

	struct PosKey
	{
		uint		mFrame;	// Frame index
		Vector3f	mPos;	// Position at this index

		bool operator < (const PosKey& key) const { return mFrame < key.mFrame; }
	};

	struct RotKey
	{
		uint		mFrame;	// Frame index
		Quaternion	mRot;	// Rotation at this index

		bool operator < (const RotKey& key) const { return mFrame < key.mFrame; }
	};

	typedef Array<PosKey>	PosKeys;
	typedef Array<RotKey>	RotKeys;

private:

	uint		mParent;	// Index of the parent bone, -1 if none
	String		mName;		// This bone's human-readable name
	Vector3f	mPos;		// Original position
	Quaternion	mRot;		// Original rotation
	PosKeys		mPosKeys;	// Position keys
	RotKeys		mRotKeys;	// Rotation keys
	bool		mSmoothPos;	// Whether to use spline interpolation for positions
	bool		mSmoothRot;	// Whether to use spline interpolation for rotations

public:

	R5_DECLARE_SOLO_CLASS("Bone");

	Bone() : mParent(-1) {}

	void Release();

	void SetName		(const String& name)		{ mName		= name;	}
	void SetParent		(uint index)				{ mParent	= index;}
	void SetPosition	(const Vector3f& v)			{ mPos		= v;	}
	void SetRotation	(const Quaternion& q)		{ mRot		= q;	}
	
	void SetUseSplinesForPositions	(bool val)		{ mSmoothPos = val;	}
	void SetUseSplinesForRotations	(bool val)		{ mSmoothRot = val;	}

	bool IsUsingSplinesForPositions() const			{ return mSmoothPos;}
	bool IsUsingSplinesForRotations() const			{ return mSmoothRot;}

	uint				GetParent()		const		{ return mParent;	}
	const String&		GetName()		const		{ return mName;		}
	const Vector3f&		GetPosition()	const		{ return mPos;		}
	const Quaternion&	GetRotation()	const		{ return mRot;		}
		  PosKeys&		GetAllPosKeys()				{ return mPosKeys;	}
	const PosKeys&		GetAllPosKeys()	const		{ return mPosKeys;	}
		  RotKeys&		GetAllRotKeys()				{ return mRotKeys;	}
	const RotKeys&		GetAllRotKeys()	const		{ return mRotKeys;	}
	uint				GetFirstFrame()	const;
	uint				GetLastFrame()	const;

	// These functions are not thread-safe, and are meant to be used responsibly
	PosKey* GetPosKey	(uint frame, bool createIfMissing = false);
	RotKey* GetRotKey	(uint frame, bool createIfMissing = false);
	bool DeletePosKey	(uint frame);
	bool DeleteRotKey	(uint frame);

	// Serialization
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	bool SerializeTo (TreeNode& node) const;
};