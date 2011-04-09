#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================

GLGraphics::GLGraphics() :
	mThread		(0),
	mSkyboxVBO	(0),
	mSkyboxIBO	(0),
	mQuery		(0)
{
}

//============================================================================================================

GLGraphics::~GLGraphics()
{
#ifdef _DEBUG
	System::Log("[OPENGL]  Shutting down. Summary of all managed resources:");
	System::Log("          - VBOs:       %u",	mVbos.GetSize());
	System::Log("          - FBOs:       %u",	mFbos.GetSize());
	System::Log("          - Textures:   %u",	mTextures.GetSize());
	System::Log("          - SubShaders: %u",	mSubShaders.GetSize());
	System::Log("          - Shaders:    %u",	mShaders.GetSize());
	System::Log("          - Fonts:      %u",	mFonts.GetSize());
	System::Log("          - Materials:  %u",	mMaterials.GetSize());
#endif

	Release();

	// Not sure why, but deleting this query in release mode causes a violation
	//if (mQuery != 0) glDeleteQueries(1, &mQuery);
}

//============================================================================================================
// Gets the specified sub-shader entry
//============================================================================================================

GLSubShader* GLGraphics::GetGLSubShader (const String& filename, bool createIfMissing, byte type)
{
	GLSubShader* sub = (GLSubShader*)mSubShaders.Find(filename, false);

	if (sub == 0 && createIfMissing)
	{
		// Create a new sub-shader entry
		sub	= new GLSubShader(this, filename, type);

		// Add this shader to the managed list
		mSubShaders.Expand() = sub;

		// Initialize the shader
		sub->_Init();
	}
	return sub;
}

//============================================================================================================
// Returns whether the specified point would be visible if rendered
//============================================================================================================

bool GLGraphics::IsPointVisible (const Vector3f& v)
{
	// Reset the ModelView matrix as the query is affected by it
	ResetModelMatrix();
	ResetViewMatrix();

	// Convert the 3D point to screen coordinates
	Vector2i screen (ConvertTo2D(v));

	// Occlusion query should not be performed if the point is off-screen
	if (screen.x >= 0 && screen.y >= 0 && screen.x < mSize.x && screen.y < mSize.y)
	{
		if (g_caps.mOcclusion)
		{
			// Activate the matrices as the query will need them to be properly set up
			_ActivateMatrices();

			// If the query hasn't been created yet, do that now
			if (mQuery == 0)
			{
				glGenQueries(1, &mQuery);
				CHECK_GL_ERROR;
			}

			// Depth test should be considered in occlusion tests, so ensure that it's on
			if (!mDepthTest) glEnable(GL_DEPTH_TEST);

			// Begin a query and render a single point at the specified coordinates
			glBeginQuery(GL_SAMPLES_PASSED, mQuery);
			{
				glBegin(GL_POINTS);
				glVertex3f(v.x, v.y, v.z);
				glEnd();
			}
			glEndQuery(GL_SAMPLES_PASSED);

			// Restore the depth testing value
			if (!mDepthTest) glDisable(GL_DEPTH_TEST);

			// Retrieve the query's result
			uint count (0);
			glGetQueryObjectuiv(mQuery, GL_QUERY_RESULT, &count);
			CHECK_GL_ERROR;

			// The point is visible if the number of fragments returned is not zero
			return (count != 0);
		}
	}
	return false;
}

//============================================================================================================
// Reads the buffer's color at the specified pixel
//============================================================================================================

Color4f GLGraphics::ReadColor (const Vector2i& v)
{
	Vector2i size ( mTarget ? mTarget->GetSize() : mSize );
	Vector2i pos  ( v.x, mSize.y - v.y );
	Color4f color;

	if (pos.x >= 0 && pos.y < size.x && pos.y >= 0 && pos.y < size.y)
	{
		glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, GL_FLOAT, &color);
	}
	return color;
}

//============================================================================================================
// Converts screen coordinates to world coordinates
//------------------------------------------------------------------------------------------------------------
// WARNING: glReadPixels operation is very slow on some graphics cards (Intel X3100) when used with an FBO!
//============================================================================================================

