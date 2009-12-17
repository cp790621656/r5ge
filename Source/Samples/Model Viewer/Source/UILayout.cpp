#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Used by options creation functions
//============================================================================================================

#define OFFSET		23.0f
#define WIDTH		350.0f
#define SEPARATOR	0.35f
#define PADDING		3.0f
#define LINE		17.0f
#define NEW			"[FFB948]---  New  ---"
#define CLEAR		"[FFB948]--- Clear ---"
#define TEXTURES	4

#define RESET_CAMERA		"Reset Camera [FFB948](F9)"
#define RESET_WINDOW		"Reset Window [FFB948](F6)"
#define TOGGLE_FULLSCREEN	"Toggle Fullscreen [FFB948](F5)"
#define ABOUT				"Credits"

#ifdef _WINDOWS
 #define SAVE	"Save [FFB948](Ctrl+S)"
 #define SAVEAS "Save As [FFB948](Ctrl+A)"
 #define OPEN	"Open [FFB948](Ctrl+O)"
 #define EXIT	"Exit [FFB948](Escape)"
#else
 #define SAVE	"Save [FFB948](Cmd+S)"
 #define SAVEAS "Save As [FFB948](Cmd+A)"
 #define OPEN	"Open [FFB948](Cmd+O)"
 #define EXIT	"Exit [FFB948](Escape)"
#endif

//============================================================================================================
// Not the best code, I agree, but keeping it local here makes everything much more compartmentalized.
//============================================================================================================

IFont*			_font			= 0;
UISkin*			_skin			= 0;
ITechnique*		_deferred0		= 0;
ITechnique*		_wireframe		= 0;

UIFrame*		_loadFrame		= 0;

float			_menuOffset		= 0.0f;
UIFrame*		_menuFrame		= 0;

UIMenu*			_fileMenu		= 0;
UIMenu*			_limbMenu		= 0;
UIMenu*			_meshMenu		= 0;
UIMenu*			_matMenu		= 0;
UIMenu*			_texMenu		= 0;
UIMenu*			_animMenu		= 0;

UIWindow*		_fileDialog		= 0;
UILabel*		_fileLabel		= 0;
UIInput*		_fileInput		= 0;
UIButton*		_fileOK			= 0;

UIWindow*		_confirmDialog	= 0;
UILabel*		_confirmLabel	= 0;
UIButton*		_confirmOK		= 0;

UIFrame*		_optFrame		= 0;

UIFrame*		_modelFrame		= 0;
UILabel*		_modelTri		= 0;
UILabel*		_modelSource	= 0;
UIInput*		_modelPos		= 0;
UIInput*		_modelRot		= 0;
UIInput*		_modelScale		= 0;
UICheckbox*		_modelUsePos	= 0;
UICheckbox*		_modelUseRot	= 0;
UICheckbox*		_modelUseScale	= 0;
UICheckbox*		_modelSpin		= 0;
UICheckbox*		_modelBounds	= 0;
UIButton*		_modelBake		= 0;

UIFrame*		_limbFrame		= 0;
UIInput*		_limbName		= 0;
UIList*			_limbMesh		= 0;
UIList*			_limbMat		= 0;

UIFrame*		_meshFrame		= 0;
UILabel*		_meshName		= 0;
UILabel*		_meshTri		= 0;

UIFrame*		_matFrame		= 0;
UIInput*		_matName		= 0;
UIHighlight*	_matDiff		= 0;
UIHighlight*	_matSpec		= 0;
UISlider*		_matGlow		= 0;
UISlider*		_matADT			= 0;
UIList*			_matTech		= 0;
UIList*			_matShaderList	= 0;
UIInput*		_matShaderInput	= 0;
UIList*			_matTexList  [TEXTURES]	= {0};
UIInput*		_matTexInput [TEXTURES]	= {0};

UIFrame*		_colorFrame		= 0;
UILabel*		_colorLabel		= 0;
UISlider*		_colorRed		= 0;
UISlider*		_colorGreen		= 0;
UISlider*		_colorBlue		= 0;
UISlider*		_colorAlpha		= 0;

UIFrame*		_techFrame		= 0;
UIFrame*		_detFrame		= 0;
UIFrame*		_colFrame		= 0;

UIFrame*		_texFrame		= 0;
UILabel*		_texName		= 0;
UILabel*		_texDims		= 0;
UILabel*		_texSize		= 0;
UIList*			_texWrap		= 0;
UIList*			_texFilter		= 0;
UIList*			_texFormat		= 0;
UIPicture*		_texImage		= 0;

UIFrame*		_animFrame		= 0;
UIInput*		_animName		= 0;
UIInput*		_animRange		= 0;
UIInput*		_animDuration	= 0;
UIButton*		_animPlay		= 0;
UICheckbox*		_animLoop		= 0;

String			_currentLimb;
String			_currentMesh;
String			_currentMat;
String			_currentTex;
String			_currentTech;
String			_currentAnim;

//============================================================================================================
// A pair of empty functions that are used to intercept keyboard and mouse events for frames
//============================================================================================================

bool OnIgnoreKey	(UIArea* area, const Vector2i& pos, byte key, bool isDown)	{ return true; }
bool OnIgnoreMouse	(UIArea* area, const Vector2i& pos, const Vector2i& delta)	{ return true; }

//============================================================================================================
// Helper function used by widget creation functions
//============================================================================================================

void SetRegion (UIRegion& rgn, uint line, int flag, float x, float y)
{
	float offset = PADDING * 2.0f + (PADDING + LINE) * line;
	float xp = x + PADDING;

	rgn.SetTop	  (0.0f, offset + y);
	rgn.SetBottom (0.0f, offset + LINE - y);

	if (flag == 0)
	{
		rgn.SetLeft	 (0.0f, xp + PADDING);
		rgn.SetRight (SEPARATOR, -xp);
	}
	else if (flag == 1)
	{
		rgn.SetLeft	 (SEPARATOR, xp);
		rgn.SetRight (1.0f, -xp - PADDING);
	}
	else if (flag == 2)
	{
		rgn.SetLeft	 (SEPARATOR, xp + LINE + PADDING);
		rgn.SetRight (1.0f, -xp - PADDING);
	}
}

//============================================================================================================
// Simple callback function that hides the area's parent
//============================================================================================================

bool HideParent (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		UIFrame* parent = R5_CAST(UIFrame, area->GetParent());
		if (parent != 0) parent->Hide();
	}
	return true;
}

//============================================================================================================
// Model loading routine, triggered from another thread
//============================================================================================================

