#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Base class for the material
//============================================================================================================

struct IMaterial
{
	R5_DECLARE_INTERFACE_CLASS("Material");

	typedef Array<const ITexture*> Textures;

public:

	// Material options differ for every rendering technique
	class DrawMethod
	{
	protected:

		const ITechnique*	mTechnique;
		IShader*			mShader;
		Textures			mTextures;

	public:

		DrawMethod() : mTechnique(0), mShader(0) {}

		void Clear() { mTechnique = 0; mShader = 0; for (uint i = 0; i < mTextures.GetSize(); ++i) mTextures[i] = 0; }

		void				SetTechnique (const ITechnique* tech)	{ mTechnique = tech; }
		const ITechnique*	GetTechnique() const					{ return mTechnique; }
		void				SetShader(IShader* shader)				{ mShader = shader; }
		IShader*			GetShader()								{ return mShader; }
		const IShader*		GetShader() const						{ return mShader; }

		void				SetTexture (uint textureUnit, const ITexture* tex)	{ mTextures.ExpandTo(textureUnit+1); mTextures[textureUnit] = tex;}
		const ITexture*		GetTexture (uint textureUnit) const					{ return (textureUnit < mTextures.GetSize()) ? mTextures[textureUnit] : 0; }
		const Textures&		GetAllTextures() const								{ return mTextures; }
	};

public:

	typedef Array<DrawMethod> DrawMethods;

	virtual ~IMaterial() {};

	virtual void Release()=0;

	virtual const String&	GetName()		const=0;
	virtual const Color&	GetDiffuse()	const=0;	// Diffuse color
	virtual const Color&	GetSpecular()	const=0;	// Specular color
	virtual float			GetGlow()		const=0;	// Percentage of Color unaffected by diminishing light
	virtual float			GetADT()		const=0;	// Alpha Discard Threshold for alpha testing

	virtual void SetName		(const String& name)=0;
	virtual void SetDiffuse		(const Color& color)=0;
	virtual void SetSpecular	(const Color& color)=0;
	virtual void SetGlow		(float val)=0;
	virtual void SetADT			(float val)=0;

	// Returns a mask of available techniques
	virtual uint				GetTechniqueMask()	 const=0;
	virtual const DrawMethods&	GetAllDrawMethods()  const=0;
	virtual		  DrawMethod*	GetDrawMethod		(const ITechnique* t, bool createIfMissing = true)=0;
	virtual const DrawMethod*	GetVisibleMethod	(const ITechnique* t) const=0;
	virtual		  DrawMethod*	GetVisibleMethod	(const ITechnique* t)=0;
	virtual void				DeleteDrawMethod	(const ITechnique* t)=0;
	virtual void				ClearAllDrawMethods()=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;

	// Helper function that can be used to determine whether the material can be drawn with this technique
	inline bool IsVisibleWith (const ITechnique* tech) const
	{
		return (GetDiffuse().GetColor4ub().a != 0) && ((GetTechniqueMask() & tech->GetMask()) != 0);
	}
};