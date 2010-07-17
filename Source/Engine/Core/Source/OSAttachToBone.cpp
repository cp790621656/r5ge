#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// The owner's absolute coordinates need to be calculated in the update
//============================================================================================================

void OSAttachToBone::OnUpdate()
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
	mObject->OverrideAbsolutePosition( pos + (mObject->GetRelativePosition() * parent->GetAbsoluteScale()) * rot );
	mObject->OverrideAbsoluteRotation( rot * mObject->GetRelativeRotation() );
	mObject->SetDirty();
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSAttachToBone::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Bone", mName);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSAttachToBone::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Bone")
	{
		mName = node.mValue.AsString();
		mBoneIndex = -1;
	}
}