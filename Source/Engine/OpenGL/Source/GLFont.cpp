#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Try to load the font file if it was specified in an intuitive format (ie: "Arial 14")
//============================================================================================================

GLFont::GLFont (const String& name, IGraphics* graphics) : mName(name), mGraphics(graphics), mTex(0), mSerializable(false)
{
	if (name.IsValid())
	{
		String sFont, sSize;
		uint fontIndex = name.GetWord(sFont);
		name.GetWord(sSize, fontIndex);

		uint fontSize (15);
		if (sSize.IsValid()) sSize >> fontSize;

		if (fontSize > 0)
		{
			// If the font loads fine just based on the name alone, don't bother serializing it
			mSerializable = !Load(sFont, fontSize);
		}
	}
}

//============================================================================================================
// INTERNAL: Updates the texture
//============================================================================================================

bool GLFont::_Reload()
{
	const Memory& buffer (mFont.GetBuffer());

	if (buffer.GetSize() > 0)
	{
		uint width (mFont.GetWidth());
		mSerializable = true;
		
		if (mTex == 0) mTex = mGraphics->GetTexture(mName);

		mTex->Set( buffer.GetBuffer(), width, width, 1, ITexture::Format::Luminance );
		mTex->SetFiltering( ITexture::Filter::Nearest );
		mTex->SetSerializable(false);

		mFont.Release();
		return true;
	}
	return false;
}

//============================================================================================================
// Release all used memory
//============================================================================================================

void GLFont::Release()
{
	mLock.Lock();
	{
		mSerializable = false;
		mFont.Release();

		if (mTex != 0)
		{
			mTex->Release();
			mTex = 0;
		}
	}
	mLock.Unlock();
}

//============================================================================================================
// Loads the specified font file, creating a font of specified size
//============================================================================================================

bool GLFont::Load (const String& filename, byte fontSize, byte padding)
{
	mLock.Lock();
	bool retVal = mFont.Load(filename, fontSize, padding) && _Reload();
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Create the font using the specified input memory buffer and font size
//============================================================================================================

bool GLFont::Load (const byte* buffer, uint bufferSize, byte fontSize, byte padding)
{
	mLock.Lock();
	bool retVal = mFont.Load(buffer, bufferSize, fontSize, padding) && _Reload();
	mLock.Unlock();
	return retVal;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool GLFont::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	mLock.Lock();
	{
		mSerializable = true;
		String source;
		uint size (0);

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if		(tag == "Source")		value >> source;
			else if (tag == "Size")			value >> size;
			else if (tag == "Serializable") value >> mSerializable;
		}

		if (source.IsValid() && size > 0 && (!IsValid() || forceUpdate) && mFont.Load(source, size))
			_Reload();
	}
	mLock.Unlock();
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool GLFont::SerializeTo (TreeNode& root) const
{
	if (!IsValid() || !mSerializable || mFont.GetSource().IsEmpty() || mFont.GetSize() == 0)
		return false;

	TreeNode& node = root.AddChild("Font", mName);
	node.AddChild("Source", mFont.GetSource());
	node.AddChild("Size", mFont.GetSize());
	return true;
}