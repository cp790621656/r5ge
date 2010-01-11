#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Low-level graphics controller -- closest level of interaction with the renderer API
//============================================================================================================

class GLController : public IGraphics
{
	struct TextureUnit
	{
		uint mType;
		uint mId;

		TextureUnit() : mType(0), mId(0) {}
	};

	struct BufferEntry
	{
		const IVBO*		mVbo;
		const void*		mPtr;
		bool			mEnabled;

		BufferEntry() : mVbo(0), mPtr((void*)(-1)), mEnabled(false) {}
	};

	template <typename Matrix>
	struct MatrixEntry
	{
		Matrix	mMat;			// Actual matrix
		bool	mRecalculate;	// Whether the matrix needs to be recalculated
		bool	mOverwritten;	// Whether the matrix has been set to something other than the default value

		MatrixEntry() : mRecalculate(false), mOverwritten(false) {}
	};

	typedef MatrixEntry<Matrix43> Mat43;
	typedef MatrixEntry<Matrix44> Mat44;

protected:

	FrameStats	mStats;				// Current frame statistics

	bool		mFog;				// Whether fog is on
	bool		mDepthWrite;		// Whether writing to depth buffer happens
	bool		mDepthTest;			// Whether depth test is on
	bool		mColorWrite;		// Whether writing to the color buffer happens
	bool		mAlphaTest;			// Whether alpha testing is on
	bool		mStencilTest;		// Whether stencil test is on
	bool		mWireframe;			// Whether wireframe mode is currently on
	byte		mLighting;			// Whether lighting is on
	byte		mBlending;			// Active blending mode
	byte		mCulling;			// Active culling mode
	
	float		mAdt;				// Alpha testing will discard fragments with alpha less than this value
	float		mThickness;			// Point and line size when drawing those primitives
	bool		mNormalize;			// Whether to automatically normalize normals
	bool		mDepthOffset;		// Whether depth offset is currently active
	Vector2i	mSize;				// Viewport size
	Vector2f	mFogRange;			// Fog range
	Vector2f	mClipRange;			// Clipping range (near & far)
	float		mFov;				// Active field of view in degrees
	uint		mAf;				// Current anisotropy level
	Color4f		mBackground;		// Current window background color
	bool		mSimpleMaterial;	// Whether current active color defines the material colors
	int			mActiveLightCount;	// Number of activated lights

	uint		mProjection;		// Camera projection
	Vector3f	mEye;				// Camera's eye point
	Vector3f	mDir;				// Camera's 'LookAt' direction
	Vector3f	mUp;				// Camera's 'up' direction

	Mat43		mModel;				// World transformation matrix
	Mat43		mView;				// View matrix, changes with camera's position and rotation
	Mat44		mProj;				// Projection matrix, changes with the viewport dimensions, FOV, near/fear planes, and aspect ratio

	Mat43		mMV;				// World * View matrix
	Mat44		mMVP;				// Cached ModelView*Projection matrix
	Mat43		mIMV;				// Inverse ModelView matrix
	Mat44		mIP;				// Inverse Projection matrix
	Mat44		mIMVP;				// Inverse ModelView*Projection matrix

	bool		mResetView;			// Whether to reset the view matrix next time we draw
	bool		mResetProj;			// Whether to reset the projection matrix next time we draw

	Color		mMatDiff;			// Active diffuse and ambient color
	Color		mMatSpec;			// Active specular color
	float		mMatGlow;			// Active material glow (emission alpha value)

	const IRenderTarget*	mTarget;		// Active rendering target
	const ITechnique*		mTechnique;		// Active rendering technique
	const IMaterial*		mMaterial;		// Active material
	const IShader*			mShader;		// Active shader
	const ITexture*			mSkybox;		// Active skybox cubemap texture

	Array<TextureUnit>		mTu;			// Texture units
	Array<bool>				mLu;			// Light units
	uint					mActiveTU;		// Active texture unit
	BufferEntry				mBuffers[16];	// Active buffers

public:

	GLController();

protected:

	// Counts image units and resizes the mTu array
	uint _CountImageUnits();

public:

	// Finish all draw operations
	virtual void Flush();

	// State control functions
	virtual void SetFog				(bool val);
	virtual void SetDepthWrite		(bool val);
	virtual void SetDepthTest		(bool val);
	virtual void SetColorWrite		(bool val);
	virtual void SetAlphaTest		(bool val);
	virtual void SetStencilTest		(bool val);
	virtual void SetWireframe		(bool val);
	virtual void SetLighting		(uint val);
	virtual void SetBlending		(uint val);
	virtual void SetCulling			(uint val);
	virtual void SetADT				(float val);
	virtual void SetThickness		(float val);
	virtual void SetNormalize		(bool val);
	virtual void SetDepthOffset		(bool val);
	virtual void SetViewport		(const Vector2i& size);
	virtual void SetFogRange		(const Vector2f& range);
	virtual void SetBackgroundColor	(const Color4f& color);
	virtual void SetDefaultAF		(uint level) { GLTexture::SetDefaultAF(mAf = level); }

protected:

	// Whether active color sets the material color
	void SetSimpleMaterial	(bool val);

public:

