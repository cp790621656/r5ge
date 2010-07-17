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
		queue->mWidget	== this)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetCalculatedLeft()	);
		float top	 ( mRegion.GetCalculatedTop()	);
		float right	 ( mRegion.GetCalculatedRight()  );
		float bottom ( mRegion.GetCalculatedBottom() );

		Color4ub color (255, 255, 255, Float::ToRangeByte(mRegion.GetCalculatedAlpha()) );

		v.Expand().Set( left,  top,		0.0f, 1.0f, color );
		v.Expand().Set( left,  bottom,	0.0f, 0.0f, color );
		v.Expand().Set( right, bottom,	1.0f, 0.0f, color );
		v.Expand().Set( right, top,		1.0f, 1.0f, color );
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

bool UIShadedArea::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;

	if (node.mTag == "Shader")
	{
		SetShader( mUI->GetShader( value.AsString() ) );
		return true;
	}
	else if (node.mTag == "Textures")
	{
		if (node.mValue.IsStringArray())
		{
			const Array<String>& sa = node.mValue.AsStringArray();
			for (uint i = 0; i < sa.GetSize(); ++i) SetTexture(i, mUI->GetTexture(sa[i]));
		}
		return true;
	}
	else if (node.mTag == "Texture")
	{
		SetTexture(0, mUI->GetTexture(node.mValue.GetString()));
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIShadedArea::OnSerializeTo (TreeNode& node) const
{
	TreeNode& shader = node.AddChild("Shader");
	if (mShader != 0) shader.mValue = mShader->GetName();

	TreeNode& child = node.AddChild("Textures");

	if (mTex.IsValid())
	{
		Array<String>& sa = child.mValue.ToStringArray();

		for (uint i = 0; i < mTex.GetSize(); ++i)
		{
			sa.Expand() = mTex[i]->GetName();
		}
	}
}