Vector3f GLGraphics::ConvertTo3D (const Vector2i& v, bool unproject)
{
	ASSERT(mTarget == 0 || g_caps.mVendor != DeviceInfo::Vendor::Intel,
		"This function is ungodly slow on Intel cards");

	Vector2i size ( mTarget ? mTarget->GetSize() : mSize );
	Vector2i pos  ( v.x, mSize.y - v.y );

	float depth (0.0f);

	if (pos.x >= 0 && pos.y < size.x && pos.y >= 0 && pos.y < size.y)
	{
		glReadPixels(pos.x, pos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
	}

	// To calculate the linearly scaled value between near and far plane:
	//float range = mFarPlane - mNearPlane;
    //float val = (mFarPlane * mNearPlane) / (mFarPlane - depth * range);

	// To convert the above value to 0-1 range:
	//val = (val - mNearPlane) / range;
    //val = Float::Clamp(val, 0.0f, 1.0f);

	Vector3f out ((float)pos.x / size.x, (float)pos.y / size.y, depth);
	if (unproject) out = GetInverseMVPMatrix().Unproject(out);
	return out;
}

//============================================================================================================
// Converts world coordinates to screen coordinates
//============================================================================================================

Vector2i GLGraphics::ConvertTo2D (const Vector3f& v)
{
	// Same as ((v * viewProjMat) * 0.5f + 0.5f) * size, with inverted Y (y = size.y - y)
	Vector2i size ( mTarget != 0 ? mTarget->GetSize() : mSize );
	Vector2i out  ( GetModelViewProjMatrix().Project(v) * size );
	out.y = size.y - out.y;
	return out;
}

//============================================================================================================
// Returns the distance between 'v' and the depth-determined point of intersection on the ray from the
// camera's eyepoint to 'v'.
//------------------------------------------------------------------------------------------------------------
// Return value is the distance between the specified point and the actual point where ray hits
// the depth value. Positive return value means the specified point is visible. Negative return
// value means the point is occluded.
//------------------------------------------------------------------------------------------------------------
// WARNING: Just like GLGraphics::ConvertTo3D(), this function is *very* slow when executed on some videocards,
//          such as the Intel's X3100 because instead of reading a single pixel Intel cards reads the whole
//          frame buffer and only then copy the requested pixel. As such, try to limit this function's use.
//          Try to use GLGraphics::IsPointVisible() whenever possible instead.
//============================================================================================================

//float GLGraphics::GetDistanceToDepthAt (const Vector3f& v)
//{
//	Vector2i size	( mTarget ? mTarget->GetSize() : mSize );
//	Vector2f rel	( GetModelViewProjMatrix().Project(v) );
//	Vector2i pos	( rel * size );
//
//	float depth (0.0f);
//	if (pos.x >= 0 && pos.y < size.x && pos.y >= 0 && pos.y < size.y )
//	{
//		glReadPixels(pos.x, pos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
//		CHECK_GL_ERROR;
//
//		if (depth < 1.0f)
//		{
//			Vector3f p (GetInverseMVPMatrix().Unproject(rel, depth));
//			float distanceToV = mEye.GetDistanceTo(v);
//			float distanceToD = mEye.GetDistanceTo(p);
//			return distanceToD - distanceToV;
//		}
//		else return 0.0f;
//	}
//	return -mEye.GetDistanceTo(v);
//}

//============================================================================================================
// Initializes the graphics manager
//============================================================================================================

bool GLGraphics::Init (float version)
{
	if (mThread == 0)
	{
		mThread = Thread::GetID();
		if (!InitOpenGL(version)) return false;

		// Set the default states
		SetDepthWrite	(true);
		SetDepthTest	(true);
		SetAlphaTest	(true);
		SetAlphaCutoff	(0.003921568627451f);
		SetBlending		(true);
		SetBlending		(Blending::Replace);
		SetCulling		(Culling::Back);
		SetFogRange		(Vector2f(50.0f, 100.0f));
		SetCameraRange	(Vector3f(0.3f, 100.0f, 90.0f));
		SetFog			(true);

		// For simplicity  purposes, only linear type fog is supported as it's very easy to create
		// a much nicer, animated fog using post-processing effects, especially with the deferred approach.
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glHint(GL_FOG_HINT, GL_FASTEST);

		// In order to match non-shader lighting calculations for specular to shader calculations,
		// specular component should be calculated separately, and added after the texture is applied.
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

		// Default scene ambient color is black -- let lights control ambient lighting
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Color4f(0, 0, 0, 1));

#ifdef _WINDOWS
		// By default, the buffer swapping should not be synchronized with the monitor's refresh rate
		if (glSwapInterval) glSwapInterval(0);
#endif
		return true;
	}
	return (mThread != 0);
}

