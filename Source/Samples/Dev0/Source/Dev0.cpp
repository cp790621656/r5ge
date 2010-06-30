//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================
// Application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	TreeNode root;
	Array<String> folders;
	Array<String> files;

	puts("SVN Multi-Repository Updater tool by Michael Lyashenko v1.0.1\n");

	if (root.Load(argc > 1 ? argv[1] : "repositories.txt"))
	{
		String command, url, path;

		FOREACH(i, root.mChildren)
		{
			const TreeNode& node = root.mChildren[i];
			url = node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString();
			if (url.IsEmpty()) continue;

			path = node.mTag;
			if (!path.EndsWith("/")) path << "/";

			if (!System::ReadFolder(path + ".svn", folders, files))
			{
				printf("\nChecking out %s\n\n", path.GetBuffer());
				command = "svn checkout """;
				command << url;
				command << """ """;
				command << path;
				command << """";
			}
			else
			{
				printf("\nUpdating %s\n", path.GetBuffer());
				command = "svn up """;
				command << path;
				command << """";
			}
			System::Execute(command.GetBuffer());
		}
	}
	else
	{
		System::Execute("svn up");
	}
	printf("\nDone!\n");
	getchar();
	return 0;
}