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

	ITexture* mMSAAColor;
	IRenderTarget* mMSAATarget;

	ITexture* mNormalColor;
	IRenderTarget* mNormalTarget;

public:

	TestApp();
	~TestApp();

	// Forward resizing events to the graphics controller
	virtual void OnResize (const Vector2i& size) { mGraphics->SetViewport(size); }

	// If escape gets pressed, close the window
	virtual bool OnKeyPress (const Vector2i& pos, byte key, bool isDown)
	{
		if (key == Key::Escape) mWindow->Close();
		return false;
	}

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

	// Create a render target that will be used to draw to texture
	mMSAAColor = mGraphics->CreateRenderTexture();
	mMSAATarget = mGraphics->CreateRenderTarget();
	mMSAATarget->SetMSAA(8);
	mMSAATarget->AttachColorTexture(0, mMSAAColor, ITexture::Format::RGBA);

	// Create the final render target that will contain the blitted result of anti-aliased rendering
	mNormalColor = mGraphics->CreateRenderTexture();
	mNormalTarget = mGraphics->CreateRenderTarget();
	mNormalTarget->AttachColorTexture(0, mNormalColor, ITexture::Format::RGBA);
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
			const Vector2i& s = mWindow->GetSize();

			// Draw into a multi-sampled render target
			mMSAATarget->SetSize(s);
			mGraphics->SetActiveRenderTarget(mMSAATarget);
			mGraphics->SetScreenProjection(true);
			mGraphics->SetBackgroundColor(Color4f(0, 0, 0, 0));
			mGraphics->Clear();
			DrawTriangle();

			// Copy the result into a regular render target
			mNormalTarget->SetSize(s);
			mMSAATarget->CopyTo(mNormalTarget);

			// NOTE: Deferred rendering MSAA process:
			// - Clear the MSAA target.
			// - Render the scene's materials into the MSAA target.
			// - Blit the result into a regular material target.
			// - Clear the MSAA target.
			// - Render lighting into the MSAA target.
			// - Blit the result into a regular lighting target.
			// - Combine step happens with regular targets.

			// Added cost: 2 blits (color+color+depth, color+color), and the anti-aliasing cost.
			// NOTE: It might help if I was rendering to a MSAA storage buffer rather than a texture. <-- Implement this.
			// NOTE: Would it help if the secondary texture was not MSAA'd?

			// Final step -- render to the screen
			mGraphics->SetActiveRenderTarget(0);
			mGraphics->SetScreenProjection(true);
			mGraphics->SetBackgroundColor(Color4f(0.25f, 0.25f, 0.25f, 1));
			mGraphics->Clear();
			mGraphics->SetActiveTexture(0, mNormalColor);
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