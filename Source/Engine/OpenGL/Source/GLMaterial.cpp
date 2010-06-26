#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Clears the material, resetting everything to default values
//============================================================================================================

void GLMaterial::Release()
{
	mDiffuse.Clear();
	mSpecular = Color4f(0.0f, 0.0f, 0.0f, 0.2f);

	mGlow			= 0.0f;
	mAdt			= 0.003921568627451f;
	mSerializable	= false;

	mMethods.Lock();
	mMask = 0;
	mMethods.Clear();
	mMethods.Unlock();
}

//============================================================================================================
// Material rendering parameters differ for every technique
//============================================================================================================

IMaterial::DrawMethod*  GLMaterial::GetDrawMethod (const ITechnique* t, bool createIfMissing)
{
	DrawMethod* ptr (0);

	if (t != 0)
	{
		mMethods.Lock();
		{
			// Go through all material option entries
			for (uint i = 0; i < mMethods.GetSize(); ++i)
			{
				if (mMethods[i].GetTechnique() == t)
				{
					ptr = &mMethods[i];
					break;
				}
			}

			if (ptr == 0 && createIfMissing)
			{
				mSerializable = true;
				mMask |= t->GetMask();

				// Try to reuse unassigned entries first
				for (uint i = 0; i < mMethods.GetSize(); ++i)
				{
					if (mMethods[i].GetTechnique() == 0)
					{
						ptr = &mMethods[i];
						ptr->SetTechnique(t);
						break;
					}
				}

				// Nothing to reuse -- add a new entry
				if (ptr == 0)
				{
					ptr = &mMethods.Expand();
					ptr->SetTechnique(t);
				}
			}
		}
		mMethods.Unlock();
	}
	return ptr;
}

//============================================================================================================
// Returns a rendering method associated with the technique, but only if it's actually visible
//============================================================================================================

const IMaterial::DrawMethod* GLMaterial::GetVisibleMethod (const ITechnique* t) const
{
	if (mDiffuse.GetColor4ub().a == 0) return 0;
	GLMaterial* ptr = const_cast<GLMaterial*>(this);
	return ptr->GetDrawMethod(t, false);
}

//============================================================================================================
// Deletes a technique parameter entry
//============================================================================================================

void GLMaterial::DeleteDrawMethod (const ITechnique* t)
{
	if (t != 0)
	{
		mMethods.Lock();
		{
			// Go through all material option entries
			for (uint i = 0; i < mMethods.GetSize(); ++i)
			{
				DrawMethod& ren = mMethods[i];

				if (ren.GetTechnique() == t)
				{
					mMask &= ~(t->GetMask());
					ren.Clear();
					break;
				}
			}
		}
		mMethods.Unlock();
	}
}

//============================================================================================================
// Clears all current rendering methods
//============================================================================================================

void GLMaterial::ClearAllDrawMethods()
{
	if (mMethods.IsValid())
	{
		mMethods.Lock();
		{
			mMethods.Clear();
			mMask = 0;
		}
		mMethods.Unlock();
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void SerializeMethodFrom (IMaterial::DrawMethod& m, const TreeNode& root, IGraphics* graphics)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if (tag == IShader::ClassID())
		{
			m.SetShader( (value.IsValid()) ? graphics->GetShader(
				value.IsString() ? value.AsString() : value.GetString()) : 0 );
		}
		else if (tag == "Textures")
		{
			if (value.IsStringArray())
			{
				const Array<String>& sa = value.AsStringArray();
				for (uint i = 0; i < sa.GetSize(); ++i) m.SetTexture(i, graphics->GetTexture(sa[i]));
			}
		}
		// LEGACY SUPPORT, WILL BE REMOVED
		else if (tag.BeginsWith(ITexture::ClassID()))
		{
			String left, right;
			uint textureUnit = 0;

			if ( tag.Split(left, ' ', right) && left == ITexture::ClassID() && right >> textureUnit )
			{
				m.SetTexture( textureUnit, graphics->GetTexture(
					value.IsString() ? value.AsString() : value.GetString()) );
			}
		}
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void SerializeMethodTo (const IMaterial::DrawMethod& m, TreeNode& root)
{
	const ITechnique* tech = m.GetTechnique();

	if (tech != 0)
	{
		TreeNode& node = root.AddChild( ITechnique::ClassID(), tech->GetName() );

		const IShader* shader = m.GetShader();
		if (shader != 0) node.AddChild( IShader::ClassID(), shader->GetName() );

		const IMaterial::Textures& list = m.GetAllTextures();

		if (list.IsValid())
		{
			Array<String>& sa = node.AddChild("Textures").mValue.ToStringArray();

			list.Lock();
			{
				for (uint i = 0; i < list.GetSize(); ++i)
				{
					if (list[i] != 0) sa.Expand() = list[i]->GetName();
				}
			}
			list.Unlock();
		}
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool GLMaterial::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	// If the material has already been loaded, don't try to do so again
	if ( mMask != 0 && !forceUpdate ) return true;

	Color4f c;
	float f (0.0f);

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if ( tag == "Diffuse" || tag == "Color" )
		{
			if (value >> c) SetDiffuse(c);
		}
		else if ( tag == "Specular"	 )	{ if (value >> c) SetSpecular(c); }
		else if ( tag == "Glow"		 )	{ if (value >> f) SetGlow(f); }
		else if ( tag == "Shininess" )
		{
			if (value >> f)
			{
				Color4f spec (mSpecular.GetColor4f());
				spec.a = f / 128.0f;
				SetSpecular(spec);
			}
		}
		else if ( tag == "ADT" )
		{
			value >> mAdt;
		}
		else if ( tag == ITechnique::ClassID() )
		{
			const ITechnique* tech = mGraphics->GetTechnique(value.IsString() ? value.AsString() : value.GetString());
			DrawMethod* ren = GetDrawMethod(tech, true);
			SerializeMethodFrom(*ren, node, mGraphics);
		}
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool GLMaterial::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		TreeNode& node = root.AddChild( ClassID(), mName );

		node.AddChild("Diffuse", mDiffuse.GetColor4f());
		node.AddChild("Specular", mSpecular.GetColor4f());
		node.AddChild("Glow", mGlow);
		node.AddChild("ADT", mAdt);

		mMethods.Lock();
		{
			for (uint i = 0; i < mMethods.GetSize(); ++i)
				SerializeMethodTo(mMethods[i], node);
		}
		mMethods.Unlock();
	}
	return true;
}