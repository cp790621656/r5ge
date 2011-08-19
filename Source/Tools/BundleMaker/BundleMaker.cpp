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

		bool extractTGA = false;
		char param = 0;

		// Collect all files referenced via arguments
		for (int i = 1; i < argc; ++i)
		{
			String arg (argv[i]);
			arg.Replace("\\", "/");

			if (arg[0] == '-')
			{
				if		(arg == "-o") param = 1;
				else if (arg == "-c") param = 2;
				else if (arg == "-t") extractTGA = true;
			}
			else if (param == 1 || arg.EndsWith(".r5d"))
			{
				param = 0;
				bundleFile = arg;
			}
			else if (param == 2)
			{
				param = 0;
				String& ext = extensionList.Expand();
				ext = ".";
				ext << arg;
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

					if (!b.Extract(asset.name, mem))
					{
						puts("Failed to extract");
					}
					else
					{
						if (extractTGA && asset.name.EndsWith(".r5t"))
						{
							Image img;

							if (img.Load(mem.GetBuffer(), mem.GetSize()))
							{
								String file = asset.name;
								file.Replace(".r5t", ".tga");
								img.Save(file);
								puts("-> TGA");
							}
							else
							{
								puts("INVALID");
							}
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

					bool isR5A = file.EndsWith(".r5a");
					bool isR5B = !isR5B && file.EndsWith(".r5b");
					bool isTGA = file.EndsWith(".tga");
					bool isPNG = !isTGA && file.EndsWith(".png");

					// Automatically compress R5A, R5B, TGA and PNG files
					if (isR5A || isR5B)
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

						if (isR5A) file.Replace(".r5a", ".r5c");
						if (isR5B) file.Replace(".r5b", ".r5c");

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
		puts("R5 Bundle Maker Tool v.1.4.0 by Michael Lyashenko");
		puts("Usage: BundleMaker [file/folder 1] [file/folder 2] [...]");
		puts("    -c <extension> -- compress files with the specified extension");
		puts("    -o <filename>  -- output filename");
		puts("    -t             -- save all textures in TGA format\n");

		puts("Example 1: Bundle the specified set of files:");
		puts("           BundleMaker model.r5c texture0.png texture1.png -o bundle.r5d\n");

		puts("Example 2: Unbundle everything inside the specified bundle:");
		puts("           BundleMaker bundle.r5d\n");

		puts("Example 3: Create a bundle called 'bundle.r5d' with all JPGs and PNGs:");
		puts("           BundleMaker *.jpg *.png -o bundle.r5d\n");

		puts("Example 4: Compress FBX and HTML extension files:");
		puts("           BundleMaker *.* -c fbx -c html -o bundle.r5d");
	}
	puts("\nPress Enter to exit...");
	getchar();
	return 0;
}