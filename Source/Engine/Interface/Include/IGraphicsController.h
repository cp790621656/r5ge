#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Low-level graphics controller -- closest level of interaction with the renderer API
//============================================================================================================

struct IGraphicsController
{
	typedef ITechnique::Lighting	Lighting;
	typedef ITechnique::Blending	Blending;
	typedef ITechnique::Culling		Culling;
	typedef ITechnique::Sorting		Sorting;

	struct Projection
	{
		enum
		{
			Orthographic = 0,
			Perspective
		};
	};

	struct DataType
	{
		enum
		{
			Invalid		= 0,
			Byte		= 0x1401,
			Int			= 0x1404,
			Float		= 0x1406,
		};
	};

	// NOTE: http://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/attributes.php
	// Scroll down to "Custom Vertex Attributes" for the reason behind these numbers.
	struct Attribute
	{
		enum
		{
			Position		= 0,
			Tangent			= 1,
			Normal			= 2,
			Color			= 3,
			SecondaryColor	= 4,
			FogCoord		= 5,
			BoneWeight		= 6,
			BoneIndex		= 7,
			TexCoord0		= 8,
			TexCoord1		= 9,
			TexCoord2		= 10,
			TexCoord3		= 11,
			TexCoord4		= 12,
			TexCoord5		= 13,
			TexCoord6		= 14,
			TexCoord7		= 15,
		};
	};

	struct Condition
	{
		enum
		{
			Never			= 0,
			Less			= 1,
			Equal			= 2,
			LessOrEqual		= 3,
			Greater			= 4,
			NotEqual		= 5,
			GreaterOrEqual	= 6,
			Always			= 7,
		};
	};

	struct Operation
	{
		enum
		{
			Keep		= 0,
			SetToZero	= 1,
			Replace		= 2,
			Increment	= 3,
			Decrement	= 4,
			Invert		= 5,
		};
	};

	struct FrameStats
	{
		uint mTriangles;
		uint mDrawCalls;
		uint mMatSwitches;
		uint mTexSwitches;
		uint mBufferBinds;
		uint mShaderSwitches;
		uint mLightSwitches;
		uint mTechSwitches;

		FrameStats() { Clear(); }

		void Clear()
		{
			mTriangles		= 0;
			mDrawCalls		= 0;
			mMatSwitches	= 0;
			mTexSwitches	= 0;
			mBufferBinds	= 0;
			mShaderSwitches	= 0;
			mLightSwitches	= 0;
			mTechSwitches	= 0;
		}
	};

public:

	// Should finish all drawing operations
	virtual void Flush()=0;

	// Property control
	virtual void SetFog				(bool val)=0;
	virtual void SetDepthWrite		(bool val)=0;
	virtual void SetDepthTest		(bool val)=0;
	virtual void SetColorWrite		(bool val)=0;
	virtual void SetAlphaTest		(bool val)=0;
	virtual void SetStencilTest		(bool val)=0;
	virtual void SetWireframe		(bool val)=0;
	virtual void SetLighting		(uint val)=0;
	virtual void SetBlending		(uint val)=0;
	virtual void SetCulling			(uint val)=0;
	virtual void SetADT				(float val = 0.003921568627451f)=0;
	virtual void SetThickness		(float val)=0;
	virtual void SetNormalize		(bool val)=0;
	virtual void SetDepthOffset		(bool val)=0;
	virtual void SetViewport		(const Vector2i& size )=0;
	virtual void SetFogRange		(const Vector2f& range)=0;
	virtual void SetBackgroundColor	(const Color4f& color )=0;
	virtual void SetDefaultAF		(uint level)=0;

	// Statistics about the current frame
	virtual const FrameStats&	GetFrameStats()			const=0;
	virtual bool				GetFog()				const=0;
	virtual bool				GetDepthWrite()			const=0;
	virtual bool				GetDepthTest()			const=0;
	virtual bool				GetAlphaTest()			const=0;
	virtual bool				GetStencilTest()		const=0;
	virtual bool				GetWireframe()			const=0;
	virtual uint				GetLighting()			const=0;
	virtual uint				GetBlending()			const=0;
	virtual uint				GetCulling()			const=0;
	virtual float				GetADT()				const=0;
	virtual float				GetThickness()			const=0;
	virtual bool				GetNormalize()			const=0;
	virtual bool				GetDepthOffset()		const=0;
	virtual uint				GetDefaultAF()			const=0;
	virtual const Vector2i&		GetViewport()			const=0;
	virtual const Vector2f&		GetFogRange()			const=0;
	virtual const Color4f&		GetBackgroundColor()	const=0;
	virtual const ITexture*		GetActiveSkybox()		const=0;
	virtual const ITechnique*	GetActiveTechnique()	const=0;
	virtual const Vector2i&		GetActiveViewport()		const=0;

	// Camera orientation retrieval
	virtual const Vector3f&		GetCameraPosition()		const=0;
	virtual const Vector3f&		GetCameraDirection()	const=0;
	virtual const Vector3f&		GetCameraUpVector()		const=0;
	virtual const Vector2f&		GetCameraRange()		const=0;
	virtual float				GetCameraFOV()			const=0;

