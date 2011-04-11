#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Retrieves an OpenGL draw mode based on the specified primitive type
//============================================================================================================

inline void GetGLPrimitive (uint	primitive,
							uint	vertexCount,
							uint&	glPrimitive,
							uint&	triangleCount)
{
	switch (primitive)
	{
	case IGraphicsManager::Primitive::Triangle:
		glPrimitive = GL_TRIANGLES;
		triangleCount = vertexCount / 3;
		break;

	case IGraphicsManager::Primitive::TriangleStrip:
		glPrimitive = GL_TRIANGLE_STRIP;
		triangleCount = vertexCount > 1 ? vertexCount - 2 : 0;
		break;

	case IGraphicsManager::Primitive::Quad:
		glPrimitive = GL_QUADS;
		triangleCount = vertexCount / 2;
		break;

	case IGraphicsManager::Primitive::QuadStrip:
		glPrimitive = GL_QUAD_STRIP;
		triangleCount = vertexCount > 1 ? vertexCount - 2 : 0;
		break;

	case IGraphicsManager::Primitive::TriangleFan:
		glPrimitive = GL_TRIANGLE_FAN;
		triangleCount = vertexCount > 1 ? vertexCount - 2 : 0;
		break;

	case IGraphicsManager::Primitive::Line:
		glPrimitive = GL_LINES;
		triangleCount = vertexCount / 2;
		break;

	case IGraphicsManager::Primitive::LineStrip:
		glPrimitive = GL_LINE_STRIP;
		triangleCount = vertexCount > 0 ? vertexCount - 1 : 0;
		break;

	case IGraphicsManager::Primitive::Point:
		glPrimitive = GL_POINTS;
		triangleCount = vertexCount;
		break;

	default:
		glPrimitive = 0;
		triangleCount = 0;
		break;
	}
}

//============================================================================================================
// Helper conversion arrays
//============================================================================================================

uint ConvertCondition[] =
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS,
};

//------------------------------------------------------------------------------------------------------------

uint ConvertOperation[] =
{
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_DECR,
	GL_INVERT,
};

//============================================================================================================
// GLGraphics GLController constructor
//============================================================================================================

GLController::GLController() :
	mFog			(false),
	mDepthWrite		(false),
	mDepthTest		(false),
	mColorWrite		(true),
	mAlphaTest		(false),
	mStencilTest	(false),
	mScissorTest	(false),
	mWireframe		(false),
	mLighting		(Lighting::None),
	mBlending		(Blending::None),
	mCulling		(Culling::None),
	mAdt			(0.0f),
	mThickness		(1.0f),
	mNormalize		(false),
	mAf				(0),
	mSimpleMaterial	(false),
	mTarget			(0),
	mTechnique		(0),
	mMaterial		(0),
	mShader			(0),
	mSkybox			(0),
	mShadowmap		(0),
	mActiveTU		(0),
	mNextTU			(0) {}

//============================================================================================================
// Changes the currently active texture unit
//============================================================================================================

inline uint GLController::_CountImageUnits()
{
	if (mTu.IsEmpty())
	{
		uint maxIU = glGetInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
		ASSERT( maxIU > 0, "Could not retrieve the maximum number of texture units" );
		mTu.ExpandTo(maxIU);
		mNextTex.ExpandTo(maxIU);
		mNextTex.MemsetZero();
	}
	return mTu.GetSize();
}

//============================================================================================================
// Finish all draw operations
//============================================================================================================

void GLController::Flush()
{
	_BindAllTextures();
	_ActivateMatrices();
	glFlush();
}

//============================================================================================================
// Sets the fog on/off
//============================================================================================================

void GLController::SetFog (bool val)
{
	if (mFogRange.x == mFogRange.y) val = false;

	if (mFog != val)
	{
		mFog = val;
		mTechnique = 0;

		if (mFog)
		{
			glEnable(GL_FOG);
			SetFogRange(mFogRange);
		}
		else
		{
			const Vector3f& range = GetCameraRange();
			glFogf(GL_FOG_START, range.y);
			glFogf(GL_FOG_END, range.y);
			glDisable(GL_FOG);
		}
	}
}

//============================================================================================================
// Sets writing to Z-buffer
//============================================================================================================

