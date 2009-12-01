#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Decal (projected texture object)
//============================================================================================================

class Decal : public Object
{
public:


protected:

	Matrix43	mMatrix;
	IShader*	mShader;
	Color4ub	mColor;

	Array<const ITexture*> mTextures;

	// Decal objects should be drawn right after the terrain and before everything else
	Decal() : mShader(0), mColor(0xFFFFFFFF) {}

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Decal", Decal, Object, Object);

	const IShader*	GetShader()	const { return mShader;	}
	const Color4ub&	GetColor()	const { return mColor;	}

	void SetShader	 (IShader* shader);
	void SetColor	 (const Color4ub& val) { mColor = val; }

	Array<const ITexture*>& GetTextureArray() { return mTextures; }

protected:

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Cull the object based on the viewing frustum
	virtual CullResult OnCull (CullParams& params, bool isParentVisible, bool render);

	// Draws the light on-screen if it's visible
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& root) const;
	virtual bool OnSerializeFrom  (const TreeNode& root);
};