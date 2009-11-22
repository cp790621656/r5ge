#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Default group properties are that of a solid object group
//============================================================================================================

GLTechnique::GLTechnique (const String& name) :
		mName			(name),
		mMask			(0),
		mFog			(true),
		mDepthWrite		(true),
		mDepthTest		(true),
		mColorWrite		(true),
		mAlphaTest		(true),
		mWireframe		(false),
		mLighting		(Lighting::OneSided),
		mBlending		(Blending::Normal),
		mCulling		(Culling::Back),
		mSorting		(Sorting::None),
		mSerializable	(false)
{
	static uint counter = 0;
	mMask = 1 << counter++;

	if (counter > 32)
	{
		WARNING("Number of techniques exceeded implementation limits! (32)");
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool GLTechnique::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	mSerializable = true;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if		(tag == "Fog")			value >> mFog;
		else if (tag == "Depth Write")	value >> mDepthWrite;
		else if (tag == "Depth Test")	value >> mDepthTest;
		else if (tag == "Color Write")	value >> mColorWrite;
		else if (tag == "Alpha Test")	value >> mAlphaTest;
		else if (tag == "Wireframe")	value >> mWireframe;
		else if (tag == "Serializable") value >> mSerializable;
		else if (value.IsString())
		{
			const String& s (value.AsString());

			if (tag == "Lighting")
			{
				if		(s == "One-sided")	mLighting = Lighting::OneSided;
				else if (s == "Two-sided")	mLighting = Lighting::TwoSided;
				else						mLighting = Lighting::None;
			}
			else if (tag == "Blending")
			{
				if		(s == "Normal")		mBlending = Blending::Normal;
				else if (s == "Modulate")	mBlending = Blending::Modulate;
				else if (s == "Add")		mBlending = Blending::Add;
				else if (s == "Subtract")	mBlending = Blending::Subtract;
				else						mBlending = Blending::None;
			}
			else if (tag == "Culling")
			{
				if		(s == "Front")		mCulling = Culling::Front;
				else if (s == "Back")		mCulling = Culling::Back;
				else						mCulling = Culling::None;
			}
			else if (tag == "Sorting")
			{
				if		(s == "Back to Front")	mSorting = Sorting::BackToFront;
				else if	(s == "Front to Back")	mSorting = Sorting::FrontToBack;
				else							mSorting = Sorting::None;
			}
		}
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool GLTechnique::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild("Technique");
		node.mValue = mName;

		const char* lighting = "None";
		if		(mLighting == Lighting::OneSided)	lighting = "One-sided";
		else if	(mLighting == Lighting::TwoSided)	lighting = "Two-sided";

		const char* blending = "None";
		if		(mBlending == Blending::Normal)		blending = "Normal";
		else if (mBlending == Blending::Modulate)	blending = "Modulate";
		else if (mBlending == Blending::Add)		blending = "Add";
		else if (mBlending == Blending::Subtract)	blending = "Subtract";

		const char* culling = "None";
		if		(mCulling == Culling::Front)		culling = "Front";
		else if	(mCulling == Culling::Back)			culling = "Back";

		const char* sorting = "None";
		if		(mSorting == Sorting::BackToFront)	sorting = "Back to Front";
		else if	(mSorting == Sorting::FrontToBack)	sorting = "Front to Back";

		node.AddChild("Fog", mFog);
		node.AddChild("Depth Write", mDepthWrite);
		node.AddChild("Depth Test", mDepthTest);
		node.AddChild("Color Write", mColorWrite);
		node.AddChild("Alpha Test", mAlphaTest);
		node.AddChild("Wireframe", mWireframe);
		node.AddChild("Lighting", lighting);
		node.AddChild("Blending", blending);
		node.AddChild("Culling", culling);
		node.AddChild("Sorting", sorting);

		return true;
	}
	return false;
}