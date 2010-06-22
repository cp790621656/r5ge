#include "../Include/Dev5.h"
using namespace R5;

//============================================================================================================

#define WIDTH  900
#define HEIGHT 600
#define XCOUNT 30
#define YCOUNT 20

#define GREY	Color3f(0.85f, 0.85f, 0.85f)
#define WALL	Color3f(0.212f, 0.106f, 0.149f)
#define OPEN	Color3f(0.5f, 1.0f, 0.5f)
#define CLOSED	Color3f(0.5f, 0.8f, 0.5f)
#define START	Color3f(0.5f, 0.5f, 1.0f)
#define END		Color3f(0.5f, 0.75f, 1.0f)
#define FINAL	Color3f(0.8f, 0.8f, 0.5f)
#define ACTIVE	Color3f(1.0f, 1.0f, 0.5f)

//============================================================================================================
// Estimation method
//============================================================================================================
// 0 = Normal
// 1 = Overestimate by 25%
// 2 = Overestimate by 50%
// 3 = Overestimate by 75%
// 4 = Overestimate by 100%
//============================================================================================================

uint g_overestimation = 0;
bool g_backtracking = true;

//============================================================================================================
// Helper function that returns the estimated distance between two points
//============================================================================================================

uint GetEstimate (const Vector2i& start, const Vector2i& end)
{
	uint xDist = abs( start.x - end.x );
	uint yDist = abs( start.y - end.y );

	uint val =  (xDist > yDist) ? 14 * yDist + 10 * (xDist - yDist) :
								  14 * xDist + 10 * (yDist - xDist);

	return val + (val * g_overestimation) / 4;
}

//============================================================================================================
// Helper function that compares the cost of two nodes
//============================================================================================================

typedef TestApp::Node* NodePtr;

inline bool IsCloser (NodePtr left, NodePtr right)
{
	if (left->mTotal < right->mTotal)
	{
		return true;
	}
	if (left->mTotal == right->mTotal)
	{
		// Even if nodes are equal, left side still takes precedence (I want node added last to come first)
		return (left->mEstimate <= right->mEstimate);
	}
	return false;
}

//============================================================================================================
// Helper function that inserts the specified node into the linked list starting at 'start'
//============================================================================================================

void Insert (NodePtr node, NodePtr& start)
{
	// If the starting node is absent or has a higher cost, add this node at the very beginning
	if ( start == 0 || IsCloser(node, start) )
	{
		node->mNext = start;
		start = node;
	}
	else
	{
		for (NodePtr ptr = start;; ptr = ptr->mNext)
		{
			// If there is no followup node
			if (ptr->mNext == 0)
			{
				ptr->mNext = node;
				break;
			}

			// If the next node is more expensive, insert the newly created node here
			if ( IsCloser(node, ptr->mNext) )
			{
				node->mNext = ptr->mNext;
				ptr->mNext = node;
				break;
			}
		}
	}
}

//============================================================================================================
// Resets the path, returning to the default starting values
//============================================================================================================

void TestApp::Restart()
{
	while (mOpen != 0)
	{
		Node* next = mOpen->mNext;
		mOpen->mNext = 0;
		mOpen->mActive = false;
		mOpen = next;
	}

	while (mClosed != 0)
	{
		Node* next = mClosed->mNext;
		mClosed->mNext = 0;
		mClosed->mActive = false;
		mClosed = next;
	}

	mOpen = 0;
	mClosed = 0;

	if (mStart != 0 && mEnd != 0)
	{
		mOpen				= mStart;
		mOpen->mParent		= 0;
		mOpen->mNext		= 0;
		mOpen->mCost		= 0;
		mOpen->mEstimate	= GetEstimate(mStart->mPos, mEnd->mPos);
		mOpen->mTotal		= mOpen->mEstimate;
		mOpen->mActive		= true;
		mOpen->mOpen		= true;
	}
}

