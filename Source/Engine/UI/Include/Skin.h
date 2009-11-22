#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Skin is a collection of faces associated with a texture
//============================================================================================================

class Root;
class Skin
{
protected:

	Root*				mUI;
	String				mName;
	bool				mSerializable;
	const ITexture*		mTex;
	PointerArray<Face>	mFaces;

public:

	Skin (Root* ui, const String& name) : mUI(ui), mName(name), mSerializable(true), mTex(0) {}

	const String&	GetName()	 const	{ return mName; }
	const ITexture* GetTexture() const	{ return mTex;  }

	void SetName	(const String& name)  { mName = name; }
	void SetTexture (const ITexture* tex);

	// Retrieves or creates a face with the specified name
	const Face* GetFace(const String& name) const;
	Face* GetFace(const String& name, bool create = true);
	
	// Serialization
	bool IsSerializable() const { return mSerializable; }
	void SetSerializable(bool val) { mSerializable = val; }
	bool SerializeFrom (const TreeNode& root);
	bool SerializeTo (TreeNode& root) const;
};