//============================================================================================================
// Release the graphical resources
//============================================================================================================

void GLGraphics::Release()
{
	if (mThread != 0)
	{
		ASSERT(mThread == Thread::GetID(),
			"R5::IGraphics::Release() must be called from the same thread as R5::IGraphics::Init()");

		mThread = 0;
		mSkyboxVBO = 0;
		mSkyboxIBO = 0;

		mFonts.Release();
		mShaders.Release();
		mSubShaders.Release();
		mTextures.Release();
		mMaterials.Release();
		mTechs.Release();
		mFbos.Release();
		mVbos.Release();

#ifdef _DEBUG
		System::Log("[OPENGL]  Resources have been released. Executing remaining callbacks.");
		System::Log("          - Delegates: %u", mDelegates.GetSize());
#endif

		// Call any pending callbacks
		for (uint i = 0; i < mDelegates.GetSize(); ++i)
		{
			DelegateEntry& entry = mDelegates[i];
			entry.callback(this, entry.param);
		}
		mDelegates.Release();

#ifdef _DEBUG
		System::Log("          - Done.");
#endif
	}
}

//============================================================================================================
// Adds a delayed callback function that should be executed on the next frame (at BeginFrame)
//============================================================================================================

void GLGraphics::ExecuteBeforeNextFrame(const DelayedDelegate& callback, void* param)
{
	mDelegates.Lock();
	{
		DelegateEntry& entry = mDelegates.Expand();
		entry.callback = callback;
		entry.param = param;
	}
	mDelegates.Unlock();
}

//============================================================================================================
// Clear the screen or the off-screen target, rendering the skybox if necessary
//============================================================================================================

void GLGraphics::Clear (bool color, bool depth, bool stencil)
{
	if (!color && !depth && !stencil) return;

	if (mTarget != 0)
	{
		if (!mTarget->HasColor())	 color   = false;
		if (!mTarget->HasDepth())	 depth   = false;
		if (!mTarget->HasStencil())	 stencil = false;
	}

	bool skyboxIsPresent = (mSkybox != 0 && ( mSkybox->GetType() == ITexture::Type::EnvironmentCubeMap ));

	// Remember color/depth write setting
	bool colorWrite (mColorWrite);
	bool depthWrite (mDepthWrite);
	bool depthTest  (mDepthTest);

	uint flag (0);

	if (skyboxIsPresent && (mTarget == 0 || mTarget->IsUsingSkybox()))
	{
		if (depth)
		{
			flag |= GL_DEPTH_BUFFER_BIT;
			SetDepthWrite(true);
		}

		if (stencil)
		{
			flag |= GL_STENCIL_BUFFER_BIT;
		}

		if (flag)
		{
			glClear(flag);
		}

		if (color)
		{
			SetScreenProjection(false);
			Draw(Drawable::Skybox);
		}
	}
	else
	{
		if (color)
		{
			flag |= GL_COLOR_BUFFER_BIT;

			if (!mColorWrite) SetColorWrite(true);
			glClearColor(mBackground.r, mBackground.g, mBackground.b, mBackground.a);
		}

		if (depth)
		{
			flag |= GL_DEPTH_BUFFER_BIT;
			if (!mDepthWrite) SetDepthWrite(true);
		}

		if (stencil)
		{
			flag |= GL_STENCIL_BUFFER_BIT;
		}

		if (flag) glClear(flag);
	}

	// Restore previous settings
	if (color && mColorWrite != colorWrite)
	{
		SetColorWrite(colorWrite);
	}

	if (depth)
	{
		if (mDepthWrite != depthWrite)	SetDepthWrite(depthWrite);
		if (mDepthTest  != depthTest)	SetDepthTest(depthTest);
	}
	CHECK_GL_ERROR;
}

