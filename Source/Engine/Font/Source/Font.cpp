#include "../Include/_All.h"

// Whether the FreeType library will be compiled and used
#define R5_USE_FREETYPE

//============================================================================================================
//  FreeType Library
//============================================================================================================

#ifdef R5_USE_FREETYPE
#include <freetype/config/ftheader.h>
#include FT_FREETYPE_H

#ifdef _WINDOWS
	#pragma comment(lib, "freetype.lib")
#endif

#define FT_FLAGS (FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL)

using namespace R5;

//============================================================================================================
// Since FreeType library is not multi-threaded, we have to limit access to a single thread at a time
//============================================================================================================

struct FreeType
{
	FT_Library			mLib;
	Thread::Lockable	mLock;

	FreeType()				{ FT_Init_FreeType( &mLib ); }
	~FreeType()				{ FT_Done_FreeType(  mLib ); }
	operator FT_Library&()	{ return mLib; }

	void Lock()		const	{ mLock.Lock();	}
	void Unlock()	const	{ mLock.Unlock();	}
};

//============================================================================================================
// Global access node
//============================================================================================================

FreeType  g_lib;

#else
using namespace R5;
#endif

//============================================================================================================
// Locates a font file, if it's nearby
//============================================================================================================

bool _Locate (String& file)
{
	if (file.IsEmpty()) return false;
	if (System::FileExists(file)) return true;

	String name (System::GetFilenameFromPath(file));

	file = "Fonts/" + name;
	if (System::FileExists(file)) return true;

	file << ".r5f";
	if (System::FileExists(file)) return true;

#ifdef R5_USE_FREETYPE
	file.Replace(".r5f", ".ttf", true);
	if (System::FileExists(file)) return true;
#endif
	return false;
}

//============================================================================================================
// Loads the specified font file, creating a font of specified size
//============================================================================================================

bool Font::Load (const String& filename, byte fontSize, byte padding)
{
	// Temporary memory buffer used to load the file
	Memory in;

	mLoadingFN = filename;

	// Try to load the file, and ensure that it has enough data for a header, at least
	if (!_Locate(mLoadingFN) || !in.Load(mLoadingFN) || !(in.GetSize() > 5))
	{
		mLoadingFN = filename;
		mLoadingFN << " ";
		mLoadingFN << (uint)fontSize;
	}

	if (in.IsValid() || (_Locate(mLoadingFN) && in.Load(mLoadingFN) && in.GetSize() > 5))
	{
		if ( Font::Load(in.GetBuffer(), in.GetSize(), fontSize, padding) )
		{
			mSource = mLoadingFN;
			mLoadingFN.Release();
			return true;
		}
	}
	mLoadingFN.Release();
	return false;
}

//============================================================================================================
// Create the font using the specified input memory buffer and font size
//============================================================================================================

