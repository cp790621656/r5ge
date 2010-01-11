#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Skin is a collection of faces associated with a texture
//============================================================================================================

class UIRoot;
class UISkin
{
protected:

	typedef PointerArray<UIFace> Faces;

	UIRoot*			mUI;
	String			mName;
	bool			mSerializable;
	const ITexture*	mTex;
	Faces			mFaces;

public:

	UISkin (UIRoot* ui, const String& name) : mUI(ui), mName(name), mSerializable(true), mTex(0) {}

	const String&	GetName()	 const	{ return mName; }
	const ITexture* GetTexture() const	{ return mTex;  }

	void SetName	(const String& name)  { mName = name; }
	void SetTexture (const ITexture* tex);

	// Retrieves or creates a face with the specified name
	const UIFace* GetFace(const String& name) const;
	UIFace* GetFace(const String& name, bool create = true);
	
	// Serialization
	bool IsSerializable() const { return mSerializable; }
	void SetSerializable(bool val) { mSerializable = val; }
	bool SerializeFrom (const TreeNode& root);
	bool SerializeTo (TreeNode& root) const;
};