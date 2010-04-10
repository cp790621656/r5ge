#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// FreeType-generated texture mapped font class. This is a stand-alone class that does not depend on the
// Graphics class. It can be used to load font textures for the purpose of saving them out as files.
//============================================================================================================

class Font
{
public:

	typedef IFont::Vertex		Vertex;
	typedef IFont::Vertices		Vertices;
	typedef IFont::Tags			Tags;

public:

	//  Individual glyph information struct
	struct Glyph
	{
		byte	mWidth;		// Actual character width for this glyph
		float	mLeft;		// left, right, top, and bottom are all texture coordinates
		float	mRight;
		float	mTop;
		float	mBottom;

		Glyph() : mWidth(0), mLeft(0), mRight(0), mTop(0), mBottom(0) {}
	};

protected:

	String			mSource;	// Actual filename from which the font has been loaded
	String			mLoadingFN;	// Temporary filename, internal use

	byte			mSize;		// Font size (maximum height) in pixels
	byte			mPadding;	// Padding between characters inside the image buffer
	byte			mGlyphSize;	// Calculated glyph size that fits 'mSize' by 'mSize' glyph + 'mPadding'
	Array<Glyph>	mGlyphs;	// Actual glyph information for all printable characters

	Memory			mBuffer;	// Memory buffer that will hold texture information
	uint			mWidth;		// Width of the texture (it's always square)
	uint			mFormat;	// Texture format

public:

	Font() : mSize(0), mPadding(0), mGlyphSize(0), mWidth(0), mFormat(ITexture::Format::Invalid) {}
	~Font() { mBuffer.Release(); }

private:

	// INTERNAL: Returns the glyph width for the specified character
	uint _GetCharWidth (byte ch) const { ch -= 32; return mGlyphs[ch < mGlyphs.GetSize() ? ch : 0].mWidth; }

public:

	void			Release()				{ mBuffer.Release(); }
	const String&	GetSource()		const	{ return mSource; }
	byte			GetSize()		const	{ return mSize; }
	byte			GetPadding()	const	{ return mPadding; }
	byte			GetGlyphSize()	const	{ return mGlyphSize; }

	// Read-only access to glyphs
	const Array<Glyph>&	GetGlyphs()	const	{ return mGlyphs; }

	// All the functions necessary to create a texture
	const Memory&	GetBuffer()		const	{ return mBuffer; }
	uint			GetWidth()		const	{ return mWidth; }
	uint			GetFormat()		const	{ return mFormat; }

	// Allows replacement of current glyphs
	void SetGlyphs (byte fontSize, byte padding, byte glyphSize, const Array<Glyph>& glyphs);

	// Allows replacement of the current texture
	void SetBuffer (const void* buffer, uint width, uint format);

	// Loads the specified font file, creating a font of specified size
	bool Load (const String& filename, byte fontSize = 0, byte padding = 0);

	// Create the font using the specified input memory buffer and font size
	bool Load (const byte* buffer, uint bufferSize, byte fontSize = 0, byte padding = 0);

	// Saves the font in R5F format into the specified memory buffer
	bool Save (Memory& mem) const;

	// Saves the font into the specified file (R5F font format, or TGA texture + R5A glyph definition file)
	bool Save (const String& filename) const;

	// Figures out the length of the text if it was printed
	uint GetLength
	(
		const String&	text,						// String to be used
		uint			start		= 0,			// Index of the first character
		uint			end			= -1,			// Index of the last character
		uint			tags		= Tags::Skip	// How to process color tags in [RRGGBB] format
	)
	const;

	// Returns the number of printable characters that will fit onto the line of specified width
	uint CountChars
	(
		const String&	text,						// String to be used
		uint			width,						// Bounding maximum width into which the string will fit
		uint			start		= 0,			// Index of the first character
		uint			end			= -1,			// Index of the last character (capped by text.GetLength())
		bool			inReverse	= false,		// Whether to perform the operation from the end of the string
		bool			round		= false,		// Whether to round to the nearest character rather than to whatever fits fully
		uint			tags		= Tags::Skip	// How to process color tags in [RRGGBB] format
	)
	const;

	// Returns the color this string ends with. Returns 'true' if color has been changed, 'false' otherwise.
	bool UpdateColor
	(
		const String&	text,						// String to be used
		Color4ub&		color,						// Text's final color
		uint			start		= 0,			// Index of the first character
		uint			end			= -1			// Index of the last character (capped by text.GetLength())
	)
	const;

	// Prints the requested string into the output vertex array
	bool Print
	(
		Vertices&		out,						// Output vertex array (printing should APPEND to whatever is already is in the buffer)
		const Vector2f&	pos,						// Initial position (top-left)
		const Color4ub&	color,						// Color to use
		const String&	text,						// Text to be printed
		uint			start		= 0,			// Index of the first character in the text string
		uint			end			= -1,			// Index of the last character (capped by text.GetLength())
		uint			tags		= Tags::Ignore	// How to process color tags in [RRGGBB] format
	)
	const;
};