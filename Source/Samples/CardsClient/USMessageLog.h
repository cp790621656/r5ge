#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Simple status bar
// Author: Michael Lyashenko
//============================================================================================================

class USMessageLog : public UIScript
{
public:

	R5_DECLARE_INHERITED_CLASS(USMessageLog, UIScript, UIScript);

	static void Show (const String& text);

	virtual void OnInit();
	virtual void OnDestroy();
};