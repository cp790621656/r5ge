#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Simple screen-aligned billboard
//============================================================================================================

class Billboard : public Object
{
protected:

	Color4ub			mColor;		// Color tint
	const ITexture*		mTex;		// Texture used to draw the Billboard
	const ITechnique*	mTech;		// Technique used by the billboard

	Billboard() : mColor(0xFFFFFFFF), mTex(0), mTech(0) { mCalcAbsBounds = false; }

public:

	R5_DECLARE_INHERITED_CLASS("Billboard", Billboard, Object, Object);

	const Color4ub&		GetColor()		const	{ return mColor;	}
	const ITexture*		GetTexture()	const	{ return mTex;		}
	const ITechnique*	GetTechnique()	const	{ return mTech;		}

	void SetColor		(const Color4ub& color)	{ mColor = color;	}
	void SetTexture		(const ITexture* tex)	{ mTex = tex;		}
	void SetTechnique	(const ITechnique* tech){ mTech = tech;		}

protected:

	virtual void OnUpdate();
	virtual bool OnFill (FillParams& params);
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);

	// Draws the actual billboard reusing the common buffers
	void DrawBillboard();

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& root);
};