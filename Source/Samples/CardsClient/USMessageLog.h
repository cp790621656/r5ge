#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Simple status bar
// Author: Michael Lyashenko
//============================================================================================================

class USMessageLog : public UIScript
{
public:

	R5_DECLARE_INHERITED_CLASS("USMessageLog", USMessageLog, UIScript, UIScript);

	static void Show (const String& text);

	virtual void OnInit();
	virtual void OnDestroy();
};