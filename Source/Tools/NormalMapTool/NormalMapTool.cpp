//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Normal map tool can be used to flip X and/or Y coordinate in normal maps
// Author: Michael Lyashenko
//============================================================================================================

#include "../../Engine/Image/Include/_All.h"
using namespace R5;

//============================================================================================================

#define LOAD_DONE	printf("Loaded %s\n", filename.GetBuffer())
#define SAVE_DONE	printf("Saved out '%s'\n", filename.GetBuffer())
#define READ_ERROR	printf("ERROR: Unable to read '%s'\n", filename.GetBuffer())
#define WRITE_ERROR printf("ERROR: Unable to write '%s'\n", filename.GetBuffer())

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

	if (argc > 1)
	{
		String filename;
		bool x = false;
		bool y = true;
		Image image;

		for (int i = 1; i < argc; ++i)
		{
			filename = argv[i];

			if (filename.BeginsWith("-"))
			{
				if (filename == "-x")
				{
					x = true;
					y = false;
				}
				else if (filename == "-y")
				{
					x = false;
					y = true;
				}
				else
				{
					x = true;
					y = true;
				}
				continue;
			}
			
			if (image.Load(filename))
			{
				LOAD_DONE;

				uint format  = image.GetFormat();
				uint width	 = image.GetWidth();
				uint height  = image.GetHeight();
				uint bpp	 = ITexture::GetBitsPerPixel(format) / 8;
				uint bytes	 = width * height * bpp;
				byte* buffer = (byte*)image.GetBuffer();
			
				if (format == ITexture::Format::RGB ||
					format == ITexture::Format::RGBA)
				{
					for (uint b = 0; b < bytes; )
					{
						if (x) buffer[b]	= 255 - buffer[b];
						if (y) buffer[b+1]	= 255 - buffer[b+1];
						b += bpp;
					}

					if (filename.EndsWith(".tga") || filename.EndsWith(".r5t"))
					{
						if (image.Save(filename))
						{
							SAVE_DONE;
						}
						else
						{
							WRITE_ERROR;
						}
					}
					else
					{
						String ext = System::GetExtensionFromFilename(filename);
						String file;
						filename.GetString(file, 0, filename.GetLength() - (ext.GetLength() + 1));
						filename = file;
						filename << ".tga";

						if (image.Save(filename))
						{
							SAVE_DONE;
						}
						else
						{
							WRITE_ERROR;
						}
					}
				}
				else
				{
					printf("ERROR: Can't flip '%s': only RGB and RGBA formats are supported.\n");
				}
			}
			else
			{
				READ_ERROR;
			}
		}
	}
	else
	{
		puts("Usage: NormalMapTool -[X][Y] filename");
		puts("Example 1: NormalMapTool -xy texture0.tga");
		puts("Example 2: NormalMapTool -y texture1.jpg");
		puts("Example 3: FontConverter texture2.png");
		puts("You can also drag the file in question onto this executable in order to flip the Y.");
	}
	puts("Press Enter to exit...");
	getchar();
	return 0;
}