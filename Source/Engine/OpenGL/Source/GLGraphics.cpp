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
	System::Log("          - VBOs:      %u",	mVbos.GetSize());
	System::Log("          - FBOs:      %u",	mFbos.GetSize());
	System::Log("          - Textures:  %u",	mTextures.GetSize());
	System::Log("          - Shaders:   %u",	mShaders.GetSize());
	System::Log("          - Fonts:     %u",	mFonts.GetSize());
	System::Log("          - Materials: %u",	mMaterials.GetSize());
#endif

	Release();

	// Not sure why, but deleting this query in release mode causes a violation
	//if (mQuery != 0) glDeleteQueries(1, &mQuery);
}

//============================================================================================================
// Shader callback function for R5_time uniform
//------------------------------------------------------------------------------------------------------------
// R5_time.x = Current time in seconds
// R5_time.y = Irregular wavy sin(time), used for wind
// R5_time.z = sin(R5_time.z) gives a 360 degree rotation every 1000 seconds
//============================================================================================================

void SetUniform_Time (const String& name, Uniform& uniform)
{
	uniform.mType = Uniform::Type::Float3;
	uniform.mVal[0] = Time::GetTime();
	uniform.mVal[1] = (0.6f * Float::Sin(uniform.mVal[0] * 0.421f) +
					   0.3f * Float::Sin(uniform.mVal[0] * 1.737f) +
					   0.1f * Float::Cos(uniform.mVal[0] * 2.786f)) * 0.5f + 0.5f;
	uniform.mVal[2] = Float::Fract(uniform.mVal[0] * 0.001f) * TWOPI;
}

//============================================================================================================
// Shader callback function for R5_eyePos
//============================================================================================================

void GLGraphics::SetUniform_EyePos (const String& name, Uniform& uniform)
{
	uniform = mEye;
}

//============================================================================================================
// Shader callback function for R5_pixelSize
//------------------------------------------------------------------------------------------------------------
// Can be used to figure out 0-1 range full-screen texture coordinates in the fragment shader:
// gl_FragCoord.xy * R5_pixelSize
//============================================================================================================

void GLGraphics::SetUniform_PixelSize (const String& name, Uniform& uniform)
{
	Vector2f size ( mTarget ? mTarget->GetSize() : mSize );
	uniform.mType = Uniform::Type::Float2;
	uniform.mVal[0] = 1.0f / size.x;
	uniform.mVal[1] = 1.0f / size.y;
}

//============================================================================================================
// Shader callback function for R5_clipRange
//------------------------------------------------------------------------------------------------------------
// R5_clipRange.x = near
// R5_clipRange.y = far
// R5_clipRange.z = near * far
// R5_clipRange.w = far - near
//------------------------------------------------------------------------------------------------------------
// Formula used to calculate fragment's linear depth:
//------------------------------------------------------------------------------------------------------------
// (R5_clipRange.z / (R5_clipRange.y - gl_FragCoord.z * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;
//============================================================================================================

void GLGraphics::SetUniform_ClipRange (const String& name, Uniform& uniform)
{
	uniform = Quaternion(mClipRange.x,
						 mClipRange.y,
						 mClipRange.x * mClipRange.y,
						 mClipRange.y - mClipRange.x);
}

//============================================================================================================
// Shader callback for R5_projectionMatrix
//============================================================================================================

