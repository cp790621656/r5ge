#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple status bar
//============================================================================================================

class USMessageLog : public UIScript
{
public:

	R5_DECLARE_INHERITED_CLASS("USMessageLog", USMessageLog, UIScript, UIScript);

	static void Show (const String& text);

	virtual void OnInit();
	virtual void OnDestroy();
};