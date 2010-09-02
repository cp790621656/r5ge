//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// BundleMaker is a tool that can create and extract asset bundles
//============================================================================================================

#include "../../Engine/Image/Include/_All.h"
using namespace R5;

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

	//"Models/Buildings/Barracks.r5c" "Textures/Buildings/Barracks" "Shaders/Deferred/building"
	bool showUsage = true;

	printf("Working folder: %s\n", System::GetCurrentPath().GetBuffer());

	if (argc > 1)
	{
		String bundleFile;
		Array<String> files;

		// Collect all files referenced via arguments
		for (int i = 1; i < argc; ++i)
		{
			String arg (argv[i]);
			arg.Replace("\\", "/");

			if (arg.EndsWith(".r5d")) bundleFile = arg;
			else System::GetFiles(arg, files, true);
		}

		// If no bundle file was specified, use the default one
		if (bundleFile.IsEmpty() && files.IsValid()) bundleFile = "Bundle.r5d";

		// Get the file's name and the directory from the bundle's filename
		String name (System::GetFilenameFromPath(bundleFile));
		String dir  (System::GetPathFromFilename(bundleFile));

		if (bundleFile.IsEmpty())
		{
			showUsage = true;
		}
		else if (files.IsEmpty())
		{
			showUsage = false;

			Bundle b;

			if (b.Load(bundleFile))
			{
				printf("Bundle: %s\n", b.GetName().GetBuffer());

				const Array<Bundle::Asset>& assets = b.GetAllAssets();

				Memory mem, temp;

				FOREACH(i, assets)
				{
					const Bundle::Asset& asset = assets[i];
					printf("  %s... ", asset.name.GetBuffer());
					String assetName;
					assetName << asset.name;

					if (!b.Extract(assetName, mem))
					{
						puts("Failed to extract");
					}
					else if (mem.Save(asset.name))
					{
						puts("extracted");
					}
					else
					{
						puts("SKIPPED");
					}
				}
			}
			else
			{
				printf("Failed to parse %s\n", bundleFile.GetBuffer());
			}
		}
		else if (files.IsValid())
		{
			showUsage = false;

			Memory mem;
			Memory temp;
			mem.Append("//R5D", 5);

			printf("Creating %s...\n", bundleFile.GetBuffer());

			FOREACH(i, files)
			{
				String file (files[i]);
				printf("  %s ", file.GetBuffer());

				if (temp.Load(file) && temp.GetSize() > 0)
				{
					bool compressed = false;

					// Automatically compress R5A, R5B, and TGA files
					if (file.EndsWith(".r5a") || file.EndsWith(".r5b"))
					{
						TreeNode root;

						if (!root.Load(temp.GetBuffer(), temp.GetSize()))
						{
							puts("invalid format");
							continue;
						}

						Memory raw;
						root.SerializeTo(raw);

						temp.Clear();
						temp.Append("//R5C", 5);
						Compress(raw, temp);

						file.Replace(".r5a", ".r5c");
						file.Replace(".r5b", ".r5c");

						printf("compressed to ");
					}
					else if (file.EndsWith(".tga"))
					{
						Image img;

						if (!img.Load(temp.GetBuffer(), temp.GetSize()))
						{
							puts("invalid format");
							continue;
						}

						temp.Clear();

						if (!img.Save(temp, "r5t"))
						{
							puts("save failed");
							continue;
						}
						file.Replace(".tga", ".r5t");
						printf("compressed to ");
					}
					else if (file.EndsWith(".txt") || file.EndsWith(".frag") || file.EndsWith(".vert"))
					{
						Memory comp;

						if (!Compress(temp, comp))
						{
							puts("failed to load");
							continue;
						}

						temp = comp;
						compressed = true;
						printf("compressed to ");
					}

					mem.Append(file);
					mem.Append(compressed);
					mem.AppendSize(temp.GetSize());
					mem.Append(temp.GetBuffer(), temp.GetSize());
					printf("%s bytes\n", String::GetFormattedSize(temp.GetSize()).GetBuffer());
				}
				else
				{
					puts("SKIPPED");
				}
			}
				
			if (mem.GetSize() > 5 && mem.Save(bundleFile))
			{
				printf("Bundle size: %s bytes\n", String::GetFormattedSize(mem.GetSize()).GetBuffer());
			}
			else
			{
				puts("Failed to create the asset bundle!");
			}
		}
	}
	
	if (showUsage)
	{
		puts("R5 Bundle Maker Tool v.1.0.0 by Michael Lyashenko");
		puts("Usage: BundleMaker [file/folder 1] [file/folder 2] [...]");
		puts("Example 1: BundleMaker bundle.r5d model.r5c texture0.png texture1.png");
		puts("Example 2: BundleMaker bundle.r5d");
		puts("Example 3: BundleMaker bundle.r5d *.jpg *.png");
	}
#ifdef _DEBUG
	getchar();
#endif
	return 0;
}