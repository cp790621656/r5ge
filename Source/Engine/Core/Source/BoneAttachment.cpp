#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// The owner's absolute coordinates need to be calculated in the update
//============================================================================================================

void BoneAttachment::OnUpdate()
{
	// If this script is not running on a model instance, we might as well just destroy it
	const ModelInstance* parent = R5_CAST(ModelInstance, mObject->GetParent());
	if (parent == 0) { DestroySelf(); return; }

	// If a model instance is not referencing any model, there is nothing we can do
	const Model* model = parent->GetModel();
	if (model == 0) return;

	// If we haven't found the bone yet, find it
	if (mBoneIndex == -1) mBoneIndex = model->GetBoneIndex(mName);

	// Retrieve the bone transform
	const BoneTransform* tm = model->GetBoneTransform(mBoneIndex);
	if (tm == 0) return;

	// Start with the animation-transformed bone position and transform it into world space
	Vector3f pos (tm->mAbsolutePos);
	pos *= parent->GetAbsoluteScale();
	pos *= parent->GetAbsoluteRotation();
	pos += parent->GetAbsolutePosition();

	// Do the same with bone's rotation
	Quaternion rot (parent->GetAbsoluteRotation(), tm->mAbsoluteRot);

	// Use the absolute bone orientation as the parent of this object
	mObject->OverrideAbsolutePosition( pos + mObject->GetRelativePosition() * rot );
	mObject->OverrideAbsoluteRotation( rot * mObject->GetRelativeRotation() );
	mObject->SetDirty();
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool BoneAttachment::SerializeTo (TreeNode& root) const
{
	TreeNode& node = root.AddChild( Script::ClassID(), GetClassID() );
	node.AddChild("Bone", mName);
	return true;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool BoneAttachment::SerializeFrom (const TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];

		if (node.mTag == "Bone")
		{
			mName = node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString();
			mBoneIndex = -1;
		}
	}
	return true;
}