//============================================================================================================
// Pre-render
//============================================================================================================

void GLGraphics::BeginFrame()
{
	mStats.Clear();

	if (mDelegates.IsValid())
	{
		mDelegates.Lock();
		{
			for (int i = 0, imax = mDelegates.GetSize(); i < imax; ++i)
			{
				DelegateEntry& entry = mDelegates[i];
				entry.callback(this, entry.param);
			}
			mDelegates.Clear();
		}
		mDelegates.Unlock();
	}
}

//============================================================================================================
// Post-render: Release any unused resources
//============================================================================================================

void GLGraphics::EndFrame()
{
	// NOTE: Intel driver seems to have a bug in it where it resets the blending mode to its default
	// value when the frame finishes the draw process (src_alpha, one_minus_src_alpha).
	// This call here effectively fixes the issue, but it's obviously a work-around rather than a true fix.
	SetBlending(IGraphics::Blending::Replace);
	mTechnique = 0;
}

//============================================================================================================
// Draws the active skybox cube
//============================================================================================================

uint GLGraphics::Draw (uint drawable)
{
	uint result (0);

	// Arrays are not used for draw operations below
	SetActiveVertexAttribute( Attribute::Normal,	 0, 0, 0, 0, 0 );
	SetActiveVertexAttribute( Attribute::Tangent,	 0, 0, 0, 0, 0 );
	SetActiveVertexAttribute( Attribute::Color,		 0, 0, 0, 0, 0 );
	SetActiveVertexAttribute( Attribute::BoneIndex,  0, 0, 0, 0, 0 );
	SetActiveVertexAttribute( Attribute::BoneWeight, 0, 0, 0, 0, 0 );
	SetActiveVertexAttribute( Attribute::TexCoord1,	 0, 0, 0, 0, 0 );

	if (drawable == Drawable::Skybox)
	{
		if (mSkybox == 0) return result;

		if (mSkyboxVBO == 0)
		{
			Array<Vector3f> vertices;
			Array<ushort> indices;

			Shape::Box(vertices, indices, 20.0f, true);

			mSkyboxVBO = CreateVBO();
			mSkyboxVBO->Lock();
			mSkyboxVBO->Set(vertices, IVBO::Type::Vertex);
			mSkyboxVBO->Unlock();

			mSkyboxIBO = CreateVBO();
			mSkyboxIBO->Lock();
			mSkyboxIBO->Set(indices, IVBO::Type::Index);
			mSkyboxIBO->Unlock();
		}

		SetFog(false);
		SetDepthTest(false);
		SetDepthWrite(false);
		SetAlphaTest(false);
		SetColorWrite(true);
		SetLighting(Lighting::None);
		SetBlending(Blending::Replace);

		glEnable(GL_COLOR_MATERIAL);
		SetActiveMaterial(mSkybox);
		SetActiveShader(0);
		glColor3ub(255, 255, 255);

		// Overwrite the ModelView matrix, changing it to use the current camera's position as origin
		ResetModelViewMatrix();
		const Vector3f& eye = GetCameraPosition();
		Matrix43 view (GetViewMatrix());
		view.PreTranslate(eye);
		SetViewMatrix(view);

		// Set all active vertex attributes
		SetActiveVertexAttribute( Attribute::TexCoord0, mSkyboxVBO, 0, DataType::Float, 3, sizeof(Vector3f) );
		SetActiveVertexAttribute( Attribute::Vertex,	mSkyboxVBO, 0, DataType::Float, 3, sizeof(Vector3f) );

		// Draw the skybox
		result = DrawIndices( mSkyboxIBO, Primitive::Triangle, 36 );

		// Reset the ModelView matrix back to its default values
		ResetViewMatrix();
		return result;
	}

	// All draw operations that follow don't use texture coordinate arrays
	SetActiveVertexAttribute(IGraphics::Attribute::TexCoord0, 0, 0, 0, 0, 0);

	if (drawable == Drawable::FullscreenQuad)
	{
		Vector2i size ( mTarget ? mTarget->GetSize() : mSize );

		_BindAllTextures();
		_ActivateMatrices();

		glBegin(GL_QUADS);
		{
			glTexCoord2i(0, 0);
			glVertex2i(0, 0);

			glTexCoord2i(0, 1);
			glVertex2i(0, size.y);

			glTexCoord2i(1, 1);
			glVertex2i(size.x, size.y);

			glTexCoord2i(1, 0);
			glVertex2i(size.x, 0);
		}
		glEnd();

		result = 2;
	}
	else if (drawable == Drawable::InvertedQuad)
	{
		Vector2i size ( mTarget ? mTarget->GetSize() : mSize );

		_BindAllTextures();
		_ActivateMatrices();

		glBegin(GL_QUADS);
		{
			glTexCoord2i(0, 1);
			glVertex2i(0, 0);

			glTexCoord2i(0, 0);
			glVertex2i(0, size.y);

			glTexCoord2i(1, 0);
			glVertex2i(size.x, size.y);

			glTexCoord2i(1, 1);
			glVertex2i(size.x, 0);
		}
		glEnd();

		result = 2;
	}
	else if (drawable == Drawable::Plane)
	{
		ResetModelViewMatrix();

		_BindAllTextures();
		_ActivateMatrices();

		glBegin(GL_QUADS);
		{
			glNormal3f(0, 0, 1);

			glTexCoord2f(0, 0);
			glVertex2i  (0, 1);

			glTexCoord2f(0,	1);
			glVertex2i  (0, 0);

			glTexCoord2f(1, 1);
			glVertex2i  (1, 0);

			glTexCoord2f(1, 0);
			glVertex2i  (1, 1);
		}
		glEnd();

		result = 2;
	}
	else if (drawable == Drawable::Grid)
	{
		byte light = mLighting;
		
		SetFog(true);
		SetDepthTest(true);
		SetDepthWrite(true);
		SetAlphaTest(false);
		SetColorWrite(true);
		SetLighting(Lighting::None);
		SetBlending(Blending::None);
		SetActiveMaterial((const IMaterial*)0);
		SetActiveVertexAttribute( Attribute::Color, 0, 0, 0, 0, 0 );
		Flush();

		ResetModelViewMatrix();

		_BindAllTextures();
		_ActivateMatrices();

		glBegin(GL_LINES);
		{
			glColor3ub(50, 50, 50);

			for (int i = 1; i < 11; ++i)
			{
				glVertex2i(i,  10);
				glVertex2i(i, -10);

				glVertex2i(-i,  10);
				glVertex2i(-i, -10);

				glVertex2i( 10, i);
				glVertex2i(-10, i);

				glVertex2i( 10, -i);
				glVertex2i(-10, -i);
			}

			glColor3ub(255, 0, 0);
			glVertex2i( 11, 0);
			glVertex2i(-10, 0);
			
			glColor3ub(0, 255, 0);
			glVertex2i(0,  11);
			glVertex2i(0, -10);

			glColor3ub(255, 255, 255);
		}
		glEnd();

		if (light) SetLighting(light);
		result = 42;
	}
	else if (drawable == Drawable::Axis )
	{
		byte light = mLighting;
		
		SetFog(true);
		SetDepthTest(true);
		SetDepthWrite(true);
		SetAlphaTest(false);
		SetColorWrite(true);
		SetLighting(Lighting::None);
		SetBlending(Blending::Replace);
		SetActiveMaterial((const IMaterial*)0);

		_BindAllTextures();
		_ActivateMatrices();

		glBegin(GL_LINES);
		{
			glColor3ub(255, 0, 0);
			glVertex3i(0, 0, 0);
			glVertex3i(1, 0, 0);
			
			glColor3ub(0, 255, 0);
			glVertex3i(0, 0, 0);
			glVertex3i(0, 1, 0);

			glColor3ub(0, 0, 255);
			glVertex3i(0, 0, 0);
			glVertex3i(0, 0, 1);

			glColor3ub(255, 255, 255);
		}
		glEnd();

		if (light) SetLighting(light);
		result = 3;
	}
	// Nothing is drawn -- no triangles
	return result;
};

