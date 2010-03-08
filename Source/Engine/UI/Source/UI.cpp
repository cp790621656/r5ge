#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UI::UI (IGraphics* graphics, IWindow* window) : mGraphics(graphics), mWindow(window)
{
	ASSERT(mGraphics != 0, "IGraphics* can't be NULL!");
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
// Draws a single queue, returning the number of triangles drawn
//============================================================================================================

uint UI::DrawQueue (UIQueue* queue)
{
	CustomQueue* q = (CustomQueue*)queue;
	uint tri (0);

	if (q != 0 && q->mVertexCount)
	{
		ASSERT(q->mGraphics == mGraphics, "Uhh... graphics don't match?");

		mGraphics->SetBlending				( q->mIgnoreAlpha ? IGraphics::Blending::None : IGraphics::Blending::Normal );
		mGraphics->SetActiveTexture			( 0, q->mTex );
		mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord0,	q->mVbo, 8,  IGraphics::DataType::Float, 2, sizeof(Vertex) );
		mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Color,		q->mVbo, 16, IGraphics::DataType::Byte,  4, sizeof(Vertex) );
		mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Position,	q->mVbo, 0,  IGraphics::DataType::Float, 2, sizeof(Vertex) );

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
// Draw the entire User Interface
//============================================================================================================

void UI::OnPreDraw() const
{
	mGameStats = mGraphics->GetFrameStats();
	mUIStats.Clear();

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
	mGraphics->SetActiveProjection		( IGraphics::Projection::Orthographic );
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
	mGraphics->SetActiveVertexAttribute	( IGraphics::Attribute::Position,  0 );
	mGraphics->SetActiveShader			( 0 );
}