//============================================================================================================
// Advances the search by one iteration
//============================================================================================================

bool TestApp::Advance()
{
	// If there are no open nodes to work with or we've reached the end, then there is nothing left to do
	if (mOpen == 0 || mStart == mEnd || mOpen == mEnd) return false;

	// Remove the first open node and move it to the closed list
	Node* node	= mOpen;
	mOpen		= mOpen->mNext;
	node->mNext	= mClosed;
	node->mOpen	= false;
	mClosed		= node;

	// We want to go through the surrounding nodes
	int cx	 = node->mPos.x;
	int cy	 = node->mPos.y;
	int xmin = cx - 1;
	int ymin = cy - 1;
	int xmax = cx + 2;
	int ymax = cy + 2;

	// Run through all surrounding nodes
	for (int y = ymin; y < ymax; ++y)
	{
		for (int x = xmin; x < xmax; ++x)
		{
			// Skip the node we're on
			if (x == cx && y == cy) continue;

			// Skip nodes outside the grid
			if (x < 0 || y < 0 || x >= XCOUNT || y >= YCOUNT) continue;

			// Get the node
			Node* near = &mNodes[y * XCOUNT + x];

			// Skip impassable nodes
			if (!near->mPassable) continue;

			// Movement cost is 14 diagonally, 10 otherwise
			uint movement = ((x == cx || y == cy) ? 10 : 14);

			// If the node is active, see if we can reach the current node quicker via this one
			if (near->mActive)
			{
				uint cost = near->mCost + movement;

				// If a better path was found, update the parent and the cost
				if (cost < node->mCost)
				{
					node->mParent	= near;
					node->mCost		= cost;
					node->mTotal	= node->mCost + node->mEstimate;
				}

				if (g_backtracking)
				{
					// Check if we can reach that nearby node faster by going through this one
					cost = node->mCost + movement;

					if (cost < near->mCost)
					{
						near->mParent	= node;
						near->mCost		= cost;
						near->mTotal	= near->mCost + near->mEstimate;
					}
				}
			}
			else
			{
				// The node is not yet active -- make it active
				near->mParent	= node;
				near->mNext		= 0;
				near->mCost		= node->mCost + movement;
				near->mEstimate	= GetEstimate(near->mPos, mEnd->mPos);
				near->mTotal	= near->mCost + near->mEstimate;
				near->mActive	= true;
				near->mOpen		= true;

				// Insert this node into the open list, sorting it in the process
				Insert(near, mOpen);

				// If we've reached the end, we're done
				if (near == mEnd) return false;
			}
		}
	}
	return true;
}

//============================================================================================================
// Initialize the application and create the grid
//============================================================================================================