void ModelViewer::Load()
{
	_loadFrame->Show();

	SetStatusText("Loading, please wait...");

	// Release the model
	mModel->Release(false, false, false);

	// Release all meshes
	Core::Meshes& meshes = mCore->GetAllMeshes();

	meshes.Lock();
	{
		// NOTE: Meshes have to be released first as we are not in the graphics thread here
		for (uint i = 0; i < meshes.GetSize(); ++i) meshes[i]->Release();
		meshes.Clear();
	}
	meshes.Unlock();

	// Load the model from the specified file
	if ( mModel->Load(mLoadFilename) )
	{
		_fileInput->AddToHistory(mLoadFilename);
		_loadFrame->Hide();
		mInst->SetDirty();
		mModel->SetDirty();
		mResetCamera = true;
		SetStatusText( String("Loaded '%s'", mLoadFilename.GetBuffer()) );
	}
	else
	{
		_loadFrame->Hide();
		SetStatusText("Either path is invalid or the requested file cannot be opened", Color3f(1.0f, 0.0f, 0.0f));
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void ModelViewer::SerializeTo (TreeNode& root) const
{
	if (_fileInput != 0)
	{
		const UIInput::HistoryList& history = _fileInput->GetHistory();

		if (history.IsValid())
		{
			TreeNode& node = root.AddChild("File History");

			history.Lock();
			node.mValue = history;
			history.Unlock();
		}
	}
}

//============================================================================================================
// Create the initial user interface and bind all callbacks
//============================================================================================================

bool ModelViewer::CreateUI()
{
	_font		= mUI->GetDefaultFont();
	_skin		= mUI->GetDefaultSkin();
	_deferred0	= mGraphics->GetTechnique("Deferred");
	_wireframe	= mGraphics->GetTechnique("Wireframe");

	if (_font == 0)
	{
		_font = mGraphics->GetFont("Arial 15");
	}

	if (_font == 0 || _skin == 0) return false;

	// Create the menu frame at the top of the screen
	{
		_menuFrame = AddWidget<UIFrame>(mUI, "Menu Frame");
		_menuFrame->SetSerializable(false);
		{
			UIRegion& rgn = _menuFrame->GetRegion();
			rgn.SetTop	 (0.0f, -3.0f);
			rgn.SetBottom(0.0f, 24.0f);
			rgn.SetRight (0.0f, 0.0f);
		}
	}

	// File menu
	{
		_fileMenu = AddMenuItem("File");
		_fileMenu->AddEntry(OPEN);
		_fileMenu->AddEntry(SAVE);
		_fileMenu->AddEntry(SAVEAS);
		_fileMenu->AddEntry(EXIT);
		_fileMenu->SetOnValueChange( bind(&ModelViewer::OnFileMenuSelection, this) );
		_fileMenu->SetSticky(false);
		_fileMenu->SetTooltip("File menu allows you to open and save models");
	}

	// View menu
	{
		UIMenu* menu = AddMenuItem("View");
		menu->AddEntry(RESET_CAMERA);
		menu->AddEntry(RESET_WINDOW);
		menu->AddEntry(TOGGLE_FULLSCREEN);
		menu->AddEntry(ABOUT);
		menu->SetSticky(false);
		menu->SetTooltip("Click and find out!");
		menu->SetOnValueChange( bind(&ModelViewer::OnViewMenuSelection, this) );
	}

	// The options menu is different as it has to be saved for options to stay persistent
	{
		_optFrame = FindWidget<UIFrame>(mUI, "Options Frame");

		if (_optFrame != 0)
		{
			UIButton* btn = AddMenuButton("Options");
			btn->SetTooltip("Non-model related options: rendering method, background, bloom");

			_optFrame->SetAlpha(0.0f);
			_optFrame->SetOnKey( &OnIgnoreKey );
			_optFrame->SetOnMouseMove( &OnIgnoreMouse );

			UISubPicture*	bnd = FindWidget<UISubPicture>	(_optFrame, "Options Background", false);
			UIList*			ren = FindWidget<UIList>		(_optFrame, "Rendering Method");
			UIList*			bgd = FindWidget<UIList>		(_optFrame, "Background");
			UICheckbox*		chk = FindWidget<UICheckbox>	(_optFrame, "Bloom Checkbox");
			UISlider*			sld = FindWidget<UISlider>		(_optFrame, "Bloom Slider");

			if (bnd != 0)
			{
				bnd->SetOnKey( &OnIgnoreKey );
				bnd->SetOnMouseMove( &OnIgnoreMouse );
			}

			if (ren != 0)
			{
				ren->SetOnStateChange( bind(&ModelViewer::OnDrawMode, this) );
				OnDrawMode(ren);
			}

			if (bgd != 0)
			{
				bgd->SetOnStateChange( bind(&ModelViewer::OnBackground, this) );
				OnBackground(bgd);
			}

			if (chk != 0)
			{
				chk->SetOnStateChange( bind(&ModelViewer::OnBloomToggle, this) );
				OnBloomToggle(chk);
			}

			if (sld != 0)
			{
				sld->SetOnValueChange( bind(&ModelViewer::OnBloomChange, this) );
				OnBloomChange(sld);
			}
		}
	}

	// Model options menu
	{
		UIButton* btn = AddMenuButton("Model");
		btn->SetOnFocus( bind(&ModelViewer::OnFillModelMenu, this) );
		btn->SetTooltip("Options affecting the entire model");

		_modelFrame = AddArea("Model", 8);

		AddCaption(_modelFrame, 0, "Triangles:");
		AddCaption(_modelFrame, 1, "Source:");
		AddCaption(_modelFrame, 2, "Position:");
		AddCaption(_modelFrame, 3, "Rotation:");
		AddCaption(_modelFrame, 4, "Scale:");
		AddCaption(_modelFrame, 5, "Spin:");
		AddCaption(_modelFrame, 6, "Show Bounds:");
		AddCaption(_modelFrame, 7, "Transform:");

		_modelTri		= AddLabel (_modelFrame, 0, "Model Triangles");
		_modelSource	= AddLabel (_modelFrame, 1, "Model Source");
		_modelPos		= AddInput (_modelFrame, 2, "Model Position Value", 2);
		_modelRot		= AddInput (_modelFrame, 3, "Model Rotation Value", 2);
		_modelScale		= AddInput (_modelFrame, 4, "Model Scale Value",	2);
		_modelBake		= AddButton(_modelFrame, 7, "Model Bake");

		UILabel* spinLabel	= AddLabel (_modelFrame, 5, "Model Spin Label", 2);
		UILabel* boundsLabel	= AddLabel (_modelFrame, 6, "Model Bounds Label", 2);

		_modelPos->SetTooltip	("Positional offset. Don't forget to check the checkbox to the left.");
		_modelRot->SetTooltip	("You can specify a directional vector if you prefer. [0 0 1] will make the model face up.");
		_modelScale->SetTooltip	("Scaling multiplier. Don't forget to check the checkbox to the left.");

		spinLabel->SetText		(" - for easier visualization");
		boundsLabel->SetText	(" - and bones if present");

		_modelBake->SetText		("Bake All Transforms");
		_modelBake->SetTooltip	("[FF8800]WARNING![FFFFFF] Pressing this button will make all transforms permanent!");

		_modelUsePos	= AddCheckbox(_modelFrame, 2, "Model Position");
		_modelUseRot	= AddCheckbox(_modelFrame, 3, "Model Rotation");
		_modelUseScale	= AddCheckbox(_modelFrame, 4, "Model Scale");
		_modelSpin		= AddCheckbox(_modelFrame, 5, "Model Spin");
		_modelBounds	= AddCheckbox(_modelFrame, 6, "Model Bounds");

		_modelBake->SetOnKey	( bind(&ModelViewer::OnModelBake,	this) );
		_modelPos->SetOnFocus	( bind(&ModelViewer::OnModelChange, this) );
		_modelRot->SetOnFocus	( bind(&ModelViewer::OnModelChange, this) );
		_modelScale->SetOnFocus	( bind(&ModelViewer::OnModelChange, this) );

		_modelUsePos->SetOnStateChange	( bind(&ModelViewer::OnModelToggle, this) );
		_modelUseRot->SetOnStateChange	( bind(&ModelViewer::OnModelToggle, this) );
		_modelUseScale->SetOnStateChange( bind(&ModelViewer::OnModelToggle, this) );
		_modelSpin->SetOnStateChange	( bind(&ModelViewer::OnModelSpin,	this) );
		_modelBounds->SetOnStateChange	( bind(&ModelViewer::OnModelBounds,	this) );
	}

	// Limb menu
	{
		_limbMenu = AddMenuItem		("Limbs");
		_limbMenu->SetTooltip		("Limbs are what models are made out of, associating meshes with materials");
		_limbMenu->SetOnValueChange	( bind(&ModelViewer::OnLimbMenuSelection,	this) );
		_limbMenu->SetOnFocus		( bind(&ModelViewer::OnFillLimbMenu,		this) );

		_limbFrame = AddArea("Limbs", 3);

		AddCaption(_limbFrame, 0, "Limb:");
		AddCaption(_limbFrame, 1, "Mesh:");
		AddCaption(_limbFrame, 2, "Material:");

		_limbName	= AddInput(_limbFrame, 0, "Limb Name Value");
		_limbMesh	= AddList (_limbFrame, 1, "Limb Mesh List");
		_limbMat	= AddList (_limbFrame, 2, "Limb Mat List");

		_limbName->SetOnFocus		( bind(&ModelViewer::OnLimbSelect,		this) );
		_limbMesh->SetOnFocus		( bind(&ModelViewer::OnMeshListFocus,	this) );
		_limbMesh->SetOnValueChange	( bind(&ModelViewer::OnLimbChange,		this) );
		_limbMat->SetOnFocus		( bind(&ModelViewer::OnMatListFocus,	this) );
		_limbMat->SetOnValueChange	( bind(&ModelViewer::OnLimbChange,		this) );
	}

	// Mesh menu
	{
		_meshMenu = AddMenuItem		("Meshes");
		_meshMenu->SetTooltip		("Mesh panel allows you to see the number of triangles in each mesh");
		_meshMenu->SetOnValueChange	( bind(&ModelViewer::OnMeshMenuSelection,	this) );
		_meshMenu->SetOnFocus		( bind(&ModelViewer::OnFillMeshMenu,		this) );

		_meshFrame = AddArea("Meshes", 2);

		AddCaption(_meshFrame, 0, "Mesh:");
		AddCaption(_meshFrame, 1, "Triangles:");

		_meshName	= AddLabel(_meshFrame, 0, "Mesh Name");
		_meshTri	= AddLabel(_meshFrame, 1, "Mesh Triangles");
	}

	// Animation menu
	{
		_animMenu = AddMenuItem		("Animations");
		_animMenu->SetTooltip		("Animations are available for skinned models");
		_animMenu->SetOnValueChange	( bind(&ModelViewer::OnAnimMenuSelection,	this) );
		_animMenu->SetOnFocus		( bind(&ModelViewer::OnFillAnimMenu,		this) );

		_animFrame = AddArea("Animations", 4);

		AddCaption(_animFrame, 0, "Anim. Name:");
		AddCaption(_animFrame, 1, "Frame Range:");
		AddCaption(_animFrame, 2, "Duration:");
		AddCaption(_animFrame, 3, "Playback:");

		_animName		= AddInput		(_animFrame, 0, "Animation Name");
		_animRange		= AddInput		(_animFrame, 1, "Animation Range");
		_animDuration	= AddInput		(_animFrame, 2, "Animation Duration");
		_animLoop		= AddCheckbox	(_animFrame, 3, "Animation Loop");
		_animPlay		= AddButton		(_animFrame, 3, "Animation Play", 2);

		_animName->SetOnFocus			( bind(&ModelViewer::OnAnimNameSelect,	this) );
		_animName->SetOnValueChange		( bind(&ModelViewer::OnAnimNameValue,	this) );
		_animRange->SetOnFocus			( bind(&ModelViewer::OnAnimProperties,	this) );
		_animDuration->SetOnFocus		( bind(&ModelViewer::OnAnimProperties,	this) );
		_animLoop->SetOnStateChange		( bind(&ModelViewer::OnAnimLoop,		this) );
		_animPlay->SetOnKey				( bind(&ModelViewer::OnAnimPlay,		this) );

		_animPlay->SetText				("Play");
		_animName->SetTooltip			("You can change this name to whatever you like");
		_animRange->SetTooltip			("Animation range has 2 values (start and end), such as: [FF8800]4 7");
		_animDuration->SetTooltip		("Fade in, play, and fade out durations, such as: [FF8800]0.5 2.0 0.5");
		_animLoop->SetTooltip			("Checking this box will automatically loop the animation");
		_animPlay->SetTooltip			("Clicking this button with start or stop playing this animation");
	}

	// Material menu
	{
		_matMenu = AddMenuItem		("Materials");
		_matMenu->SetTooltip		("Material properties -- fully modifiable for your pleasure");
		_matMenu->SetOnValueChange	( bind(&ModelViewer::OnMatMenuSelection,	this) );
		_matMenu->SetOnFocus		( bind(&ModelViewer::OnFillMatMenu,			this) );

		_matFrame = AddArea("Materials", 7 + TEXTURES);

		AddCaption(_matFrame, 0, "Material:");
		AddCaption(_matFrame, 1, "Diffuse:");
		AddCaption(_matFrame, 2, "Specular:");
		AddCaption(_matFrame, 3, "Glow:");
		AddCaption(_matFrame, 4, "ADT:");
		AddCaption(_matFrame, 5, "Technique:");
		AddCaption(_matFrame, 6, "Shader:");
		
		_matName		= AddInput		(_matFrame, 0, "Material Name");
		_matDiff		= AddHighlight	(_matFrame, 1, "Material Diffuse");
		_matSpec		= AddHighlight	(_matFrame, 2, "Material Specular");
		_matGlow		= AddSlider		(_matFrame, 3, "Material Glow");
		_matADT			= AddSlider		(_matFrame, 4, "Material ADT");
		_matTech		= AddList		(_matFrame, 5, "Material Technique");
		_matShaderList	= AddList		(_matFrame, 6, "Material Shader");
		_matShaderInput	= AddInput		(_matFrame, 6, "Material Input");

		_matGlow->SetTooltip("Percentage of material affected by light. 0% is normal, 100% makes it immune to light");
		_matADT->SetTooltip("Alpha Discard Threshold: Alpha test will fail below this value");

		_matName->SetTooltip		("You can change this name to whatever you like");
		_matName->SetOnFocus		( bind(&ModelViewer::OnMatNameSelect,	this) );
		_matName->SetOnValueChange	( bind(&ModelViewer::OnMatNameValue,	this) );
		_matGlow->SetOnValueChange  ( bind(&ModelViewer::OnMatProperties,	this) );
		_matADT->SetOnValueChange   ( bind(&ModelViewer::OnMatProperties,	this) );
		_matDiff->SetOnFocus		( bind(&ModelViewer::OnMatColor,		this) );
		_matSpec->SetOnFocus		( bind(&ModelViewer::OnMatColor,		this) );

		_matShaderList->SetTooltip			("Active shader used by this material");
		_matShaderList->SetOnFocus			( bind(&ModelViewer::OnShaderListFocus,	this) );
		_matShaderList->SetOnValueChange	( bind(&ModelViewer::OnMatShaderList,	this) );
		_matShaderInput->SetTooltip			("Hit 'Escape' to cancel or 'Enter' to confirm");
		_matShaderInput->SetOnFocus			( bind(&ModelViewer::OnMatShaderInput,	this) );
		_matShaderInput->SetOnValueChange	( bind(&ModelViewer::OnMatShaderValue,	this) );
		_matShaderInput->SetAlpha(0.0f);

		// Although materials support any number of techniques, we only care about two.
		// They are both deferred in this rendering pipeline. Only difference is the order.
		
		_matTech->AddEntry( _deferred0->GetName() );
		_matTech->AddEntry( _wireframe->GetName() );
		_matTech->SetOnValueChange( bind(&ModelViewer::OnMatTech, this) );
		_matTech->SetTooltip("Material technique used to render this material. Leave at 'Deferred' for most cases.");

		for (uint i = 0; i < TEXTURES; ++i)
		{
			AddCaption(_matFrame, 7+i, String("Texture %u:", i));

			_matTexList [i] = AddList (_matFrame, 7+i, String("MaterialTexList %u",  i));
			_matTexInput[i] = AddInput(_matFrame, 7+i, String("MaterialTexInput %u", i));
			_matTexInput[i]->SetAlpha(0.0f);

			_matTexList	[i]->SetTooltip( String("Texture bound to texture unit %u", i) );
			_matTexInput[i]->SetTooltip("Hit 'Escape' to cancel or 'Enter' to confirm");

			_matTexList	[i]->SetOnFocus			( bind(&ModelViewer::OnTexListFocus,	this) );
			_matTexList [i]->SetOnValueChange	( bind(&ModelViewer::OnMatTexList,		this) );
			_matTexInput[i]->SetOnFocus			( bind(&ModelViewer::OnMatTexInput,		this) );
			_matTexInput[i]->SetOnValueChange	( bind(&ModelViewer::OnMatTexValue,		this) );
		}
	}

	// Texture menu
	{
		_texMenu = AddMenuItem		("Textures");
		_texMenu->SetTooltip		("Texture menu allows you to reload and change the basic properties of textures");
		_texMenu->SetOnValueChange	( bind(&ModelViewer::OnTexMenuSelection,	this) );
		_texMenu->SetOnFocus		( bind(&ModelViewer::OnFillTexMenu,			this) );

		_texFrame = AddArea("Textures", 23);

		AddCaption(_texFrame, 0, "Texture:");
		AddCaption(_texFrame, 1, "Dimensions:");
		AddCaption(_texFrame, 2, "Size in memory:");
		AddCaption(_texFrame, 3, "Wrap mode:");
		AddCaption(_texFrame, 4, "Filtering:");

		UIButton* btn = AddButton(_texFrame, 5, "Texture Reload", 0);
		btn->SetOnFocus( bind(&ModelViewer::OnTexReload, this) );
		btn->SetText("Reload as:");
		btn->SetTooltip("Clicking this button will discard then reload this texture");

		_texName	= AddLabel(_texFrame, 0, "Texture Name");
		_texDims	= AddLabel(_texFrame, 1, "Texture Dims");
		_texSize	= AddLabel(_texFrame, 2, "Texture Size");
		_texWrap	= AddList (_texFrame, 3, "Texture Wrap");
		_texFilter	= AddList (_texFrame, 4, "Texture Filter");
		_texFormat	= AddList (_texFrame, 5, "Texture Format");

		_texSize->SetTooltip("This shows how many bytes of video memory your texture is currently using");

		_texWrap->AddEntry( ITexture::WrapModeToString( ITexture::WrapMode::Repeat ) );
		_texWrap->AddEntry( ITexture::WrapModeToString( ITexture::WrapMode::Mirror ) );
		_texWrap->AddEntry( ITexture::WrapModeToString( ITexture::WrapMode::ClampToEdge ) );
		_texWrap->AddEntry( ITexture::WrapModeToString( ITexture::WrapMode::ClampToZero ) );
		_texWrap->AddEntry( ITexture::WrapModeToString( ITexture::WrapMode::ClampToOne ) );
		_texWrap->SetOnValueChange( bind(&ModelViewer::OnTexChange, this) );
		_texWrap->SetTooltip("Stick with the 'Repeat' wrapping method unless you're certain otherwise.");

		_texFilter->AddEntry( ITexture::FilterToString( ITexture::Filter::Nearest ) );
		_texFilter->AddEntry( ITexture::FilterToString( ITexture::Filter::Linear ) );
		_texFilter->AddEntry( ITexture::FilterToString( ITexture::Filter::Mipmap ) );
		_texFilter->AddEntry( ITexture::FilterToString( ITexture::Filter::Anisotropic ) );
		_texFilter->SetOnValueChange( bind(&ModelViewer::OnTexChange, this) );
		_texFilter->SetTooltip("Anisotropic filtering produces best quality. Linear filtering works best for UI textures.");

		_texFormat->AddEntry( ITexture::FormatToString( ITexture::Format::Alpha ) );
		_texFormat->AddEntry( ITexture::FormatToString( ITexture::Format::RGB ) );
		_texFormat->AddEntry( ITexture::FormatToString( ITexture::Format::RGBA ) );
		_texFormat->AddEntry( ITexture::FormatToString( ITexture::Format::DXT1 ) );
		_texFormat->AddEntry( ITexture::FormatToString( ITexture::Format::DXT3 ) );
		_texFormat->AddEntry( ITexture::FormatToString( ITexture::Format::DXT5 ) );
		_texFormat->SetTooltip("DXT3 is the all-around best choice. RGB is for normal maps, RGBA is for UI textures.");

		_texImage = AddWidget<UIPicture>(_texFrame, "Texture Image");
		_texImage->SetOnKey( &OnIgnoreKey );
		_texImage->SetOnMouseMove( &OnIgnoreMouse );
		UIRegion& tr = _texImage->GetRegion();
		tr.SetLeft		(0.0f,  5.0f);
		tr.SetRight		(1.0f, -5.0f);
		tr.SetTop		(0.0f, 127.0f);
		tr.SetBottom	(1.0f, -5.0f);
	}

	// Color menu
	{
		_colorFrame = AddArea("Color", 4);
		_colorFrame->SetOnFocus( bind(&ModelViewer::OnColorSelect, this) );

		AddCaption(_colorFrame, 0, "Red:");
		AddCaption(_colorFrame, 1, "Green:");
		AddCaption(_colorFrame, 2, "Blue:");

		_colorLabel = AddCaption(_colorFrame, 3, "Alpha:");
		_colorRed	= AddSlider (_colorFrame, 0, "Color Red");
		_colorGreen = AddSlider (_colorFrame, 1, "Color Green");
		_colorBlue	= AddSlider (_colorFrame, 2, "Color Blue");
		_colorAlpha	= AddSlider (_colorFrame, 3, "Color Alpha");

		_colorRed->SetColor		( Color3f(1.0f, 0.0f, 0.0f) );
		_colorGreen->SetColor	( Color3f(0.0f, 1.0f, 0.0f) );
		_colorBlue->SetColor	( Color3f(0.0f, 0.0f, 1.0f) );

		_colorRed->SetOnValueChange		( bind(&ModelViewer::OnColor, this) );
		_colorGreen->SetOnValueChange	( bind(&ModelViewer::OnColor, this) );
		_colorBlue->SetOnValueChange	( bind(&ModelViewer::OnColor, this) );
		_colorAlpha->SetOnValueChange	( bind(&ModelViewer::OnColor, this) );

		_colorRed->SetOnFocus	( bind(&ModelViewer::OnColorSelect, this) );
		_colorGreen->SetOnFocus	( bind(&ModelViewer::OnColorSelect, this) );
		_colorBlue->SetOnFocus	( bind(&ModelViewer::OnColorSelect, this) );
		_colorAlpha->SetOnFocus	( bind(&ModelViewer::OnColorSelect, this) );
	}

	// File dialog
	{
		// Dialog window
		{
			_fileDialog = AddWidget<UIWindow>(mUI, "File Dialog");
			_fileDialog->SetSerializable(false);
			_fileDialog->SetResizable(false);
			_fileDialog->SetAlpha(0.0f);
			_fileDialog->SetSkin(_skin);
			_fileDialog->SetFont(_font);
			_fileDialog->SetTitlebarHeight(20);
			_fileDialog->SetOnKey( &OnIgnoreKey );
			_fileDialog->SetOnMouseMove( &OnIgnoreMouse );
		}

		// Caption label
		{
			_fileLabel = AddWidget<UILabel>(_fileDialog, "File Dialog Label");
			_fileLabel->SetLayer(1, false);
			_fileLabel->SetAlignment( UILabel::Alignment::Center );
			_fileLabel->SetFont(_font);
			_fileLabel->SetReceivesEvents(false);

			UIRegion& rgn = _fileLabel->GetRegion();
			rgn.SetLeft		(0.0f,   5.0f);
			rgn.SetRight	(1.0f,  -5.0f);
			rgn.SetTop		(0.0f,   5.0f);
			rgn.SetBottom	(1.0f, -50.0f);
		}

		// Input field
		{
			_fileInput = AddWidget<UIInput>(_fileDialog, "File Dialog Input");
			_fileInput->SetSkin(_skin);
			_fileInput->SetFont(_font);
			_fileInput->SetFace("Dark Area");
			_fileInput->SetOnValueChange( bind(&ModelViewer::OnFileInputValue, this) );
			_fileInput->SetMaxHistorySize(10);

			// Copy over the history and release the cached list as it's no longer needed
			if (mSavedHistory.IsValid())
			{
				mSavedHistory.Lock();
				{
					for (uint i = 0; i < mSavedHistory.GetSize(); ++i)
					{
						_fileInput->AddToHistory( mSavedHistory[i] );
					}
					mSavedHistory.Release();
				}
				mSavedHistory.Unlock();
			}

			UIRegion& rgn = _fileInput->GetRegion();
			rgn.SetLeft		(0.0f,   5.0f);
			rgn.SetRight	(1.0f,  -5.0f);
			rgn.SetTop		(1.0f, -50.0f);
			rgn.SetBottom	(1.0f, -29.0f);
		}

		// Cancel button
		{
			UIButton* cancel = AddWidget<UIButton>(_fileDialog, "File Dialog Cancel");
			cancel->SetOnFocus( &HideParent );
			cancel->SetSkin(_skin);
			cancel->SetFont(_font);
			cancel->SetText("Cancel");

			UIRegion& rgn = cancel->GetRegion();
			rgn.SetLeft		(0.5f,  10.0f);
			rgn.SetRight	(0.5f, 150.0f);
			rgn.SetTop		(1.0f, -25.0f);
			rgn.SetBottom	(1.0f,  -5.0f);
		}

		// OK button
		{
			_fileOK = AddWidget<UIButton>(_fileDialog, "File Dialog OK");
			_fileOK->SetOnKey( bind(&ModelViewer::OnFileDialogOK, this) );
			_fileOK->SetSkin(_skin);
			_fileOK->SetFont(_font);

			UIRegion& rgn = _fileOK->GetRegion();
			rgn.SetLeft		(0.5f, -150.0f);
			rgn.SetRight	(0.5f,  -10.0f);
			rgn.SetTop		(1.0f,  -25.0f);
			rgn.SetBottom	(1.0f,   -5.0f);
		}
	}

	// Confirm dialog
	{
		// Dialog window
		{
			_confirmDialog = AddWidget<UIWindow>(mUI, "Confirm Dialog");
			_confirmDialog->SetSerializable(false);
			_confirmDialog->SetAlpha(0.0f);
			_confirmDialog->SetSkin(_skin);
			_confirmDialog->SetFont(_font);
			_confirmDialog->SetTitlebarHeight(20);
			_confirmDialog->SetOnKey( &OnIgnoreKey );
			_confirmDialog->SetOnMouseMove( &OnIgnoreMouse );
		}

		// Caption label
		{
			_confirmLabel = AddWidget<UILabel>(_confirmDialog, "Confirm Dialog Label");
			_confirmLabel->SetLayer(1, false);
			_confirmLabel->SetAlignment( UILabel::Alignment::Center );
			_confirmLabel->SetFont(_font);
			_confirmLabel->SetReceivesEvents(false);

			UIRegion& rgn = _confirmLabel->GetRegion();
			rgn.SetLeft		(0.0f,   5.0f);
			rgn.SetRight	(1.0f,  -5.0f);
			rgn.SetTop		(0.0f,   5.0f);
			rgn.SetBottom	(1.0f, -50.0f);
		}

		// Cancel button
		{
			UIButton* cancel = AddWidget<UIButton>(_confirmDialog, "Confirm Dialog Cancel");
			cancel->SetOnFocus( &HideParent );
			cancel->SetText("Cancel");
			cancel->SetSkin(_skin);
			cancel->SetFont(_font);

			UIRegion& rgn = cancel->GetRegion();
			rgn.SetLeft		(0.5f,  10.0f);
			rgn.SetRight	(0.5f, 150.0f);
			rgn.SetTop		(1.0f, -25.0f);
			rgn.SetBottom	(1.0f,  -5.0f);
		}

		// OK button
		{
			_confirmOK = AddWidget<UIButton>(_confirmDialog, "Confirm Dialog OK");
			_confirmOK->SetOnKey( bind(&ModelViewer::OnConfirmDialogOK, this) );
			_confirmOK->SetSkin(_skin);
			_confirmOK->SetFont(_font);

			UIRegion& rgn = _confirmOK->GetRegion();
			rgn.SetLeft		(0.5f, -150.0f);
			rgn.SetRight	(0.5f,  -10.0f);
			rgn.SetTop		(1.0f,  -25.0f);
			rgn.SetBottom	(1.0f,   -5.0f);
		}
	}

	// Loading frame
	{
		_loadFrame = AddWidget<UIFrame>(mUI, "Loading Frame");
		_loadFrame->SetSerializable(false);
		_loadFrame->SetOnKey( &OnIgnoreKey );
		_loadFrame->SetOnMouseMove( &OnIgnoreMouse );
		_loadFrame->SetAlpha(0.0f);

		UIHighlight* hlt = AddWidget<UIHighlight>(_loadFrame, "Loading Frame Background");
		hlt->SetColor( Color4f(0.0f, 0.0f, 0.0f, 0.0f) );
		hlt->SetAlpha(0.85f);
		hlt->SetReceivesEvents(false);

		UIRegion& rgn = hlt->GetRegion();
		rgn.SetBottom(1.0f, -20.0f);
	}
	return true;
}

//============================================================================================================
// Trigger a save
//============================================================================================================

void ModelViewer::OnFileSave()
{
	if ( mModel != 0 && mModel->GetNumberOfTriangles() > 0 )
	{
		const String& source (mModel->GetFilename());

		if ( mModel->Save( source ) )
		{
			SetStatusText( String("Saved as '%s'", source.GetBuffer()) );
		}
		else
		{
			ShowSaveAsDialog();
		}
	}
	else
	{
		SetStatusText( "Only valid, visible geometry can be saved", Color3f(1.0f, 0.0f, 0.0f) );
	}
}

//============================================================================================================
// Shows the "Open file" dialog
//============================================================================================================

void ModelViewer::ShowOpenDialog()
{
	if (_fileDialog != 0)
	{
		UIRegion& rgn = _fileDialog->GetRegion();
		rgn.SetLeft		(0.5f, -200.0f);
		rgn.SetRight	(0.5f,  200.0f);
		rgn.SetTop		(0.5f, -50.0f);
		rgn.SetBottom	(0.5f,  50.0f);

		_fileDialog->SetText("Open File");
		_fileLabel->SetText("What file do you want to load?");
		_fileInput->SetText( mModel->GetFilename() );
		_fileOK->SetText("Load");
		_fileDialog->Show();
	}
}

//============================================================================================================
// Shows the "Save as" dialog
//============================================================================================================

void ModelViewer::ShowSaveAsDialog()
{
	if (_fileDialog != 0)
	{
		UIRegion& rgn = _fileDialog->GetRegion();
		rgn.SetLeft		(0.5f, -200.0f);
		rgn.SetRight	(0.5f,  200.0f);
		rgn.SetTop		(0.5f, -50.0f);
		rgn.SetBottom	(0.5f,  50.0f);

		_fileDialog->SetText("Save As");
		_fileLabel->SetText("Save as what file?");
		_fileInput->SetText( mModel->GetFilename() );
		_fileOK->SetText("Save");
		_fileDialog->Show();
	}
}

//============================================================================================================
// Shows the "Confirm clear materials" dialog
//============================================================================================================

void ModelViewer::ShowConfirmClearMatsDialog()
{
	if (_confirmDialog != 0)
	{
		UIRegion& rgn = _confirmDialog->GetRegion();
		rgn.SetLeft		(0.5f, -200.0f);
		rgn.SetRight	(0.5f,  200.0f);
		rgn.SetTop		(0.5f, -50.0f);
		rgn.SetBottom	(0.5f,  50.0f);

		_confirmDialog->SetText("Confirm Clear Request");
		_confirmLabel->SetText("Are you sure you want to clear all unused materials?");
		_confirmOK->SetText("Clear Materials");
		_confirmDialog->Show();
	}
}

//============================================================================================================
// Shows the "Confirm clear textures" dialog
//============================================================================================================

void ModelViewer::ShowConfirmClearTexDialog()
{
	if (_confirmDialog != 0)
	{
		UIRegion& rgn = _confirmDialog->GetRegion();
		rgn.SetLeft		(0.5f, -200.0f);
		rgn.SetRight	(0.5f,  200.0f);
		rgn.SetTop		(0.5f, -50.0f);
		rgn.SetBottom	(0.5f,  50.0f);

		_confirmDialog->SetText("Confirm Clear Request");
		_confirmLabel->SetText("Are you sure you want to clear all unused textures?");
		_confirmOK->SetText("Clear Textures");
		_confirmDialog->Show();
	}
}

//============================================================================================================
// Shows the "Confirm clear animations" dialog
//============================================================================================================

void ModelViewer::ShowConfirmClearAnimDialog()
{
	if (_confirmDialog != 0)
	{
		UIRegion& rgn = _confirmDialog->GetRegion();
		rgn.SetLeft		(0.5f, -200.0f);
		rgn.SetRight	(0.5f,  200.0f);
		rgn.SetTop		(0.5f, -50.0f);
		rgn.SetBottom	(0.5f,  50.0f);

		_confirmDialog->SetText("Confirm Clear Request");
		_confirmLabel->SetText("Are you sure you want to clear invalid animations?");
		_confirmOK->SetText("Clear Animations");
		_confirmDialog->Show();
	}
}

//============================================================================================================
// Updates the current material information panel based on the selected material
//============================================================================================================

void ModelViewer::UpdateMatPanel (const IMaterial* mat)
{
	if (mat != 0)
	{
		_matName->SetText ( mat->GetName() );
		_matDiff->SetColor( mat->GetDiffuse() );
		_matSpec->SetColor( mat->GetSpecular() );
		_matGlow->SetValue( mat->GetGlow() );
		_matADT->SetValue ( mat->GetADT() );
	}
}

//============================================================================================================
// Updates the current texture information panel based on the selected texture
//============================================================================================================

void ModelViewer::UpdateTexPanel (const ITexture* tex)
{
	if (tex != 0)
	{
		tex->GetTextureID();

		const Vector2i& s = tex->GetSize();

		_texName->SetText  ( tex->GetName() );
		_texDims->SetText  ( String("%ux%u", s.x, s.y) );
		_texSize->SetText  ( String::GetFormattedSize(tex->GetSizeInMemory()) );
		_texWrap->SetText  ( ITexture::WrapModeToString(tex->GetWrapMode()) );
		_texFilter->SetText( ITexture::FilterToString(tex->GetFiltering()) );
		_texFormat->SetText( ITexture::FormatToString(tex->GetFormat()) );
		_texImage->SetTexture( tex );
	}
}

//============================================================================================================
// Updates the current animation information panel based on the selected animation
//============================================================================================================

void ModelViewer::UpdateAnimPanel (const Animation* anim)
{
	if (anim != 0)
	{
		const Vector2i& frames = anim->GetFrames();
		const Vector3f& duration = anim->GetDuration();

		_animName->SetText		( anim->GetName() );
		_animRange->SetText		( String("%u %u", frames.x, frames.y) );
		_animDuration->SetText	( String("%.2f %.2f %.2f", duration.x, duration.y, duration.z) );
		_animLoop->SetState		( UICheckbox::State::Checked, anim->IsLooping() );

		float end = mModel->GetTimeToAnimationEnd(anim);

		if (end == 0.0f)
		{
			_animPlay->SetText("Play");
			_animPlay->SetState( UIButton::State::Enabled, true );
		}
		else
		{
			_animPlay->SetText("Stop");
			_animPlay->SetState( UIButton::State::Enabled, true );
		}
	}
}

//============================================================================================================
// Updates the technique section of the material panel
//============================================================================================================

void ModelViewer::UpdateTechPanel (const DrawMethod* method)
{
	if (method != 0)
	{
		const IShader* shader = method->GetShader();
		_matShaderList->SetText( (shader != 0 ? shader->GetName() : "") );

		// Clear the current texture lists and set the visible text values to current textures
		for (uint b = 0; b < TEXTURES; ++b)
		{
			const ITexture* myTex = method->GetTexture(b);
			_matTexList[b]->SetText( (myTex != 0 ? myTex->GetName() : "") );
		}
	}
}

//============================================================================================================
// Adds a new menu item to the top of the screen
//============================================================================================================

UIMenu* ModelViewer::AddMenuItem (const String& name)
{
	UIMenu* menu = AddWidget<UIMenu>(_menuFrame, name);

	if (menu != 0)
	{
		float width = 12.0f + (float)_font->GetLength(name);
		float left  = _menuOffset;
		float right = left + width;

		_menuOffset = right;

		UIRegion& par = _menuFrame->GetRegion();
		par.SetRight(0.0f, right);

		UIRegion& rgn = menu->GetRegion();
		rgn.SetLeft(0.0f, left);
		rgn.SetRight(0.0f, right);

		menu->SetSerializable(false);
		menu->SetSticky(true);
		menu->SetSkin(_skin);
		menu->SetText(name);
		menu->SetAlignment( UILabel::Alignment::Center );
		menu->SetFont(_font);
		menu->SetOnStateChange( bind(&ModelViewer::ToggleOff, this) );
	}
	return menu;
}

//============================================================================================================
// Adds a new menu button to the top of the screen
//============================================================================================================

UIButton* ModelViewer::AddMenuButton (const String& name)
{
	UIButton* btn = AddWidget<UIButton>(_menuFrame, name);

	if (btn != 0)
	{
		float width = 12.0f + (float)_font->GetLength(name);
		float left  = _menuOffset;
		float right = left + width;

		_menuOffset = right;

		UIRegion& par = _menuFrame->GetRegion();
		par.SetRight(0.0f, right);

		UIRegion& rgn = btn->GetRegion();
		rgn.SetLeft(0.0f, left);
		rgn.SetRight(0.0f, right);

		btn->SetSerializable(false);
		btn->SetSkin(_skin);
		btn->SetSticky(true);
		btn->SetText(name);
		btn->SetAlignment( UILabel::Alignment::Center );
		btn->SetFont(_font);
		btn->SetOnStateChange( bind(&ModelViewer::ToggleBoth, this) );
	}
	return btn;
}

//============================================================================================================
// Adds a new frame (with a background)
//============================================================================================================

UIFrame* ModelViewer::AddArea (const String& name, uint lines)
{
	UIWindow* frame = AddWidget<UIWindow>(mUI, name + " Frame");

	//frame->SetSkin(_skin);

	frame->SetSerializable(false);
	frame->SetAlpha(0.0f);
	frame->SetOnKey( &OnIgnoreKey );
	frame->SetOnMouseMove( &OnIgnoreMouse );

	UIRegion& rgn = frame->GetRegion();
	rgn.SetRight (0.0f, WIDTH);
	rgn.SetTop	 (0.0f, OFFSET);
	rgn.SetBottom(0.0f, OFFSET + (PADDING + LINE) * lines + PADDING * 4.0f);

	UISubPicture* pic = AddWidget<UISubPicture>(frame, name + " Background");
	pic->SetSkin(_skin, false);
	pic->SetFace("Grey Area");
	pic->SetReceivesEvents(false);
	pic->SetSerializable(false);

	return frame;
}

//============================================================================================================
// Adds a new caption to the specified area
//============================================================================================================

UILabel* ModelViewer::AddCaption (UIArea* parent, uint line, const String& text)
{
	UILabel* lbl = AddLabel(parent, line, String("Caption %u", line), 0);
	lbl->SetReceivesEvents(false);
	lbl->SetSerializable(false);
	lbl->SetText(text);
	lbl->SetColor( 0x6ECEFF );
	return lbl;
}

//============================================================================================================
// Adds a new text label to the specified area
//============================================================================================================

UILabel* ModelViewer::AddLabel (UIArea* parent, uint line, const String& name, int offset)
{
	UILabel* lbl = AddWidget<UILabel>(parent, name);

	SetRegion(lbl->GetRegion(), line, offset, 0, 0);

	lbl->SetSerializable(false);
	lbl->SetReceivesEvents(false);
	lbl->SetLayer(1, false);
	lbl->SetAlignment( (offset == 0 ? UILabel::Alignment::Right : UILabel::Alignment::Left) );
	lbl->SetFont(_font);

	return lbl;
}

//============================================================================================================
// Adds a new button tot he specified frame
//============================================================================================================

UIButton*	ModelViewer::AddButton (UIArea* parent, uint line, const String& name, int offset)
{
	UIButton* btn = AddWidget<UIButton>(parent, name);

	SetRegion(btn->GetRegion(), line, offset, 0, -PADDING * 0.6f);

	btn->SetSerializable(false);
	btn->SetAlignment( UILabel::Alignment::Center );
	btn->SetSkin(_skin);
	btn->SetFont(_font);

	return btn;
}

//============================================================================================================
// Adds a new checkbox to the specified area
//============================================================================================================

UICheckbox* ModelViewer::AddCheckbox (UIArea* parent, uint line, const String& name)
{
	UICheckbox* chk = AddWidget<UICheckbox>(parent, name);

	float top = PADDING * 2 + (PADDING + LINE) * line;

	UIRegion& rgn = chk->GetRegion();
	rgn.SetLeft	  (SEPARATOR, PADDING * 2.0f);
	rgn.SetRight  (SEPARATOR, PADDING + LINE);
	rgn.SetTop	  (0.0f, top);
	rgn.SetBottom (0.0f, top + LINE - PADDING);

	chk->SetSerializable(false);
	chk->SetSkin(_skin);

	return chk;
}

//============================================================================================================
// Adds a new input field to the specified area
//============================================================================================================

UIInput* ModelViewer::AddInput (UIArea* parent, uint line, const String& name, int offset)
{
	UIInput* inp = AddWidget<UIInput>(parent, name);

	SetRegion(inp->GetRegion(), line, offset, -PADDING * 0.3f, -PADDING * 0.6f);

	inp->SetSerializable(false);
	inp->SetFont(_font);
	inp->SetSkin(_skin);
	inp->SetFace("Dark Area");

	return inp;
}

//============================================================================================================
// Adds a new drop-down list to the specified area
//============================================================================================================

UIList* ModelViewer::AddList (UIArea* parent, uint line, const String& name, int offset)
{
	UIList* list = AddWidget<UIList>(parent, name);

	SetRegion(list->GetRegion(), line, offset, 0, -PADDING * 0.6f);

	list->SetSerializable(false);
	list->SetFont(_font);
	list->SetSkin(_skin);
	list->SetSymbol("Down Arrow");
	list->SetAlignment( (offset == 0 ? UILabel::Alignment::Right : UILabel::Alignment::Left) );

	return list;
}

//============================================================================================================
// Adds a new slider to the specified area
//============================================================================================================

UISlider* ModelViewer::AddSlider (UIArea* parent, uint line, const String& name, int offset)
{
	UISlider* slider = AddWidget<UISlider>(parent, name);
	SetRegion(slider->GetRegion(), line, offset, 1, 0);
	slider->SetSerializable(false);
	slider->SetSkin(_skin);
	return slider;
}

//============================================================================================================
// Adds a new highlight to the specified area
//============================================================================================================

UIHighlight* ModelViewer::AddHighlight (UIArea* parent, uint line, const String& name, int offset)
{
	UIHighlight* hlt = AddWidget<UIHighlight>(parent, name);
	SetRegion(hlt->GetRegion(), line, offset, 1, 0);
	hlt->SetSerializable(false);
	hlt->SetLayer(1, false);
	return hlt;
}

//============================================================================================================
// Delegate function triggered when an "OK" button is clicked on the file dialog menu
//============================================================================================================

bool ModelViewer::OnFileDialogOK (UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	// Ensure that the click was on the button itself
	if ( key == Key::MouseLeft && !isDown && area->GetRegion().Contains(pos) )
	{
		_ConfirmFileDialog();
	}
	return true;
}

//============================================================================================================
// Delegate triggered when the "OK" button is pressed in a confirmation dialog window
//============================================================================================================

bool ModelViewer::OnConfirmDialogOK	(UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	// Ensure that the click was on the button itself
	if ( key == Key::MouseLeft && !isDown && area->GetRegion().Contains(pos) )
	{
		UIButton* btn		= R5_CAST(UIButton, area);
		UIFrame*	parent	= R5_CAST(UIFrame, area->GetParent());

		const String& text = btn->GetText();

		if ( text == "Clear Materials" )
		{
			const UIMenu::Entries& entries = _matMenu->GetAllEntries();

			entries.Lock();
			{
				for (uint i = 0; i < entries.GetSize(); ++i)
				{
					const String& entry = entries[i];

					if ( entry.IsValid() && entry != CLEAR && entry != NEW )
					{
						IMaterial* mat = mGraphics->GetMaterial(entry, false);

						// Only release this material if it's not in use
						if (mat != 0 && !mModel->IsUsingMaterial(mat))
						{
							mat->Release();
						}
					}
				}
			}
			entries.Unlock();

			SetStatusText("All unused materials were cleared away");
		}
		else if ( text == "Clear Textures" )
		{
			const UIMenu::Entries& entries = _texMenu->GetAllEntries();

			entries.Lock();
			{
				for (uint i = 0; i < entries.GetSize(); ++i)
				{
					const String& entry = entries[i];

					if ( entry.IsValid() && entry != CLEAR && entry != NEW )
					{
						ITexture* tex = mGraphics->GetTexture(entry, false);

						// Only release this texture if it's not currently used
						if ( tex != 0 && tex != _skin->GetTexture() && !mModel->IsUsingTexture(tex) )
						{
							tex->Release();
						}
					}
				}
			}
			entries.Unlock();

			SetStatusText("All unused textures were cleared away");
		}
		else if ( text == "Clear Animations" )
		{
			Skeleton* skel = mModel->GetSkeleton();

			if (skel != 0)
			{
				mModel->StopAllAnimations(0.0f);

				Skeleton::Animations& anims = skel->GetAllAnimations();

				anims.Lock();
				{
					for (uint i = anims.GetSize(); i > 0; )
					{
						Animation* anim = anims[--i];

						if ( anim == 0 )
						{
							anims.RemoveAt(i);
						}
						else if (anim->GetDuration().IsZero())
						{
							anims.RemoveAt(i);
						}
					}
				}
				anims.Unlock();
			}

			SetStatusText("All animations with duration of 0 were cleared away");
		}

		// Hide the request frame
		if (parent != 0) parent->Hide();
	}
	return true;
}

//============================================================================================================
// Helper callback functions that toggle the visibility of the options frames
//============================================================================================================

bool ModelViewer::ToggleBoth (UIArea* area)
{
	UIButton* btn = (UIButton*)area;
	bool pressed = ((btn->GetState() & UIButton::State::Pressed) != 0);

	UIFrame* frame = FindWidget<UIFrame>(mUI, btn->GetName() + " Frame", false);

	if (frame != 0)
	{
		if (pressed)
		{
			frame->Show();
			frame->BringToFront();
		}
		else frame->Hide();
	}
	return true;
}

//============================================================================================================

bool ModelViewer::ToggleOff (UIArea* area)
{
	UIButton* btn = (UIButton*)area;
	bool pressed = ((btn->GetState() & UIButton::State::Pressed) != 0);

	if (!pressed)
	{
		UIFrame* frame = FindWidget<UIFrame>(mUI, btn->GetName() + " Frame", false);

		if ( frame != 0 )
		{
			frame->Hide();
		}
	}
	return true;
}

//============================================================================================================
// Updates the limb menu items
//============================================================================================================

bool ModelViewer::OnFillLimbMenu (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		_limbMenu->ClearAllEntries();

		Model::Limbs& limbs = mModel->GetAllLimbs();

		limbs.Lock();
		{
			for (uint i = 0; i < limbs.GetSize(); ++i)
			{
				_limbMenu->AddEntry( limbs[i]->GetName() );
			}
		}
		limbs.Unlock();
	}
	return true;
}

//============================================================================================================
// Updates the mesh menu items
//============================================================================================================

bool ModelViewer::OnFillMeshMenu (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		_meshMenu->ClearAllEntries();

		Core::Meshes& meshes = mCore->GetAllMeshes();

		meshes.Lock();
		{
			for (uint i = 0; i < meshes.GetSize(); ++i)
			{
				Mesh* mesh = meshes[i];

				if (mesh != 0 && mesh->GetNumberOfTriangles() > 0)
				{
					_meshMenu->AddEntry( mesh->GetName() );
				}
			}
		}
		meshes.Unlock();
	}
	return true;
}

