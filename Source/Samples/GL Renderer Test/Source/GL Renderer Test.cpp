#include "../../../Engine/OpenGL/Include/_All.h"
using namespace R5;

//====================================================================================================
// Box object
//====================================================================================================

struct Box 
{
	const ITexture*		mTex;
	Vector3f			mPos;
	Vector3f			mSize;
	Array<Vector3f>		mVerts;
	Array<Vector2f>		mTexcoords;
	Array<Vector3f>		mNormals;

	Box( const Vector3f pos, const Vector3f& size, const ITexture* tex) { Init( pos, size, tex ); };
	
	void SetSize( const Vector3f& size ) { Init( mPos, size, mTex ); }

	void SetPos( const Vector3f& pos) { mPos = pos; }

	void SetTexture( const ITexture* tex ) { mTex = tex; }

	void AddFace(	const Vector3f& p1,
					const Vector3f& p2,
					const Vector3f& p3,
					const Vector3f& p4,
					const Vector3f& normal	)
	{
		mVerts.Expand() = p1;
		mVerts.Expand() = p2;
		mVerts.Expand() = p3;
		mVerts.Expand() = p4;

		mTexcoords.Expand().Set(	0,			0			);
		mTexcoords.Expand().Set(	0,			mSize.y	);
		mTexcoords.Expand().Set(	mSize.x,	mSize.y	);
		mTexcoords.Expand().Set(	mSize.x,	0			);

		for (ushort i = 0; i < 4; ++i)
			mNormals.Expand() = normal;
	}

	void Init( const Vector3f& pos, const Vector3f& size, const ITexture* tex)
	{
		mPos = pos;
		mSize = size;
		mTex = tex;

		// front
		AddFace(	Vector3f(	0,			0,		mSize.z		),
					Vector3f(	0,			0,		0			),
					Vector3f(	mSize.x,	0,		0			),
					Vector3f(	mSize.x,	0,		mSize.z		),
					Vector3f(	0,			-1,		0			) );

		// right
		AddFace(	Vector3f(	mSize.x,	0,			mSize.z	),
					Vector3f(	mSize.x,	0,			0		),
					Vector3f(	mSize.x,	mSize.y,	0		),
					Vector3f(	mSize.x,	mSize.y,	mSize.z	),
					Vector3f(	1,			0,			0		) );

		// back
		AddFace(	Vector3f(	mSize.x,	mSize.y,	mSize.z	),
					Vector3f(	mSize.x,	mSize.y,	0		),
					Vector3f(	0,			mSize.y,	0		),
					Vector3f(	0,			mSize.y,	mSize.z	),
					Vector3f(	0,			1,			0		) );
		// left
		AddFace(	Vector3f(	0,			mSize.y,	mSize.z	),
					Vector3f(	0,			mSize.y,	0		),
					Vector3f(	0,			0,			0		),
					Vector3f(	0,			0,			mSize.z	),
					Vector3f(	-1,			0,			0		) );

		// bottom
		AddFace(	Vector3f(	mSize.x,	0,			0		),
					Vector3f(	mSize.x,	mSize.y,	0		),
					Vector3f(	0,			mSize.y,	0		),
					Vector3f(	0,			0,			0		),
					Vector3f(	0,			0,			-1		) );

		// top
		AddFace(	Vector3f(	0,			mSize.y,	mSize.z	),
					Vector3f(	0,			0,			mSize.z	),
					Vector3f(	mSize.x,	0,			mSize.z	),
					Vector3f(	mSize.x,	mSize.y,	mSize.z	),
					Vector3f(	0,			0,			1		) );
	}

	void Render (IGraphics* graphics)
	{
		graphics->SetWorldMatrix(mPos);
		graphics->SetActiveTexture(0, mTex);
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,		mVerts );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		mNormals );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	mTexcoords );
		graphics->DrawVertices(IGraphicsManager::Primitive::Quad, mVerts.GetSize());		
		graphics->ResetWorldMatrix();
	}
};

//====================================================================================================

struct FakeLight : public ILight
{
	Vector3f	mPos;
	Vector3f	mDir;
	Color4f		mAmbient;
	Color4f		mDiffuse;
	Color4f		mSpecular;
	Vector3f*	mParams;

	FakeLight() :	mPos		(0, 0, 0),
					mDir		(0, 0, -1),
					mAmbient	(0, 0, 0),
					mDiffuse	(1, 1, 1),
					mSpecular	(1, 1, 1),
					mParams	(0) {}

	virtual uint			GetLightType()		const	{ return ILight::Type::Directional; }
	virtual const Color4f&	GetAmbient()		const	{ return mAmbient; }
	virtual const Color4f&	GetDiffuse()		const	{ return mDiffuse; }
	virtual const Color4f&	GetSpecular()		const	{ return mSpecular; }
	virtual const Vector3f&	GetPosition()		const	{ return mPos; }
	virtual const Vector3f&	GetDirection()		const	{ return mDir; }
	virtual const Vector3f*	GetAttenParams()	const	{ return mParams; }
	virtual const Vector3f*	GetSpotParams()		const	{ return 0; }
};

//====================================================================================================

class TestApp : public IEventReceiver
{
    IWindow*	mWindow;
	IGraphics*	mGraphics;
	Vector2i	mMousePos;

public:

