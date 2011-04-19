#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Area affected by a shader
// Author: Michael Lyashenko
//============================================================================================================

class UIShadedArea : public UIWidget
{
protected:

	const IShader*			mShader;
	Array<const ITexture*>	mTex;

public:

	UIShadedArea() : mShader(0) {}

	const IShader* GetShader() const { return mShader; }
	void SetShader (const IShader* shader);
	
	const ITexture* GetTexture (uint i) const			{ return (mTex.GetSize() > i) ? mTex[i] : 0; }
	void SetTexture (uint index, const ITexture* tex)	{ mTex.ExpandTo(index+1); mTex[index] = tex; }

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIShadedArea", UIShadedArea, UIWidget, UIWidget);

	// Area functions
	virtual void SetDirty()	{ OnDirty(0, mLayer, this); }
	virtual void OnFill (UIQueue* queue);

	// Called before and after rendering the queue, respectively
	virtual void OnPreDraw (IGraphics* graphics) const;
	virtual void OnPostDraw(IGraphics* graphics) const;

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};