#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Expanded text line, adding start and end boundaries
// Author: Michael Lyashenko
//============================================================================================================

class UIBasicLabel : public UITextLine
{
protected:

	uint mStart;
	uint mEnd;

public:

	UIBasicLabel() : mStart(0), mEnd(0) {}

	// Area creation
	R5_DECLARE_INHERITED_CLASS(UIBasicLabel, UITextLine, UIWidget);

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue);
};