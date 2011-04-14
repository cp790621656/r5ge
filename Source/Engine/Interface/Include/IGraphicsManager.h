#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Higher level of renderer API interaction, handles resource management for graphical resources
// Author: Michael Lyashenko
//============================================================================================================

struct IGraphicsManager
{
	// Delegate for a delayed callback that should be executed on the graphics thread
	typedef FastDelegate<void (IGraphicsManager* graphics, void* param)> DelayedDelegate;

	struct Primitive
	{
		enum
		{
			Line,
			LineStrip,
			Triangle,
			TriangleStrip,
			TriangleFan,
			Quad,
			QuadStrip,
			Point,
		};
	};

	struct Drawable
	{
		enum
		{
			Skybox,
			FullscreenQuad,
			InvertedQuad,
			Plane,
			Grid,
			Axis,
		};
	};

	// Resources should be stored in named pointer arrays
	typedef ResourceArray<ITechnique>	Techniques;
	typedef ResourceArray<IMaterial>	Materials;
	typedef ResourceArray<ITexture>		Textures;
	typedef ResourceArray<ISubShader>	SubShaders;
	typedef ResourceArray<IShader>		Shaders;
	typedef ResourceArray<IFont>		Fonts;

	// Returns whether the specified point would be visible if rendered
	virtual bool IsPointVisible (const Vector3f& v)=0;

	// Reads the buffer's color at the specified pixel
	virtual Color4f ReadColor (const Vector2i& pos)=0;

	// Converts screen coordinates to world coordinates and vice versa
	virtual Vector3f  ConvertTo3D (const Vector2i& pos, bool unproject = true)=0;
	virtual Vector2i  ConvertTo2D (const Vector3f& pos)=0;

	// Initialize/release the graphics manager
	virtual bool Init (float version = 2.0f)=0;
	virtual void Release()=0;

	// Adds a delayed callback function that should be executed on the next frame (at BeginFrame)
	virtual void ExecuteBeforeNextFrame(const DelayedDelegate& callback, void* param = 0)=0;

	// Clear the screen or the off-screen target, rendering the skybox if necessary (pre-render)
	virtual void Clear (bool color = true, bool depth = true, bool stencil = true)=0;

	// Pre/post-render
	virtual void BeginFrame()=0;
	virtual void EndFrame()=0;

	// Draws a pre-defined drawable object such as a skybox or a full-screen quad
	virtual uint Draw(uint drawable)=0;

	// Direct access to managed resource arrays
	virtual Techniques&	GetAllTechniques()=0;
	virtual Materials&	GetAllMaterials()=0;
	virtual Textures&	GetAllTextures()=0;
	virtual SubShaders&	GetAllSubShaders()=0;
	virtual Shaders&	GetAllShaders()=0;
	virtual Fonts&		GetAllFonts()=0;

	// Managed unnamed resources
	virtual IVBO*			CreateVBO()=0;
	virtual ITexture*		CreateRenderTexture(const char* name = 0)=0;
	virtual IRenderTarget*	CreateRenderTarget()=0;

	// Resource removal
	virtual void DeleteVBO			(const IVBO*			ptr)=0;
	virtual void DeleteTexture		(const ITexture*		ptr)=0;
	virtual void DeleteRenderTarget	(const IRenderTarget*	ptr)=0;

	// Managed named resources (to release their memory just call their ::Release() function)
	virtual ITechnique*		GetTechnique	(const String& name, bool createIfMissing = true)=0;
	virtual IMaterial*		GetMaterial		(const String& name, bool createIfMissing = true)=0;
	virtual ITexture*		GetTexture		(const String& name, bool createIfMissing = true)=0;
	virtual ISubShader*		GetSubShader	(const String& name, bool createIfMissing = true)=0;
	virtual IShader*		GetShader		(const String& name, bool createIfMissing = true)=0;
	virtual IFont*			GetFont			(const String& name, bool createIfMissing = true)=0;
};