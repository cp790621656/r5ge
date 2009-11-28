#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Set up all render states and activate the material before moving down to QuadTree's OnDraw
//============================================================================================================

uint Terrain::OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut)
{
	if ( (mMat->GetTechniqueMask() & tech->GetMask()) != 0 )
	{
		graphics->SetActiveMaterial(mMat);
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Normal,		0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Color,		0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Tangent,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord0,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord1,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneIndex,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneWeight,	0 );

		return QuadTree::OnDraw(graphics, tech, insideOut);
	}
	return 0;
}