#include "../Include/_All.h"
using namespace R5;

//==============================================================================================
// Changes the associated texture
//==============================================================================================

void UISkin::SetTexture (const ITexture* tex)
{
	if (mTex != tex)
	{
		if (mTex) mUI->_TextureChanged(mTex);
		mTex = tex;
		if (mTex) mUI->_TextureChanged(mTex);
	}
}

//==============================================================================================
// Retrieves a face with the specified name
//==============================================================================================

const UIFace* UISkin::GetFace (const String& name) const
{
	const UIFace* face(0);

	if (name.IsValid())
	{
		// Try to find the requested face
		for (uint i = 0; i < mFaces.GetSize(); ++i)
		{
			if (mFaces[i]->mName == name)
			{
				face = mFaces[i];
				break;
			}
		}
	}
	return face;
}

//==============================================================================================
// Retrieves or creates a face with the specified name
//==============================================================================================

UIFace* UISkin::GetFace(const String& name, bool create)
{
	UIFace* face(0);

	// Try to find the requested face
	for (uint i = 0; i < mFaces.GetSize(); ++i)
	{
		if (mFaces[i]->mName == name)
		{
			face = mFaces[i];
			break;
		}
	}

	// If not found, create and add it
	if (create && face == 0)
	{
		face = new UIFace();
		face->mName = name;
		mFaces.Expand() = face;
	}
	return face;
}

//==============================================================================================
// Serialization -- Load
//==============================================================================================

bool UISkin::SerializeFrom (const TreeNode& root)
{
	mSerializable = true;
	bool skinChanged (false);

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if (tag == "Texture")
		{
			ITexture* tex = mUI->GetTexture( value.IsString() ? value.AsString() : value.GetString() );

			if (mTex != tex)
			{
				if (mTex) mUI->_TextureChanged(mTex);
				mTex = tex;
				tex->SetFiltering( ITexture::Filter::Linear );
				tex->SetSerializable(false);
				skinChanged = true;
			}
		}
		else if (tag == "Face")
		{
			GetFace( value.IsString() ? value.AsString() : value.GetString() )->SerializeFrom(node);
			skinChanged = true;
		}
		else if (tag == "Serializable")
		{
			value >> mSerializable;
		}
	}

	if (skinChanged)
	{
		if (mTex) mUI->_TextureChanged(mTex);
	}
	return true;
}

//==============================================================================================
// Serialization -- Save
//==============================================================================================

bool UISkin::SerializeTo (TreeNode& root) const
{
	if (mSerializable && mTex != 0)
	{
		TreeNode& node = root.AddChild("Skin", mName);
		node.AddChild("Texture", mTex->GetName());
		for (uint i = 0; i < mFaces.GetSize(); ++i) mFaces[i]->SerializeTo(node);
		return true;
	}
	return false;
}