//============================================================================================================
// Updates the material menu items
//============================================================================================================

bool ModelViewer::OnFillMatMenu (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		_matMenu->ClearAllEntries();

		const IGraphics::Materials& mats = mGraphics->GetAllMaterials();

		mats.Lock();
		{
			for (uint i = 0; i < mats.GetSize(); ++i)
			{
				const IMaterial* mat = mats[i];

				if (mat != 0 && mat->GetDiffuse().GetColor4ub().a > 0)
				{
					_matMenu->AddEntry( mat->GetName() );
				}
			}
		}
		mats.Unlock();

		_matMenu->AddEntry(NEW);
		_matMenu->AddEntry(CLEAR);
	}
	return true;
}

//============================================================================================================
// Updates the texture menu items
//============================================================================================================

bool ModelViewer::OnFillTexMenu (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		_texMenu->ClearAllEntries();

		const IGraphics::Textures& texs = mGraphics->GetAllTextures();

		texs.Lock();
		{
			for (uint i = 0; i < texs.GetSize(); ++i)
			{
				const ITexture* tex = texs[i];

				if (tex != 0 &&
					tex->GetType() == ITexture::Type::TwoDimensional &&
					tex->GetSource(0).IsValid())
				{
					_texMenu->AddEntry( tex->GetName() );
				}
			}
		}
		texs.Unlock();

		_texMenu->AddEntry(CLEAR);
	}
	return true;
}

