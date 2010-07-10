//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"
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
	System::SetCurrentPath("../../../Resources/Config");
	
	Array<String> files;
	Array<String> folders;
	String temp;

	System::ReadFolder(System::GetCurrentPath(), folders, files);

	FOREACH(i, files)
	{
		const String& file = files[i];

		if (file.EndsWith(".txt") && temp.Load(file))
		{
			printf("Processing '%s'... ", file.GetBuffer());
			uint count (0);
			/*count += temp.Replace(UIWidget::ClassID(), "UIWidget");
			count += temp.Replace(UIHighlight::ClassID(), "UIHighlight");
			count += temp.Replace(UIPicture::ClassID(), "UIPicture");
			count += temp.Replace(UISubPicture::ClassID(), "UISubPicture");
			count += temp.Replace(UILabel::ClassID(), "UILabel");
			count += temp.Replace(UITextArea::ClassID(), "UITextArea");
			count += temp.Replace(UIInput::ClassID(), "UIInput");
			count += temp.Replace(UIContext::ClassID(), "UIContext");
			count += temp.Replace(UIMenu::ClassID(), "UIMenu");
			count += temp.Replace(UIList::ClassID(), "UIList");
			count += temp.Replace(UIWindow::ClassID(), "UIWindow");
			count += temp.Replace(UIAnimatedFrame::ClassID(), "UIFrame");
			count += temp.Replace(UIAnimatedSlider::ClassID(), "UISlider");
			count += temp.Replace(UIAnimatedButton::ClassID(), "UIButton");
			count += temp.Replace(UIAnimatedCheckbox::ClassID(), "UICheckbox");
			count += temp.Replace(UIShadedArea::ClassID(), "UIShadedArea");
			count += temp.Replace(UIStats::ClassID(), "UIStats");
			count += temp.Replace("USEventUIListener", "USEventListener");*/
			//count += temp.Replace("UIWindow\n", "Window\n");
			count += temp.Replace("\t\n", "\n");
			printf("%u\n", count);
			temp.Save(file);
		}
	}

	printf("\nDone!\n");
	getchar();
	return 0;
}