#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Development Testing Application
//============================================================================================================

class ModelViewer
{
	typedef DirectionalLight Light;
	typedef IMaterial::RenderingMethod RenderingMethod;

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
	Root*			mUI;
	Core*			mCore;
	Scene*			mScene;
	DebugCamera*	mCam;
	Light*			mLight;
	Object*			mStage;
	ModelInstance*	mInst;
	Model*			mModel;
	DrawParams		mParams;
	bool			mAnimate;
	Highlight*		mSbHighlight;
	Label*			mSbLabel;
	float			mTimestamp;
	bool			mResetCamera;
	String			mLoadFilename;
	Array<String>	mSavedHistory;

public:

	ModelViewer();
	~ModelViewer();

	const char* GetVersion() const { return "1.3.5"; }

	void  Run();
	void  OnDraw();
	uint  OnDeferredDraw(const ITechnique* tech, bool insideOut);
	float UpdateFPS();
	void  SetStatusText (const String& text, const Color3f& color = Color3f(1.0f));

	bool OnKey			(const Vector2i& pos, byte key, bool isDown);
	bool OnMouseMove	(const Vector2i& pos, const Vector2i& delta);
	bool OnScroll		(const Vector2i& pos, float delta);
	bool SerializeFrom	(const TreeNode& root);
	bool SerializeTo	(TreeNode& root) const;

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
	void UpdateTechPanel		(const RenderingMethod* method);

protected: // Functions that facilitate widget creation with common properties already set up

	Menu*		AddMenuItem		(const String& name);
	Button*		AddMenuButton	(const String& name);
	Frame*		AddArea			(const String& name, uint lines);
	Label*		AddCaption		(Area* parent, uint line, const String& text);
	Label*		AddLabel		(Area* parent, uint line, const String& name, int offset = 1);
	Button*		AddButton		(Area* parent, uint line, const String& name, int offset = 1);
	Checkbox*	AddCheckbox		(Area* parent, uint line, const String& name);
	Input*		AddInput		(Area* parent, uint line, const String& name, int offset = 1);
	List*		AddList			(Area* parent, uint line, const String& name, int offset = 1);
	Slider*		AddSlider		(Area* parent, uint line, const String& name, int offset = 1);
	Highlight*	AddHighlight	(Area* parent, uint line, const String& name, int offset = 1);

private: // Numerous callback functions triggered by the UI elements

	bool OnFileInputValue		(Area* area) { _ConfirmFileDialog(); return true; }
	bool OnFileDialogOK			(Area* area, const Vector2i& pos, byte key, bool isDown);
	bool OnConfirmDialogOK		(Area* area, const Vector2i& pos, byte key, bool isDown);

	bool ToggleBoth				(Area* area);
	bool ToggleOff				(Area* area);

	bool OnFillModelMenu		(Area* area, bool hasFocus);
	bool OnFillLimbMenu			(Area* area, bool hasFocus);
	bool OnFillMeshMenu			(Area* area, bool hasFocus);
	bool OnFillMatMenu			(Area* area, bool hasFocus);
	bool OnFillTexMenu			(Area* area, bool hasFocus);
	bool OnFillAnimMenu			(Area* area, bool hasFocus);

	bool OnFileMenuSelection	(Area* area);
	bool OnViewMenuSelection	(Area* area);
	bool OnLimbMenuSelection	(Area* area);
	bool OnMeshMenuSelection	(Area* area);
	bool OnMatMenuSelection		(Area* area);
	bool OnTexMenuSelection		(Area* area);
	bool OnAnimMenuSelection	(Area* area);

	bool OnRenderMode			(Area* area);
	bool OnBackground			(Area* area);
	bool OnBloomToggle			(Area* area);
	bool OnBloomChange			(Area* area);

	bool OnModelBake			(Area* area, const Vector2i& pos, byte key, bool isDown);
	bool OnModelChange			(Area* area, bool hasFocus)		{ if (!hasFocus) _UpdateModelData();	return true; }
	bool OnModelToggle			(Area* area)					{ _UpdateModelData();					return true; }
	bool OnModelSpin			(Area* area);
	bool OnModelBounds			(Area* area);

	bool OnLimbSelect			(Area* area, bool hasFocus)		{ if (!hasFocus) _UpdateLimbData();		return true; }
	bool OnLimbChange			(Area* area)					{ _UpdateLimbData();					return true; }

	bool OnMatNameSelect		(Area* area, bool hasFocus);
	bool OnMatNameValue			(Area* area);
	bool OnMatProperties		(Area* area);
	bool OnMatTech				(Area* area);
	bool OnMatShaderList		(Area* area);
	bool OnMatShaderInput		(Area* area, bool hasFocus);
	bool OnMatShaderValue		(Area* area);
	bool OnMatTexList			(Area* area);
	bool OnMatTexInput			(Area* area, bool hasFocus);
	bool OnMatTexValue			(Area* area);
	bool OnMatColor				(Area* area, bool hasFocus);

	bool OnColor				(Area* area);
	bool OnColorSelect			(Area* area, bool hasFocus);

	bool OnTexReload			(Area* area, bool hasFocus);
	bool OnTexChange			(Area* area);

	bool OnAnimNameSelect		(Area* area, bool hasFocus);
	bool OnAnimNameValue		(Area* area);
	bool OnAnimProperties		(Area* area, bool hasFocus);
	bool OnAnimLoop				(Area* area);
	bool OnAnimPlay				(Area* area, const Vector2i& pos, byte key, bool isDown);

	// List selection callbacks -- they fill their appropriate lists with all the relevant entries
	bool OnMeshListFocus		(Area* area, bool hasFocus);
	bool OnMatListFocus			(Area* area, bool hasFocus);
	bool OnTexListFocus			(Area* area, bool hasFocus);
	bool OnShaderListFocus		(Area* area, bool hasFocus);

private:

	// Returns a pointer to the current rendering method
	RenderingMethod* _GetCurrentMethod();

	// Changes the current material's values
	void _SetMatShader  (const String& name);
	void _SetMatTexture (uint index, const String& name);

	// Functions that update their respective graphics/scene counterparts based on UI values
	void _UpdateModelData();
	void _UpdateLimbData();
	void _ConfirmFileDialog();
	void _CopyHistory();
};