void GLController::SetDepthWrite (bool val)
{
	if (mDepthWrite != val)
	{
		if (val)
		{
			val = (mTarget == 0 || mTarget->HasDepth());
			if (mDepthWrite == val) return;
		}

		mTechnique = 0;
		glDepthMask( (mDepthWrite = val) ? 1 : 0 );
	}
}

//============================================================================================================
// Enables or disables the depth testing
//============================================================================================================

void GLController::SetDepthTest (bool val)
{
	if ( mDepthTest != val )
	{
		mTechnique = 0;
		if (mDepthTest = val) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
	}
}

//============================================================================================================
// Toggles writing of color information
//============================================================================================================

void GLController::SetColorWrite (bool val)
{
	if ( mColorWrite != val )
	{
		if (val)
		{
			val = (mTarget == 0 || mTarget->HasColor());
			if (mColorWrite == val) return;
		}

		mTechnique = 0;
		if (mColorWrite = val) glColorMask(true, true, true, true);
		else glColorMask(false, false, false, false);
	}
}

//============================================================================================================
// Toggles the alpha testing
//============================================================================================================

void GLController::SetAlphaTest (bool val)
{
	if ( mAlphaTest != val )
	{
		mTechnique = 0;
		if (mAlphaTest = val) glEnable(GL_ALPHA_TEST);
		else glDisable(GL_ALPHA_TEST);
	}
}

//============================================================================================================
// Toggles the stencil test
//============================================================================================================

void GLController::SetStencilTest (bool val)
{
	if ( mStencilTest != val )
	{
		if (mStencilTest = val) glEnable(GL_STENCIL_TEST);
		else glDisable(GL_STENCIL_TEST);
	}
}

//============================================================================================================
// Toggles the scissor test
//============================================================================================================

void GLController::SetScissorTest (bool val)
{
	if ( mScissorTest != val )
	{
		if (mScissorTest = val) glEnable(GL_SCISSOR_TEST);
		else glDisable(GL_SCISSOR_TEST);
	}
}

//============================================================================================================
// Whether the geometry is rendered in wireframe
//============================================================================================================

void GLController::SetWireframe (bool  val)
{
	if (mWireframe != val)
	{
		mTechnique = 0;
		if (mWireframe = val)	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

//============================================================================================================
// Toggles lighting on/off
//============================================================================================================

void GLController::SetLighting (uint val)
{
	if (mLighting != val)
	{
		mTechnique = 0;

		if (val == Lighting::None)
		{
			// Turn off lighting
			glDisable(GL_LIGHTING);
		}
		else
		{
			if (val == Lighting::OneSided)
			{
				// Enable normal one-sided lighting
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
				glDisable(GL_VERTEX_PROGRAM_TWO_SIDE);
			}
			else
			{
				// Enable two-sided lighting
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
				glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
			}

			if (mLighting == Lighting::None)
			{
				// If lighting was off, turn it on
				glEnable(GL_LIGHTING);
			}
		}

		// Update the local state
		mLighting = val;
	}
}

//============================================================================================================
// Toggles blending on/off
//============================================================================================================

void GLController::SetBlending (uint val)
{
	if (mBlending != val)
	{
		mTechnique = 0;
		mBlending = val;

		if (mBlending == Blending::None)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			if		(mBlending == Blending::Replace)		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			else if (mBlending == Blending::Add)		glBlendFunc(GL_ONE, GL_ONE);
			else if (mBlending == Blending::Subtract)	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
			else if (mBlending == Blending::Modulate)	glBlendFunc(GL_DST_COLOR, GL_ZERO);

			glEnable(GL_BLEND);
		}
	}
}

//============================================================================================================
// Changes the culling -- front-faced, back-faced, or no culling
//============================================================================================================

void GLController::SetCulling (uint val)
{
	if (mCulling != val)
	{
		mTechnique = 0;
		mCulling = val;

		if (mCulling == Culling::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glCullFace( (mCulling == Culling::Front) ? GL_FRONT : GL_BACK );
			glEnable(GL_CULL_FACE);
		}
	}
}

//============================================================================================================
// Alpha testing will discard fragments with alpha less than the specified value
//============================================================================================================

void GLController::SetAlphaCutoff (float val)
{
	if (mAlphaTest && Float::IsNotEqual(mAdt, val))
	{
		mMaterial = (const IMaterial*)(-1);
		glAlphaFunc(GL_GREATER, mAdt = val);
	}
}

//============================================================================================================
// Sets the point/line thickness
//============================================================================================================

void GLController::SetThickness (float val)
{
	val = Float::Clamp(val, 1.0f, 10.0f);

	if (mThickness != val)
	{
		mThickness = val;

		glPointSize(val);
		glLineWidth(val);

		if (val == 1.0f)
		{
			glDisable(GL_POINT_SMOOTH);
			glDisable(GL_LINE_SMOOTH);
		}
		else
		{
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_LINE_SMOOTH);
		}
	}
}

