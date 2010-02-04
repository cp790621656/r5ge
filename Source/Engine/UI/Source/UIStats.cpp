#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// INTERNAL: Adds a new label to the list
//============================================================================================================

UILabel* UIStats::_AddLabel (const String& name)
{
	UILabel* lbl = AddWidget<UILabel>(name, false);
	lbl->SetSerializable(false);
	lbl->SetReceivesEvents(false);

	UIRegion& rgn = lbl->GetRegion();
	rgn.SetLeft(0.0f, 2.0f);
	rgn.SetRight(1.0f, -2.0f);
	return lbl;
}

//============================================================================================================
// Function called after the parent and root have been set
//============================================================================================================

void UIStats::OnInit()
{
	mLabels.Expand() = _AddLabel("FPS");
	mLabels.Expand() = _AddLabel("Triangles");
	mLabels.Expand() = _AddLabel("Draw Calls");
	mLabels.Expand() = _AddLabel("Buffer Binds");
	mLabels.Expand() = _AddLabel("Matrix Switches");
	mLabels.Expand() = _AddLabel("Texture Switches");
	mLabels.Expand() = _AddLabel("Shader Switches");
	mLabels.Expand() = _AddLabel("Technique Switches");
	mLabels.Expand() = _AddLabel("Light Switches");
	mIsDirty = true;
}

//============================================================================================================
// Change the layer of the labels
//============================================================================================================

void UIStats::OnLayerChanged()
{
	for (uint i = mLabels.GetSize(); i > 0; )
		mLabels[--i]->SetLayer(mLayer);
}

//============================================================================================================
// Only the update event is necessary for this widget
//============================================================================================================

bool UIStats::OnUpdate (bool dimensionsChanged)
{
	if (mIsDirty)
	{
		const IFont* font = GetFont();
		if (font == 0) return false;
		uint size = font->GetSize();

		for (uint i = 0; i < mLabels.GetSize(); ++i)
		{
			UIRegion& rgn = mLabels[i]->GetRegion();
			rgn.SetTop(0.0f, 2.0f + (float)(size * i));
			rgn.SetBottom(0.0f, 2.0f + (float)(size * (i + 1)));
		}
	}

	// This widget only works with the R5 UI
	UI* ui = R5_CAST(UI, mUI);

	if (ui == 0)
	{
		DestroySelf();
	}
	else
	{
		const FrameStats& stats = (mName.BeginsWith("Game")) ? ui->GetGameStats() : ui->GetUIStats();

		mLabels[0]->SetText( String("[FF5555]%u[FFFFFF] FPS", Time::GetFPS()) );
		mLabels[1]->SetText( String("[FF5555]%u[FFFFFF] triangles", stats.mTriangles) );
		mLabels[2]->SetText( String("[FF5555]%u[FFFFFF] draw calls", stats.mDrawCalls) );
		mLabels[3]->SetText( String("[FF5555]%u[FFFFFF] buffer binds", stats.mBufferBinds) );
		mLabels[4]->SetText( String("[FF5555]%u[FFFFFF] matrix switches", stats.mMatSwitches) );
		mLabels[5]->SetText( String("[FF5555]%u[FFFFFF] texture switches", stats.mTexSwitches) );
		mLabels[6]->SetText( String("[FF5555]%u[FFFFFF] shader switches", stats.mShaderSwitches) );
		mLabels[7]->SetText( String("[FF5555]%u[FFFFFF] technique switches", stats.mTechSwitches) );
		mLabels[8]->SetText( String("[FF5555]%u[FFFFFF] light switches", stats.mLightSwitches) );

		if (mIsDirty)
		{
			mIsDirty = false;
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIStats::OnSerializeTo (TreeNode& node) const
{
	const IFont* font = GetFont();
	if (font != 0) node.AddChild("Font", font->GetName());
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIStats::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Font")
	{
		IFont* font = mUI->GetFont(node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString());

		for (int i = mLabels.GetSize(); i > 0; )
		{
			mLabels[--i]->SetFont(font);
			mIsDirty = true;
		}
		return true;
	}
	return false;
}