#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Set up all render states and activate the material before moving down to QuadTree's OnDraw
//============================================================================================================

uint Terrain::OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, bool insideOut)
{
	if ( mMat != 0 && group  == mMat->GetUID() && mMat->IsVisibleWith(tech) )
	{
		IGraphics* graphics = mCore->GetGraphics();

		graphics->SetActiveMaterial(mMat);
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Normal,		0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Color,		0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Tangent,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord0,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord1,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneIndex,	0 );
		graphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneWeight,	0 );

		return QuadTree::OnDraw(storage, group, tech, insideOut);
	}
	return 0;
}

//============================================================================================================
// Called when the object is being saved
//============================================================================================================

void Terrain::OnSerializeTo (TreeNode& root) const
{
	if (mMat != 0) root.AddChild(IMaterial::ClassID(), mMat->GetName());
}

//============================================================================================================
// Called when the object is being loaded
//============================================================================================================

bool Terrain::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == IMaterial::ClassID())
	{
		IGraphics* graphics = mCore->GetGraphics();

		if (graphics != 0)
		{
			mMat = graphics->GetMaterial( node.mValue.IsString() ?
				node.mValue.AsString() : node.mValue.GetString() );
		}
		return true;
	}
	return false;
}