//============================================================================================================
// Whether to automatically normalize normals
//============================================================================================================

void GLController::SetNormalize (bool val)
{
	if ( mNormalize != val )
	{
		if (mNormalize = val) glEnable(GL_RESCALE_NORMAL);
		else glDisable(GL_RESCALE_NORMAL);
	}
}

//============================================================================================================
// Changes the screen dimensions
//============================================================================================================

void GLController::SetViewport (const Vector2i& size)
{
	if (mSize != size)
	{
		mSize = size;
		if (mTarget == 0) mTrans.SetTargetSize(mSize);
	}
}

//============================================================================================================
// Changes the scissor rectangle
//============================================================================================================

void GLController::SetScissorRect (const Rect& rect)
{
	if (mScissorRect != rect)
	{
		mScissorRect = rect;
		const Vector2i& size = GetActiveViewport();
		glScissor((GLint)rect.left, (GLint)size.y - (GLint)rect.bottom,
			(GLsizei)rect.GetWidth(), (GLsizei)rect.GetHeight());
	}
}

//============================================================================================================
// Sets the fog starting and ending distance
//============================================================================================================

void GLController::SetFogRange (const Vector2f& range)
{
	mFogRange = range;

	if (mFogRange.x > 1.0001f || mFogRange.y > 1.0001f)
	{
		glFogf(GL_FOG_START, mFogRange.x);
		glFogf(GL_FOG_END, mFogRange.y);
	}
	else
	{
		const Vector3f& r = GetCameraRange();
		float dist = r.y - r.x;
		glFogf(GL_FOG_START, r.x + mFogRange.x * dist);
		glFogf(GL_FOG_END, r.x + mFogRange.y * dist);
	}
}

//============================================================================================================
// Changes the clear and the fog color in one call
//============================================================================================================

void GLController::SetBackgroundColor (const Color4f& color)
{
	mBackground = color;
	glFogfv(GL_FOG_COLOR, mBackground);
}

//============================================================================================================
// Whether current active color defines the material colors
//============================================================================================================

inline void GLController::SetSimpleMaterial (bool val)
{
	if ( mSimpleMaterial != val )
	{
		if (mSimpleMaterial = val) glEnable(GL_COLOR_MATERIAL);
		else glDisable(GL_COLOR_MATERIAL);
	}
}

//============================================================================================================
// GPU device information
//============================================================================================================

const IGraphics::DeviceInfo& GLController::GetDeviceInfo() const
{
	return g_caps;
}

//============================================================================================================
// Convenience function -- sets the view matrix
//============================================================================================================

void GLController::SetCameraOrientation (const Vector3f& eye, const Vector3f& dir, const Vector3f& up)
{
	mTrans.CancelModelMatrixOverride();
	mTrans.CancelViewMatrixOverride();
	mTrans.SetViewMatrix(eye, dir, up);
}

//============================================================================================================
// Convenience function -- sets the projection matrix
//============================================================================================================

void GLController::SetCameraRange (const Vector3f& range)
{
	mTrans.CancelProjectionMatrixOverride();
	mTrans.SetProjectionRange(range);
}

//============================================================================================================
// Sets the active camera properties
//============================================================================================================

void GLController::SetActiveRenderTarget (const IRenderTarget* tar)
{
	if (mTarget != tar)
	{
		// Ensure that all textures are bound prior to switching render targets
		_BindAllTextures();

		if (tar != 0)
		{
			// If the target has been specified, we'll be rendering to an off-screen buffer
			tar->Activate();
			mTrans.SetTargetSize(tar->GetSize());
		}
		// Should be rendering to screen, but a target is currently active
		else if (mTarget != 0)
		{
			// Deactivate the off-screen target
			mTarget->Deactivate();
			mTrans.SetTargetSize(mSize);
		}
		CHECK_GL_ERROR;

		// Update the target
		mTarget = tar;
	}
}

