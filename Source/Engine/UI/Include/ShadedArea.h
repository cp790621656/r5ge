#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Area affected by a shader
//============================================================================================================

class ShadedArea : public Area
{
protected:

	const IShader*			mShader;
	Array<const ITexture*>	mTex;

public:

	ShadedArea() : mShader(0) {}

	const IShader* GetShader() const { return mShader; }
	void SetShader (const IShader* shader);
	
	const ITexture* GetTexture (uint i) const			{ return (mTex.GetSize() > i) ? mTex[i] : 0; }
	void SetTexture (uint index, const ITexture* tex)	{ mTex.ExpandTo(index+1); mTex[index] = tex; }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Shaded Area", ShadedArea, Area, Area);

	// Area functions
	virtual void SetDirty()					{ OnDirty(0, mLayer, this); }
	virtual void OnFill (Queue* queue);

	// Called before and after rendering the queue, respectively
	virtual void OnPreRender (IGraphics* graphics) const;
	virtual void OnPostRender(IGraphics* graphics) const;

	// Serialization
	virtual bool CustomSerializeFrom (const TreeNode& root);
	virtual void CustomSerializeTo (TreeNode& root) const;
};