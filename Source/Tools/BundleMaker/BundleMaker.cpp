//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// BundleMaker is a tool that can create and extract asset bundles
// Author: Michael Lyashenko
//============================================================================================================

#include "../../Engine/Image/Include/_All.h"
using namespace R5;

//============================================================================================================
// Main application entry point
//============================================================================================================

int main(int argc, char* argv[])
{
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());

#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif

	bool showUsage = true;

	String folder (System::GetCurrentPath());
	printf("Working folder: %s\n", folder.GetBuffer());

	if (argc > 1)
	{
		String bundleFile;
		Array<String> files;

		// Files with these extensions that will be compressed automatically
		Array<String> extensionList;
		extensionList.Expand() = ".txt";
		extensionList.Expand() = ".frag";
		extensionList.Expand() = ".vert";
		extensionList.Expand() = ".shader";

		// Collect all files referenced via arguments
		for (int i = 1; i < argc; ++i)
		{
			String arg (argv[i]);
			arg.Replace("\\", "/");

			if (arg.EndsWith(".r5d"))
			{
				bundleFile = arg;
			}
			else if (arg[0] == '-')
			{
				// Additional extensions to be compressed are specified via a dash
				String& ext = extensionList.Expand();
				ext = arg;
				ext[0] = '.';
			}
			else System::GetFiles(arg, files, true);
		}

		// If no bundle file was specified, use the default one
		if (bundleFile.IsEmpty() && files.IsValid()) bundleFile = "Bundle.r5d";

		// Get the file's name and the directory from the bundle's filename
		String name (System::GetFilenameFromPath(bundleFile));
		String dir  (System::GetPathFromFilename(bundleFile));

		// Remove the folder references
		FOREACH(i, files)
		{
			if (files[i].BeginsWith(folder)) files[i].Erase(0, folder.GetSize());
		}
		if (dir.BeginsWith(folder)) dir.Erase(0, folder.GetSize());

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

					bool isTGA = file.EndsWith(".tga");
					bool isPNG = !isTGA && file.EndsWith(".png");

					// Automatically compress R5A, R5B, TGA and PNG files
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
					else if (isTGA || isPNG)
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
						if (isTGA) file.Replace(".tga", ".r5t");
						if (isPNG) file.Replace(".png", ".r5t");
						printf("compressed to ");
					}
					else
					{
						FOREACH(b, extensionList)
						{
							const String& ext (extensionList[b]);

							if (file.EndsWith(ext))
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
								break;
							}
						}
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
		puts("R5 Bundle Maker Tool v.1.3.1 by Michael Lyashenko");
		puts("Usage: BundleMaker [file/folder 1] [file/folder 2] [...]\n");

		puts("Example 1: Bundle the specified set of files:");
		puts("           BundleMaker bundle.r5d model.r5c texture0.png texture1.png\n");

		puts("Example 2: Unbundle everything inside the specified bundle:");
		puts("           BundleMaker bundle.r5d\n");

		puts("Example 3: Create a bundle called 'bundle.r5d' with all JPGs and PNGs:");
		puts("           BundleMaker bundle.r5d *.jpg *.png\n");

		puts("Example 4: Compress FBX and HTML extension files:");
		puts("           BundleMaker bundle.r5d *.* -fbx -html");
	}
	puts("\nPress Enter to exit...");
	getchar();
	return 0;
}