TestApp::TestApp()
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);
	mRoot		= mUI->AddWidget<UIFrame>("Root");
	mStart		= 0;
	mEnd		= 0;
	mOpen		= 0;
	mClosed		= 0;

	mWin->Create("R5 Engine: Dev5 (Pathfinding)", 100, 100, WIDTH, HEIGHT, IWindow::Style::Normal);

	mGraphics->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

	mCore->AddOnKey( bind(&TestApp::OnKeyPress, this) );

	IFont* font0 = mGraphics->GetFont("Arial 12");
	IFont* font1 = mGraphics->GetFont("Arial 15");

	mStatus = mRoot->AddWidget<UILabel>("Status");
	mStatus->SetFont(font1);
	mStatus->GetRegion().SetTop(1.0f, -20.0f);
	mStatus->SetText("Keys: [S]tart, [E]nd, [A]dvance, [C]omplete, [O]verestimate, [B]acktrace");
	mStatus->SetShadow(true);
	mStatus->SetLayer(2);
	mStatus->SetAlignment( UILabel::Alignment::Center );
	mStatus->SetEventHandling( UIWidget::EventHandling::None );
	
	float fx = ((float)WIDTH  / XCOUNT) / WIDTH;
	float fy = ((float)HEIGHT / YCOUNT) / HEIGHT;

	for (uint y = 0; y < YCOUNT; ++y)
	{
		for (uint x = 0; x < XCOUNT; ++x)
		{
			String identifier ( String(" %u %u", x, y) );

			UIHighlight*	hlt  = mRoot->AddWidget<UIHighlight>("Highlight" + identifier);
			UILabel*		lbl0 = hlt->AddWidget<UILabel>("Label0" + identifier);
			UILabel*		lbl1 = hlt->AddWidget<UILabel>("Label1" + identifier);
			UILabel*		lbl2 = hlt->AddWidget<UILabel>("Label2" + identifier);
			UIRegion&		rgn  = hlt->GetRegion();

			rgn.SetLeft		(fx * x, 1.0f);
			rgn.SetTop		(fy * y, 1.0f);
			rgn.SetRight	(fx * (x + 1), -1.0f);
			rgn.SetBottom	(fy * (y + 1), -1.0f);

			USEventListener* listener = hlt->AddScript<USEventListener>();
			listener->SetOnKey( bind(&TestApp::OnHighlightKey, this) );
			listener->SetOnMouseMove( bind(&TestApp::OnHighlightMove, this) );

			lbl0->SetFont(font0);
			lbl0->SetEventHandling( UIWidget::EventHandling::None );
			lbl0->SetAlignment( UILabel::Alignment::Center );
			lbl0->SetColor(0);
			lbl0->SetShadow(false);
			lbl0->SetLayer(1, false);
			lbl0->GetRegion().SetBottom(0.333f, 0.0f);

			lbl1->SetFont(font1);
			lbl1->SetEventHandling( UIWidget::EventHandling::None );
			lbl1->SetAlignment( UILabel::Alignment::Center );
			lbl1->SetColor(0);
			lbl1->SetShadow(false);
			lbl1->SetLayer(1, false);

			lbl2->SetFont(font0);
			lbl2->SetEventHandling( UIWidget::EventHandling::None );
			lbl2->SetAlignment( UILabel::Alignment::Center );
			lbl2->SetColor(0);
			lbl2->SetShadow(false);
			lbl2->SetLayer(1, false);
			lbl2->GetRegion().SetTop(0.667f, 0.0f);

			Node& node = mNodes.Expand();
			node.mPos.Set(x, y);

			node.mHlt			= hlt;
			node.mLblCost		= lbl0;
			node.mLblEstimate	= lbl2;
			node.mLblTotal		= lbl1;
		}
	}
	Update();
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
	while ( mCore->Update() );
}

//============================================================================================================
// Goes through all nodes and updates the visible grid, making the changes visible
//============================================================================================================

void TestApp::Update()
{
	for (uint i = 0; i < mNodes.GetSize(); ++i)
	{
		Node* node = &mNodes[i];

		if		(node == mStart)	node->mHlt->SetColor( START );
		else if (node == mEnd)		node->mHlt->SetColor( END );
		else if (!node->mPassable)	node->mHlt->SetColor( WALL );
		else if (node->mActive)
		{
			if (node->mOpen)		node->mHlt->SetColor( OPEN );
			else					node->mHlt->SetColor( CLOSED );
		}
		else						node->mHlt->SetColor( GREY );

		if (node->mActive)
		{
			node->mLblCost->SetText		( String("%u", node->mCost) );
			node->mLblEstimate->SetText	( String("%u", node->mEstimate) );
			node->mLblTotal->SetText	( String("%u", node->mTotal) );
		}
		else
		{
			node->mLblCost->SetText("");
			node->mLblEstimate->SetText("");
			node->mLblTotal->SetText("");
		}
	}

	Node* closest = mClosed;

	if (closest != 0)
	{
		closest->mHlt->SetColor( ACTIVE );
		closest = closest->mParent;
	}

	while (closest != 0 && closest != mStart)
	{
		if (closest != mEnd) closest->mHlt->SetColor( FINAL );
		closest = closest->mParent;
	}
}

