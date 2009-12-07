#include "../Include/Noise Test.h"
using namespace R5;

//============================================================================================================
// Test application class
//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mCore(0), mCam(0), mUI(0), mRegenerate(true)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics);
	mCore		= new Core(mWin, mGraphics, mUI, mScene);

	mCore->SetSleepDelay(5);

	mUI->SetOnKey("128",  bind(&TestApp::Generate, this));
	mUI->SetOnKey("256",  bind(&TestApp::Generate, this));
	mUI->SetOnKey("512",  bind(&TestApp::Generate, this));
	mUI->SetOnKey("1024", bind(&TestApp::Generate, this));
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
// Sets up the user interface lists
//============================================================================================================

void TestApp::InitUI()
{
	UIWindow* parent = FindWidget<UIWindow>(mUI, "Options");
	UISkin*	skin	 = mUI->GetSkin("Default");
	IFont*	font	 = mUI->GetFont("Arial");

	if (parent != 0)
	{
		float padding = 2.0f;
		Array<String> filters;
		Noise::GetRegisteredFilters(filters);

		// Create and populate the 10 filter selection drop-down lists and property fields
		for (uint i = 0; i < 10; ++i)
		{
			String listName  ("Filter ");
			String inputName ("Input " );

			UIList*  list  = AddWidget<UIList> (parent, listName  << i);
			UIInput* input = AddWidget<UIInput>(parent, inputName << i);

			if (list != 0)
			{
				UIRegion& r = list->GetRegion();
				r.SetLeft	(0.0f,  padding);
				r.SetRight	(0.5f, -padding);
				r.SetTop	(0.0f,  padding + i * 20);
				r.SetBottom	(0.0f,  padding + i * 20 + 18);

				list->SetSkin(skin);
				list->SetFont(font);
				list->ClearAllEntries();
				list->AddEntry("");

				for (uint f = 0; f < filters.GetSize(); ++f)
					list->AddEntry(filters[f]);

				// Every selection means tooltips might need to change as well
				list->SetOnStateChange( bind(&TestApp::UpdateTooltips, this) );
			}

			if (input != 0)
			{
				UIRegion& r = input->GetRegion();
				r.SetLeft	(0.5f,  padding);
				r.SetRight	(1.0f, -padding);
				r.SetTop	(0.0f,  padding + i * 20);
				r.SetBottom	(0.0f,  padding + i * 20 + 18);

				input->SetSkin(skin);
				input->SetFace("Grey Area");
				input->SetFont(font);
			}
		}
		parent->SetAlpha(1.0f);
	}
}

//============================================================================================================
// Application loop
//============================================================================================================

void TestApp::Run()
{
    if (*mCore << "Config/Noise Test.txt")
	{
		// Initialize the generated UI components
		InitUI();

		// Generate the noise and update the tooltips
		Generate(0, Vector2i(), Key::MouseLeft, false);
		UpdateTooltips(0);

		mCam = FindObject<Camera>(mScene, "Default Camera");

		if (mCam != 0)
		{
			mCore->SetListener( bind(&TestApp::OnDraw, this) );
			mCore->SetListener( bind(&Camera::OnMouseMove, mCam) );
			mCore->SetListener( bind(&Camera::OnScroll, mCam) );

			while (mCore->Update());
			*mCore >> "Config/Noise Test.txt";
		}
	}
}

//============================================================================================================
// Draw the scene
//============================================================================================================

void TestApp::OnDraw()
{
	if (mRegenerate) Regenerate();
	mScene.Cull(mCam);
	mScene.DrawAllForward();
}

//============================================================================================================
// Regenerate the noise
//============================================================================================================