	// Statistics about the current frame
	virtual const FrameStats&	GetFrameStats()			const	{ return mStats;		}
	virtual bool				GetFog()				const	{ return mFog;			}
	virtual bool				GetDepthWrite()			const	{ return mDepthWrite;	}
	virtual bool				GetDepthTest()			const	{ return mDepthTest;	}
	virtual bool				GetAlphaTest()			const	{ return mAlphaTest;	}
	virtual bool				GetStencilTest()		const	{ return mStencilTest;	}
	virtual bool				GetWireframe()			const	{ return mWireframe;	}
	virtual uint				GetLighting()			const	{ return mLighting;		}
	virtual uint				GetBlending()			const	{ return mBlending;		}
	virtual uint				GetCulling()			const	{ return mCulling;		}
	virtual float				GetADT()				const	{ return mAdt;			}
	virtual float				GetThickness()			const	{ return mThickness;	}
	virtual bool				GetNormalize()			const	{ return mNormalize;	}
	virtual bool				GetDepthOffset()		const	{ return mDepthOffset;	}
	virtual uint				GetDefaultAF()			const	{ return mAf;			}
	virtual const Vector2i&		GetViewport()			const	{ return mSize;			}
	virtual const Vector2f&		GetFogRange()			const	{ return mFogRange;		}
	virtual const Color4f&		GetBackgroundColor()	const	{ return mBackground;	}
	virtual const ITexture*		GetActiveSkybox()		const	{ return mSkybox;		}
	virtual const ITechnique*	GetActiveTechnique()	const	{ return mTechnique;	}
	virtual const IShader*		GetActiveShader()		const	{ return mShader;		}
	virtual const Vector2i&		GetActiveViewport()		const	{ return mTarget == 0 ? mSize : mTarget->GetSize(); }

	// Camera orientation retrieval
	virtual const Vector3f&		GetCameraPosition()		const	{ return mEye;			}
	virtual const Vector3f&		GetCameraDirection()	const	{ return mDir;			}
	virtual const Vector3f&		GetCameraUpVector()		const	{ return mUp;			}
	virtual const Vector2f&		GetCameraRange()		const	{ return mClipRange;	}
	virtual float				GetCameraFOV()			const	{ return mFov;			}

	// Matrix retrieval
	virtual const Matrix43&		GetModelMatrix();
	virtual const Matrix43&		GetViewMatrix();
	virtual const Matrix44&		GetProjectionMatrix();

	virtual const Matrix43&		GetModelViewMatrix();
	virtual const Matrix44&		GetModelViewProjMatrix();
	virtual const Matrix43&		GetInverseModelViewMatrix();
	virtual const Matrix44&		GetInverseProjMatrix();
	virtual const Matrix44&		GetInverseMVPMatrix();

	// Model matrix manipulation -- resets the View matrix back to default
	virtual void SetModelMatrix (const Matrix43& mat);
	virtual void ResetModelMatrix();

	// ModelView matrix manipulation -- overrides the Model component
	virtual void SetModelViewMatrix (const Matrix43& mat);
	virtual void ResetModelViewMatrix();

	// Camera control functions. Range X = near, Y = far, Z = field of view, in degrees.
	virtual void SetCameraOrientation		( const Vector3f& eye, const Vector3f& dir, const Vector3f& up );
	virtual void SetCameraRange				( const Vector3f& range );

	// Active object control
	virtual void SetActiveRenderTarget		( const IRenderTarget* tar );
	virtual void SetActiveTechnique			( const ITechnique* ptr, bool insideOut = false );
	virtual bool SetActiveMaterial			( const IMaterial* ptr );
	virtual bool SetActiveMaterial			( const ITexture* ptr );
	virtual uint SetActiveShader			( const IShader* ptr, bool forceUpdateUniforms = false );
	virtual void SetActiveSkybox			( const ITexture* ptr ) { mSkybox = ptr; }
	virtual void SetActiveColor				( const Color& c );
	virtual void SetActiveProjection		( uint projection );
	virtual void SetActiveVBO				( const IVBO* vbo, uint type = IVBO::Type::Invalid );
	virtual void SetActiveTexture			( uint textureUnit, const ITexture* ptr );
	virtual void SetActiveLight				( uint index, const Light* ptr );
	virtual void SetActiveDepthFunction		( uint condition );
	virtual void SetActiveStencilFunction	( uint condition, uint val, uint mask );
	virtual void SetActiveStencilOperation	( uint testFail, uint depthFail, uint pass );
	virtual void SetActiveVertexAttribute	( uint			attribute,		// Attribute::Vertex, Attribute::Normal, etc
											  const IVBO*	vbo,			// VBO, 0 if none
											  const void*	ptr,			// Pointer to the data
											  uint			dataType,		// Data type, such as "Type::Float" for Vector3f
											  uint			elements,		// Number of data type elements, ie "3" for Vector3f
											  uint			stride );		// Size of each vertex entry in bytes

	// Draw bound vertices
	virtual uint DrawVertices	( uint primitive, uint vertexCount );
	virtual uint DrawIndices	( const IVBO* vbo, uint primitive, uint indexCount )		{ return _DrawIndices(vbo, 0, primitive, indexCount); }
	virtual uint DrawIndices	( const ushort* indices, uint primitive, uint indexCount )	{ return _DrawIndices(0, indices, primitive, indexCount); }

protected:

	// Allow GLTexture class to access _BindTexture()
	friend class GLTexture;

	// Draw using an index array
	uint _DrawIndices ( const IVBO* vbo, const ushort* ptr, uint primitive, uint indexCount );

	// Activate all appropriate matrices
	void _ActivateMatrices();

	// Changes the currently active texture unit
	bool _SetActiveTextureUnit (uint textureUnit);

	// Binds the specified texture
	bool _BindTexture (uint glType, uint glID);

	// Marks all view-affected matrices as needing to be recalculated
	void _ViewHasChanged();

	// Marks all projection-affected matrices as needing to be recalculated
	void _ProjHasChanged();
};