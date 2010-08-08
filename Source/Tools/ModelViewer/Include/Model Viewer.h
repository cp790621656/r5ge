#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Model Viewer -- Legacy application that keeps getting "patched", but not rewritten. As such its code is
// crap, but it does what it's supposed to do -- allows viewing and modification of basic model properties.
//============================================================================================================

class ModelViewer
{
	typedef DirectionalLight Light;
	typedef IMaterial::DrawMethod DrawMethod;

public:

	IWindow*		mWin;
	IGraphics*		mGraphics;
	UIManager*		mUI;
	Core*			mCore;
	Scene*			mScene;
	OSDrawDeferred*	mDraw;
	DebugCamera*	mCam;
	Light*			mLight;
	Object*			mStage;
	ModelInstance*	mInst;
	Model*			mModel;
	bool			mAnimate;
	UIHighlight*	mSbHighlight;
	UILabel*		mSbLabel;
	float			mTimestamp;
	uint			mResetCamera;
	String			mLoadFilename;
	Array<String>	mSavedHistory;

public:

	ModelViewer();
	~ModelViewer();

	const char* GetVersion() const { return "1.7.1"; }

	void  Run();
	void  OnDraw();
	float UpdateFPS();
	void  SetStatusText (const String& text, const Color3f& color = Color3f(1.0f));

	uint OnKeyPress		(const Vector2i& pos, byte key, bool isDown);
	uint OnMouseMove	(const Vector2i& pos, const Vector2i& delta);
	uint OnScroll		(const Vector2i& pos, float delta);

private:

	// Private functions
	void _ResetWindow()			{ mWin->SetSize( Vector2i(900, 600) ); mWin->SetStyle(IWindow::Style::Normal); }
	void _ToggleFullscreen()	{ mWin->SetStyle( mWin->GetStyle() == IWindow::Style::FullScreen ? IWindow::Style::Normal : IWindow::Style::FullScreen); }

public: // All of the functions below this line can be found in UILayout.cpp

	void Load();
	bool CreateUI();
	void OnFileSave();
	void ShowOpenDialog();
	void ShowSaveAsDialog();
	void ShowAboutInfo() { SetStatusText( String("R5 Model Viewer v.%s. Copyright (c) 2009-2010 Michael Lyashenko. [www.nextrevision.com]", GetVersion()) ); }
	void ShowConfirmClearMatsDialog();
	void ShowConfirmClearTexDialog();
	void ShowConfirmClearAnimDialog();

	// Functions that update UI values based on their respective graphics/scene counterparts
	void UpdateMatPanel	(const IMaterial* mat);
	void UpdateTexPanel	(const ITexture* tex);
	void UpdateAnimPanel(const Animation* anim);
	void UpdateTechPanel(const DrawMethod* method);

protected: // Functions that facilitate widget creation with common properties already set up

	UIMenu*			AddMenuItem		(const String& name);
	UIButton*		AddMenuButton	(const String& name);
	UIFrame*		AddArea			(const String& name, uint lines);
	UILabel*		AddCaption		(UIWidget* parent, uint line, const String& text);
	UILabel*		AddLabel		(UIWidget* parent, uint line, const String& name, int offset = 1);
	UIButton*		AddButton		(UIWidget* parent, uint line, const String& name, int offset = 1);
	UICheckbox*		AddCheckbox		(UIWidget* parent, uint line, const String& name);
	UIInput*		AddInput		(UIWidget* parent, uint line, const String& name, int offset = 1);
	UIList*			AddList			(UIWidget* parent, uint line, const String& name, int offset = 1);
	UISlider*		AddSlider		(UIWidget* parent, uint line, const String& name, int offset = 1);
	UIHighlight*	AddHighlight	(UIWidget* parent, uint line, const String& name, int offset = 1);

private: // Numerous callback functions triggered by the UI elements

	void OnConfirmDialogOK		(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);

