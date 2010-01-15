#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Development Testing Application
//============================================================================================================

class ModelViewer
{
	typedef DirectionalLight Light;
	typedef IMaterial::DrawMethod DrawMethod;

	struct DrawParams
	{
		bool	mGrid;
		bool	mBloom;
		uint	mSsao;
		float	mThreshold;

		DrawParams() : mGrid(true), mBloom(false), mSsao(0), mThreshold(0.75f) {}
	};

public:

	IWindow*		mWin;
	IGraphics*		mGraphics;
	UIManager*			mUI;
	Core*			mCore;
	Scene			mScene;
	DebugCamera*	mCam;
	Light*			mLight;
	Object*			mStage;
	ModelInstance*	mInst;
	Model*			mModel;
	DrawParams		mParams;
	bool			mAnimate;
	UIHighlight*	mSbHighlight;
	UILabel*		mSbLabel;
	float			mTimestamp;
	bool			mResetCamera;
	String			mLoadFilename;
	Array<String>	mSavedHistory;

public:

	ModelViewer();
	~ModelViewer();

	const char* GetVersion() const { return "1.3.7"; }

	void  Run();
	void  OnDraw();
	uint  OnDeferredDraw(const Deferred::TechniqueList& techs, bool insideOut);
	float UpdateFPS();
	void  SetStatusText (const String& text, const Color3f& color = Color3f(1.0f));

	bool OnKey			(const Vector2i& pos, byte key, bool isDown);
	bool OnMouseMove	(const Vector2i& pos, const Vector2i& delta);
	bool OnScroll		(const Vector2i& pos, float delta);
	bool SerializeFrom	(const TreeNode& root);
	void SerializeTo	(TreeNode& root) const;

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
	void ShowAboutInfo() { SetStatusText( String("R5 Model Viewer v.%s. Copyright (c) 2009 Michael Lyashenko. [www.nextrevision.com]", GetVersion()) ); }
	void ShowConfirmClearMatsDialog();
	void ShowConfirmClearTexDialog();
	void ShowConfirmClearAnimDialog();

	// Functions that update UI values based on their respective graphics/scene counterparts
	void UpdateMatPanel			(const IMaterial* mat);
	void UpdateTexPanel			(const ITexture* tex);
	void UpdateAnimPanel		(const Animation* anim);
	void UpdateTechPanel		(const DrawMethod* method);

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

	bool OnFileInputValue		(UIWidget* widget) { _ConfirmFileDialog(); return true; }
	bool OnFileDialogOK			(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	bool OnConfirmDialogOK		(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);

	bool ToggleBoth				(UIWidget* widget, uint state, bool isSet);
	bool ToggleOff				(UIWidget* widget, uint state, bool isSet);

	bool OnFillModelMenu		(UIWidget* widget, bool hasFocus);
	bool OnFillLimbMenu			(UIWidget* widget, bool hasFocus);
	bool OnFillMeshMenu			(UIWidget* widget, bool hasFocus);
	bool OnFillMatMenu			(UIWidget* widget, bool hasFocus);
	bool OnFillTexMenu			(UIWidget* widget, bool hasFocus);
	bool OnFillAnimMenu			(UIWidget* widget, bool hasFocus);

	bool OnFileMenuSelection	(UIWidget* widget);
	bool OnViewMenuSelection	(UIWidget* widget);
	bool OnLimbMenuSelection	(UIWidget* widget);
	bool OnMeshMenuSelection	(UIWidget* widget);
	bool OnMatMenuSelection		(UIWidget* widget);
	bool OnTexMenuSelection		(UIWidget* widget);
	bool OnAnimMenuSelection	(UIWidget* widget);

	bool OnDrawMode				(UIWidget* widget, uint state, bool isSet);
	bool OnBackground			(UIWidget* widget, uint state, bool isSet);
	bool OnBloomToggle			(UIWidget* widget, uint state, bool isSet);
	bool OnBloomChange			(UIWidget* widget);

	bool OnModelBake			(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);
	bool OnModelChange			(UIWidget* widget, bool hasFocus) { if (!hasFocus) _UpdateModelData(); return true; }
	bool OnModelToggle			(UIWidget* widget, uint state, bool isSet) { _UpdateModelData(); return true; }
	bool OnModelSpin			(UIWidget* widget, uint state, bool isSet);
	bool OnModelBounds			(UIWidget* widget, uint state, bool isSet);

	bool OnLimbSelect			(UIWidget* widget, bool hasFocus)	{ if (!hasFocus) _UpdateLimbData(); return true; }
	bool OnLimbChange			(UIWidget* widget)					{ _UpdateLimbData(); return true; }

	bool OnMatNameSelect		(UIWidget* widget, bool hasFocus);
	bool OnMatNameValue			(UIWidget* widget);
	bool OnMatProperties		(UIWidget* widget);
	bool OnMatTech				(UIWidget* widget);
	bool OnMatShaderList		(UIWidget* widget);
	bool OnMatShaderInput		(UIWidget* widget, bool hasFocus);
	bool OnMatShaderValue		(UIWidget* widget);
	bool OnMatTexList			(UIWidget* widget);
	bool OnMatTexInput			(UIWidget* widget, bool hasFocus);
	bool OnMatTexValue			(UIWidget* widget);
	bool OnMatColor				(UIWidget* widget, bool hasFocus);

	bool OnColor				(UIWidget* widget);
	bool OnColorSelect			(UIWidget* widget, bool hasFocus);

	bool OnTexReload			(UIWidget* widget, bool hasFocus);
	bool OnTexChange			(UIWidget* widget);

	bool OnAnimNameSelect		(UIWidget* widget, bool hasFocus);
	bool OnAnimNameValue		(UIWidget* widget);
	bool OnAnimProperties		(UIWidget* widget, bool hasFocus);
	bool OnAnimLoop				(UIWidget* widget, uint state, bool isSet);
	bool OnAnimPlay				(UIWidget* widget, const Vector2i& pos, byte key, bool isDown);

	// List selection callbacks -- they fill their appropriate lists with all the relevant entries
	bool OnMeshListFocus		(UIWidget* widget, bool hasFocus);
	bool OnMatListFocus			(UIWidget* widget, bool hasFocus);
	bool OnTexListFocus			(UIWidget* widget, bool hasFocus);
	bool OnShaderListFocus		(UIWidget* widget, bool hasFocus);

private:

	// Returns a pointer to the current rendering method
	DrawMethod* _GetCurrentMethod();

	// Changes the current material's values
	void _SetMatShader  (const String& name);
	void _SetMatTexture (uint index, const String& name);

	// Functions that update their respective graphics/scene counterparts based on UI values
	void _UpdateModelData();
	void _UpdateLimbData();
	void _ConfirmFileDialog();
	void _CopyHistory();
};