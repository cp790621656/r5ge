#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Complete graphics controller class -- combines controller and manager functionality into a single class
// Author: Michael Lyashenko
//============================================================================================================

struct IGraphics : public IGraphicsController, public IGraphicsManager
{
	R5_DECLARE_INTERFACE_CLASS("Graphics");

	virtual ~IGraphics() {};

	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo   (TreeNode& root) const=0;
};