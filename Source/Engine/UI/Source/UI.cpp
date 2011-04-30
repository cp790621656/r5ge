#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UI::UI (IGraphics* graphics, IWindow* window) : mGraphics(graphics), mWindow(window)
{
	ASSERT(mGraphics != 0, "IGraphics* can't be NULL!");
#ifdef _DEBUG
	System::Log("[UI]      Created the UI system");
#endif
}

//============================================================================================================
// We need additional functionality on release
//============================================================================================================

void UI::Release()
{
#ifdef _DEBUG
	System::Log("[UI]      Releasing the UI system");
#endif
	UIManager::Release();
	mWindow = 0;
}

//============================================================================================================
// Retrieves the specified font
//============================================================================================================

IFont* UI::GetFont (const String& name)
{
	IFont* font = mGraphics->GetFont(name);
	if (font != 0 && mDefaultFont == 0) mDefaultFont = font;
	return font;
}

//============================================================================================================
// Updates the buffer associated with the rendering queue
//============================================================================================================

void UI::UpdateBuffer (UIQueue* queue)
{
	CustomQueue* q = (CustomQueue*)queue;

	if (q)
	{
		q->mVertexCount = queue->mVertices.GetSize();

		if ( q->mVertexCount != 0 )
		{
			if (q->mVbo == 0)
				q->mVbo = mGraphics->CreateVBO();

			q->mVbo->Lock();
			q->mVbo->Set (q->mVertices, IVBO::Type::Vertex, q->mDynamic);
			q->mVbo->Unlock();
		}
		else if ( q->mVbo != 0 )
		{
			q->mVbo->Release();
		}
	}
}

//============================================================================================================
// Draw the entire User Interface
//============================================================================================================

void UI::OnPreDraw() const
{
	mGameStats = mGraphics->GetFrameStats();
	mUIStats.Clear();

	mGraphics->SetActiveRenderTarget	( 0 );
	mGraphics->SetFog					( false );
	mGraphics->SetDepthTest				( false );
	mGraphics->SetDepthWrite			( false );
	mGraphics->SetColorWrite			( true  );
	mGraphics->SetAlphaTest				( false );
	mGraphics->SetStencilTest			( false );
	mGraphics->SetWireframe				( false );
	mGraphics->SetLighting				( IGraphics::Lighting::None );
	mGraphics->SetCulling				( IGraphics::Culling::Back );
	mGraphics->SetActiveMaterial		( 0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord1,	0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Tangent,	0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Normal,		0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneIndex,	0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneWeight,	0 );
	mGraphics->SetScreenProjection		( true );
}

//============================================================================================================
// Sets the draw region
//============================================================================================================

void UI::SetClipRect (const Rect& rect)
{
	if (mRects.IsEmpty()) mGraphics->SetScissorTest(true);
	mRects.Expand() = rect;
	mGraphics->SetScissorRect(rect);
}

//============================================================================================================
// Draws a single queue, returning the number of triangles drawn
//============================================================================================================

uint UI::DrawQueue (UIQueue* queue)
{
	CustomQueue* q = (CustomQueue*)queue;
	uint tri (0);

	if (q != 0 && q->mVertexCount)
	{
		ASSERT(q->mGraphics == mGraphics, "Uhh... graphics don't match?");

		mGraphics->SetBlending				( q->mIgnoreAlpha ? IGraphics::Blending::None : IGraphics::Blending::Replace );
		mGraphics->SetActiveTexture			( 0, q->mTex );
		mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord0,	q->mVbo, 8,  IGraphics::DataType::Float, 2, sizeof(Vertex) );
		mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Color,		q->mVbo, 16, IGraphics::DataType::Byte,  4, sizeof(Vertex) );
		mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Vertex,	q->mVbo, 0,  IGraphics::DataType::Float, 2, sizeof(Vertex) );

		if (q->mWidget == 0)
		{
			tri = mGraphics->DrawVertices( IGraphicsManager::Primitive::Quad, q->mVertexCount );
		}
		else
		{
			q->mWidget->OnPreDraw(mGraphics);
			tri = mGraphics->DrawVertices( IGraphicsManager::Primitive::Quad, q->mVertexCount );
			q->mWidget->OnPostDraw(mGraphics);
		}
	}
	return tri;
}

//============================================================================================================
// Restores the previous draw region
//============================================================================================================

void UI::RestoreClipRect()
{
	mRects.Shrink();

	if (mRects.IsEmpty())
	{
		mGraphics->SetScissorTest(false);
	}
	else
	{
		mGraphics->SetScissorRect(mRects.Back());
	}
}

//============================================================================================================
// Cleanup
//============================================================================================================

void UI::OnPostDraw() const
{
	mUIStats  = mGraphics->GetFrameStats();
	mUIStats -= mGameStats;

	mGraphics->SetActiveTexture			( 0, 0 );
	mGraphics->SetActiveShader			( 0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord0, 0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Color,	   0 );
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Vertex,  0 );
	mGraphics->SetActiveShader			( 0 );
}