//============================================================================================================
// Updates the animation menu items
//============================================================================================================

bool ModelViewer::OnFillAnimMenu (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		_animMenu->ClearAllEntries();

		Skeleton* skel = mModel->GetSkeleton();

		if (skel == 0)
		{
			SetStatusText("This model has no associated skeleton and thus cannot have any animations");
			return true;
		}

		const Skeleton::Animations& anims = skel->GetAllAnimations();

		anims.Lock();
		{
			for (uint i = 0; i < anims.GetSize(); ++i)
			{
				const Animation* anim = anims[i];

				if ( anim != 0 && anim->IsValid() )
				{
					_animMenu->AddEntry( anim->GetName() );
				}
			}
		}
		anims.Unlock();

		_animMenu->AddEntry(NEW);
		_animMenu->AddEntry(CLEAR);
	}
	return true;
}

//============================================================================================================
// Updates the model's information panel
//============================================================================================================

bool ModelViewer::OnFillModelMenu (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		String temp;

		_modelSource->SetText(mModel->GetFilename());

		if (mModel->GetNumberOfTriangles()	>> temp)	_modelTri->SetText(temp);
		if (mInst->GetRelativePosition()	>> temp)	_modelPos->SetText(temp);
		if (mInst->GetRelativeRotation()	>> temp)	_modelRot->SetText(temp);
		if (mInst->GetRelativeScale()		>> temp)	_modelScale->SetText(temp);

		bool usePos		= !(mInst->GetRelativePosition().IsZero());
		bool useRot		= !(mInst->GetRelativeRotation().IsIdentity());
		bool useScale	=  (mInst->GetRelativeScale() != 1.0f);

		_modelUsePos->SetState	( UICheckbox::State::Checked, usePos );
		_modelUseRot->SetState	( UICheckbox::State::Checked, useRot );
		_modelUseScale->SetState( UICheckbox::State::Checked, useScale );
	}
	return true;
}