void GLGraphics::SetUniform_PM (const String& name, Uniform& uniform)
{
	uniform = GetProjectionMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseViewMatrix
//============================================================================================================

void GLGraphics::SetUniform_IVM (const String& name, Uniform& uniform)
{
	uniform = GetInverseModelViewMatrix();
}

//============================================================================================================
// Shader callback for R5_inverseProjMatrix
//============================================================================================================

void GLGraphics::SetUniform_IPM (const String& name, Uniform& uniform)
{
	uniform = GetInverseProjMatrix();
}

//============================================================================================================
// Shader callback function for R5_inverseViewRotationMatrix
//============================================================================================================

void GLGraphics::SetUniform_IVRM (const String& name, Uniform& uniform)
{
	const Matrix43& mv = GetModelViewMatrix();
	uniform.mType = Uniform::Type::Float9;
	uniform.mVal[0] = mv[0];
	uniform.mVal[1] = mv[4];
	uniform.mVal[2] = mv[8];
	uniform.mVal[3] = mv[1];
	uniform.mVal[4] = mv[5];
	uniform.mVal[5] = mv[9];
	uniform.mVal[6] = mv[2];
	uniform.mVal[7] = mv[6];
	uniform.mVal[8] = mv[10];
}

//============================================================================================================
// Shader callback function for R5_worldTransformMatrix
//============================================================================================================

void GLGraphics::SetUniform_WTM (const String& name, Uniform& uniform)
{
	uniform = GetModelMatrix();
}

//============================================================================================================
// Shader callback function for R5_worldRotationMatrix
//============================================================================================================

void GLGraphics::SetUniform_WRM (const String& name, Uniform& uniform)
{
	const Matrix43& model = GetModelMatrix();
	uniform.mType = Uniform::Type::Float9;
	uniform.mVal[0] = model[0];
	uniform.mVal[1] = model[1];
	uniform.mVal[2] = model[2];
	uniform.mVal[3] = model[4];
	uniform.mVal[4] = model[5];
	uniform.mVal[5] = model[6];
	uniform.mVal[6] = model[8];
	uniform.mVal[7] = model[9];
	uniform.mVal[8] = model[10];
}

//============================================================================================================
// Returns whether the specified point would be visible if rendered
//============================================================================================================

bool GLGraphics::IsPointVisible (const Vector3f& v)
{
	// Reset the ModelView matrix as the query is affected by it
	ResetModelViewMatrix();

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
// Converts screen coordinates to world coordinates
//------------------------------------------------------------------------------------------------------------
// WARNING: glReadPixels operation is very slow on some graphics cards (Intel X3100) when used with an FBO!
//============================================================================================================

Vector3f GLGraphics::ConvertTo3D (const Vector2i& v)
{
	Vector2i size ( mTarget ? mTarget->GetSize() : mSize );
	Vector2i pos  ( v.x, mSize.y - v.y );

	float depth (0.0f);
	if (pos.x >= 0 && pos.y < size.x && pos.y >= 0 && pos.y < size.y)
		glReadPixels(pos.x, pos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	// To calculate the linearly scaled value between near and far plane:
	//float range = mFarPlane - mNearPlane;
    //float val = (mFarPlane * mNearPlane) / (mFarPlane - depth * range);

	// To convert the above value to 0-1 range:
	//val = (val - mNearPlane) / range;
    //val = Float::Clamp(val, 0.0f, 1.0f);

	return GetInverseMVPMatrix().Unproject( Vector2f(pos) / size, depth);
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

bool GLGraphics::Init()
{
	if ( mThread == 0 && InitOpenGL(2.0f) )
	{
		mThread = Thread::GetID();

		// Set the default states
		SetDepthWrite	(true);
		SetDepthTest	(true);
		SetAlphaTest	(true);
		SetADT			(0.003921568627451f);
		SetBlending		(true);
		SetFog			(true);
		SetBlending		(Blending::Normal);
		SetCulling		(Culling::Back);
		SetFogRange		( Vector2f(30.0f, 75.0f) );
		SetCameraRange	( Vector3f(0.3f, 100.0f, 90.0f) );

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
			entry.callback(entry.param);
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

void GLGraphics::AddDelayedDelegate(DelayedDelegate callback, void* param)
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
			SetActiveProjection(IGraphics::Projection::Perspective);
			Draw(Drawable::Skybox);
		}
	}
	else
	{
		if (color)
		{
			flag |= GL_COLOR_BUFFER_BIT;

			SetColorWrite(true);

			if ( mTarget )
			{
				const Color4f& color ( mTarget->GetBackgroundColor() );
				glClearColor(color.r, color.g, color.b, color.a);
			}
			else
			{
				glClearColor(mBackground.r, mBackground.g, mBackground.b, mBackground.a);
			}
		}

		if (depth)
		{
			flag |= GL_DEPTH_BUFFER_BIT;
			SetDepthWrite(true);
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
				entry.callback(entry.param);
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
	// Nothing for now
}

//============================================================================================================
// Draws the active skybox cube
//============================================================================================================

uint GLGraphics::Draw (uint drawable)
{
	uint result (0);

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
		SetBlending(Blending::Normal);

		SetActiveMaterial(mSkybox);
		SetSimpleMaterial(true);
		SetActiveShader(0);
		glColor3ub(255, 255, 255);

		// Overwrite the ModelView matrix, changing it to use the current camera's position as origin
		Matrix43 view = GetViewMatrix();
		view.PreTranslate(mEye);
		SetModelViewMatrix(view);

		// Set all active vertex attributes
		SetActiveVertexAttribute( Attribute::Normal,	 0, 0, 0, 0, 0 );
		SetActiveVertexAttribute( Attribute::Color,		 0, 0, 0, 0, 0 );
		SetActiveVertexAttribute( Attribute::Tangent,	 0, 0, 0, 0, 0 );
		SetActiveVertexAttribute( Attribute::BoneIndex,  0, 0, 0, 0, 0 );
		SetActiveVertexAttribute( Attribute::BoneWeight, 0, 0, 0, 0, 0 );
		SetActiveVertexAttribute( Attribute::TexCoord1,	 0, 0, 0, 0, 0 );
		SetActiveVertexAttribute( Attribute::TexCoord0, mSkyboxVBO, 0, DataType::Float, 3, sizeof(Vector3f) );
		SetActiveVertexAttribute( Attribute::Position,	mSkyboxVBO, 0, DataType::Float, 3, sizeof(Vector3f) );

		// Draw the skybox
		result = DrawIndices( mSkyboxIBO, Primitive::Triangle, 36 );

		// Reset the ModelView matrix back to its default values
		ResetModelViewMatrix();
		return 12;
	}
	else if (drawable == Drawable::FullscreenQuad)
	{
		Vector2i size ( mTarget ? mTarget->GetSize() : mSize );

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
		SetBlending(Blending::Normal);
		SetActiveMaterial((const IMaterial*)0);

		ResetModelViewMatrix();
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
		SetBlending(Blending::Normal);
		SetActiveMaterial((const IMaterial*)0);

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

				if (name == "Opaque")
				{
					// Solid material group -- no alpha testing or blending needed
					tech->SetAlphaTest(false);
					tech->SetBlending(ITechnique::Blending::None);
				}
				else if (name == "Transparent")
				{
					// Group for objects with transparency, drawn after solids -- default everything
					tech->SetSorting(ITechnique::Sorting::BackToFront);
				}
				else if (name == "Glow")
				{
					// Glow is drawn with no depth writing, using additive blending and no lighting
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetAlphaTest(false);
					tech->SetBlending(ITechnique::Blending::Add);
					tech->SetLighting(ITechnique::Lighting::None);
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
				}
				else if (name == "Particle")
				{
					tech->SetDepthWrite(false);
					tech->SetLighting(ITechnique::Lighting::None);
					tech->SetSorting(ITechnique::Sorting::BackToFront);
				}
				else if (name == "Opaque Depth")
				{
					tech->SetFog(false);
					tech->SetColorWrite(false);
					tech->SetAlphaTest(false);
					tech->SetBlending(ITechnique::Blending::None);
					tech->SetLighting(ITechnique::Lighting::None);
				}
				else if (name == "Transparent Depth")
				{
					tech->SetFog(false);
					tech->SetColorWrite(false);
					tech->SetBlending(ITechnique::Blending::None);
					tech->SetLighting(ITechnique::Lighting::None);
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
					tech->SetFog(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
				}
				else if (name == "Post-Deferred")
				{
					tech->SetFog(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
				}
				else if (name == "Wireframe")
				{
					tech->SetFog(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
					tech->SetWireframe(true);
				}
				else if (name == "Decal")
				{
					tech->SetFog(false);
					tech->SetDepthWrite(false);
					tech->SetLighting(IGraphics::Lighting::None);
					tech->SetBlending(IGraphics::Blending::None);
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
	IShader* shader = 0;

	if (name.IsValid())
	{
		mShaders.Lock();
		{
			for (uint i = 0; i < mShaders.GetSize(); ++i)
			{
				shader = mShaders[i];

				if (shader != 0 && shader->GetName() == name)
				{
					mShaders.Unlock();
					return shader;
				}
			}

			if (createIfMissing)
			{
				shader = new GLShader(name);
				shader->RegisterUniform( "R5_time",						 &SetUniform_Time );
				shader->RegisterUniform( "R5_worldEyePosition",			 bind(&GLGraphics::SetUniform_EyePos,		this) );
				shader->RegisterUniform( "R5_pixelSize",				 bind(&GLGraphics::SetUniform_PixelSize,	this) );
				shader->RegisterUniform( "R5_clipRange",				 bind(&GLGraphics::SetUniform_ClipRange,	this) );
				shader->RegisterUniform( "R5_projectionMatrix",			 bind(&GLGraphics::SetUniform_PM,			this) );
				shader->RegisterUniform( "R5_inverseViewMatrix",		 bind(&GLGraphics::SetUniform_IVM,			this) );
				shader->RegisterUniform( "R5_inverseProjMatrix",		 bind(&GLGraphics::SetUniform_IPM,			this) );
				shader->RegisterUniform( "R5_inverseViewRotationMatrix", bind(&GLGraphics::SetUniform_IVRM,			this) );
				shader->RegisterUniform( "R5_worldTransformMatrix",		 bind(&GLGraphics::SetUniform_WTM,			this) );
				shader->RegisterUniform( "R5_worldRotationMatrix",		 bind(&GLGraphics::SetUniform_WRM,			this) );
				mShaders.Expand() = shader;
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
// Deletes a vertex buffer object if it can be found in the managed array
//============================================================================================================

void GLGraphics::DeleteVBO (const IVBO* ptr)
{
	if (ptr != 0)
	{
		mVbos.Lock();
		{
			for (uint i = 0; i < mVbos.GetSize(); ++i)
			{
				if ( mVbos[i] == ptr )
				{
					delete mVbos[i];
					mVbos[i] = 0;
					break;
				}
			}
		}
		mVbos.Unlock();
	}
}

//============================================================================================================
// Deletes a frame buffer object if it can be found in the managed array
//============================================================================================================

void GLGraphics::DeleteRenderTarget (const IRenderTarget* ptr)
{
	mFbos.Lock();
	{
		for (uint i = 0; i < mFbos.GetSize(); ++i)
		{
			if ( mFbos[i] == ptr )
			{
				delete mFbos[i];
				mFbos[i] = 0;
				break;
			}
		}
	}
	mFbos.Unlock();
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

#define EXECUTE(type, fnc)														\
	{																			\
		type* p = fnc(value.IsString() ? value.AsString() : value.GetString());	\
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
		else if (tag == "Skybox") skybox = (value.IsString() ? value.AsString() : value.GetString());
		else if	(tag == ITechnique::ClassID())	EXECUTE(ITechnique,	GetTechnique)
		else if (tag == ITexture::ClassID())	EXECUTE(ITexture,	GetTexture)
		else if (tag == IMaterial::ClassID())	EXECUTE(IMaterial,	GetMaterial)
		else if (tag == IFont::ClassID())		EXECUTE(IFont,		GetFont)
		else if (tag == IShader::ClassID())		EXECUTE(IShader,	GetShader)
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
	node.AddChild("Background Color", mBackground);
	node.AddChild("Fog Range", mFogRange);

	if ( mSkybox && mSkybox->GetType() == ITexture::Type::EnvironmentCubeMap )
		node.AddChild("Skybox", mSkybox->GetName());

	SAVE(mTechs);
	SAVE(mTextures);
	SAVE(mMaterials);
	SAVE(mFonts);
	SAVE(mShaders);

	return true;
}