bool Font::Load (const byte* buffer, uint bufferSize, byte fontSize, byte padding)
{
	if (buffer == 0 || bufferSize < 5) return false;

	// See if the file is in the R5F format
	if (buffer[0] == '/' &&
		buffer[1] == '/' &&
		buffer[2] == 'R' &&
		buffer[3] == '5' &&
		buffer[4] == 'F')
	{
		buffer += 5;
		bufferSize -= 5;

		return (Memory::Extract(buffer, bufferSize, mSize) &&
				Memory::Extract(buffer, bufferSize, mPadding) &&
				Memory::Extract(buffer, bufferSize, mGlyphSize) &&
				Memory::Extract(buffer, bufferSize, mGlyph, sizeof(Glyph) * 95) &&
				Memory::Extract(buffer, bufferSize, mWidth) &&
				Decompress(buffer, bufferSize, mBuffer));
	}

#ifndef R5_USE_FREETYPE
	return false;
#else
	// If this point was reached then the font is in FreeType format
	if (fontSize == 0) return false;

	// Safety check
	ASSERT(((uint)fontSize + (padding << 1) < 100), "Requested font is excessively large");
	
	g_lib.Lock();
	{
		mSize = fontSize;
		mPadding = padding;

		mBuffer.Release();
		FT_Face face;

		// Try to create the font
		if ( FT_New_Memory_Face( g_lib, (byte*)buffer, bufferSize, 0, &face ) )
		{
			g_lib.Unlock();
			return false;
		}

		// Set the pixel size
		if ( FT_Set_Pixel_Sizes( face, 0, mSize ) )
		{
			FT_Done_Face( face );
			g_lib.Unlock();
			return false;
		}

		Vector2i topLeft, bottomRight;

		// Run through all glyphs and figure out some useful measurements
		for (int i = 0; i < 95; ++i)
		{
			if ( FT_Load_Char(face, i + 32, FT_FLAGS) == 0 )
			{
				FT_GlyphSlot	slot	= face->glyph;
				FT_Bitmap*		bitmap	= &slot->bitmap;

				int left	= slot->metrics.horiBearingX >> 6;
				int right	= left + bitmap->width;
				int top		= slot->bitmap_top;
				int bottom	= top - bitmap->rows;

				if ( left	< topLeft.x     )	topLeft.x		= left;
				if ( right	> bottomRight.x )	bottomRight.x	= right;
				if ( top	> topLeft.y     )	topLeft.y		= top;
				if ( bottom	< bottomRight.y )	bottomRight.y	= bottom;
			}
		}

		int width  = bottomRight.x - topLeft.x;
		int height = topLeft.y - bottomRight.y;

		// Figure out the size of each glyph
		mGlyphSize = ((width > height) ? width : height) + (mPadding << 1);

		if (mGlyphSize == 0)
		{
			FT_Done_Face( face );
			g_lib.Unlock();
			return false;
		}

		// Maximum dimensions are 10 glyphs by 10 glyphs
		uint maxWidth = mGlyphSize * 10;

		// Figure out a valid power-of-two texture width
		mWidth = 16;
		while (mWidth < maxWidth) mWidth = mWidth << 1;

		// As a safety precaution, don't allow extremely large fonts
		ASSERT(mWidth <= 2048, "Requested font is excessively large");

		// Allocate a new buffer to hold all pixel information
		ushort* buffer = (ushort*)mBuffer.Resize(mWidth * mWidth * 2);
		mBuffer.MemSet(0);

		if (buffer == 0)
		{
			FT_Done_Face( face );
			g_lib.Unlock();
			return false;
		}

		// Relative width of individual glyphs
		float textureGlyphWidth = (float)mGlyphSize / mWidth;

		// Run through all glyphs again, but this time, save their bitmap information
		for (uint y = 0; y < 10; ++y)
		{
			for (uint x = 0; x < 10; ++x)
			{
				uint glyphIndex = y * 10 + x;

				// Glyph index range is 0-94
				if (glyphIndex > 94) break;

				// Ignore errors
				if ( FT_Load_Char(face, 32 + glyphIndex, FT_FLAGS) ) continue;

				FT_Bitmap* bitmap = &face->glyph->bitmap;
				FT_Glyph_Metrics* metrics = &face->glyph->metrics;

				mGlyph[glyphIndex].mWidth	= (byte)(metrics->horiAdvance >> 6);
				mGlyph[glyphIndex].mLeft	= textureGlyphWidth * x;
				mGlyph[glyphIndex].mRight	= textureGlyphWidth * (x + 1);
				mGlyph[glyphIndex].mTop		= textureGlyphWidth * (mWidth - y);
				mGlyph[glyphIndex].mBottom	= textureGlyphWidth * (mWidth - y - 1);
				
				int left = metrics->horiBearingX >> 6;
				int top  = face->glyph->bitmap_top;

				int pixelOffsetX = left - topLeft.x;
				int pixelOffsetY = topLeft.y - top;

				uint textureX = x * mGlyphSize + pixelOffsetX;
				uint textureY = y * mGlyphSize + pixelOffsetY;

				uint bitmapWidth	= bitmap->width;
				uint bitmapHeight	= bitmap->rows;
				uint paddingOffset	= (mWidth - 1) * mPadding;

				// Go through all pixels in the bitmap
				for (uint bitmapY = 0; bitmapY < bitmapHeight; ++bitmapY)
				{
					for (uint bitmapX = 0; bitmapX < bitmapWidth; ++bitmapX)
					{
						uint bitmapIndex	= bitmapY * bitmapWidth + bitmapX;
						byte pixel			= bitmap->buffer[bitmapIndex];
						
						// If the pixel is not black...
						if (pixel > 0)
						{
							// Texture should have the first character at the top-left corner,
							// but (0, 0) is actually the bottom-right. This is why we flip the
							// Y coordinate here, matching the flipped texCoords set above. Doing so
							// makes the texture look upright when viewed, rather than upside-down.

							uint textureIndex = (mWidth - (textureY + bitmapY) - 1) * mWidth +
								textureX + bitmapX - paddingOffset;

							// Copy the pixel as alpha over onto the main texture using white (255) for color
							buffer[textureIndex] = 255 | (pixel << 8);
						}
					}
				}
			}
		}

		// We're done with this face
		FT_Done_Face( face );
	}
	g_lib.Unlock();
	return true;
#endif
}

//============================================================================================================
// Saves the font in R5F format into the specified memory buffer
//============================================================================================================