//============================================================================================================
// Menu callback functions
//============================================================================================================

bool ModelViewer::OnFileMenuSelection (UIArea* area)
{
	const String& item = _fileMenu->GetText();

	if (item == SAVE)
	{
		OnFileSave();
	}
	else if (item == SAVEAS)
	{
		ShowSaveAsDialog();
	}
	else if (item == OPEN)
	{
		ShowOpenDialog();
	}
	if (item == EXIT)
	{
		mCore->Shutdown();
	}
	return false;
}

//============================================================================================================

bool ModelViewer::OnViewMenuSelection (UIArea* area)
{
	UIMenu* menu = R5_CAST(UIMenu, area);

	if (menu != 0)
	{
		const String& item = menu->GetText();

		if (item == RESET_CAMERA)
		{
			mResetCamera = true;
		}
		else if (item == RESET_WINDOW)
		{
			_ResetWindow();
		}
		else if (item == TOGGLE_FULLSCREEN)
		{
			_ToggleFullscreen();
		}
		else if (item == ABOUT)
		{
			ShowAboutInfo();
		}
	}
	return false;
}

//============================================================================================================

bool ModelViewer::OnLimbMenuSelection (UIArea* area)
{
	if (mModel != 0)
	{
		const String& name = _limbMenu->GetText();

		Limb* limb = (name.IsValid() ? mModel->GetLimb(name, false) : 0);

		if (limb != 0)
		{
			_currentLimb = name;

			_limbName->SetText( _currentLimb );
			_limbMesh->SetText( (limb->GetMesh()	 == 0 ? "" : limb->GetMesh()->GetName()) );
			_limbMat->SetText ( (limb->GetMaterial() == 0 ? "" : limb->GetMaterial()->GetName()) );

			_limbFrame->Show();
			_limbFrame->BringToFront();
			return false;
		}
	}
	_currentLimb.Clear();
	_limbFrame->Hide();
	return false;
}

