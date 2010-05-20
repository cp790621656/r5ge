#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Base class for textures (Images stored on the videocard)
//============================================================================================================

struct ITexture
{
	// Texture format
	struct Format
	{
		enum
		{
			Optimal			= 0x00,	// 0000 0000:  Optimal value, used to specify the "choose whatever works best" option for the destination format
			Invalid			= 0x00,	// 0000 0000:  Invalid texture format
			Alpha			= 0x01,	// 0000 0001:  1 channel  (will be kept as a single alpha channel texture)
			Luminance		= 0x03,	// 0000 0011:  2 channels (luminance and alpha, will be converted to RGBA)
			RGB				= 0x04,	// 0000 0100:  3 channels
			RGBA			= 0x05,	// 0000 0101:  4 channels
			RGB30A2			= 0x07, // 0000 0111:  4 channels, 10 bytes each for red, green, and blue, and 2 for alpha
			DXT1			= 0x09,	// 0000 1001:  4 channels, x8 compression with 1 bit alpha
			DXT3			= 0x0B, // 0x00 1011:  4 channels, x4 compression with sharp alpha
			DXTN			= 0x0C,	// 0x00 1100:  4 channels, x4 compression with B component stored in alpha (meant for normal maps)
			DXT5			= 0x0D, // 0x00 1101:  4 channels, x4 compression with interpolated gradient alpha
			Float			= 0x11,	// 0001 0001:  4-bit floating point alpha channel
			RGB16F			= 0x14,	// 0001 0100:  16-bit floating point format (R16 G16 B16)
			RGBA16F			= 0x15,	// 0001 0101:  16-bit floating point format (R16 G16 B16 A16)
			RGB32F			= 0x24,	// 0010 0100:  32-bit floating point format (R32 G32 B32)
			RGBA32F			= 0x25,	// 0010 0101:  32-bit floating point format (R32 G32 B32 A32)
			Depth			= 0x40,	// 0100 0000:  Depth texture
			Stencil			= 0x80, // 1000 0000:  Stencil format (8-bit)
			DepthStencil	= 0xC0,	// 1100 0000:  24-bit depth with 8 bit-stencil

			Compressed		= 0x08,	// 0000 1000:  Not a valid texture type, but can be used with the AND operator to check if the texture has been compressed
			HDR				= 0x30,	// 0001 0000:  Not a valid texture type, but can be used with the AND operator to check if the texture is HDR
		};
	};

	// Texture type
	struct Type
	{
		enum
		{
			Invalid = 0,
			OneDimensional,
			TwoDimensional,
			ThreeDimensional,
			EnvironmentCubeMap
		};
	};

	// Texture filtering mode
	struct Filter
	{
		enum
		{
			Default		= 0x0,
			Nearest		= 0x1,	// Nearest pixel is chosen, no filtering
			Linear		= 0x2,	// Linear filtering
			Mipmap		= 0x4,	// Mipmap filtering
			Anisotropic	= 0xC,	// Anisotropic mipmap filtering
		};
	};

	// Texture wrapping mode
	struct WrapMode
	{
		enum
		{
			Default = 0,
			Repeat,			// Default repeated mode
			Mirror,			// Mirrored edge
			ClampToEdge,	// Clamped to edge
			ClampToZero,	// Clamped to border color of (0, 0, 0, 0)
			ClampToOne,		// Clamped to border color of (1, 1, 1, 1)
		};
	};

protected:

	uint mUID;

	ITexture() : mUID(GenerateUID()) {}

public: //============================================================================================================

	R5_DECLARE_INTERFACE_CLASS("Texture");

	// Retrieves a unique identifier for this texture
	uint GetUID() const { return mUID; }

	virtual ~ITexture() {};

	// Releases all resources used by the texture (should be thread-safe)
	virtual void Release()=0;

	virtual const String&	GetName()	const=0;	// All textures need a name
	virtual const Vector2i& GetSize()	const=0;	// Texture's dimensions
	
	virtual bool	IsValid()			const=0;	// Whether the texture can be used (tries to load it)
	virtual uint	GetDepth()			const=0;	// Texture's depth dimension (for 3D textures)
	virtual uint	GetFormat()			const=0;	// Texture's format
	virtual uint	GetFiltering()		const=0;	// No filtering, linear filtering, mip-map, anisotropic
	virtual uint	GetWrapMode()		const=0;	// Whether the texture is repeated, clamped, or mirrored
	virtual ulong	GetSizeInMemory()	const=0;	// Buffer size in bytes
	virtual uint	GetMaxSize()		const=0;	// Returns the maximum possible texture width/height
	virtual uint	GetTextureID()		const=0;	// Returns the bound texture ID, updates the timestamp
	virtual uint	GetType()			const=0;	// Current ITexture::Type
	virtual ulong	GetLastUsedTime()	const=0;	// Returns the last timestamp when GetTextureID() was called

	// Returns the valid path to the texture's source
	virtual const String& GetSource (uint index) const=0;

	// Saves the image's color data into the specified memory buffer
	// NOTE: Must be called from the graphics thread!
	virtual bool GetBuffer (Memory& mem)=0;

