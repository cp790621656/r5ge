#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================
// Note to self: Reading filenames and directories:
// http://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1046380353&id=1044780608
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
		uint errors = 0;
		String source;

		for (int i = 1; i < argc; ++i)
		{
			TreeNode tree;
			String filename (argv[i]);

			if (filename.EndsWith(".txt"))
			{
				if (source.Load(argv[i]))
				{
					if (!source.BeginsWith("//R5A"))
					{
						source = String("//R5A\n") + source;
					}

					if (tree.Load((const byte*)source.GetBuffer(), source.GetLength()))
					{
						source.Clear();

						if (tree.SerializeTo(source))
						{
							source.Save(argv[i]);
						}
					}
					else
					{
						printf("Unable to load '%s'\n", argv[i]);
						++errors;
					}
				}
			}
		}

		if (errors != 0) getchar();
	}
	else
	{
		printf("You must specify a filename to clean up.\n");
		getchar();
	}
	return 0;
}