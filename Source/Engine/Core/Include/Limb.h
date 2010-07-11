#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Models are made up of limbs that link meshes and materials together.
//============================================================================================================

class Limb
{
	friend class Prop;
	friend class ModelTemplate;

protected:

	String		mName;			// Name of the limb
	Mesh*		mMesh;			// Pointer to a mesh used by the limb
	Cloud*		mCloud;			// Alternatively, a cloud mesh can be specified instead
	IMaterial*	mMat;			// Pointer to the material used by the mesh
	bool		mIsVisible;		// Whether the limb is currently visible
	bool		mSerializable;	// Whether this limb needs to be serialized

public:

	R5_DECLARE_SOLO_CLASS("Limb");

	Limb (const String& name) : mName(name), mMesh(0), mCloud(0), mMat(0), mIsVisible(false), mSerializable(false) {}

	bool				IsValid()			const	{ return (mMesh != 0 || mCloud != 0) && (mMat != 0); }
	bool				IsVisible()			const	{ return mIsVisible; }
	bool				IsSerializable()	const	{ return mSerializable; }
	const String&		GetName()			const	{ return mName;	}
	const Mesh*			GetMesh()			const	{ return mMesh;	}
	const Cloud*		GetCloud()			const	{ return mCloud; }
	const IMaterial*	GetMaterial()		const	{ return mMat;	}
	Mesh*				GetMesh()					{ return mMesh;	}
	Cloud*				GetCloud()					{ return mCloud; }
	IMaterial*			GetMaterial()				{ return mMat;	}
	
	bool IsVisibleWith	(uint mask) const		{ return mIsVisible && IsValid() && ((mMat->GetTechniqueMask() & mask) != 0); }
	void SetName		(const String& name)	{ if (mName		!= name && mName.IsValid())	mSerializable = true; mName		= name;	}
	void SetMesh		(Mesh* mesh)			{ if (mMesh		!= mesh && mMesh	!= 0)	mSerializable = true; mMesh		= mesh;	}
	void SetMesh		(Cloud* bm)				{ if (mCloud	!= bm	&& mCloud	!= 0)	mSerializable = true; mCloud	= bm;	}
	void SetMaterial	(IMaterial* mat)		{ if (mMat		!= mat  && mMat		!= 0)	mSerializable = true; mMat		= mat;	}
	void SetVisible		(bool val)				{ mIsVisible = val; }
	
	// Convenience functions
	void Set (Mesh* mesh, IMaterial* mat)	{ SetMesh(mesh); SetMaterial(mat); }
	void Set (Cloud* mesh, IMaterial* mat)	{ SetMesh(mesh); SetMaterial(mat); }
};