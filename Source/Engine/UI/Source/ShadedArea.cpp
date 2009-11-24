#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Sets the active shader
//============================================================================================================

void UIShadedArea::SetShader (const IShader* shader)
{
	if (mShader != shader)
	{
		if (mShader != 0) SetDirty();
		mShader = shader;
		if (mShader != 0) SetDirty();
	}
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIShadedArea::OnFill (UIQueue* queue)
{
	if (queue->mTex	== 0 &&
		queue->mLayer	== mLayer &&
		queue->mArea	== this)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetLeft()	);
		float top	 ( mRegion.GetTop()	);
		float right	 ( mRegion.GetRight()  );
		float bottom ( mRegion.GetBottom() );

		Color4ub color (255, 255, 255, Float::ToRangeByte(mRegion.GetAlpha()) );

		v.Expand().Set( left,  top,		0.0f, 0.0f, color );
		v.Expand().Set( left,  bottom,	0.0f, 1.0f, color );
		v.Expand().Set( right, bottom,	1.0f, 1.0f, color );
		v.Expand().Set( right, top,		1.0f, 0.0f, color );
	}
}

//============================================================================================================
// Activate the shader and bind the textures
//============================================================================================================

void UIShadedArea::OnPreDraw (IGraphics* graphics) const
{
	for (uint i = 0; i < mTex.GetSize(); ++i)
		graphics->SetActiveTexture(i, mTex[i]);

	graphics->SetActiveShader(mShader);
}

//============================================================================================================
// Disable the shader and deactivate all textures
//============================================================================================================

void UIShadedArea::OnPostDraw (IGraphics* graphics) const
{
	graphics->SetActiveShader(0);

	for (uint i = 0; i < mTex.GetSize(); ++i)
		graphics->SetActiveTexture(i, 0);
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIShadedArea::CustomSerializeFrom(const TreeNode& root)
{
	const Variable& value = root.mValue;

	if (root.mTag == "Shader")
	{
		SetShader( mRoot->GetShader( value.IsString() ? value.AsString() : value.GetString() ) );
		return true;
	}
	else
	{
		String left, right;
		
		if (root.mTag.Split(left, ' ', right))
		{
			if (left == "Texture")
			{
				uint index (0);

				if (right >> index)
				{
					SetTexture(index, mRoot->GetTexture( value.IsString() ?
						value.AsString() : value.GetString() ));
				}
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIShadedArea::CustomSerializeTo(TreeNode& root) const
{
	TreeNode& shader = root.AddChild("Shader");
	if (mShader != 0) shader.mValue = mShader->GetName();

	for (uint i = 0; i < mTex.GetSize(); ++i)
	{
		const ITexture* tex = mTex[i];

		if (tex != 0)
		{
			root.AddChild( String("Texture %u", i), tex->GetName() );
		}
	}
}