//============================================================================================================

bool ModelViewer::OnMeshMenuSelection (UIArea* area)
{
	_currentMesh = _meshMenu->GetText();

	if (_currentMesh.IsValid())
	{
		const Mesh* mesh = mCore->GetMesh(_currentMesh, false);

		if (mesh != 0)
		{
			String tri;
			mesh->GetNumberOfTriangles() >> tri;

			_meshName->SetText( _currentMesh );
			_meshTri->SetText( tri );
			_meshFrame->Show();
			_meshFrame->BringToFront();
			return false;
		}
	}
	_currentMesh.Clear();
	_meshFrame->Hide();
	return false;
}

//============================================================================================================

bool ModelViewer::OnMatMenuSelection (UIArea* area)
{
	_currentMat.Clear();
	_currentTech.Clear();

	if (_matFrame != 0)
	{
		_currentMat = _matMenu->GetText();

		if (_currentMat.IsValid())
		{
			if (_currentMat == CLEAR)
			{
				_currentMat.Clear();
				_matMenu->SetState( UIButton::State::Pressed, false );
				ShowConfirmClearMatsDialog();
			}
			else
			{
				bool isNew = (_currentMat == NEW);
				if (isNew) _currentMat.StripTags();

				IMaterial* mat = mGraphics->GetMaterial(_currentMat, isNew);

				if (mat != 0)
				{
					if ( isNew && mat->GetDiffuse().GetColor4ub().a == 0 )
					{
						// Make the new material visible right away
						mat->SetDiffuse( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
					}

					// Figure out which rendering method this material is currently using
					IMaterial::DrawMethod* method0 = mat->GetDrawMethod(_deferred0, false);
					IMaterial::DrawMethod* method1 = mat->GetDrawMethod(_wireframe, false);

					// Ensure that we have at least one deferred rendering method available with this material
					if (method0 != 0)
					{
						_currentTech = _deferred0->GetName();
						UpdateTechPanel(method0);
					}
					else if (method1 != 0)
					{
						_currentTech = _wireframe->GetName();
						UpdateTechPanel(method1);
					}
					else
					{
						// Looks like no deferred approach is available, so create a new one.
						method0 = mat->GetDrawMethod(_deferred0, true);

						// Basic deferred rendering shader using only material color
						method0->SetShader( mGraphics->GetShader("Deferred/Material") );

						// Set the material's diffuse color if this material is not visible
						if (mat->GetDiffuse().GetColor4ub().a == 0)
							mat->SetDiffuse( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

						// Update the current technique
						_currentTech = _deferred0->GetName();
						UpdateTechPanel(method0);
					}

					// See if the selected material is currently in use and if the changes would be visible
					{
						bool found (false);
						const Model::Limbs& limbs = mModel->GetAllLimbs();

						limbs.Lock();
						{
							for (uint i = 0; i < limbs.GetSize(); ++i)
							{
								const Limb* limb = limbs[i];

								if (limb != 0 && limb->GetMaterial() == mat)
								{
									found = true;
									break;
								}
							}
						}
						limbs.Unlock();

						if (isNew)
						{
							SetStatusText("You can associate this material with a limb, thus making it visible, via the 'Limbs' menu.");
						}
						else if (!found)
						{
							SetStatusText("The material you selected is not currently in use, but you can associate it via the 'Limbs' menu.");
						}
					}

					// Update all the basic material information
					UpdateMatPanel(mat);

					// Reflect the current technique selection
					_matTech->SetText( _currentTech );

					// Show the frame and bring it to the front
					_matFrame->Show();
					_matFrame->BringToFront();
					return false;
				}
			}
		}
	}
	_matFrame->Hide();
	return false;
}

//============================================================================================================

bool ModelViewer::OnTexMenuSelection (UIArea* area)
{
	_currentTex = _texMenu->GetText();

	if (_currentTex == CLEAR)
	{
		_currentTex.Clear();
		_texMenu->SetState( UIButton::State::Pressed, false );
		ShowConfirmClearTexDialog();
	}
	else
	{
		const ITexture* tex = mGraphics->GetTexture(_currentTex, false);

		if (tex != 0)
		{
			UpdateTexPanel(tex);
			_texFrame->Show();
			_texFrame->BringToFront();
			return false;
		}
	}

	_currentTex.Clear();
	_texFrame->Hide();
	return false;
}

//============================================================================================================

bool ModelViewer::OnAnimMenuSelection (UIArea* area)
{
	_currentAnim = _animMenu->GetText();

	if (_currentAnim == CLEAR)
	{
		_currentAnim.Clear();
		_animMenu->SetState( UIButton::State::Pressed, false );
		ShowConfirmClearAnimDialog();
	}
	else
	{
		bool isNew = (_currentAnim == NEW);
		if (isNew) _currentAnim.StripTags();

		const Animation* anim = mModel->GetAnimation(_currentAnim, isNew);

		if (anim != 0)
		{
			UpdateAnimPanel(anim);
			_animFrame->Show();
			_animFrame->BringToFront();
			return false;
		}
	}

	_currentAnim.Clear();
	_animFrame->Hide();
	return false;
}

//============================================================================================================
// Program options panel callbacks
//============================================================================================================

bool ModelViewer::OnDrawMode (UIArea* area)
{
	UIList* list = (UIList*)area;
	const String& value (list->GetText());

	if		(value == "Deferred")			mParams.mSsao = 0;
	else if (value == "Low Quality SSAO")	mParams.mSsao = 1;
	else if (value == "High Quality SSAO")	mParams.mSsao = 2;

	return true;
}

//============================================================================================================

bool ModelViewer::OnBackground (UIArea* area)
{
	UIList* list = (UIList*)area;
	const String& value (list->GetText());

	if (value == "Black Color")
	{
		mGraphics->SetBackgroundColor( Color4f(0.0f, 0.0f, 0.0f, 1.0f) );
		mGraphics->SetActiveSkybox(0);
	}
	else if (value == "White Color")
	{
		mGraphics->SetBackgroundColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
		mGraphics->SetActiveSkybox(0);
	}
	else if (value == "Grey Color")
	{
		mGraphics->SetBackgroundColor( Color4f(0.25f, 0.25f, 0.25f, 1.0f) );
		mGraphics->SetActiveSkybox(0);
	}
	else
	{
		mGraphics->SetBackgroundColor( Color4f(0.0f, 0.0f, 0.0f, 1.0f) );
		mGraphics->SetActiveSkybox( mGraphics->GetTexture(value) );
	}
	return true;
}

//============================================================================================================

bool ModelViewer::OnBloomToggle	(UIArea* area)
{
	UICheckbox* chk = (UICheckbox*)area;
	mParams.mBloom = ((chk->GetState() & UICheckbox::State::Checked) != 0);
	return true;
}

//============================================================================================================

bool ModelViewer::OnBloomChange	(UIArea* area)
{
	UISlider* sld = (UISlider*)area;
	mParams.mThreshold = sld->GetValue();
	return true;
}

//============================================================================================================
// Force-applies all active transformations to all of the model's meshes
//============================================================================================================

bool ModelViewer::OnModelBake (UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	// Ensure that the click was on the button itself
	if ( key == Key::MouseLeft && !isDown && area->GetRegion().Contains(pos) )
	{
		const Vector3f&		pos		= mInst->GetRelativePosition();
		const Quaternion&	rot		= mInst->GetRelativeRotation();
		float				scale	= mInst->GetRelativeScale();

		Skeleton* skel = mModel->GetSkeleton();

		if (skel != 0)
		{
			Skeleton::Bones& bones = skel->GetAllBones();

			bones.Lock();
			{
				for (uint i = 0; i < bones.GetSize(); ++i)
				{
					Skeleton::BonePtr bone = bones[i];

					if (bone != 0)
					{
						Bone::PosKeys& posKeys = bone->GetAllPosKeys();
						Bone::RotKeys& rotKeys = bone->GetAllRotKeys();

						if (bone->GetParent() == -1)
						{
							// If this is a parent bone, all transform components should affect it
							Vector3f bonePos ( bone->GetPosition() );
							bonePos *= scale;
							bonePos *= rot;
							bonePos += pos;

							bone->SetPosition( bonePos );
							bone->SetRotation( rot * bone->GetRotation() );

							// Position keys are scaled, rotated, then translated
							for (uint b = 0; b < posKeys.GetSize(); ++b)
							{
								Bone::PosKey& key = posKeys[b];
								key.mPos *= scale;
								key.mPos *= rot;
								key.mPos += pos;
							}

							// Rotation keys are simply rotated
							for (uint b = 0; b < rotKeys.GetSize(); ++b)
							{
								Bone::RotKey& key = rotKeys[b];
								key.mRot = rot * key.mRot;
							}
						}
						else
						{
							// Not a parent bone -- only the scaling component should affect it
							bone->SetPosition( bone->GetPosition() * scale );

							for (uint b = 0; b < posKeys.GetSize(); ++b)
							{
								Bone::PosKey& key = posKeys[b];
								key.mPos *= scale;
							}
						}
					}
				}
			}
			bones.Unlock();
		}

		// Get a list of meshes used by the model
		Array<Mesh*> meshes;
		{
			Model::Limbs& limbs = mModel->GetAllLimbs();

			limbs.Lock();
			{
				for (uint i = 0; i < limbs.GetSize(); ++i)
				{
					Limb* limb (limbs[i]);

					if (limb != 0)
					{
						Mesh* mesh = limb->GetMesh();

						if (mesh != 0 && mesh->IsValid())
						{
							meshes.AddUnique(limb->GetMesh());
						}
					}
				}
			}
			limbs.Unlock();
		}

		// If there were meshes to work with, transform all of their vertices
		if (meshes.IsValid())
		{
			Matrix43 mat (pos, rot, scale);

			for (uint b = 0; b < meshes.GetSize(); ++b)
			{
				Mesh* mesh = meshes[b];

				mesh->Lock();
				{
					Mesh::Vertices& vertices = mesh->GetVertexArray();
					Mesh::Normals&	normals  = mesh->GetNormalArray();
					Mesh::Tangents& tangents = mesh->GetTangentArray();

					for (uint i = vertices.GetSize(); i > 0; ) vertices[--i] *= mat;
					for (uint i = normals.GetSize();  i > 0; ) normals [--i] *= rot;
					for (uint i = tangents.GetSize(); i > 0; ) tangents[--i] *= rot;

					mesh->Update(true, false, false, false, false, false);
				}
				mesh->Unlock();
			}
		}

		// Reset the current transforms
		mInst->SetRelativeRotation( Quaternion() );
		mInst->SetRelativePosition( Vector3f() );
		mInst->SetRelativeScale( 1.0f );

		// Remove then re-add the skeleton, forcing an update
		mModel->SetSkeleton(0);
		mModel->SetSkeleton(skel);
		mModel->SetDirty();

		// Update the model window
		OnFillModelMenu(0, true);
	}
	return true;
}

//============================================================================================================
// Whether to spin the model around the model (or spin it around)
//============================================================================================================

bool ModelViewer::OnModelSpin (UIArea* area)
{
	UICheckbox* chk = (UICheckbox*)area;
	mAnimate = ((chk->GetState() & UICheckbox::State::Checked) != 0);
	if (!mAnimate) mStage->SetRelativeRotation( Quaternion() );
	return true;
}

//============================================================================================================
// Whether to show the model's bounds
//============================================================================================================

bool ModelViewer::OnModelBounds (UIArea* area)
{
	UICheckbox* chk = (UICheckbox*)area;
	bool bounds = ((chk->GetState() & UICheckbox::State::Checked) != 0);
	if (mInst != 0) mInst->SetShowOutline(bounds);
	return true;
}

//============================================================================================================
// Delegate triggered when the material name input field gains or loses focus
//============================================================================================================

bool ModelViewer::OnMatNameSelect (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		_matName->SetText(_currentMat);
	}
	return true;
}

//============================================================================================================
// Delegate triggered when the material name changes
//============================================================================================================

bool ModelViewer::OnMatNameValue (UIArea* area)
{
	const String& name = _matName->GetText();

	if (name.IsValid() && _currentMat != name)
	{
		IMaterial* mat = mGraphics->GetMaterial(name, false);

		if ( mat == 0 )
		{
			// Get the current material
			mat = mGraphics->GetMaterial(_currentMat, false);

			if (mat != 0)
			{
				// Change its name
				mat->SetName(name);
				_currentMat = name;
			}
		}
		else
		{
			_matName->SetText(_currentMat);
			SetStatusText("A material with the requested name already exists", Color3f(1.0f, 0.0f, 0.0f));
		}
	}
	return true;
}

//============================================================================================================
// Responds to standard material properties changes
//============================================================================================================

bool ModelViewer::OnMatProperties (UIArea* area)
{
	IMaterial* mat = mGraphics->GetMaterial(_currentMat, false);

	if (mat != 0)
	{
		mat->SetGlow ( _matGlow->GetValue() );
		mat->SetADT	 ( _matADT->GetValue()  );
	}
	return true;
}

//============================================================================================================
// Triggered when a new material technique is selected from the drop-down menu
//============================================================================================================

bool ModelViewer::OnMatTech (UIArea* area)
{
	if (_matTech == area)
	{
		_currentTech = _matTech->GetText();

		const ITechnique* tech = mGraphics->GetTechnique(_matTech->GetText(), false);

		if (tech != 0)
		{
			IMaterial* mat = mGraphics->GetMaterial( _currentMat, false );

			if (mat != 0)
			{
				if (tech != _deferred0) mat->DeleteDrawMethod(_deferred0);
				if (tech != _wireframe) mat->DeleteDrawMethod(_wireframe);

				IMaterial::DrawMethod* method = mat->GetDrawMethod(tech, true);

				// Copy over the shader
				method->SetShader( mGraphics->GetShader(_matShaderList->GetText(), false) );

				// Copy over all textures
				for (uint i = 0; i < TEXTURES; ++i)
				{
					method->SetTexture( i, mGraphics->GetTexture(_matTexList[i]->GetText(), false) );
				}

				// Mark the model as dirty so it can rebuild its technique mask
				mModel->SetDirty();
				UpdateTechPanel(method);
				return true;
			}
		}
	}
	UpdateTechPanel(0);
	return true;
}

//============================================================================================================
// Triggered on material shader list selection change
//============================================================================================================

bool ModelViewer::OnMatShaderList (UIArea* area)
{
	if (_matShaderList != 0)
	{
		const String& sel = _matShaderList->GetText();

		if (sel == NEW)
		{
			_matShaderList->SetAlpha(0.0f);
			_matShaderInput->SetAlpha(1.0f);
			_matShaderInput->SetFocus(true);
			_matShaderInput->BringToFront();
		}
		else
		{
			_SetMatShader(sel);
		}
	}
	return true;
}

//============================================================================================================
// Triggered when material shader field gains or loses focus
//============================================================================================================

bool ModelViewer::OnMatShaderInput (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		_matShaderInput->SetAlpha(0.0f);
		_matShaderList->SetAlpha(1.0f);
	}
	return true;
}

