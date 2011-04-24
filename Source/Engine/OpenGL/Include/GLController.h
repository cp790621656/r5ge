#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Low-level graphics controller -- closest level of interaction with the renderer API
// Author: Michael Lyashenko
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

protected:

	FrameStats	mStats;				// Current frame statistics
	GLTransform mTrans;				// Matrix transforms

	bool		mFog;				// Whether fog is on
	bool		mDepthWrite;		// Whether writing to depth buffer happens
	bool		mDepthTest;			// Whether the depth test is on
	bool		mColorWrite;		// Whether writing to the color buffer happens
	bool		mAlphaTest;			// Whether the alpha testing is on
	bool		mStencilTest;		// Whether the stencil test is on
	bool		mScissorTest;		// Whether the scissor test is on
	bool		mWireframe;			// Whether the wireframe mode is currently on
	bool		mMatIsDirty;		// Whether some parts of the material were altered since it was set
	byte		mLighting;			// Whether the lighting is on
	byte		mBlending;			// Active blending mode
	byte		mCulling;			// Active culling mode
	
	float		mAdt;				// Alpha testing will discard fragments with alpha less than this value
	float		mThickness;			// Point and line size when drawing those primitives
	Vector2i	mSize;				// Screen size
	Rect		mScissorRect;		// Scissor rectangle
	Vector2f	mFogRange;			// Fog range
	uint		mAf;				// Current anisotropy level
	Color4f		mBackground;		// Current window background color
	Color		mColor;				// Current active vertex color

	const IRenderTarget*	mTarget;		// Active rendering target
	const ITechnique*		mTechnique;		// Active rendering technique
	const IMaterial*		mMaterial;		// Active material
	const GLSurfaceShader*	mShader;		// Active shader
	const ITexture*			mSkybox;		// Active skybox cubemap texture
	const ITexture*			mShadowmap;		// Shadowmap texture, cached once it's retrieved

	Array<TextureUnit>		mTu;			// Texture units
	Array<GLTexture*>		mNextTex;		// Textures that will be activated prior to next draw call
	mutable Array<ILight>	mLu;			// Light units
	uint					mActiveTU;		// Active texture unit
	uint					mNextTU;		// Next texture unit that will be activated on texture bind
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
	virtual void SetScissorTest		(bool val);
	virtual void SetWireframe		(bool val);
	virtual void SetLighting		(uint val);
	virtual void SetBlending		(uint val);
	virtual void SetCulling			(uint val);
	virtual void SetAlphaCutoff		(float val);
	virtual void SetThickness		(float val);
	virtual void SetDepthOffset		(uint val) { mTrans.SetDepthOffset(val); }
	virtual void SetViewport		(const Vector2i& size);
	virtual void SetScissorRect		(const Rect& rect);
	virtual void SetFogRange		(const Vector2f& range);
	virtual void SetBackgroundColor	(const Color4f& color);
	virtual void SetDefaultAF		(uint level) { GLTexture::SetDefaultAF(mAf = level); }