//============================================================================================================
// Activates the specified render group
//============================================================================================================

void GLController::SetActiveTechnique (const ITechnique* ptr, bool insideOut)
{
	if (ptr != 0)
	{
		byte culling = ptr->GetCulling();

		if (insideOut)
		{
			// If inside-out mode was requested, flip culling
			if		(culling == Culling::Front)	culling = Culling::Back;
			else if (culling == Culling::Back)	culling = Culling::Front;
		}

		if (mTechnique == ptr)
		{
			// Same group as last time, but culling might be inverted
			if (mCulling != culling)
			{
				SetCulling(culling);
			}

		}
		// If the technique is changing
		else if (mTechnique != ptr)
		{
			// Activate all the new states
			SetFog			( ptr->GetFog()			);
			SetDepthWrite	( ptr->GetDepthWrite()	);
			SetDepthTest	( ptr->GetDepthTest()	);
			SetColorWrite	( ptr->GetColorWrite()	);
			SetAlphaTest	( ptr->GetAlphaTest()	);
			SetWireframe	( ptr->GetWireframe()	);
			SetLighting		( ptr->GetLighting()	);
			SetBlending		( ptr->GetBlending()	);
			SetCulling		( culling				);

			// Invalidate any active material
			mMaterial = (const IMaterial*)(-1);
			++mStats.mTechSwitches;
		}
	}
	mTechnique = ptr;
}

//============================================================================================================
// Activates the specified material
//============================================================================================================

bool GLController::SetActiveMaterial (const IMaterial* ptr)
{
	if (mMaterial != ptr)
	{
		static uint maxIU = _CountImageUnits();

		// If the material is invisible under the current technique, consider options to be invalid
		const IMaterial::DrawMethod* ren = ( (ptr != 0 && mTechnique != 0) ?
			ptr->GetVisibleMethod(mTechnique) : 0 );

		if (ren == 0)
		{
			for (uint i = maxIU; i > 0; )
				SetActiveTexture(--i, 0);

			SetAlphaCutoff(0.003921568627451f);
			SetActiveShader(0);
			SetSimpleMaterial(true);
			SetActiveColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
			CHECK_GL_ERROR;
			return false;
		}
		else
		{
			const Color4f diff (ptr->GetDiffuse());
			const float glow (ptr->GetGlow());

			Color4f spec, emis;

			if (ren->mShader != 0 && ren->mShader->GetFlag(IShader::Flag::Material))
			{
				// Shader with R5_MATERIAL tags used -- specular channel holds special values
				spec.r = ptr->GetSpecularity();
				spec.g = ptr->GetSpecularHue();
				spec.b = ptr->GetReflectiveness();
				spec.a = ptr->GetShininess();
				emis.g = ptr->GetOcclusion();
				emis.r = 1.0f - emis.g;
				emis.a = glow;
			}
			else
			{
				spec   = diff;
				spec  *= ptr->GetSpecularity();
				spec.a = ptr->GetShininess();

				emis   = diff;
				emis  *= glow;
				emis.a = glow;
			}

			// Shininess should be clamped between 1 and 128
			int shininess = Float::RoundToInt(ptr->GetShininess() * 128.0f);
			if		(shininess < 1)		shininess = 1;
			else if (shininess > 128)	shininess = 128;

			// Disable the simple material
			SetSimpleMaterial(false);

			// Set all material properties
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
			glMateriali (GL_FRONT_AND_BACK, GL_SHININESS, shininess);

			// Materials have their own AlphaCutoff
			SetAlphaCutoff( ptr->GetAlphaCutoff() * diff.a );
			CHECK_GL_ERROR;

			// Retrieve all textures used by the material
			uint lastTex = ren->mTextures.GetSize();
			if (lastTex > maxIU) lastTex = maxIU;

			// Activate the appropriate shader
			SetActiveShader(ren->mShader);

			// Activate all textures
			ren->mTextures.Lock();
			{
				bool bindShadow = (mShader != 0 && mShader->GetFlag(IShader::Flag::Shadowed));

				// Bind all of the material's textures
				for (uint i = maxIU; i > 0; )
				{
					const ITexture* tex = (--i < lastTex) ? ren->mTextures[i] : 0;

					// Automatically bind the shadowmap texture after the last texture
					if (i == lastTex && bindShadow)
					{
						if (mShadowmap == 0) mShadowmap = GetTexture("R5_Shadowmap");
						tex = mShadowmap;
					}

					// Bind the material's texture
					SetActiveTexture(i, tex);
				}
			}
			ren->mTextures.Unlock();
		}

		// Remember the currently active material
		mMaterial = ptr;
	}
	return true;
}

