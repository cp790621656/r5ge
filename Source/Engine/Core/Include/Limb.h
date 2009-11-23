#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
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
	IMaterial*	mMat;			// Pointer to the material used by the mesh
	bool		mSerializable;	// Whether this limb needs to be serialized

public:

	R5_DECLARE_SOLO_CLASS("Limb");

	Limb (const String& name) : mName(name), mMesh(0), mMat(0), mSerializable(false) {}

	bool			 IsValid()		  const	{ return (mMesh != 0 && mMat != 0); }
	bool			 IsSerializable() const	{ return mSerializable; }
	const String&	 GetName()		  const	{ return mName;	}
	const Mesh*		 GetMesh()		  const	{ return mMesh;	}
	const IMaterial* GetMaterial()	  const	{ return mMat;	}
	Mesh*			 GetMesh()				{ return mMesh;	}
	IMaterial*		 GetMaterial()			{ return mMat;	}
	
	bool IsVisibleWith	(uint mask) const				{ return (mMesh != 0 && mMat != 0 && ((mMat->GetTechniqueMask() & mask) != 0)); }
	void SetName		(const String& name)			{ if (mName != name && mName.IsValid())	mSerializable = true; mName = name;	}
	void SetMesh		(Mesh* mesh)					{ if (mMesh != mesh && mMesh != 0)		mSerializable = true; mMesh = mesh;	}
	void SetMaterial	(IMaterial* mat)				{ if (mMat  != mat  && mMat  != 0)		mSerializable = true; mMat  = mat;	}
	void Set			(Mesh* mesh, IMaterial* mat)	{ SetMesh(mesh); SetMaterial(mat); }
};