	// Matrix retrieval
	virtual const Matrix43&		GetModelMatrix()=0;
	virtual const Matrix43&		GetViewMatrix()=0;
	virtual const Matrix44&		GetProjectionMatrix()=0;
	virtual const Matrix43&		GetModelViewMatrix()=0;
	virtual const Matrix44&		GetModelViewProjMatrix()=0;
	virtual const Matrix43&		GetInverseModelViewMatrix()=0;
	virtual const Matrix44&		GetInverseProjMatrix()=0;
	virtual const Matrix44&		GetInverseMVPMatrix()=0;

	// Model matrix manipulation -- resets the View matrix back to default
	virtual void SetModelMatrix (const Matrix43& mat)=0;
	virtual void ResetModelMatrix()=0;

	// ModelView matrix manipulation
	virtual void SetModelViewMatrix (const Matrix43& mat)=0;
	virtual void ResetModelViewMatrix()=0;

	// Camera control functions. Range X = near, Y = far, Z = field of view, in degrees.
	virtual void SetCameraOrientation		( const Vector3f& eye, const Vector3f& dir, const Vector3f& up )=0;
	virtual void SetCameraRange				( const Vector3f& range )=0;

	// Active state control
	virtual void SetActiveRenderTarget		( const IRenderTarget* tar )=0;
	virtual void SetActiveTechnique			( const ITechnique* ptr, bool insideOut = false )=0;
	virtual bool SetActiveMaterial			( const IMaterial* ptr )=0;
	virtual bool SetActiveMaterial			( const ITexture* ptr )=0;
	virtual uint SetActiveShader			( const IShader* ptr, bool forceUpdateUniforms = false )=0;
	virtual void SetActiveSkybox			( const ITexture* ptr )=0;
	virtual void SetActiveColor				( const Color& c )=0;
	virtual void SetActiveProjection		( uint projection )=0;
	virtual void SetActiveVBO				( const IVBO* vbo, uint type = IVBO::Type::Invalid )=0;
	virtual void SetActiveTexture			( uint textureUnit, const ITexture* ptr )=0;
	virtual void SetActiveLight				( uint index, const Light* ptr )=0;
	virtual void SetActiveDepthFunction		( uint condition )=0;
	virtual void SetActiveStencilFunction	( uint condition, uint val, uint mask )=0;
	virtual void SetActiveStencilOperation	( uint testFail, uint depthFail, uint pass )=0;
	virtual void SetActiveVertexAttribute	( uint			attribute,		// Attribute::Vertex, Attribute::Normal, etc
											  const IVBO*	vbo,			// VBO, 0 if none
											  const void*	ptr,			// Pointer to the data
											  uint			dataType,		// Data type, such as "Type::Float" for Vector3f
											  uint			elements,		// Number of data type elements, ie "3" for Vector3f
											  uint			stride )=0;		// Size of each vertex entry in bytes

public: // Convenience functions

	inline void SetActiveMaterial			( int zero )					{ SetActiveMaterial((const IMaterial*)0); }
	inline void SetActiveVertexAttribute	( uint attribute, int zero )	{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)0, DataType::Invalid, 0, 0); }
	
	inline void SetActiveVertexAttribute	( uint			attribute,
											  const IVBO*	vbo,
											  int			offset,
											  uint			dataType,
											  uint			elements,
											  uint			stride )		{ SetActiveVertexAttribute(attribute, vbo, (const void*)offset, dataType, elements, stride); }

	inline void SetActiveVertexAttribute	( uint			attribute,
											  const void*	ptr,
											  uint			dataType,
											  uint			elements,
											  uint			stride )		{ SetActiveVertexAttribute(attribute, (const IVBO*)0, ptr, dataType, elements, stride); }

	inline void SetActiveVertexAttribute	( uint attribute, const Vector2f* v )			{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Float,  2, sizeof(Vector2f)); }
	inline void SetActiveVertexAttribute	( uint attribute, const Vector3f* v )			{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Float,  3, sizeof(Vector3f)); }
	inline void SetActiveVertexAttribute	( uint attribute, const Vector4f* v )			{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Float,  4, sizeof(Vector4f)); }
	inline void SetActiveVertexAttribute	( uint attribute, const Color4ub* v )			{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Byte,   4, sizeof(Color4ub)); }

	inline void SetActiveVertexAttribute	( uint attribute, const Array<Vector2f>& v )	{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Float,  2, sizeof(Vector2f)); }
	inline void SetActiveVertexAttribute	( uint attribute, const Array<Vector3f>& v )	{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Float,  3, sizeof(Vector3f)); }
	inline void SetActiveVertexAttribute	( uint attribute, const Array<Vector4f>& v )	{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Float,  4, sizeof(Vector4f)); }
	inline void SetActiveVertexAttribute	( uint attribute, const Array<Color4ub>& v )	{ SetActiveVertexAttribute(attribute, (const IVBO*)0, (const void*)v, DataType::Byte,   4, sizeof(Color4ub)); }

public: // Draw functions

	virtual uint DrawVertices	( uint primitive, uint vertexCount )=0;
	virtual uint DrawIndices	( const IVBO* vbo, uint primitive, uint indexCount )=0;
	virtual uint DrawIndices	( const ushort* indices, uint primitive, uint indexCount )=0;
};