void TestApp::Regenerate()
{
	// Get the noise texture
	static ITexture*	noiseTex	= mGraphics->GetTexture("Noise");
	static ITexture*	nmapTex		= mGraphics->GetTexture("Normal Map");

	static UILabel*		seedLabel	= FindWidget<UILabel> (mUI, "Seed");
	static UILabel*		timeLabel	= FindWidget<UILabel> (mUI, "Time");
	static UIWindow*	window		= FindWidget<UIWindow>(mUI, "Texture Window");

	if (mNoise.GetWidth() > 0 && mNoise.GetHeight() > 0)
	{
		mRegenerate = false;

		// Get the current time so the Time::GetMilliseconds() returns the latest value
		Time::Update();
		ulong startTime = Time::GetMilliseconds();

		// Use the timestamp as the seed
		mNoise.SetSeed(3246234234);
		//mNoise.SetSeamless(false);

		// Generate the noise
		const float* buffer = mNoise.GetBuffer();
		uint width  = mNoise.GetWidth();
		uint height = mNoise.GetHeight();
		
		// Get the time difference
		Time::Update();
		ulong diff = Time::GetMilliseconds() - startTime;

		// Create the noise texture
		noiseTex->Set(buffer, width, height, 1,
				  ITexture::Format::Float,
				  ITexture::Format::Luminance);

		// Generate normals
		Array<Color4ub> colors;
		Image::Utilities::HeightMapToNormalMap(buffer, width, height, colors, mNoise.IsSeamless());

		// Create the normal map texture
		nmapTex->Set(colors, width, height, 1,
				ITexture::Format::RGBA,
				ITexture::Format::RGB);

		// Cleanup
		colors.Release();
		mNoise.Release();

		// Update the UI information
		if (seedLabel) seedLabel->SetText( String("Seed: %u", startTime) );
		if (timeLabel) timeLabel->SetText( String("Time: %u", diff) );
		if (window)
		{
			Vector2i size = window->GetSizeForContent((float)mNoise.GetWidth(), mNoise.GetHeight()*2.0f);

			UIRegion& rgn = window->GetRegion();
			rgn.SetLeft		(1.0f, -(float)size.x);
			rgn.SetRight	(1.0f, 0.0f);
			rgn.SetTop		(0.0f, 0.0f);
			rgn.SetBottom	(0.0f, (float)size.y);

			window->SetAlpha(1.0f, 0.15f);
			window->SetKeyboardFocus();
		}
	}
}

//============================================================================================================
// Triggered when "Generate" button is clicked in the UI
//============================================================================================================

bool TestApp::Generate (UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	if (!isDown && key == Key::MouseLeft)
	{
		static UIWindow* parent = FindWidget<UIWindow>(mUI, "Options");

		ushort dimension (256);
		UIButton* btn = R5_CAST(UIButton, area);
		if (btn != 0) btn->GetText() >> dimension;

		if (parent != 0)
		{
			mNoise.Release(true);
			mNoise.SetSize(dimension, dimension);

			// Retrieve the current filters and fill the noise
			for (uint i = 0; i < 10; ++i)
			{
				String listName  ("Filter ");
				String inputName ("Input " );

				UIList*  list  = FindWidget<UIList> (parent, listName  << i, false);
				UIInput* input = FindWidget<UIInput>(parent, inputName << i, false);

				if (list != 0 && input != 0)
				{
					const String& filter (list->GetText());
					const String& params (input->GetText());

					if (filter.IsValid())
					{
						mNoise.ApplyFilter(filter, params);
					}
				}
			}
			mRegenerate = true;
		}
	}
	return true;
}

//============================================================================================================
// Triggered when filters change
//============================================================================================================

bool TestApp::UpdateTooltips (UIArea* area)
{
	static UIWindow* parent = FindWidget<UIWindow>(mUI, "Options");
	ASSERT(parent != NULL, "Missing parameter window");

	// Retrieve the current filters and fill the noise
	for (uint i = 0; i < 10; ++i)
	{
		String listName  ("Filter ");
		String inputName ("Input " );

		UIList*  list  = FindWidget<UIList> (parent, listName  << i, false);
		UIInput* input = FindWidget<UIInput>(parent, inputName << i, false);

		if (list != 0 && input != 0)
		{
			const String& filter (list->GetText());

			String tooltip;

			if (filter.IsEmpty())			tooltip = "Select a new filter from the drop-down list";
			else if (filter == "Simple")	tooltip = "Simple random noise takes no parameters";
			else if (filter == "Fractal")	tooltip = "Fractal noise (up to 3 parameters): Octaves, ridge threshold, octave multiplier";
			else if (filter == "Perlin")	tooltip = "Perlin noise (up to 3 parameters): Octaves, ridge threshold, smoothness";
			else if (filter == "Normalize")	tooltip = "Normalizes the noise to be within 0-1 range (no parameters)";
			else if (filter == "Blur")		tooltip = "Gaussian blur filter, 3 parameters: number of passes, min/max boundary";
			else if (filter == "Power")		tooltip = "'To the power of' filter, 1 parameter: Power";
			else if (filter == "Sqrt")		tooltip = "Square root filter (no parameters)";
			else if (filter == "Add")		tooltip = "Add filter, 1 parameter: Value to add";
			else if (filter == "Multiply")	tooltip = "Multiplication filter, 1 parameter: Value to multiply by";
			else if (filter == "Round")		tooltip = "Round filter, 1 parameter: Precision";
			else if (filter == "Clamp")		tooltip = "Clamp filter, 2 parameters: Low boundary, high boundary";
			else if (filter == "Mirror")	tooltip = "Mirror filter, 2 parameters: Low boundary, high boundary";

			list->SetTooltip(tooltip);
			input->SetTooltip(tooltip);
		}
	}
	return true;
}

//============================================================================================================
// Entry point for the application
//============================================================================================================

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	if (!System::SetCurrentPath("../Resources/"))
		 System::SetCurrentPath("../../../Resources/");
    TestApp app;
    app.Run();
	return 0;
}