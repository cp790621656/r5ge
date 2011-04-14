#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// OpenGL implementation of the font class
// Author: Michael Lyashenko
//============================================================================================================

class GLFont : public IFont
{
protected:

	String				mName;
	IGraphics*			mGraphics;
	Font				mFont;
	ITexture*			mTex;
	bool				mSerializable;
	Thread::Lockable	mLock;

private:

	// Only the GLGraphics manager should be able to create new fonts
	friend class GLGraphics;
	GLFont(const String& name, IGraphics* graphics);

private:

	// INTERNAL: Updates the texture
	bool _Reload();

public:

	// Common functions
	virtual void			Release();
	virtual const String&	GetName()			const { return mName; }
	virtual bool			IsValid()			const { return (mTex != 0 && mTex->GetFormat() != ITexture::Format::Invalid); }
	virtual byte			GetSize()			const { return mFont.GetSize(); }
	virtual uint			GetSizeInMemory()	const { return (mTex != 0) ? mTex->GetSizeInMemory() : 0; }
	virtual const ITexture* GetTexture()		const { return mTex; }

	// Loads the specified font file, creating a font of specified size
	virtual bool Load (const String& filename, byte fontSize, byte padding = 0);

	// Create the font using the specified input memory buffer and font size
	virtual bool Load (const byte* buffer, uint bufferSize, byte fontSize, byte padding = 0);

	// Figures out the length of the text if it was printed
	virtual uint GetLength
	(
		const String&	text,						// String to be used
		uint			start		= 0,			// Index of the first character
		uint			end			= -1,			// Index of the last character
		uint			tags		= Tags::Skip	// How to process color tags in [RRGGBB] format
	)
	const { return mFont.GetLength(text, start, end, tags); }

	// Returns the number of printable characters that will fit onto the line of specified width
	virtual uint CountChars
	(
		const String&	text,						// String to be used
		uint			width,						// Bounding maximum width into which the string will fit
		uint			start		= 0,			// Index of the first character
		uint			end			= -1,			// Index of the last character (capped by text.GetLength())
		bool			inReverse	= false,		// Whether to perform the operation from the end of the string
		bool			round		= false,		// Whether to round to the nearest character rather than to whatever fits fully
		uint			tags		= Tags::Skip	// How to process color tags in [RRGGBB] format
	)
	const { return mFont.CountChars(text, width, start, end, inReverse, round, tags); }

	// Returns the color this string ends with. Returns 'true' if color has been changed, 'false' otherwise.
	virtual bool UpdateColor
	(
		const String&	text,						// String to be used
		Color4ub&		color,						// Text's final color
		uint			start		= 0,			// Index of the first character
		uint			end			= -1			// Index of the last character (capped by text.GetLength())
	)
	const { return mFont.UpdateColor(text, color, start, end); }

	// Prints the requested string into the output vertex array
	virtual bool Print
	(
		Vertices&		out,						// Output vertex array (printing should APPEND to whatever is already is in the buffer)
		const Vector2f&	pos,						// Initial position (top-left)
		const Color4ub&	color,						// Color to use
		const String&	text,						// Text to be printed
		uint			start		= 0,			// Index of the first character in the text string
		uint			end			= -1,			// Index of the last character (capped by text.GetLength())
		uint			tags		= Tags::Ignore	// How to process color tags in [RRGGBB] format
	)
	const { return mFont.Print(out, pos, color, text, start, end, tags); }

	// Serialization
	virtual bool IsSerializable() const { return mSerializable; }
	virtual void SetSerializable(bool val) { mSerializable = val; }
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	virtual bool SerializeTo (TreeNode& root) const;
};