//============================================================================================================
// Creates a new vertex buffer object
// NOTE: Intentionally not thread-safe as this function should only be called from the graphics thread
//============================================================================================================

IVBO* GLGraphics::CreateVBO()
{
	if (g_caps.mBufferObjects)
	{
		for (uint i = 0; i < mVbos.GetSize(); ++i)
		{
			if ( mVbos[i] == 0 )
			{
				mVbos[i] = new GLVBO(this);
				return mVbos[i];
			}
		}
		mVbos.Expand() = new GLVBO(this);
		return mVbos.Back();
	}
	return 0;
}

//============================================================================================================
// Creates a new temporary texture resource
//============================================================================================================

ITexture* GLGraphics::CreateRenderTexture (const char* name)
{
	static uint counter = 0;
	mTempTex.Lock();
	String s;
	s << "[Generated] ";

	if (name != 0)
	{
		s << name;
	}
	else
	{
		s << "Render Texture ";
		s << counter++;
	}
	GLTexture* tex = new GLTexture(s, this);
	mTempTex.Expand() = tex;
	mTempTex.Unlock();
	return tex;
}

//============================================================================================================
// Creates a new frame buffer object
// NOTE: Intentionally not thread-safe as this function should only be called from the graphics thread
//============================================================================================================

IRenderTarget* GLGraphics::CreateRenderTarget()
{
	if (g_caps.mDrawBuffers)
	{
		for (uint i = 0; i < mFbos.GetSize(); ++i)
		{
			if ( mFbos[i] == 0 )
			{
				mFbos[i] = new GLFBO(this);
				return mFbos[i];
			}
		}
		mFbos.Expand() = new GLFBO(this);
		return mFbos.Back();
	}
	return 0;
}