	TestApp() : mWindow(0), mGraphics(0)
	{
		mWindow		= new GLWindow();
		mGraphics	= new GLGraphics();

		mWindow->SetGraphics(mGraphics);
		mWindow->SetEventHandler( this );
		mWindow->Create("R5: GL Renderer Test", 100, 100, 1024, 768);
		mGraphics->SetDefaultAF(8);
	}

	~TestApp()
	{
		if (mWindow)	mWindow->SetGraphics(0);
		if (mGraphics)	delete mGraphics;
		if (mWindow)	delete mWindow;
	}

    void Run()
    {
		short width		= 5;
		short height	= 5;
		short offsetx	= 0;//-width / 2;
		short offsety	= 0;//-height / 2;
		short endx		= offsetx + width;
		short endy		= offsety + height;

		mGraphics->SetBackgroundColor( Color4f(0.25f, 0.25f, 0.25f) );
		mGraphics->SetCameraOrientation (	Vector3f(2.5f, 2.5f, 7),
											Vector3f(0, 0, -1),
											Vector3f(0, 1,  0) );

		Array<Vector3f>	vertices;
		Array<ushort>	indices;
		Array<Vector3f>	normals;
		Array<Vector2f>	texCoords;

		for (short y = offsety; y <= endy; ++y)
		{
			for (short x = offsetx; x <= endx; ++x)
			{
				vertices.Expand().Set(x, y);
				normals.Expand().Set(0, 0, 1);
				texCoords.Expand().Set(x, y);			
			}
		}

		short tempWidth = width + 1;
		for (short y = 0; y < height; ++y)
		{
			for (short x = 0; x < width; ++x)
			{
				indices.Expand() = ((y + 1) * tempWidth + x);
				indices.Expand() = (y * tempWidth + x);
				indices.Expand() = (y * tempWidth + (x + 1));
				indices.Expand() = ((y + 1) * tempWidth + (x + 1));
			}
		}

		mGraphics->SetActiveColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );

		FakeLight light0;
		Vector3f params (0, 0.1f, 0.003f);
		light0.mDiffuse.Set( 1, 0, 0 );
		light0.mPos.Set(4, 4, 1);
		light0.mParams = &params;

		FakeLight light1;
		light1.mDir.Set(3, 3, -1);
		light1.mAmbient.Set( 0.2f, 0.2f, 0.2f);
		light1.mDiffuse.Set( 1, 1, 1 );

		ITexture* floorTex	= mGraphics->GetTexture("Textures/organic_df.jpg");
		ITexture* hitTex	= mGraphics->GetTexture("Textures/limestone.jpg");
		ITexture* onTex		= mGraphics->GetTexture("Textures/gemstone_df.jpg");

		Box boxTest1(	Vector3f( 0, 1, 0 ),
						Vector3f( 1, 3, 1 ),
						onTex );

		Box boxTest2(	Vector3f( 1, 2, 0 ),
						Vector3f( 1, 1, 1 ),
						onTex );

		Box boxTest3(	Vector3f( 2, 1, 0 ),
						Vector3f( 1, 3, 1 ),
						onTex );

		Box boxTest4(	Vector3f( 4, 1, 0 ),
						Vector3f( 1, 3, 1 ),
						hitTex );

		while ( mWindow->Update() )
		{
			mWindow->BeginFrame();
			mGraphics->BeginFrame();

			mGraphics->SetActiveRenderTarget(0);
			mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );
			mGraphics->SetLighting(true);
			mGraphics->SetActiveLight( 0, &light0 );
			mGraphics->SetActiveLight( 1, &light1 );
			mGraphics->Clear();

			mGraphics->SetActiveTexture(0, floorTex);
			mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,	vertices	);
			mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,		normals		);
			mGraphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,	texCoords	);
			mGraphics->DrawIndices(&indices[0], IGraphicsManager::Primitive::Quad,	indices.GetSize());

			boxTest1.Render( mGraphics );
			boxTest2.Render( mGraphics );
			boxTest3.Render( mGraphics );
			boxTest4.Render( mGraphics );

			mGraphics->EndFrame();
			mWindow->EndFrame();
			Thread::Sleep(1);
		}
	}

	virtual void OnResize(const Vector2i& size)
	{
		mGraphics->SetViewport( mWindow->GetSize() );
	}

    virtual bool OnKey(const Vector2i& pos, byte key, bool isDown)
    {
		if (isDown)
		{
			if ( key == Key::MouseLeft )
			{
				Vector3f pos ( mGraphics->ConvertTo3D(mMousePos) );
				Thread::MessageWindow("%f %f %f", pos.x, pos.y, pos.z);
			}
			else if ( key == Key::Escape )
			{
				mWindow->Close();
			}
			else if ( key == Key::F5 )
			{
				if ( mWindow->GetStyle() == IWindow::Style::FullScreen )
				{
					mWindow->SetSize( Vector2i(1024, 768) );
					mWindow->SetStyle( IWindow::Style::Normal );
				}
				else
				{
					mWindow->SetSize( Vector2i(1680, 1050) );
					mWindow->SetStyle( IWindow::Style::FullScreen );
				}
			}
		}
		return true;
    }

	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta)
	{
		mMousePos = pos;
		return true;
	}
};

//====================================================================================================

R5_MAIN_FUNCTION
{
	System::SetCurrentPath("../../../Resources/");
	TestApp app;
    app.Run();
	return 0;
}