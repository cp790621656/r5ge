#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Skin is a collection of faces associated with a texture
// Author: Michael Lyashenko
//============================================================================================================

class UIManager;
class UISkin
{
protected:

	typedef PointerArray<UIFace> Faces;

	UIManager*		mUI;
	String			mName;
	bool			mSerializable;
	const ITexture*	mTex;
	Faces			mFaces;

public:

	UISkin (UIManager* ui, const String& name) : mUI(ui), mName(name), mSerializable(true), mTex(0) {}

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