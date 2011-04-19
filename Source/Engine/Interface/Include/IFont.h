#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the Font class
// Author: Michael Lyashenko
//============================================================================================================

struct IFont
{
	R5_DECLARE_INTERFACE_CLASS("Font");

	// Font uses the same vertex as the user interface
	typedef IUI::Vertex		Vertex;
	typedef Array<Vertex>	Vertices;

	// How to treat color tags in [RRGGBB] format
	struct Tags
	{
		enum
		{
			Ignore	= 0,		// Color tags will be treated as regular characters
			Skip	= 1,		// Color tags will be skipped and not counted
			Process	= 2,		// Color tags will be processed
		};
	};

public:

	virtual ~IFont() {}

	// Common functions
	virtual void			Release()=0;
	virtual const String&	GetName()			const=0;	// Font's name
	virtual bool			IsValid()			const=0;	// Whether the font can be used
	virtual byte			GetSize()			const=0;	// Font size that it was created with
	virtual uint			GetSizeInMemory()	const=0;	// Actual amount of memory the font is taking up
	virtual const ITexture* GetTexture()		const=0;	// Return a texture associated with the font, if any

	// Loads the specified font file, creating a font of specified size
	virtual bool Load (const String& filename, byte fontSize, byte padding = 0)=0;

	// Create the font using the specified input memory buffer and font size
	virtual bool Load (const byte* buffer, uint bufferSize, byte fontSize, byte padding = 0)=0;

	// Figures out the length of the text if it was printed
	virtual uint GetLength
	(
		const String&	text,						// String to be used
		uint			start		= 0,			// Index of the first character
		uint			end			= -1,			// Index of the last character
		uint			tags		= Tags::Skip	// How to process color tags in [RRGGBB] format
	)
	const=0;

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
	const=0;

	// Returns the color this string ends with. Returns 'true' if color has been changed, 'false' otherwise.
	virtual bool UpdateColor
	(
		const String&	text,						// String to be used
		Color4ub&		color,						// Text's final color
		uint			start		= 0,			// Index of the first character
		uint			end			= -1			// Index of the last character (capped by text.GetLength())
	)
	const=0;

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
	const=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;
};