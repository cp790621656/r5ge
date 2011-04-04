#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/OpenGL/Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// GL Rendering Test: Application that exposes raw OpenGL functionality to the developer.
//============================================================================================================

class TestApp : public IEventReceiver
{
	IWindow* mWindow;
	IGraphics* mGraphics;

	ITexture* mColor;
	IRenderTarget* mTarget;

public:

	TestApp();
	~TestApp();

	// Forward resizing events to the graphics controller
	virtual void OnResize (const Vector2i& size) { mGraphics->SetViewport(size); }

	// If escape gets pressed, close the window
	virtual bool OnKeyPress (const Vector2i& pos, byte key, bool isDown) { if (key == Key::Escape) mWindow->Close(); return false; }

	void Run();
	void DrawTriangle();
};

//============================================================================================================

TestApp::TestApp()
{
	mWindow = new GLWindow();
	mGraphics = new GLGraphics();

	mWindow->SetGraphics(mGraphics);
	mWindow->SetEventHandler(this);
	mWindow->Create("GL Renderer Test", 100, 100);

	// Create a texture to draw to, instead of the screen
	mColor = mGraphics->CreateRenderTexture();
	//mColor->SetFiltering(ITexture::Filter::Linear);

	// Create a render target that will be used to draw to texture
	mTarget = mGraphics->CreateRenderTarget();
	mTarget->SetMSAA(8);
	mTarget->AttachColorTexture(0, mColor, ITexture::Format::RGBA);
}

//============================================================================================================

TestApp::~TestApp()
{
	delete mGraphics;
	delete mWindow;
}

//============================================================================================================
// Main loop
//============================================================================================================

void TestApp::Run()
{
	IShader* shader = mGraphics->GetShader("Other/TestAA");

	ulong nextUpdate = 0;

	while (mWindow->Update())
	{
		Time::Update();
		Time::IncrementFPS();

		if (nextUpdate < Time::GetMilliseconds())
		{
			nextUpdate = Time::GetMilliseconds() + 250;
			mWindow->SetTitle(String("FPS: %u", Time::GetFPS()));
		}

		mWindow->BeginFrame();
		mGraphics->BeginFrame();
		{
			mTarget->SetSize(mWindow->GetSize());
			mGraphics->SetActiveRenderTarget(mTarget);
			mGraphics->SetScreenProjection(true);
			mGraphics->SetBackgroundColor(Color4f(0, 0, 0, 0));
			mGraphics->Clear();

			DrawTriangle();

			mGraphics->SetActiveRenderTarget(0);
			mGraphics->SetScreenProjection(true);
			mGraphics->SetBackgroundColor(Color4f(0.25f, 0.25f, 0.25f, 1));
			mGraphics->Clear();

			mGraphics->SetActiveTexture(0, mColor);
			mGraphics->SetActiveShader(shader);
			mGraphics->Draw(IGraphics::Drawable::FullscreenQuad);
			mGraphics->SetActiveMaterial(0);
		}
		mGraphics->EndFrame();
		mWindow->EndFrame();
		Thread::Sleep(0);
	}
}

//============================================================================================================
// Draw a triangle
//============================================================================================================

void TestApp::DrawTriangle()
{
	const Vector2i& size = mGraphics->GetViewport();

	glBegin(GL_TRIANGLES);
	{
		glColor4ub(255, 0, 0, 255);
		glVertex2f(size.x * 0.5f, size.y * 0.15f);

		glColor4ub(0, 255, 0, 255);
		glVertex2f(size.x * 0.15f, size.y * 0.5f);

		glColor4ub(0, 0, 255, 255);
		glVertex2f(size.x * 0.85f, size.y * 0.85f);
	}
	glEnd();

	glColor4ub(255, 255, 255, 255);
}

//============================================================================================================

R5_MAIN_FUNCTION
{
	System::SetCurrentPath("../../../Resources/");
	TestApp app;
    app.Run();
	return 0;
}