//============================================================================================================
// Deletes a vertex buffer object if it can be found in the managed array
//============================================================================================================

void GLGraphics::DeleteVBO (const IVBO* ptr)
{
	if (ptr == 0) return;

	mVbos.Lock();
	{
		FOREACH(i, mVbos)
		{
			if (mVbos[i] == ptr)
			{
				mVbos[i]->Release();
				delete mVbos[i];
				mVbos[i] = 0;
				break;
			}
		}
	}
	mVbos.Unlock();
}

//============================================================================================================
// Deletes a previously created texture
//============================================================================================================

void GLGraphics::DeleteTexture (const ITexture* ptr)
{
	if (ptr == 0) return;

	mTempTex.Lock();
	{
		FOREACH(i, mTempTex)
		{
			if (mTempTex[i] == ptr)
			{
				mTempTex[i]->Release();
				mTempTex.DeleteAt(i);
				mTempTex.Unlock();
				return;
			}
		}
	}
	mTempTex.Unlock();

	mTextures.Lock();
	{
		FOREACH(i, mTextures)
		{
			if (mTextures[i] == ptr)
			{
				mTempTex[i]->Release();
				mTextures.DeleteAt(i);
				mTextures.Unlock();
				return;
			}
		}
	}
	mTextures.Unlock();
}

//============================================================================================================
// Deletes a frame buffer object if it can be found in the managed array
//============================================================================================================

void GLGraphics::DeleteRenderTarget (const IRenderTarget* ptr)
{
	if (ptr == 0) return;

	mFbos.Lock();
	{
		FOREACH(i, mFbos)
		{
			if ( mFbos[i] == ptr )
			{
				mFbos[i]->Release();
				mFbos.DeleteAt(i);
				break;
			}
		}
	}
	mFbos.Unlock();
}

//============================================================================================================
// Retrieves or creates a new render group -- contains several default presets for convenience
//============================================================================================================