//============================================================================================================
// Delegate triggered when a key is pressed over the grid
//============================================================================================================

void TestApp::OnHighlightKey (UIWidget* ptr, const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::MouseLeft || key == Key::MouseRight)
	{
		if (isDown) OnHighlightMove(ptr, pos, Vector2i(0, 0));
	}
}

//============================================================================================================
// Delegate triggered when mouse is moving over the grid
//============================================================================================================

void TestApp::OnHighlightMove (UIWidget* ptr, const Vector2i& pos, const Vector2i& delta)
{
	bool left  = mCore->IsKeyDown(Key::MouseLeft);
	bool right = mCore->IsKeyDown(Key::MouseRight);
	
	if (left || right)
	{
		UIHighlight* hlt = mUI->FindWidget<UIHighlight>(pos);

		if (hlt != 0)
		{
			if (mStart != 0 && mStart->mHlt == hlt) return;
			if (mEnd   != 0 && mEnd->mHlt   == hlt) return;

			for (uint i = 0; i < mNodes.GetSize(); ++i)
			{
				Node& node = mNodes[i];

				if (node.mHlt == hlt)
				{
					node.mPassable = !left;
					if (node.mActive) Restart();
					Update();
				}
			}
		}
	}
}

//============================================================================================================
// Application keypress callback
//============================================================================================================

uint TestApp::OnKeyPress (const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown)
	{
		if ( key == Key::Escape )
		{
			mCore->Shutdown();
			return 1;
		}
		else if ( key == Key::F5 )
		{
			mWin->SetStyle( mWin->GetStyle() == IWindow::Style::FullScreen ?
				IWindow::Style::Normal : IWindow::Style::FullScreen);
			return 1;
		}
		else if ( key == Key::F6 )
		{
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
			return 1;
		}
		else if ( key == Key::R )
		{
			Restart();
			Update();
			return 1;
		}
		else if ( key == Key::S )
		{
			UIHighlight* hlt = mUI->FindWidget<UIHighlight>(pos);

			if (hlt != 0)
			{
				for (uint i = 0; i < mNodes.GetSize(); ++i)
				{
					Node* node = &mNodes[i];

					if (node->mHlt == hlt)
					{
						mStart = node;
						Restart();
						Update();
						break;
					}
				}
			}
			return 1;
		}
		else if ( key == Key::E )
		{
			UIHighlight* hlt = mUI->FindWidget<UIHighlight>(pos);
			
			if (hlt != 0)
			{
				for (uint i = 0; i < mNodes.GetSize(); ++i)
				{
					Node* node = &mNodes[i];

					if (node->mHlt == hlt)
					{
						mEnd = node;
						Restart();
						Update();
						break;
					}
				}
			}
			return 1;
		}
		else if ( key == Key::A )
		{
			if (mStart != 0 && mEnd != 0)
			{
				Advance();
				Update();
			}
			return 1;
		}
		else if ( key == Key::C )
		{
			if (mStart != 0 && mEnd != 0)
			{
				Restart();

				uint iterations = 0;
				while (Advance()) { ++iterations; }

				Update();

				mStatus->SetText( String("Iterations: %u", iterations) );
			}
			return 1;
		}
		else if ( key == Key::O )
		{
			if (++g_overestimation > 4) g_overestimation = 0;
			mStatus->SetText( String("Overestimation: %u%%", g_overestimation * 25) );
			return 1;
		}
		else if ( key == Key::B )
		{
			g_backtracking = !g_backtracking;
			mStatus->SetText( g_backtracking ? "Backtracking is ON" : "Backtracking is OFF" );
			return 1;
		}
	}
	return 0;
}


//============================================================================================================
// Application entry point
//============================================================================================================

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");
    TestApp app;
    app.Run();
	return 0;
}