public:

	// Statistics about the current frame
	virtual const DeviceInfo&	GetDeviceInfo()			const;
	virtual const FrameStats&	GetFrameStats()			const	{ return mStats;		}
	virtual bool				GetFog()				const	{ return mFog;			}
	virtual bool				GetDepthWrite()			const	{ return mDepthWrite;	}
	virtual bool				GetDepthTest()			const	{ return mDepthTest;	}
	virtual bool				GetAlphaTest()			const	{ return mAlphaTest;	}
	virtual bool				GetStencilTest()		const	{ return mStencilTest;	}
	virtual bool				GetScissorTest()		const	{ return mScissorTest;	}
	virtual bool				GetWireframe()			const	{ return mWireframe;	}
	virtual uint				GetLighting()			const	{ return mLighting;		}
	virtual uint				GetBlending()			const	{ return mBlending;		}
	virtual uint				GetCulling()			const	{ return mCulling;		}
	virtual float				GetAlphaCutoff()		const	{ return mAdt;			}
	virtual float				GetThickness()			const	{ return mThickness;	}
	virtual uint				GetDepthOffset()		const	{ return mTrans.GetDepthOffset();	}
	virtual uint				GetDefaultAF()			const	{ return mAf;			}
	virtual const Vector2i&		GetViewport()			const	{ return mTarget == 0 ? mSize : mTarget->GetSize();	}
	virtual const Rect&			GetScissorRect()		const	{ return mScissorRect;	}
	virtual const Vector2f&		GetFogRange()			const	{ return mFogRange;		}
	virtual const Color4f&		GetBackgroundColor()	const	{ return mBackground;	}

	virtual const ITexture*		GetActiveSkybox()		const	{ return mSkybox;		}
	virtual const ITechnique*	GetActiveTechnique()	const	{ return mTechnique;	}
	virtual const IMaterial*	GetActiveMaterial()		const	{ return mMaterial;		}
	virtual const IShader*		GetActiveShader()		const	{ return mShader;		}
	virtual const Vector2i&		GetActiveViewport()		const	{ return (mTarget == 0) ? mSize : mTarget->GetSize(); }
	virtual const IRenderTarget* GetActiveRenderTarget() const	{ return mTarget; }

	// Access to lights
	virtual const ILight& GetActiveLight (uint index) const;
	virtual void SetActiveLight (uint index, const ILight* ptr);

	// Camera orientation retrieval
	virtual const Vector3f&		GetCameraPosition()		const	{ return mTrans.mView.pos;		}
	virtual const Vector3f&		GetCameraDirection()	const	{ return mTrans.mView.dir;		}
	virtual const Vector3f&		GetCameraUpVector()		const	{ return mTrans.mView.up;		}
	virtual const Vector3f&		GetCameraRange()		const	{ return mTrans.mProj.range;	}
	virtual const Bounds&		GetCameraNearBounds()	const	{ return mTrans.mCam;			}

	// Matrix retrieval
	virtual const Matrix43&		GetModelMatrix()			{ return mTrans.GetModelMatrix(); }
	virtual const Matrix43&		GetViewMatrix()				{ return mTrans.GetViewMatrix(); }
	virtual const Matrix44&		GetProjectionMatrix()		{ return mTrans.GetProjectionMatrix(); }
	virtual const Matrix43&		GetModelViewMatrix()		{ return mTrans.GetModelViewMatrix(); }
	virtual const Matrix44&		GetModelViewProjMatrix()	{ return mTrans.GetModelViewProjMatrix(); }
	virtual const Matrix43&		GetInverseModelViewMatrix()	{ return mTrans.GetInverseModelViewMatrix(); }
	virtual const Matrix44&		GetInverseProjMatrix()		{ return mTrans.GetInverseProjMatrix(); }
	virtual const Matrix44&		GetInverseMVPMatrix()		{ return mTrans.GetInverseMVPMatrix(); }

	// Model matrix manipulation
	virtual void SetModelMatrix (const Matrix43& mat)		{ mTrans.OverrideModelMatrix(mat); }
	virtual void ResetModelMatrix()							{ mTrans.CancelModelMatrixOverride(); }

	// View matrix manipulation
	virtual void SetViewMatrix (const Matrix43& mat)		{ mTrans.OverrideViewMatrix(mat); }
	virtual void ResetViewMatrix()							{ mTrans.CancelViewMatrixOverride(); }

	// Projection matrix manipulation
	virtual void SetProjectionMatrix (const Matrix44& mat)	{ mTrans.OverrideProjectionMatrix(mat); }
	virtual void ResetProjectionMatrix()					{ mTrans.CancelProjectionMatrixOverride(); }

	// Convenience camera control functions. Range X = near, Y = far, Z = field of view (in degrees)
	virtual void SetCameraOrientation		( const Vector3f& eye, const Vector3f& dir, const Vector3f& up );
	virtual void SetCameraRange				( const Vector3f& range );

	// Active object control
	virtual void SetActiveRenderTarget		( const IRenderTarget* tar );
	virtual void SetActiveTechnique			( const ITechnique* ptr, bool insideOut = false );
	virtual bool SetActiveMaterial			( const IMaterial* ptr );
	virtual bool SetActiveMaterial			( const ITexture* ptr );
	virtual bool SetActiveShader			( const IShader* ptr );
	virtual void SetActiveSkybox			( const ITexture* ptr ) { mSkybox = ptr; }
	virtual void SetActiveColor				( const Color& c );
	virtual void SetScreenProjection		( bool screen ) { mTrans.Set2DMode(screen); }
	virtual void SetActiveVBO				( const IVBO* vbo, uint type = IVBO::Type::Invalid );
	virtual void SetActiveTexture			( uint textureUnit, const ITexture* ptr );
	virtual void SetActiveDepthFunction		( uint condition );
	virtual void SetActiveStencilFunction	( uint condition, uint val, uint mask );
	virtual void SetActiveStencilOperation	( uint testFail, uint depthFail, uint pass );
	virtual void SetActiveVertexAttribute	( uint			attribute,		// Attribute::Vertex, Attribute::Normal, etc
											  const IVBO*	vbo,			// VBO, 0 if none
											  const void*	ptr,			// Pointer to the data
											  uint			dataType,		// Data type, such as "Type::Float" for Vector3f
											  uint			elements,		// Number of data type elements, ie "3" for Vector3f
											  uint			stride );		// Size of each vertex entry in bytes

	// Activate all matrices and bind all textures, preparing to draw
	virtual void PrepareToDraw();

	// Draw bound vertices
	virtual uint DrawVertices	( uint primitive, uint vertexCount );
	virtual uint DrawIndices	( const IVBO* vbo, uint primitive, uint indexCount )		{ return _DrawIndices(vbo, 0, primitive, indexCount); }
	virtual uint DrawIndices	( const ushort* indices, uint primitive, uint indexCount )	{ return _DrawIndices(0, indices, primitive, indexCount); }

public:

	// Allow GLTexture class to access _BindTexture()
	friend class GLTexture;

	// Draw using an index array
	uint _DrawIndices ( const IVBO* vbo, const ushort* ptr, uint primitive, uint indexCount );

	// Updates the currently active texture unit
	void _ActivateTextureUnit();

	// Binds the specified texture -- mainly called from GLTexture::Activate()
	bool _BindTexture (uint glType, uint glID);

	// Ensures that all textures are bound
	void _BindAllTextures();
};