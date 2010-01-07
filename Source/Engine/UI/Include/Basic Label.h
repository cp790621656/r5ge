#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Expanded text line, adding start and end boundaries
//============================================================================================================

class BasicLabel : public UITextLine
{
protected:

	uint mStart;
	uint mEnd;

public:

	BasicLabel() : mStart(0), mEnd(0) {}

	// Area creation
	R5_DECLARE_INHERITED_CLASS("Basic Label", BasicLabel, UITextLine, UIArea);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);
};