//============================================================================================================
// Activates this texture on Texture Unit 0, and disables all other texture units
//============================================================================================================

bool GLController::SetActiveMaterial (const ITexture* ptr)
{
	static uint count = _CountImageUnits();

	for (uint i = count; i > 1; )
		SetActiveTexture(--i, 0);

	SetActiveTexture(0, ptr);
	mMaterial = (const IMaterial*)(-1);
	return (ptr != 0 && ptr->IsValid());
}

//============================================================================================================
// Changes the currently active shader
//============================================================================================================

bool GLController::SetActiveShader (const IShader* ptr, bool forceUpdateUniforms)
{
	// "Which shader is currently active" is kept inside the Shader.cpp file,
	// so we don't check for inequality here.
	if (ptr != 0)
	{
		GLShader* shader = (GLShader*)ptr;

		for (;;)
		{
			// If this is a surface shader, we might need to activate a different shader
			if (mTechnique != 0 && shader->GetFlag(IShader::Flag::Surface))
			{
				const String& shaderName = shader->GetName();

				if (mTechnique->GetFlag(ITechnique::Flag::Deferred))
				{
					if (shader->mDeferred == 0)
					{
						// Remember this alternate version of the shader
						shader->mDeferred = (GLShader*)GetShader(shaderName + " [Deferred]");

						// Copy over registered uniforms
						FOREACH(i, shader->mUniforms)
						{
							GLShader::UniformEntry& ent = shader->mUniforms[i];
							shader->mDeferred->RegisterUniform(ent.mName, ent.mDelegate);
						}
					}
					shader = shader->mDeferred;
				}
				else if (mTechnique->GetFlag(ITechnique::Flag::Shadowed))
				{
					if (shader->mShadowed == 0)
					{
						shader->mShadowed = (GLShader*)GetShader(shaderName + " [Shadowed]");

						FOREACH(i, shader->mUniforms)
						{
							GLShader::UniformEntry& ent = shader->mUniforms[i];
							shader->mShadowed->RegisterUniform(ent.mName, ent.mDelegate);
						}
					}
					shader = shader->mShadowed;
				}

				// Activate the shader
				if (shader->Activate(false)) ++mStats.mShaderSwitches;
				break;
			}

			// Activate the shader
			if (shader->Activate(false)) ++mStats.mShaderSwitches;

			// If this is actually a surface shader, we might need to activate a different one
			if (mTechnique != 0 && shader->GetFlag(IShader::Flag::Surface)) continue;
			break;
		}

		// Remember the shader that we activated
		mMaterial = (const IMaterial*)(-1);
		mShader = shader;
		return true;
	}
	else if (mShader != 0)
	{
		mShader->Deactivate();
		mShader = 0;
		mMaterial = (const IMaterial*)(-1);
		return true;
	}
	return false;
}

//============================================================================================================
// Changes the active color
//============================================================================================================

void GLController::SetActiveColor (const Color& color)
{
	if (mColor != color)
	{
		mColor = color;
		glColor4fv(color.GetColor4f());
	}
}

//============================================================================================================
// Activates the vertex buffer object
//============================================================================================================

void GLController::SetActiveVBO (const IVBO* ptr, uint type)
{
	if (ptr != 0)
	{
		GLVBO::Activate( ptr->GetID(), ptr->GetType() );
	}
	else
	{
		GLVBO::Activate(0, type);
	}
}

//============================================================================================================
// Sets the active texture on the specified texture unit
//============================================================================================================

void GLController::SetActiveTexture (uint textureUnit, const ITexture* tex)
{
	static uint maxIU = _CountImageUnits();

	if (textureUnit < maxIU)
	{
		mNextTex[textureUnit] = (GLTexture*)tex;
		mMaterial = (const IMaterial*)(-1);
	}
}