ITechnique* GLGraphics::GetTechnique (const String& name, bool createIfMissing)
{
	ITechnique* tech (0);

	if (name.IsValid())
	{
		// Get the render group
		mTechs.Lock();
		{
			for (uint i = 0; i < mTechs.GetSize(); ++i)
			{
				tech = mTechs[i];

				if (tech != 0 && tech->GetName() == name)
				{
					// If it's found, just return it
					mTechs.Unlock();
					return tech;
				}
			}

			// Not found -- add a new one
			if (createIfMissing)
			{
				tech = new GLTechnique(name);

				if (name == "Depth")
				{
					tech->SetFog(false);
					tech->SetColorWrite(false);
					tech->SetAlphaTest(true);
					tech->SetLighting(ITechnique::Lighting::None);
					tech->SetBlending(ITechnique::Blending::None);
				}
				else if (name == "Glow")
				{
					// Glow is drawn with no depth writing, using additive blending and no lighting
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetAlphaTest(false);
					tech->SetBlending(ITechnique::Blending::Add);
					tech->SetLighting(ITechnique::Lighting::None);
					tech->SetSorting(ITechnique::Sorting::BackToFront);
				}
				else if (name == "Glare")
				{
					// Glare is drawn last, as it goes on top of everything and doesn't use depth, blending or lighting
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetDepthTest(false);
					tech->SetAlphaTest(false);
					tech->SetBlending(ITechnique::Blending::Add);
					tech->SetLighting(ITechnique::Lighting::None);
					tech->SetSorting(ITechnique::Sorting::BackToFront);
				}
				else if (name == "Particle")
				{
					tech->SetDepthWrite(false);
					tech->SetLighting(ITechnique::Lighting::None);
					tech->SetSorting(ITechnique::Sorting::BackToFront);
				}
				else if (name == "Post Process")
				{
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetDepthTest(false);
					tech->SetAlphaTest(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
				}
				else if (name == "Deferred")
				{
					tech->SetFlag(ITechnique::Flag::Deferred, true);
					tech->SetFog(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
				}
				else if (name == "Decal")
				{
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
				}
				else if (name == "Projected Texture")
				{
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::Replace);
				}

				// Wireframe technique
				if (name.Contains("Wireframe"))
				{
					tech->SetFog(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
					tech->SetWireframe(true);
				}

				// Solid material group -- no alpha testing or blending needed
				if (name.Contains("Opaque"))
				{
					tech->SetAlphaTest(false);
					tech->SetBlending(ITechnique::Blending::None);
				}
				else if (name.Contains("Transparent"))
				{
					// Group for objects with transparency, drawn after solids -- default everything
					tech->SetSorting(ITechnique::Sorting::BackToFront);
				}

				if (name.Contains("Shadowed"))
				{
					tech->SetFlag(ITechnique::Flag::Shadowed, true);
				}

				// Newly created techniques should not be serializable until something changes
				tech->SetSerializable(false);
				mTechs.Expand() = tech;
			}
			else tech = 0;
		}
		mTechs.Unlock();
	}
	return tech;
}

//============================================================================================================
// The material class uses graphics to associate materials with textures
//============================================================================================================

IMaterial* GLGraphics::GetMaterial (const String& name, bool createIfMissing)
{
	IMaterial* mat = 0;

	if (name.IsValid())
	{
		mMaterials.Lock();
		{
			for (uint i = 0; i < mMaterials.GetSize(); ++i)
			{
				mat = mMaterials[i];

				if (mat != 0 && mat->GetName() == name)
				{
					mMaterials.Unlock();
					return mat;
				}
			}

			if (createIfMissing)
			{
				GLMaterial* glMat = new GLMaterial(name);
				glMat->_SetGraphics(this);
				mMaterials.Expand() = glMat;
				mat = glMat;
			}
			else mat = 0;
		}
		mMaterials.Unlock();
	}
	return mat;
}

//============================================================================================================
// Retrieves or creates a texture with the specified name
//============================================================================================================

ITexture* GLGraphics::GetTexture (const String& name, bool createIfMissing)
{
	ITexture* tex = 0;

	if (name.IsValid())
	{
		mTextures.Lock();
		{
			for (uint i = 0; i < mTextures.GetSize(); ++i)
			{
				tex = mTextures[i];

				if (tex != 0 && tex->GetName() == name)
				{
					mTextures.Unlock();
					return tex;
				}
			}

			if (createIfMissing)
			{
				GLTexture* myTex = new GLTexture(name, this);
				mTextures.Expand() = myTex;
				tex = myTex;
			}
			else tex = 0;
		}
		mTextures.Unlock();
	}
	return tex;
}

//============================================================================================================
// Creates a new shader entry and automatically binds common uniform callbacks
//============================================================================================================

IShader* GLGraphics::GetShader (const String& name, bool createIfMissing)
{
	GLShader* shader = 0;

	if (name.IsValid())
	{
		mShaders.Lock();
		{
			for (uint i = 0; i < mShaders.GetSize(); ++i)
			{
				shader = (GLShader*)mShaders[i];

				if (shader != 0 && shader->mName == name)
				{
					mShaders.Unlock();
					return shader;
				}
			}

			if (createIfMissing)
			{
				// Add this new shader and initialize it
				mShaders.Expand() = (shader = new GLShader());
				shader->Init(this, name);
			}
			else shader = 0;
		}
		mShaders.Unlock();
	}
	return shader;
}

//============================================================================================================
// Retrieves or creates a font with the specified name
//============================================================================================================

IFont* GLGraphics::GetFont (const String& name, bool createIfMissing)
{
	IFont* font = 0;

	if (name.IsValid())
	{
		mFonts.Lock();
		{
			for (uint i = 0; i < mFonts.GetSize(); ++i)
			{
				font = mFonts[i];

				if (font != 0 && font->GetName() == name)
				{
					mFonts.Unlock();
					return font;
				}
			}

			if (createIfMissing)
			{
				font = new GLFont(name, this);
				mFonts.Expand() = font;
			}
			else font = 0;
		}
		mFonts.Unlock();
	}
	return font;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

#define EXECUTE(type, fnc)														\
	{																			\
		type* p = fnc(value.AsString());	\
		bool isSerializable = p->IsSerializable();								\
		p->SerializeFrom(node, forceUpdate);									\
		if (!isSerializable && !serializable) p->SetSerializable(false);		\
	}

//============================================================================================================

bool GLGraphics::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	if (mThread == 0) return false;
	String skybox;

	bool serializable = true;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if (tag == ClassID())
		{
			return SerializeFrom(node, forceUpdate);
		}
		else if (tag == "Serializable")
		{
			value >> serializable;
		}
		else if (tag == "Default AF" || tag == "Anisotropic Filter")
		{
			uint val;
			if (value >> val) SetDefaultAF(val);
		}
		else if (tag == "Background Color")
		{
			Color4f color;
			if (value >> color) SetBackgroundColor(color);
		}
		else if (tag == "Fog Range")
		{
			Vector2f range;
			if (value >> range) SetFogRange(range);
		}
		else if (tag == "Skybox") skybox = (value.AsString());
		else if	(tag == ITechnique::ClassID())	EXECUTE(ITechnique,	GetTechnique)
		else if (tag == ITexture::ClassID())	EXECUTE(ITexture,	GetTexture)
		else if (tag == IMaterial::ClassID())	EXECUTE(IMaterial,	GetMaterial)
		else if (tag == IFont::ClassID())		EXECUTE(IFont,		GetFont)
		else if (tag == "DXT")
		{
			// Turn off DXT compression if requested
			if (value.IsBool() && !value.AsBool())
			{
				g_caps.mDXTCompression = false;
			}
		}
	}

	// If a skybox was found at some point, set it now
	// NOTE: I did it this way so it can be saved first, but loaded last, so it doesn't cause
	// an extra texture search while it's being loaded with a name like "Environment". Other than
	// that this is a purely aesthetic change and doesn't really affect anything besides a log entry.
	if (skybox.IsValid()) mSkybox = GetTexture(skybox);
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

#define SAVE(arr)	{ arr.Lock(); for (uint i = 0; i < arr.GetSize(); ++i) arr[i]->SerializeTo(node); arr.Unlock(); }

bool GLGraphics::SerializeTo (TreeNode& root) const
{
	TreeNode& node = root.AddChild( ClassID() );
	node.AddChild("Default AF", mAf);

	if ( mSkybox && mSkybox->GetType() == ITexture::Type::EnvironmentCubeMap )
		node.AddChild("Skybox", mSkybox->GetName());

	SAVE(mTechs);
	SAVE(mTextures);
	SAVE(mMaterials);
	SAVE(mFonts);

	return true;
}