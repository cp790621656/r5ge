#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Base class for the material
// Author: Michael Lyashenko
//============================================================================================================

struct IMaterial
{
	R5_DECLARE_INTERFACE_CLASS("Material");

	typedef Array<const ITexture*> Textures;

	// Material options differ for every rendering technique
	struct DrawMethod
	{
		const ITechnique*	mTechnique;
		IShader*			mShader;
		Textures			mTextures;

		// NOTE: Deferred and shadowed versions of the shader will be set (and chosen) automatically by the
		// graphics controller when activating a shader if the shader happens to be a surface shader.

		DrawMethod() : mTechnique(0), mShader(0) {}

		void Clear()
		{
			mTechnique		= 0;
			mShader			= 0;
			mTextures.Release();
		}

		void SetTexture (uint textureUnit, const ITexture* tex)
		{
			mTextures.ExpandTo(textureUnit+1);
			mTextures[textureUnit] = tex;
		}

		const ITexture* GetTexture (uint textureUnit) const
		{
			return (textureUnit < mTextures.GetSize()) ? mTextures[textureUnit] : 0;
		}
	};

	typedef Array<DrawMethod> DrawMethods;

protected:

	uint mUID;

	IMaterial() : mUID(GenerateUID()) {}

public:

	// Retrieves a unique identifier for this texture
	uint GetUID() const { return mUID; }

	virtual ~IMaterial() {};

	virtual void Release()=0;

	// Name of the material
	virtual const String& GetName() const=0;

	// Diffuse color
	virtual const Color4f& GetDiffuse() const=0;

	// Percentage of Color unaffected by diminishing light
	virtual float GetGlow() const=0;

	// How much the specular component is affected by material's color
	// Specular Color = mix(specular, specular * diffuse, hue)
	virtual float GetSpecularHue() const=0;

	// Material's specularity (0.0 = No specular highlights, 1.0 = Full specular highlights)
	virtual float GetSpecularity() const=0;

	// Material's shininess (0.0 = dull specularity, 1.0 = sharp specularity)
	virtual float GetShininess() const=0;

	// Material's reflectiveness, used by certain shaders
	virtual float GetReflectiveness() const=0;

	// How much the material should be affected by ambient occlusion (0.0 = Not at all, 1.0 = Fully)
	virtual float GetOcclusion() const=0;

	// Pixels will be discarded if their alpha goes below this value
	virtual float GetAlphaCutoff() const=0;

	virtual void SetName			(const String& name)=0;
	virtual void SetDiffuse			(const Color4f& color)=0;
	virtual void SetGlow			(float val)=0;
	virtual void SetSpecularHue		(float val)=0;
	virtual void SetSpecularity		(float val)=0;
	virtual void SetShininess		(float val)=0;
	virtual void SetReflectiveness	(float val)=0;
	virtual void SetOcclusion		(float val)=0;
	virtual void SetAlphaCutoff		(float val)=0;

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
		return (GetDiffuse().a > FLOAT_TOLERANCE) && ((GetTechniqueMask() & tech->GetMask()) != 0);
	}
};