//============================================================================================================
// Responds to material shader input field changes
//============================================================================================================

bool ModelViewer::OnMatShaderValue (UIArea* area)
{
	if (_matShaderInput != 0)
	{
		_SetMatShader( _matShaderInput->GetText() );
		area->SetFocus(false);
	}
	return true;
}

//============================================================================================================
// Triggered on material texture list selection change
//============================================================================================================

bool ModelViewer::OnMatTexList (UIArea* area)
{
	UIList* list = R5_CAST(UIList, area);

	if (list != 0)
	{
		const String& name = list->GetName();
		String left, right;

		if (name.String::Split(left, ' ', right))
		{
			uint index = 0;

			if (right >> index && index < TEXTURES)
			{
				const String& sel = list->GetText();

				if (sel == NEW)
				{
					_matTexList[index]->SetAlpha(0.0f);
					_matTexInput[index]->SetAlpha(1.0f);
					_matTexInput[index]->SetFocus(true);
					_matTexInput[index]->BringToFront();
				}
				else
				{
					_SetMatTexture(index, sel);
				}
			}
		}
	}
	return true;
}

//============================================================================================================
// Triggered when material texture field gains or loses focus
//============================================================================================================

bool ModelViewer::OnMatTexInput	(UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		for (uint i = 0; i < TEXTURES; ++i)
		{
			_matTexInput[i]->SetAlpha(0.0f);
			_matTexList[i]->SetAlpha(1.0f);
		}
	}
	return true;
}

//============================================================================================================
// Responds to material texture input field changes
//============================================================================================================

bool ModelViewer::OnMatTexValue (UIArea* area)
{
	UIInput* input = R5_CAST(UIInput, area);

	if (input != 0)
	{
		const String& name = input->GetName();
		String left, right;

		if (name.String::Split(left, ' ', right))
		{
			uint index = 0;

			if (right >> index && index < TEXTURES)
			{
				_SetMatTexture( index, input->GetText() );
			}
		}
	}
	return true;
}

//============================================================================================================
// Responds to color selection in the material pane
//============================================================================================================

bool ModelViewer::OnMatColor (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		const IMaterial* mat = mGraphics->GetMaterial(_currentMat, false);

		if (mat != 0)
		{
			Color4f color (1.0f);

			if (area->GetName() == "Material Diffuse")
			{
				_colorLabel->SetText("Alpha:");
				color = mat->GetDiffuse().GetColor4f();
			}
			else
			{
				_colorLabel->SetText("Gloss:");
				color = mat->GetSpecular().GetColor4f();
			}

			_colorRed->SetValue  (color.r);
			_colorGreen->SetValue(color.g);
			_colorBlue->SetValue (color.b);
			_colorAlpha->SetValue(color.a);

			_colorFrame->Show();
			_colorFrame->SetFocus(true);
		}
	}
	return true;
}

//============================================================================================================
// Delegate triggered when material color sliders are moved
//============================================================================================================

bool ModelViewer::OnColor (UIArea* area)
{
	// Color's alpha should never get down to 0. Diffuse alpha of 0 makes the material invisible,
	// which means it won't show up in the list of materials. We don't want that happening.

	Color4f color ( _colorRed->GetValue(),
					_colorGreen->GetValue(),
					_colorBlue->GetValue(),
					Float::Max(_colorAlpha->GetValue(), 0.003921568627451f) );

	if (_colorLabel->GetText() == "Alpha:")
	{
		_matDiff->SetColor(color);

		IMaterial* mat = mGraphics->GetMaterial(_currentMat, false);
		if (mat != 0) mat->SetDiffuse(color);
	}
	else
	{
		_matSpec->SetColor(color);

		IMaterial* mat = mGraphics->GetMaterial(_currentMat, false);
		if (mat != 0) mat->SetSpecular(color);
	}
	return true;
}

//============================================================================================================
// Delegate triggered on color panel's selections
//============================================================================================================

bool ModelViewer::OnColorSelect (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		const UIArea* input = mUI->GetFocusArea();
		if ( input == 0 || !input->IsChildOf(_colorFrame) ) _colorFrame->Hide();
	}
	return true;
}

//============================================================================================================
// Texture options panel callbacks
//============================================================================================================

