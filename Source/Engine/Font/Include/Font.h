#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// FreeType-generated texture mapped font class
//============================================================================================================

class Font
{
public:

	typedef IFont::Vertex		Vertex;
	typedef IFont::Vertices		Vertices;
	typedef IFont::Tags			Tags;

private:

	//  Individual glyph information struct
	struct Glyph
	{
		uint	mWidth;		// Actual character width for this glyph
		float	mLeft;		// left, right, top, and bottom are all texture coordinates
		float	mRight;
		float	mTop;
		float	mBottom;

		Glyph() : mWidth(0), mLeft(0), mRight(0), mTop(0), mBottom(0) {}
	};

protected:

	String	mSource;	// Actual filename from which the font has been loaded
	String	mLoadingFN;	// Temporary filename, internal use
	byte	mSize;		// Font size (maximim height) in pixels
	byte	mPadding;	// Padding between characters inside the image buffer
	byte	mGlyphSize;	// Calculated glyph size that fits 'mSize' by 'mSize' glyph + 'mPadding'
	Glyph	mGlyph[95];	// Actual glyph information for all printable characters
	Memory	mBuffer;	// Memory buffer that will hold texture information
	uint	mWidth;		// Width of the texture (it's always square)

public:

	Font() : mSize(0), mPadding(0), mGlyphSize(0), mWidth(0) {}
	~Font() { mBuffer.Release(); }

private:

	// INTERNAL: Returns the glyph width for the specified character
	uint _GetCharWidth (byte ch) const { ch -= 32; return mGlyph[ch < 95 ? ch : 0].mWidth; }

public:

	void			Release()			{ mBuffer.Release(); }
	const String&	GetSource()	const	{ return mSource; }
	byte			GetSize()	const	{ return mSize; }
	const Memory&	GetBuffer()	const	{ return mBuffer; }
	uint			GetWidth()	const	{ return mWidth; }

	// Loads the specified font file, creating a font of specified size
	bool Load (const String& filename, byte fontSize);

	// Create the font using the specified input memory buffer and font size
	bool Load (const void* buffer, uint bufferSize, byte fontSize);

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