bool Font::Save (Memory& mem) const
{
	mem.Append("//R5F", 5);
	mem.Append(mSize);
	mem.Append(mPadding);
	mem.Append(mGlyphSize);
	mem.Append(mGlyph, sizeof(Glyph) * 95);
	mem.Append(mWidth);
	return Compress(mBuffer, mem);
}

//============================================================================================================
// Saves the font into the specified file (R5F font format, or TGA texture + R5A glyph definition file)
//============================================================================================================

bool Font::Save (const String& filename) const
{
	Memory comp;
	return Save(comp) && comp.Save(filename);
}

//============================================================================================================
// Figures out the length of the text if it was printed
//============================================================================================================

uint Font::GetLength (	const String&	text,
						uint			start,
						uint			end,
						uint			tags ) const
{
	uint size (0);
	byte ch;

	if (end > text.GetLength())
		end = text.GetLength();

	for (; start < end; )
	{
		ch = text[start++];

		if ( (tags != Tags::Ignore) && (ch == '[') && (start + 6 < end) && (text[start + 6] == ']') )
		{
			start += 7;
		}
		else
		{
			size += _GetCharWidth( ch );
		}
	}
	return size;
}

//============================================================================================================
// Returns the number of printable characters that will fit onto the line of specified width
//============================================================================================================

uint Font::CountChars ( const String&	text,
						uint			width,
						uint			start,
						uint			end,
						bool			inReverse,
						bool			round,
						uint			tags ) const
{
	uint length	(0), count (0), charWidth (0);
	char ch;

	// Ensure that the end is never passed
	if (end > text.GetLength()) end = text.GetLength();

	if (inReverse)
	{
		for ( ; end > start; ++count )
		{
			ch = text[--end];

			if ( (tags != Tags::Ignore) && (ch == ']') && (end > 7) && (text[end - 7] == '[') )
			{
				count += 7;
				start -= 7;
			}
			else
			{
				charWidth = _GetCharWidth( ch );
				length   += charWidth;

				if (length > width)
				{
					// Round the result by half of the character's width if asked for
					if (round && (length - charWidth / 2) <= width) ++count;
					break;
				}
			}
		}
	}
	else
	{
		for ( ; start < end; ++count )
		{
			ch = text[start++];

			if ( (tags != Tags::Ignore) && (ch == '[') && (start + 6 < end) && (text[start + 6] == ']') )
			{
				count += 7;
				start += 7;
			}
			else
			{
				charWidth = _GetCharWidth( ch );
				length   += charWidth;

				if (length > width)
				{
					// Round the result by half of the character's width if asked for
					if (round && (length - charWidth / 2) <= width) ++count;
					break;
				}
			}
		}
	}
	return count;
}

//============================================================================================================
// Returns the color this string ends with. Returns 'true' if color has been changed, 'false' otherwise.
//============================================================================================================

bool Font::UpdateColor (const String&	text,
						Color4ub&		color,
						uint			start,
						uint			end) const
{
	if (end > text.GetLength()) end = text.GetLength();

	while (end > start)
	{
		char ch = text[--end];

		if ( (ch == ']') && (end > 7) && (text[end - 7] == '[') )
		{
			color.SetByHexString( text.GetBuffer() + end - 6, 6 );
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Prints the requested string into the output vertex array
//============================================================================================================

bool Font::Print ( Vertices&		out,
				   const Vector2f&	pos,
				   const Color4ub&	color,
				   const String&	text,
				   uint				start,
				   uint				end,
				   uint				tags ) const
{
	// Ensure that the length is never exceeded
	if (end > text.GetLength())  end = text.GetLength();

	// Only continue as long as there is something to print
	if (start < end)
	{
		Color4ub myColor (color);

		byte ch;
		float x1 = pos.x - mPadding;
		float x2;
		float y1 = pos.y - mPadding;
		float y2 = y1 + mGlyphSize;

		for ( ; start < end; )
		{
			ch = text[start++];

			if ( (tags != Tags::Ignore) && (ch == '[') && (start + 6 < end) && (text[start + 6] == ']') )
			{
				if (tags == Tags::Process) myColor.SetByHexString( text.GetBuffer() + start, 6 );
				start += 7;
			}
			else if (ch > 31)
			{
				ch -= 32;

				const Glyph& glyph = mGlyph[ch < 95 ? ch : 0];

				x2 = x1 + mGlyphSize;

				out.Expand().Set(x1, y1, glyph.mLeft,  glyph.mTop,    myColor);
				out.Expand().Set(x1, y2, glyph.mLeft,  glyph.mBottom, myColor);
				out.Expand().Set(x2, y2, glyph.mRight, glyph.mBottom, myColor);
				out.Expand().Set(x2, y1, glyph.mRight, glyph.mTop,    myColor);

				x1 += glyph.mWidth;
			}
		}
		return true;
	}
	return false;
}