//============================================================================================================
// Activates and/or changes light properties for the specified light
//============================================================================================================

void GLController::SetActiveLight (uint index, const ILight* ptr)
{
	CHECK_GL_ERROR;

	if (mLu.IsEmpty())
	{
		ASSERT( g_caps.mMaxLights > 0, "Could not retrieve the maximum number of lights" );
		mLu.ExpandTo( g_caps.mMaxLights );
		for (uint i = 0; i < mLu.GetSize(); ++i)
			mLu[i] = false;
	}

	if (index < mLu.GetSize())
	{
		bool& active (mLu[index]);
		index += GL_LIGHT0;

		if (ptr == 0)
		{
			if (active)
			{
				active = false;
				glDisable(index);
			}
		}
		else
		{
			// Activate the matrices as they will affect the lights
			_ActivateMatrices();

			// Activate the light
			if (!active)
			{
				active = true;
				glEnable(index);
			}

			if (ptr->mType == ILight::Type::Directional)
			{
				Vector3f dir (ptr->mDir);

				// Automatically adjust the light position to always be in view space
				if (mTrans.mIs2D) dir %= GetModelViewMatrix();

				// Set the light's position
				glLightfv(index, GL_POSITION, Quaternion(-dir.x, -dir.y, -dir.z, 0.0f));
			}
			else
			{
				Vector3f pos (ptr->mPos);

				// Automatically adjust the light position to always be in view space
				if (mTrans.mIs2D) pos *= GetModelViewMatrix();

				// Set the light's position
				glLightfv(index, GL_POSITION, Quaternion(pos.x, pos.y, pos.z, 1.0f));

				// Point lights are marked by having a cutoff of 180
				if (ptr->mType == ILight::Type::Point)
				{
					glLightf(index, GL_SPOT_CUTOFF, 180.0f);
				}
				//else
				//{
				//	glLightfv(index, GL_SPOT_DIRECTION, ptr->mDir);
				//	glLightf(index, GL_SPOT_EXPONENT, Float::Clamp(ptr->mSpot.x, 0.0f, 128.0f));
				//	glLightf(index, GL_SPOT_CUTOFF, Float::Clamp(ptr->mSpot.y, 0.0f, 90.0f));
				//}

				// Shaders perform completely different operations with the attenuation parameters
				// As such, point lights are not supported using the fixed-function pipeline.
				glLightf(index, GL_CONSTANT_ATTENUATION,	ptr->mAtten.x);
				glLightf(index, GL_LINEAR_ATTENUATION,		ptr->mAtten.y);
				glLightf(index, GL_QUADRATIC_ATTENUATION,	ptr->mAtten.z);
			}

			// Common light parameters
			glLightfv(index, GL_AMBIENT,  ptr->mAmbient);
			glLightfv(index, GL_DIFFUSE,  ptr->mDiffuse);
			glLightfv(index, GL_SPECULAR, ptr->mSpecular);

			++mStats.mLightSwitches;
		}
	}
	CHECK_GL_ERROR;
}

//============================================================================================================
// Sets the active depth buffer comparison function
//============================================================================================================

void GLController::SetActiveDepthFunction (uint condition)
{
#ifdef _DEBUG
	if (condition < 8) glDepthFunc(ConvertCondition[condition]);
	else ASSERT(false, "Invalid condition -- use the IGraphics::Condition values");
	CHECK_GL_ERROR;
#else
	glDepthFunc(ConvertCondition[condition]);
#endif
}

//============================================================================================================
// Set the active stencil buffer function
//============================================================================================================

void GLController::SetActiveStencilFunction (uint condition, uint val, uint mask)
{
#ifdef _DEBUG
	if (condition < 8) glStencilFunc(ConvertCondition[condition], val, mask);
	else ASSERT(false, "Invalid condition -- use the IGraphics::Condition values");
	CHECK_GL_ERROR;
#else
	glStencilFunc(ConvertCondition[condition], val, mask);
#endif
}

//============================================================================================================
// Set the active stencil buffer operation
//============================================================================================================

