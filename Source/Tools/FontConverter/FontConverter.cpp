//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// FontConverter can be used to convert R5F files into TGA + R5A pairs
//============================================================================================================

#include "../../Engine/Font/Include/_All.h"
#include "../../Engine/Image/Include/_All.h"
using namespace R5;

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

		for (uint i = 0, imax = size * size; i < imax; ++i)
		{
			byte r = buf[i << 1];
			byte a = buf[(i << 1) + 1];

			Color4ub& color = (Color4ub&)tex[i << 2];
			color.Set(r, r, r, a);
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

#ifndef _DEBUG
		for (int i = 1; i < argc; ++i)
		{
			filename = argv[i];
#else
			filename = "../../../Resources/Fonts/Arial.ttf";
			//filename = "../../../Resources/Fonts/Arial 15.r5f";
#endif
			if (filename.EndsWith(".ttf"))
			{
#ifndef _DEBUG
				if (i+1 < argc && String(argv[i+1]) >> size)
				{
					++i;
					if (i+1 < argc && String(argv[i+1]) >> padding) ++i;
				}
#endif

				Font font;

				if (font.Load(filename, (byte)size, (byte)padding))
				{
					printf("Loaded %s\n", filename.GetBuffer());

					filename.Replace(".ttf", String(" %u.r5f", font.GetSize()));
					
					if (font.Save(filename))
					{
						printf("Saved out '%s' (size %u, padding %u)\n", filename.GetBuffer(), size, padding);
					}
					else
					{
						printf("ERROR: Unable to write to '%s'\n", filename.GetBuffer());
					}
				}
				else
				{
					printf("ERROR: Unable to read '%s'\n", filename.GetBuffer());
				}
			}
			else if (filename.EndsWith(".r5f"))
			{
				Font font;
				
				if (font.Load(filename))
				{
					printf("Loaded %s\n", filename.GetBuffer());

					Memory mem;
					mem.Append(font.GetSize());
					mem.Append(font.GetPadding());
					mem.Append(font.GetGlyphs(), font.GetGlyphCount() * sizeof(Font::Glyph));
					filename.Replace(".r5f", ".fd");

					if (mem.Save(filename))
					{
						printf("Saved out '%s'\n", filename.GetBuffer());
						filename.Replace(".fd", ".tga");
						
						// Save the texture
						if (SaveImage(font, filename))
						{
							printf("Saved out '%s'\n", filename.GetBuffer());
						}
						else
						{
							printf("ERROR: Unable to write '%s'\n", filename.GetBuffer());
						}
					}
					else
					{
						printf("ERROR: Unable to write '%s'\n", filename.GetBuffer());
					}
				}
				else
				{
					printf("ERROR: Unable to read '%s'\n", filename.GetBuffer());
				}
			}
#ifndef _DEBUG
		}
	}
#endif
	getchar();
	return 0;
}