#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Pathfinding test application
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{

class TestApp
{
public:

	struct Node
	{
		Node*		mParent;	// Node that led us to this one during the path search
		Node*		mNext;		// The next node in the sorted chain
		uint		mCost;		// Cost to get to this node from the start of the search
		uint		mEstimate;	// Estimated cost to the destination
		uint		mTotal;		// Expense + Estimate
		bool		mActive;	// Whether this node is currently active and has a valid cost
		bool		mOpen;		// Whether this node is currently open

		bool		mPassable;
		UIHighlight*	mHlt;
		UILabel*		mLblCost;
		UILabel*		mLblEstimate;
		UILabel*		mLblTotal;
		Vector2i	mPos;

		Node() :	mParent		(0),
					mNext		(0),
					mCost		(0),
					mEstimate	(0),
					mTotal		(0),
					mActive		(false),
					mOpen		(false),
					mPassable	(true),
					mHlt		(0),
					mLblCost	(0),
					mLblEstimate(0),
					mLblTotal	(0) {}
	};

protected:
	
	IWindow*		mWin;
	IGraphics*		mGraphics;
	UIManager*			mUI;
	Core*			mCore;
	UIFrame*		mRoot;
	UILabel*		mStatus;

	Array<Node>		mNodes;		// Pathfinding nodes
	Node*			mStart;		// Pointer to the starting node
	Node*			mEnd;		// Pointer to the destination
	Node*			mOpen;		// Pointer to the first open node
	Node*			mClosed;	// Pointer to the first closed node

public:

	// Pathfinding-related functions are first, just for clarity reasons
	void Restart();
	bool Advance();

	TestApp();
	~TestApp();

	void Run();
	void Update();
	void OnDraw();
	bool OnHighlightKey  (UIWidget* ptr, const Vector2i& pos, byte key, bool isDown);
	bool OnHighlightMove (UIWidget* ptr, const Vector2i& pos, const Vector2i& delta);
	bool OnKey (const Vector2i& pos, byte key, bool isDown);
};

}; // namespace R5