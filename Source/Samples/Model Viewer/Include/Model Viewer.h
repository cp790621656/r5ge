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
	UIRoot*			mUI;
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
	void UpdateTechPanel		(const DrawMethod* method);

protected: // Functions that facilitate widget creation with common properties already set up

	UIMenu*			AddMenuItem		(const String& name);
	UIButton*		AddMenuButton	(const String& name);
	UIFrame*		AddArea			(const String& name, uint lines);
	UILabel*		AddCaption		(UIArea* parent, uint line, const String& text);
	UILabel*		AddLabel		(UIArea* parent, uint line, const String& name, int offset = 1);
	UIButton*		AddButton		(UIArea* parent, uint line, const String& name, int offset = 1);
	UICheckbox*		AddCheckbox		(UIArea* parent, uint line, const String& name);
	UIInput*		AddInput		(UIArea* parent, uint line, const String& name, int offset = 1);
	UIList*			AddList			(UIArea* parent, uint line, const String& name, int offset = 1);
	UISlider*		AddSlider		(UIArea* parent, uint line, const String& name, int offset = 1);
	UIHighlight*	AddHighlight	(UIArea* parent, uint line, const String& name, int offset = 1);

private: // Numerous callback functions triggered by the UI elements

	bool OnFileInputValue		(UIArea* area) { _ConfirmFileDialog(); return true; }
	bool OnFileDialogOK			(UIArea* area, const Vector2i& pos, byte key, bool isDown);
	bool OnConfirmDialogOK		(UIArea* area, const Vector2i& pos, byte key, bool isDown);

	bool ToggleBoth				(UIArea* area);
	bool ToggleOff				(UIArea* area);

	bool OnFillModelMenu		(UIArea* area, bool hasFocus);
	bool OnFillLimbMenu			(UIArea* area, bool hasFocus);
	bool OnFillMeshMenu			(UIArea* area, bool hasFocus);
	bool OnFillMatMenu			(UIArea* area, bool hasFocus);
	bool OnFillTexMenu			(UIArea* area, bool hasFocus);
	bool OnFillAnimMenu			(UIArea* area, bool hasFocus);

	bool OnFileMenuSelection	(UIArea* area);
	bool OnViewMenuSelection	(UIArea* area);
	bool OnLimbMenuSelection	(UIArea* area);
	bool OnMeshMenuSelection	(UIArea* area);
	bool OnMatMenuSelection		(UIArea* area);
	bool OnTexMenuSelection		(UIArea* area);
	bool OnAnimMenuSelection	(UIArea* area);

	bool OnDrawMode				(UIArea* area);
	bool OnBackground			(UIArea* area);
	bool OnBloomToggle			(UIArea* area);
	bool OnBloomChange			(UIArea* area);

	bool OnModelBake			(UIArea* area, const Vector2i& pos, byte key, bool isDown);
	bool OnModelChange			(UIArea* area, bool hasFocus)	{ if (!hasFocus) _UpdateModelData();	return true; }
	bool OnModelToggle			(UIArea* area)					{ _UpdateModelData();					return true; }
	bool OnModelSpin			(UIArea* area);
	bool OnModelBounds			(UIArea* area);

	bool OnLimbSelect			(UIArea* area, bool hasFocus)	{ if (!hasFocus) _UpdateLimbData();		return true; }
	bool OnLimbChange			(UIArea* area)					{ _UpdateLimbData();					return true; }

	bool OnMatNameSelect		(UIArea* area, bool hasFocus);
	bool OnMatNameValue			(UIArea* area);
	bool OnMatProperties		(UIArea* area);
	bool OnMatTech				(UIArea* area);
	bool OnMatShaderList		(UIArea* area);
	bool OnMatShaderInput		(UIArea* area, bool hasFocus);
	bool OnMatShaderValue		(UIArea* area);
	bool OnMatTexList			(UIArea* area);
	bool OnMatTexInput			(UIArea* area, bool hasFocus);
	bool OnMatTexValue			(UIArea* area);
	bool OnMatColor				(UIArea* area, bool hasFocus);

	bool OnColor				(UIArea* area);
	bool OnColorSelect			(UIArea* area, bool hasFocus);

	bool OnTexReload			(UIArea* area, bool hasFocus);
	bool OnTexChange			(UIArea* area);

	bool OnAnimNameSelect		(UIArea* area, bool hasFocus);
	bool OnAnimNameValue		(UIArea* area);
	bool OnAnimProperties		(UIArea* area, bool hasFocus);
	bool OnAnimLoop				(UIArea* area);
	bool OnAnimPlay				(UIArea* area, const Vector2i& pos, byte key, bool isDown);

	// List selection callbacks -- they fill their appropriate lists with all the relevant entries
	bool OnMeshListFocus		(UIArea* area, bool hasFocus);
	bool OnMatListFocus			(UIArea* area, bool hasFocus);
	bool OnTexListFocus			(UIArea* area, bool hasFocus);
	bool OnShaderListFocus		(UIArea* area, bool hasFocus);

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