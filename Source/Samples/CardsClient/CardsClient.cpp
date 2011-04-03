//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// CardsClient: Networked game of cards (A-hole)
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

Core* g_core = 0;

//============================================================================================================

class CardsClient
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;

public:

	CardsClient();
	~CardsClient();
	void Run();
};

//============================================================================================================

CardsClient::CardsClient() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);

	UIScript::Register<USHand>();
	UIScript::Register<USConnect>();
	UIScript::Register<USMessageLog>();
	UIScript::Register<USCard>();
	UIScript::Register<USChatInput>();

	g_core = mCore;
}

//============================================================================================================

CardsClient::~CardsClient()
{
	g_core = 0;

	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void CardsClient::Run()
{
	if (*mCore << "Config/CardsClient.txt") while (mCore->Update());
}

//============================================================================================================

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");
	CardsClient app;
	app.Run();
	return 0;
}