	void ToggleBoth				(UIWidget* widget, uint state, bool isSet);
	void ToggleOff				(UIWidget* widget, uint state, bool isSet);

	void OnFillModelMenu		(UIWidget* widget, bool hasFocus);
	void OnFillLimbMenu			(UIWidget* widget, bool hasFocus);
	void OnFillMeshMenu			(UIWidget* widget, bool hasFocus);
	void OnFillMatMenu			(UIWidget* widget, bool hasFocus);
	void OnFillTexMenu			(UIWidget* widget, bool hasFocus);
	void OnFillAnimMenu			(UIWidget* widget, bool hasFocus);

	void OnFileMenuSelection	(UIWidget* widget);
	void OnViewMenuSelection	(UIWidget* widget);
	void OnLimbMenuSelection	(UIWidget* widget);
	void OnMeshMenuSelection	(UIWidget* widget);
	void OnMatMenuSelection		(UIWidget* widget);
	void OnTexMenuSelection		(UIWidget* widget);
	void OnAnimMenuSelection	(UIWidget* widget);

	void OnDrawMode				(UIWidget* widget, uint state, bool isSet);
	void OnBackground			(UIWidget* widget, uint state, bool isSet);
	void OnBloomToggle			(UIWidget* widget, uint state, bool isSet);
	void OnBloomChange			(UIWidget* widget);

	void OnModelBake			(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	void OnModelChange			(UIWidget* widget, bool hasFocus) { if (!hasFocus) _UpdateModelData(); }
	void OnModelToggle			(UIWidget* widget, uint state, bool isSet) { _UpdateModelData(); }
	void OnModelSpin			(UIWidget* widget, uint state, bool isSet);
	void OnModelBounds			(UIWidget* widget, uint state, bool isSet);

	void OnLimbSelect			(UIWidget* widget, bool hasFocus)	{ if (!hasFocus) _UpdateLimbData(); }
	void OnLimbChange			(UIWidget* widget)					{ _UpdateLimbData(); }

	void OnMatNameSelect		(UIWidget* widget, bool hasFocus);
	void OnMatNameValue			(UIWidget* widget);
	void OnMatProperties		(UIWidget* widget);
	void OnMatTech				(UIWidget* widget);
	void OnMatShaderList		(UIWidget* widget);
	void OnMatShaderInput		(UIWidget* widget, bool hasFocus);
	void OnMatShaderValue		(UIWidget* widget);
	void OnMatTexList			(UIWidget* widget);
	void OnMatTexInput			(UIWidget* widget, bool hasFocus);
	void OnMatTexValue			(UIWidget* widget);
	void OnMatColor				(UIWidget* widget, bool hasFocus);

	void OnColor				(UIWidget* widget);
	void OnColorSelect			(UIWidget* widget, bool hasFocus);

	void OnTexReload			(UIWidget* widget, bool hasFocus);
	void OnTexChange			(UIWidget* widget);

	void OnAnimNameSelect		(UIWidget* widget, bool hasFocus);
	void OnAnimNameValue		(UIWidget* widget);
	void OnAnimProperties		(UIWidget* widget, bool hasFocus);
	void OnAnimLoop				(UIWidget* widget, uint state, bool isSet);
	void OnAnimPlay				(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);

	// List selection callbacks -- they fill their appropriate lists with all the relevant entries
	void OnMeshListFocus		(UIWidget* widget, bool hasFocus);
	void OnMatListFocus			(UIWidget* widget, bool hasFocus);
	void OnTexListFocus			(UIWidget* widget, bool hasFocus);
	void OnShaderListFocus		(UIWidget* widget, bool hasFocus);

private:

	// Returns a pointer to the current rendering method
	DrawMethod* _GetCurrentMethod();

	// Changes the current material's values
	void _SetMatShader  (const String& name);
	void _SetMatTexture (uint index, const String& name);

	// Functions that update their respective graphics/scene counterparts based on UI values
	void _UpdateModelData();
	void _UpdateLimbData();
	void _CopyHistory();
};