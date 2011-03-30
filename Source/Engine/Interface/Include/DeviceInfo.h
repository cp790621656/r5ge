#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Device information regarding capabilities of the videocard
//============================================================================================================

struct DeviceInfo
{
	struct Vendor
	{
		enum
		{
			Unknown	= 0,
			NVidia,
			ATI,
			Intel,
		};
	};

	float	mVersion;					// Detected OpenGL version
	byte	mVendor;					// Videocard vendor
	bool	mFloat16Format;				// Support for 16-bit floating point textures (half precision)
	bool	mFloat32Format;				// Support for 32-bit floating point textures (full precision)
	bool	mBufferObjects;				// Support for buffer objects (VBOs, FBOs)
	bool	mDrawBuffers;				// Support for multiple color-out attachments for Frame Buffer Objects
	bool	mDepthStencil;				// Support for Depth24-Stencil8 textures and buffers
	bool	mDXTCompression;			// Support for DXT compression
	bool	mDepthAttachments;			// Support for depth-only attachments for FBOs
	bool	mAlphaAttachments;			// Support for alpha textures assigned to FBO as color attachments
	bool	mMixedAttachments;			// Support for mixed format FBO color attachments
	bool	mOcclusion;					// Support for occlusion queries
	bool	mShaders;					// Support for GLSL shaders
	bool	mGeometryShaders;			// Support for geometry shaders (GeForce 8+)
	uint	mMaxTextureUnits_FFP;		// Maximum number of texture units that can be used with the fixed-function pipeline
	uint	mMaxTextureUnits_Shader;	// Maximum number of texture units that can be used in shaders
	uint	mMaxTextureCoords;			// Maximum number of texture coordinate arrays
	uint	mMaxTextureSize;			// Maximum texture width and height
	uint	mMaxLights;					// Maximum number of hardware lights
	uint	mMaxFBOAttachments;			// Maximum color attachments for FBOs
	uint	mMaxAnisotropicLevel;		// Maximum supported anisotropic filtering level
	uint	mTextureMemory;				// Amount of videocard memory currently used by textures
	uint	mBufferMemory;				// Amount of videocard memory currently used by VBOs
	uint	mMaxTextureMemory;			// Maximum amount of videocard memory used by textures
	uint	mMaxBufferMemory;			// Maximum amount of videocard memory used by VBOs
	uint	mMaxMemory;					// Maximum amount of combined memory used

	DeviceInfo() :
		mVersion				(1.0f),
		mVendor					(Vendor::Unknown),
		mFloat16Format			(false),
		mFloat32Format			(false),
		mBufferObjects			(false),
		mDrawBuffers			(false),
		mDepthStencil			(false),
		mDXTCompression			(false),
		mDepthAttachments		(false),
		mAlphaAttachments		(false),
		mMixedAttachments		(false),
		mOcclusion				(false),
		mShaders				(false),
		mGeometryShaders		(false),
		mMaxTextureUnits_FFP	(0),
		mMaxTextureUnits_Shader	(0),
		mMaxTextureCoords		(0),
		mMaxTextureSize			(0),
		mMaxLights				(0),
		mMaxFBOAttachments		(0),
		mTextureMemory			(0),
		mBufferMemory			(0),
		mMaxTextureMemory		(0),
		mMaxBufferMemory		(0),
		mMaxMemory				(0) {}

	virtual ~DeviceInfo() {}

	void IncreaseTextureMemory (uint val)
	{
		mTextureMemory += val;
		if (mMaxTextureMemory < mTextureMemory) mMaxTextureMemory = mTextureMemory;
		if (mMaxMemory < mTextureMemory + mBufferMemory) mMaxMemory = mTextureMemory + mBufferMemory;
	}

	void IncreaseBufferMemory  (uint val)
	{
		mBufferMemory += val;
		if (mMaxBufferMemory < mBufferMemory) mMaxBufferMemory = mBufferMemory;
		if (mMaxMemory < mTextureMemory + mBufferMemory) mMaxMemory = mTextureMemory + mBufferMemory;
	}

	void DecreaseTextureMemory (uint val) { mTextureMemory -= val; }
	void DecreaseBufferMemory  (uint val) { mBufferMemory  -= val; }
};