void GLController::SetActiveStencilOperation (uint testFail, uint depthFail, uint pass)
{
	if (testFail < 6 && depthFail < 6 && pass < 6)
	{
		glStencilOp(ConvertOperation[testFail], ConvertOperation[depthFail], ConvertOperation[pass]);
	}
#ifdef _DEBUG
	else ASSERT(false, "Invalid argument -- use the IGraphics::Operation values");
	CHECK_GL_ERROR;
#endif
}

//============================================================================================================
// Sets the currently active vertex buffer
//============================================================================================================
// Short note about this function: Attribute arrays can be used everywhere instead of Client State calls
// if the shaders are used. If they are not however, position must be specified via glVertexPointer.
// Attribute array calls also happen to require OpenGL 2.0, so for the sake of compatibility client state
// calls are used instead of attribute arrays whenever possible.
//============================================================================================================

void GLController::SetActiveVertexAttribute(
	uint		attribute,
	const IVBO*	vbo,
	const void*	ptr,
	uint		dataType,
	uint		elements,
	uint		stride )
{
	ASSERT(attribute < 16, "Invalid attribute");
	BufferEntry& buffer = mBuffers[attribute];

	bool changed = false;
	bool isEnabled = (ptr != 0 || (vbo != 0 && vbo->IsValid()));

	if (attribute == Attribute::Color)
	{
		if (isEnabled)
		{
			// Invalidate the local color -- we'll be using an array
			mColor = Color4f(-1.0f);
		}
		else
		{
			// Reset the color back to white
			SetActiveColor( Color4f(1.0f) );
		}
	}

	if (isEnabled)
	{
		if ( !buffer.mEnabled )
		{
			switch (attribute)
			{
				case Attribute::Position:	glEnableClientState(GL_VERTEX_ARRAY);			break;
				case Attribute::Normal:		glEnableClientState(GL_NORMAL_ARRAY);			break;
				case Attribute::TexCoord0:	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	break;
				case Attribute::Color:		glEnableClientState(GL_COLOR_ARRAY);			break;
				default:
				{
					if (glEnableVertexAttribArray != 0 && glVertexAttribPointer != 0)
					{
						glEnableVertexAttribArray(attribute);
						break;
					}
					return;
				}
			};

			changed = true;
		}
		else
		{
			changed = (buffer.mVbo != vbo) || (buffer.mPtr != ptr);
		}

		// Using color arrays means we need to disable complex material properties,
		// or the fixed-function pipeline will not be using the color array.
		if (attribute == Attribute::Color && mShader == 0) SetSimpleMaterial(true);

		if (changed)
		{
			buffer.mEnabled = true;
			SetActiveVBO( buffer.mVbo = vbo, IVBO::Type::Vertex );
			++mStats.mBufferBinds;

			switch (attribute)
			{
				case Attribute::Position:	glVertexPointer			(			 elements,	dataType,    stride, buffer.mPtr = ptr);	break;
				case Attribute::Normal:		glNormalPointer			(						dataType,    stride, buffer.mPtr = ptr);	break;
				case Attribute::Color:		glColorPointer			(			 elements,	dataType,    stride, buffer.mPtr = ptr);	break;
				case Attribute::TexCoord0:	glTexCoordPointer		(			 elements,	dataType,    stride, buffer.mPtr = ptr);	break;
				default:					glVertexAttribPointer	(attribute,  elements,	dataType, 0, stride, buffer.mPtr = ptr);	break;
			};
		}
		CHECK_GL_ERROR;
	}
	else if ( buffer.mEnabled )
	{
		buffer.mEnabled = false;

		switch (attribute)
		{
			case Attribute::Position:	glDisableClientState(GL_VERTEX_ARRAY);			break;
			case Attribute::Normal:		glDisableClientState(GL_NORMAL_ARRAY);			break;
			case Attribute::Color:		glDisableClientState(GL_COLOR_ARRAY);			break;
			case Attribute::TexCoord0:	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	break;
			default:					glDisableVertexAttribArray(attribute);			break;
		};
		CHECK_GL_ERROR;
	}
}

//============================================================================================================
// Draw the active buffers using an index buffer
//============================================================================================================

