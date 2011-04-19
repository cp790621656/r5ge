#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Projected texture object
// Author: Michael Lyashenko
//============================================================================================================

class ProjectedTexture : public Object
{
protected:

	Matrix43	mMatrix;
	IShader*	mAddSubtract;
	IShader*	mReplace;
	IShader*	mModulate;
	Color4f		mColor;
	ITexture*	mTex;
	byte		mBlending;
	uint		mMask;

	ProjectedTexture() : mAddSubtract(0), mReplace(0), mModulate(0), mColor(1.0f), mTex(0),
		mBlending(ITechnique::Blending::Replace), mMask(0) { mLayer = 5; mCalcAbsBounds = false; }

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Projected Texture", ProjectedTexture, Object, Object);

	const Color4f&	GetColor()		const	{ return mColor;	}
	const ITexture*	GetTexture()	const	{ return mTex;		}
	byte			GetBlending()	const	{ return mBlending; }

	void SetColor	(const Color4f& val)	{ mColor	 = val;	}
	void SetTexture	(ITexture* tex)			{ mTex		 = tex; }
	void SetBlending(byte val)				{ mBlending	 = val; }

protected:

	virtual void OnInit();
	virtual void OnUpdate();
	virtual bool OnFill (FillParams& params);
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& node) const;
	virtual bool OnSerializeFrom  (const TreeNode& node);
};