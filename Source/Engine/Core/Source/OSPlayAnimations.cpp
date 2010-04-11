#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Plays the specified animation
//============================================================================================================

void OSPlayAnimations::Set (const Array<String>& anims)
{
	if (anims.IsValid())
	{
		mAnims = anims;
		ModelInstance* inst = R5_CAST(ModelInstance, mObject);

		if (inst != 0)
		{
			Model* model = inst->GetModel();
			
			if (model != 0)
			{
				FOREACH(i, mAnims)
				{
					model->PlayAnimation(mAnims[i]);
				}
			}
		}
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSPlayAnimations::OnSerializeTo (TreeNode& node) const
{
	if (mAnims.IsValid())
	{
		node.AddChild("Animations", mAnims);
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSPlayAnimations::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Animations")
	{
		if (node.mValue.IsStringArray())
		{
			Set(node.mValue.AsStringArray());
		}
	}
}