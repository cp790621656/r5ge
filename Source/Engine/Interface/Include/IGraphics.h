#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Complete graphics controller class -- combines controller and manager functionality into a single class
//============================================================================================================

struct IGraphics : public IGraphicsController, public IGraphicsManager
{
	R5_DECLARE_INTERFACE_CLASS("Graphics");

	virtual ~IGraphics() {};

	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo   (TreeNode& root) const=0;
};