uint GLController::DrawVertices(uint primitive, uint vertexCount)
{
	uint glPrimitive(0), triangleCount(0);
	GetGLPrimitive(primitive, vertexCount, glPrimitive, triangleCount);

	if (triangleCount > 0)
	{
		// Activate the matrices
		_BindAllTextures();
		_ActivateMatrices();

		// Draw the arrays
		glDrawArrays( glPrimitive, 0, vertexCount );
		mStats.mTriangles += triangleCount;
		++mStats.mDrawCalls;
	}
	return triangleCount;
}

//============================================================================================================
// Draw using an index array
//============================================================================================================

uint GLController::_DrawIndices(const IVBO* vbo, const ushort* ptr, uint primitive, uint indexCount)
{
	uint glPrimitive(0), triangleCount(0);
	GetGLPrimitive(primitive, indexCount, glPrimitive, triangleCount);

	if (triangleCount > 0)
	{
		// Activate the VBO, if any
		SetActiveVBO( vbo, IVBO::Type::Index );

		// Activate the matrices
		_BindAllTextures();
		_ActivateMatrices();

		// Draw the indices
		glDrawElements( glPrimitive, indexCount, GL_UNSIGNED_SHORT, ptr );
		mStats.mTriangles += triangleCount;
		++mStats.mDrawCalls;
	}
	return triangleCount;
}

//============================================================================================================
// Updates the currently active texture unit
//============================================================================================================

void GLController::_ActivateTextureUnit()
{
	static uint maxTU = glGetInteger(GL_MAX_TEXTURE_UNITS);

	if (mActiveTU != mNextTU)
	{
		mActiveTU = mNextTU;
		glActiveTexture(GL_TEXTURE0 + mActiveTU);
		if (mActiveTU < maxTU) glActiveClientTexture(GL_TEXTURE0 + mActiveTU);
	}
}

//============================================================================================================
// Binds the specified texture -- mainly called from GLTexture::Activate()
//============================================================================================================

bool GLController::_BindTexture (uint glType, uint glID)
{
	// Ensure that the array is valid
	if (mTu.IsEmpty()) _CountImageUnits();

	// Currently active texture unit
	TextureUnit& tu = mTu[mNextTU];

	// If the texture ID is changing...
	if (tu.mId != glID)
	{
		uint oldType = (tu.mType == GL_TEXTURE_2D_MULTISAMPLE) ? GL_TEXTURE_2D : tu.mType;
		uint newType = (glType == GL_TEXTURE_2D_MULTISAMPLE) ? GL_TEXTURE_2D : glType;

		// If the type is changing, the old type needs to be disabled first (going from CUBE to 2D for example)
		if (tu.mType != 0 && tu.mType != glType)
		{
			// Unbind the texture
			_ActivateTextureUnit();
			glBindTexture(tu.mType, 0);
			CHECK_GL_ERROR;

			// glEnable / glDisable only works with base types
			if (oldType != newType)
			{
				glDisable(oldType);
				CHECK_GL_ERROR;
			}

			tu.mType = 0;
			tu.mId = 0;
		}

		if (glID != 0)
		{
			// Activate the texture unit
			_ActivateTextureUnit();

			// Enable the new type if it has changed
			if (oldType != newType)
			{
				glEnable(newType);
				CHECK_GL_ERROR;
			}

			// Bind the new texture
			glBindTexture(tu.mType = glType, tu.mId = glID);
			CHECK_GL_ERROR;

			++mStats.mTexSwitches;
		}
		else if (mNextTU < 8)
		{
			// We must also disable the appropriate texture coordinate buffer
			BufferEntry& buffer = mBuffers[Attribute::TexCoord0 + mNextTU];

			if (buffer.mEnabled)
			{
				// If the texture buffer is enabled, disable it
				SetActiveVertexAttribute(Attribute::TexCoord0 + mNextTU, 0, 0, 0, 0, 0);
			}
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Binds all activated textures
//============================================================================================================

void GLController::_BindAllTextures()
{
	// We always want to finish at texture 0, activating it last
	for (uint i = mNextTex.GetSize(); i > 0; )
	{
		mNextTU = --i;
		GLTexture* tex = mNextTex[mNextTU];
		if (tex != 0) tex->Activate();
		else _BindTexture(0, 0);
	}

	// Ensure that texture 0 is active
	if (mActiveTU != 0)
	{
		mActiveTU = 0;
		glActiveTexture(GL_TEXTURE0);
		glActiveClientTexture(GL_TEXTURE0);
	}
}