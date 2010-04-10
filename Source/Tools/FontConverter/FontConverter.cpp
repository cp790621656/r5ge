//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// FontConverter can be used to convert R5F files into TGA + FD pairs and back
//============================================================================================================

#include "../../Engine/Font/Include/_All.h"
#include "../../Engine/Image/Include/_All.h"
using namespace R5;

//============================================================================================================

#define LOAD_DONE	printf("Loaded %s\n", filename.GetBuffer())
#define SAVE_DONE	printf("Saved out '%s'\n", filename.GetBuffer())
#define READ_ERROR	printf("ERROR: Unable to read '%s'\n", filename.GetBuffer())
#define WRITE_ERROR printf("ERROR: Unable to write '%s'\n", filename.GetBuffer())

//============================================================================================================
// Save the texture portion of the font
//============================================================================================================

bool SaveImage (const Font& font, const String& filename)
{
	if (font.GetBuffer().IsValid())
	{
		uint size = font.GetWidth();
		Image img;

		byte* tex = (byte*)img.Reserve(size, size, 1, ITexture::Format::RGBA);
		const byte* buf = font.GetBuffer().GetBuffer();

		if (font.GetFormat() == ITexture::Format::Luminance)
		{
			for (uint i = 0, imax = size * size; i < imax; ++i)
			{
				byte r = buf[i << 1];
				byte a = buf[(i << 1) + 1];

				Color4ub& color = (Color4ub&)tex[i << 2];
				color.Set(r, r, r, a);
			}
		}
		else if (font.GetFormat() == ITexture::Format::RGBA)
		{
			memcpy(tex, buf, size * size * 4);
		}
		else
		{
			printf("ERROR: Only Luminance and RGBA formats are supported!\n");
			return false;
		}
		return img.Save(filename);
	}
	printf("ERROR: Font data is invalid?\n");
	return false;
}

//============================================================================================================
// Main application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif

#ifndef _DEBUG
	if (argc > 1)
	{
#endif
		String filename;
		uint size = 15;
		uint padding = 4;
		uint spacing = 0;

#ifndef _DEBUG
		for (int i = 1; i < argc; ++i)
		{
			filename = argv[i];
#else
			filename = "../../../Resources/Fonts/arial.ttf";
			//filename = "../../../Resources/Fonts/arial 15.r5f";
			//filename = "../../../Resources/Fonts/arial 15.fd";
#endif
			if (filename.EndsWith(".ttf"))
			{
#ifndef _DEBUG
				if (i+1 < argc && String(argv[i+1]) >> size)
				{
					++i;
					
					if (i+1 < argc && String(argv[i+1]) >> padding)
					{
						++i;
						if (i+1 < argc && String(argv[i+1]) >> spacing) ++i;
					}
				}
#endif
				Font font;

				if (font.Load(filename, (byte)size, (byte)padding, (byte)spacing))
				{
					printf("Loaded %s\n", filename.GetBuffer());

					filename.Replace(".ttf", String(" %u.r5f", font.GetSize()));
					
					if (font.Save(filename))
					{
						printf("Saved out '%s'\n - Size %u, padding %u, spacing %u\n", filename.GetBuffer(), size, padding, spacing);
					}
					else WRITE_ERROR;
				}
				else READ_ERROR;
			}
			else if (filename.EndsWith(".r5f"))
			{
				Font font;
				
				if (font.Load(filename))
				{
					LOAD_DONE;

					Memory mem;
					mem.Append(font.GetSize());
					mem.Append(font.GetPadding());
					mem.Append(font.GetGlyphSize());

					const Array<Font::Glyph>& glyphs = font.GetGlyphs();
					mem.Append(glyphs.GetSize());
					mem.Append(glyphs.GetBuffer(), glyphs.GetSizeInMemory());

					filename.Replace(".r5f", ".fd");

					if (mem.Save(filename))
					{
						SAVE_DONE;
						filename.Replace(".fd", ".tga");
						
						// Save the texture
						if (SaveImage(font, filename)) SAVE_DONE;
						else WRITE_ERROR;
					}
					else WRITE_ERROR;
				}
				else READ_ERROR;
			}
			else if (filename.EndsWith(".fd"))
			{
				Memory mem;
				
				if (mem.Load(filename))
				{
					byte fontSize, padding, glyphSize;
					uint count;

					const byte* buffer = mem.GetBuffer();
					uint buffSize = mem.GetSize();

					// Extract the font size, padding, glyph size, and the number of glyphs
					if (Memory::Extract(buffer, buffSize, fontSize) &&
						Memory::Extract(buffer, buffSize, padding) &&
						Memory::Extract(buffer, buffSize, glyphSize) &&
						Memory::Extract(buffer, buffSize, count))
					{
						Array<Font::Glyph> glyphs;
						Font::Glyph* ptr = glyphs.ExpandTo(count);

						// Extract the glyphs
						if (Memory::Extract(buffer, buffSize, ptr, glyphs.GetSizeInMemory()))
						{
							LOAD_DONE;

							printf(" - Font Size:  %u\n", fontSize);
							printf(" - Padding:    %u\n", padding);
							printf(" - Glyph Size: %u\n", glyphSize);
							printf(" - Glyphs:     %u\n", count);

							Image img;
			
							filename.Replace(".fd", ".tga");

							if (img.Load(filename) && img.IsValid() && img.GetWidth() == img.GetHeight())
							{
								LOAD_DONE;

								printf(" - Glyphs:     %u\n", count);
								printf(" - Texture:    %u (%s)\n", img.GetWidth(), ITexture::FormatToString(img.GetFormat()));

								Font font;
								font.SetGlyphs(fontSize, padding, glyphSize, glyphs);
								font.SetBuffer(img.GetBuffer(), img.GetWidth(), img.GetFormat());

								filename.Replace(".tga", ".r5f");

								// Save the font
								if (font.Save(filename)) SAVE_DONE;
								else WRITE_ERROR;
							}
							else READ_ERROR;
						}
						else READ_ERROR;
					}
					else READ_ERROR;
				}
				else READ_ERROR;
			}
#ifndef _DEBUG
		}
	}
	else
	{
		puts("Usage: FontConverter font [size] [padding] [spacing]");
		puts("Example 1: FontConverter arial.ttf 15 4 1");
		puts("Example 2: FontConverter arial.r5f");
		puts("Example 3: FontConverter arial.fd");
		puts("You can also drag the file in question onto this executable in order to convert it.");
	}
#endif
	puts("Press Enter to exit...");
	getchar();
	return 0;
}