	// Wrapping mode and filtering
	virtual void SetWrapMode  (uint wrapMode )=0;
	virtual void SetFiltering (uint filtering)=0;
	virtual void InvalidateMipmap()=0;

	// Load a single texture from the specified file
	virtual bool Load( const String& file, uint textureFormat = Format::Optimal )=0;

	// Load a cube map using the six specified textures
	virtual bool Load(	const String&	up,
						const String&	down,
						const String&	north,
						const String&	east,
						const String&	south,
						const String&	west,
						uint			textureFormat = Format::Optimal )=0;

	// Assign texture data manually
	virtual bool Set(	const void*		buffer,
						uint			width,
						uint			height,
						uint			depth,
						uint			dataFormat,
						uint			textureFormat = Format::Optimal )=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;

public: //============================================================================================================

	// Conversion helper functions, out here for consistency and because they may be used across different modules
	static uint StringToFormat (const String& value)
	{
		if		(value == "Alpha")			return Format::Alpha;
		else if (value == "Luminance")		return Format::Luminance;
		else if (value == "RGB")			return Format::RGB;
		else if (value == "RGBA")			return Format::RGBA;
		else if (value == "RGB30A2")		return Format::RGB30A2;
		else if (value == "DXT1")			return Format::DXT1;
		else if (value == "DXT3")			return Format::DXT3;
		else if (value == "DXTN")			return Format::DXTN;
		else if (value == "DXT5")			return Format::DXT5;
		else if (value == "Float")			return Format::Float;
		else if (value == "RGB16F")			return Format::RGB16F;
		else if (value == "RGBA16F")		return Format::RGBA16F;
		else if (value == "RGB32F")			return Format::RGB32F;
		else if (value == "RGBA32F")		return Format::RGBA32F;
		else if (value == "Depth")			return Format::Depth;
		else if (value == "Stencil")		return Format::Stencil;
		else if (value == "DepthStencil")	return Format::DepthStencil;
											return Format::Invalid;
	}

	static const char* FormatToString (uint format)
	{
		switch ( format )
		{
			case Format::Alpha:			return "Alpha";
			case Format::Luminance:		return "Luminance";
			case Format::RGB:			return "RGB";
			case Format::RGBA:			return "RGBA";
			case Format::RGB30A2:		return "RGB30A2";
			case Format::DXT1:			return "DXT1";
			case Format::DXT3:			return "DXT3";
			case Format::DXTN:			return "DXTN";
			case Format::DXT5:			return "DXT5";
			case Format::Float:			return "Float";
			case Format::RGB16F:		return "RGB16F";
			case Format::RGBA16F:		return "RGBA16F";
			case Format::RGB32F:		return "RGB32F";
			case Format::RGBA32F:		return "RGBA32F";
			case Format::Depth:			return "Depth";
			case Format::Stencil:		return "Stencil";
			case Format::DepthStencil:	return "DepthStencil";
		}
		return "Default";
	}

	static uint StringToFilter (const String& value)
	{
		if		(value == "Nearest")	 return Filter::Nearest;
		else if (value == "Linear")		 return Filter::Linear;
		else if (value == "Mipmap")		 return Filter::Mipmap;
		else if (value == "Anisotropic") return Filter::Anisotropic;
		else							 return Filter::Default;
	}

	static uint StringToWrapMode (const String& value)
	{
		if		(value == "Repeat")			return WrapMode::Repeat;
		else if (value == "Mirror")			return WrapMode::Mirror;
		else if (value == "Clamp to Edge")	return WrapMode::ClampToEdge;
		else if (value == "Clamp to Zero")	return WrapMode::ClampToZero;
		else if (value == "Clamp to One")	return WrapMode::ClampToOne;
		else								return WrapMode::Default;
	}

	static const char* FilterToString (uint val)
	{
		switch (val)
		{
			case Filter::Nearest:		return "Nearest";
			case Filter::Linear:		return "Linear";
			case Filter::Mipmap:		return "Mipmap";
			case Filter::Anisotropic:	return "Anisotropic";
		}
		return "Default";
	}

	static const char* WrapModeToString (uint val)
	{
		switch (val)
		{
			case WrapMode::Repeat:			return "Repeat";
			case WrapMode::Mirror:			return "Mirror";
			case WrapMode::ClampToEdge:		return "Clamp to Edge";
			case WrapMode::ClampToZero:		return "Clamp to Zero";
			case WrapMode::ClampToOne:		return "Clamp to One";
		}
		return "Default";
	}

	static uint GetBitsPerPixel (uint format)
	{
		switch (format)
		{
			case Format::Alpha:			return 8;
			case Format::Luminance:		return 16;
			case Format::RGB:			return 24;
			case Format::RGB30A2:		return 32;
			case Format::RGBA:			return 32;
			case Format::DXT1:			return 4;
			case Format::DXT3:			return 8;
			case Format::DXTN:			return 8;
			case Format::DXT5:			return 8;
			case Format::Float:			return 32;
			case Format::RGB16F:		return 48;
			case Format::RGBA16F:		return 64;
			case Format::RGB32F:		return 96;
			case Format::RGBA32F:		return 128;
			case Format::Depth:			return 24;
			case Format::Stencil:		return 8;
			case Format::DepthStencil:	return 32;
		}
		return 0;
	}
};