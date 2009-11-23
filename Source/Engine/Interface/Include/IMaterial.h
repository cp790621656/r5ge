#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Base class for the material
//============================================================================================================

struct IMaterial
{
	R5_DECLARE_INTERFACE_CLASS("Material");

	typedef Array<const ITexture*> Textures;

public:

	// Material options differ for every rendering technique
	class RenderingMethod
	{
	protected:

		const ITechnique*	mTechnique;
		IShader*			mShader;
		Textures			mTextures;

	public:

		RenderingMethod() : mTechnique(0), mShader(0) {}

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

	typedef Array<RenderingMethod> RenderingMethods;

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

	// Returns a mask of available techniques, based on (1 << ITechnique::GetID)
	virtual uint					GetTechniqueMask()		 const=0;
	virtual const RenderingMethods&	GetAllRenderingMethods() const=0;
	virtual		  RenderingMethod*	GetRenderingMethod		(const ITechnique* t, bool createIfMissing = true)=0;
	virtual const RenderingMethod*	GetVisibleMethod		(const ITechnique* t) const=0;
	virtual		  RenderingMethod*	GetVisibleMethod		(const ITechnique* t)=0;
	virtual void					DeleteRenderingMethod	(const ITechnique* t)=0;
	virtual void					ClearAllRenderingMethods()=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;
};