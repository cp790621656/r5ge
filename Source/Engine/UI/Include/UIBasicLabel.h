#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Expanded text line, adding start and end boundaries
//============================================================================================================

class UIBasicLabel : public UITextLine
{
protected:

	uint mStart;
	uint mEnd;

public:

	UIBasicLabel() : mStart(0), mEnd(0) {}

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UIBasicLabel", UIBasicLabel, UITextLine, UIWidget);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);
};