bool ModelViewer::OnTexReload (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		ITexture* tex = mGraphics->GetTexture(_currentTex, false);

		if (tex != 0)
		{
			uint format = ITexture::StringToFormat( _texFormat->GetText() );
			uint filter = tex->GetFiltering();
			uint wrap	= tex->GetWrapMode();

			tex->Load(_currentTex, format);
			tex->SetFiltering(filter);
			tex->SetWrapMode(wrap);

			UpdateTexPanel(tex);
		}
	}
	return true;
}

//============================================================================================================

bool ModelViewer::OnTexChange (UIArea* area)
{
	uint wrap = ITexture::StringToWrapMode( _texWrap->GetText() );
	uint filt = ITexture::StringToFilter( _texFilter->GetText() );

	ITexture* tex = mGraphics->GetTexture(_currentTex, false);

	if (tex != 0)
	{
		tex->SetWrapMode(wrap);
		tex->SetFiltering(filt);
	}
	return true;
}

//============================================================================================================

bool ModelViewer::OnAnimNameSelect (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		_animName->SetText(_currentAnim);
	}
	return false;
}

//============================================================================================================

bool ModelViewer::OnAnimNameValue (UIArea* area)
{
	const String& name = _animName->GetText();

	if (name.IsValid() && _currentAnim != name)
	{
		Animation* anim = mModel->GetAnimation(name, false);

		if ( anim == 0 )
		{
			// Get the current material
			anim = mModel->GetAnimation(_currentAnim, false);

			if (anim != 0)
			{
				// Change its name
				anim->SetName(name);
				_currentAnim = name;
			}
		}
		else
		{
			_animName->SetText(_currentAnim);
			SetStatusText("An animation with the requested name already exists", Color3f(1.0f, 0.0f, 0.0f));
		}
	}
	return true;
}

//============================================================================================================

bool ModelViewer::OnAnimLoop (UIArea* area)
{
	Animation* anim = mModel->GetAnimation(_currentAnim, false);

	if (anim != 0)
	{
		anim->SetLooping( (_animLoop->GetState() & UICheckbox::State::Checked) != 0 );
	}
	return true;
}

//============================================================================================================

bool ModelViewer::OnAnimProperties (UIArea* area, bool hasFocus)
{
	if (!hasFocus)
	{
		Animation* anim = mModel->GetAnimation(_currentAnim, false);

		if (anim != 0)
		{
			Vector2i range;
			Vector3f duration;

			if ( _animRange->GetText()		>> range	)	anim->SetFrames(range);
			if ( _animDuration->GetText()	>> duration )	anim->SetDuration(duration);
		}
	}
	return true;
}

//============================================================================================================

void OnAnimationEnd (Model* model, const Animation* anim, float timeToEnd)
{
	if ( timeToEnd == 0.0f && anim->GetName() == _currentAnim )
	{
		_animPlay->SetText("Play");
	}
}

//============================================================================================================

bool ModelViewer::OnAnimPlay (UIArea* area, const Vector2i& pos, byte key, bool isDown)
{
	if ( key == Key::MouseLeft && !isDown && area->GetRegion().Contains(pos) )
	{
		Animation* anim = mModel->GetAnimation(_currentAnim, false);

		if (anim != 0)
		{
			if (_animPlay->GetText() == "Play")
			{
				_animPlay->SetText("Stop");
				mModel->PlayAnimation(anim, &OnAnimationEnd);
			}
			else
			{
				_animPlay->SetText("Play");
				mModel->StopAnimation(anim, 0.25f, OnAnimationEnd);
			}
		}
	}
	return true;
}

//============================================================================================================
// Fills the list with all available meshes when it's selected
//============================================================================================================

bool ModelViewer::OnMeshListFocus (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		UIList* list = R5_CAST(UIList, area);

		if (list != 0)
		{
			list->ClearAllEntries();

			const Core::Meshes& meshes = mCore->GetAllMeshes();

			meshes.Lock();
			{
				for (uint i = 0; i < meshes.GetSize(); ++i)
					if (meshes[i] != 0 && meshes[i]->GetNumberOfTriangles() > 0)
						list->AddEntry( meshes[i]->GetName() );
			}
			meshes.Unlock();
		}
	}
	return true;
}

//============================================================================================================
// Fills the list with all available materials when it's selected
//============================================================================================================

bool ModelViewer::OnMatListFocus (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		UIList* list = R5_CAST(UIList, area);

		if (list != 0)
		{
			list->ClearAllEntries();
			list->AddEntry("");

			const IGraphics::Materials& mats = mGraphics->GetAllMaterials();

			mats.Lock();
			{
				for (uint i = 0; i < mats.GetSize(); ++i)
				{
					const IMaterial* mat = mats[i];

					if (mat != 0 && mat->GetDiffuse().GetColor4ub().a > 0)
					{
						list->AddEntry( mat->GetName() );
					}
				}
			}
			mats.Unlock();
		}
	}
	return true;
}

//============================================================================================================
// Fills the list with all available textures when it's selected
//============================================================================================================

bool ModelViewer::OnTexListFocus (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		UIList* list = R5_CAST(UIList, area);

		if (list != 0)
		{
			list->ClearAllEntries();
			list->AddEntry("");

			const IGraphics::Textures& textures = mGraphics->GetAllTextures();

			// Go through all available textures and fill in the drop-down lists
			textures.Lock();
			{
				for (uint i = 0; i < textures.GetSize(); ++i)
				{
					const ITexture* tex = textures[i];

					// Ensure that it doesn't contain the generated tag
					if (tex != 0 && tex->GetFormat() != ITexture::Format::Invalid)
					{
						const String& name (tex->GetName());

						if ( !name.BeginsWith("[Generated]") )
						{
							list->AddEntry( tex->GetName() );
						}
					}
				}
			}
			textures.Unlock();

			// Allow creation of new textures
			list->AddEntry(NEW);
		}
	}
	return true;
}

//============================================================================================================
// Fills in the list with all available shaders on selection
//============================================================================================================

bool ModelViewer::OnShaderListFocus (UIArea* area, bool hasFocus)
{
	if (hasFocus)
	{
		UIList* list = R5_CAST(UIList, area);

		if (list != 0)
		{
			const IGraphics::Shaders& shaders = mGraphics->GetAllShaders();

			list->ClearAllEntries();
			list->AddEntry("");

			shaders.Lock();
			{
				for (uint i = 0; i < shaders.GetSize(); ++i)
				{
					const IShader* shader = shaders[i];

					if (shader != 0 && shader->IsValid())
					{
						const String& name (shader->GetName());

						if ( !name.BeginsWith("SSAO/") &&
							 !name.BeginsWith("PostProcess/") &&
							 !name.BeginsWith("Deferred/Light/") &&
							 !name.BeginsWith("Deferred/Process/"))
						{
							list->AddEntry( shader->GetName() );
						}
					}
				}
			}
			shaders.Unlock();

			// Allow creation of new shaders
			list->AddEntry(NEW);
		}
	}
	return true;
}

//============================================================================================================
// Returns a pointer to the current rendering method
//============================================================================================================

ModelViewer::DrawMethod* ModelViewer::_GetCurrentMethod()
{
	IMaterial* mat = mGraphics->GetMaterial(_currentMat, false);
	if (mat == 0) return 0;

	ITechnique* tech = mGraphics->GetTechnique(_currentTech, false);
	if (tech == 0) return 0;

	return mat->GetDrawMethod(tech, false);
}

//============================================================================================================
// Changes the current material's shader
//============================================================================================================

void ModelViewer::_SetMatShader (const String& name)
{
	DrawMethod* method = _GetCurrentMethod();

	if (method != 0)
	{
		IShader* shader = mGraphics->GetShader(name, true);

		if (shader != 0 && !shader->IsValid())
		{
			SetStatusText("Unable to load the requested shader", Color3f(1.0f, 0.0f, 0.0f));
			shader->Release();
			shader = 0;
		}

		method->SetShader(shader);
		UpdateTechPanel(method);
	}
}

//============================================================================================================
// Changes the current material's texture
//============================================================================================================

void ModelViewer::_SetMatTexture (uint index, const String& name)
{
	DrawMethod* method = _GetCurrentMethod();

	if (method != 0)
	{
		ITexture* tex = mGraphics->GetTexture(name, true);

		if (tex != 0 && !tex->IsValid())
		{
			SetStatusText("Unable to load the requested texture", Color3f(1.0f, 0.0f, 0.0f));
			tex->Release();
			tex = 0;
		}

		method->SetTexture(index, tex);
		UpdateTechPanel(method);
	}
}

//============================================================================================================
// Updates the model information based on the values in the model frame
//============================================================================================================

void ModelViewer::_UpdateModelData()
{
	String text;
	Vector3f pos;
	Quaternion rot;
	float scale (1.0f);

	bool usePos		= (_modelUsePos->GetState()	  & UICheckbox::State::Checked) != 0;
	bool useRot		= (_modelUseRot->GetState()	  & UICheckbox::State::Checked) != 0;
	bool useScale	= (_modelUseScale->GetState() & UICheckbox::State::Checked) != 0;

	if (usePos)		_modelPos->GetText()	>> pos;
	if (useRot)		_modelRot->GetText()	>> rot;
	if (useScale)	_modelScale->GetText()	>> scale;

	mInst->SetRelativePosition(pos);
	mInst->SetRelativeRotation(rot);
	mInst->SetRelativeScale(scale);
}

//============================================================================================================
// Updates the limb information based on the values in the limb frame
//============================================================================================================

void ModelViewer::_UpdateLimbData()
{
	if (_currentLimb.IsEmpty()) return;

	Limb* limb = mModel->GetLimb(_currentLimb, false);

	if (limb != 0)
	{
		// Limb's name
		{
			const String& name = _limbName->GetText();

			if (name.IsEmpty())
			{
				_limbName->SetText(_currentLimb);
			}
			else if (_currentLimb != name)
			{
				_currentLimb = name;
				limb->SetName(name);
			}
		}

		// Limb's mesh
		{
			const String& name = _limbMesh->GetText();
			limb->SetMesh( (name.IsEmpty() ? 0 : mCore->GetMesh(name, false)) );
		}

		// Limb's material
		{
			const String& name = _limbMat->GetText();
			limb->SetMaterial( (name.IsEmpty() ? 0 : mGraphics->GetMaterial(name, false)) );
		}
	}
}

//============================================================================================================
// Model loading callback, simply triggers the load function from another thread
//============================================================================================================

R5_THREAD_FUNCTION(LoadModel, ptr)
{
	ModelViewer* mv = (ModelViewer*)ptr;
	mv->Load();
	return 0;
}

//============================================================================================================
// Triggers the confirmation function for the file dialog, saving or loading the model
//============================================================================================================

void ModelViewer::_ConfirmFileDialog()
{
	const String& text = _fileOK->GetText();
	const String& file = _fileInput->GetText();

	if ( text == "Save" )
	{
		if ( mModel->Save( file ) )
		{
			SetStatusText( String("Saved as '%s'", file.GetBuffer()) );
			_fileInput->AddToHistory( _fileInput->GetText() );
			_fileDialog->Hide();
		}
		else
		{
			SetStatusText("Unable to save, please provide a different path", Color3f(1.0f, 0.0f, 0.0f));
		}
	}
	else if ( text == "Load" )
	{
		_fileDialog->Hide();
		mLoadFilename = file;
		Thread::Create( LoadModel, this );
	}
}