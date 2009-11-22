#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Clear all text
//============================================================================================================

void TextArea::Clear()
{
	SetDirty();

	mParagraphs.Lock();
	mParagraphs.Clear();
	mParagraphs.Unlock();

	mLines.Lock();
	mLines.Clear();
	mLines.Unlock();

	mHeight = 0;
}

//============================================================================================================
// Add a single paragraph
//============================================================================================================

void TextArea::AddParagraph (const String& text, const IFont* font, const Color3f& color, bool shadow)
{
	if (font != 0)
	{
		mParagraphs.Lock();
		{
			while (mParagraphs.GetSize() > mMaxParagraphs)
			{
				mNeedsRebuild = true;

				// NOTE: This is a dangerous operation. Array::RemoveAt actually moves memory over,
				// without doing any copying. The advantage is that it's quick. The obvious glaring
				// disadvantage is being able to screw up *very* easily. Be *very* careful whenever
				// modifying this part of the code in the future. Strings must be released manually.

				mParagraphs[0].Release();
				mParagraphs.RemoveAt(0);
			}

			Paragraph& par = mParagraphs.Expand();
			par.mText   = text;
			par.mFont   = font;
			par.mColor  = color;
			par.mShadow = shadow;

			// Only rebuild the very last paragraph if a full rebuild is not already needed
			if (!mNeedsRebuild)
			{
				_Rebuild(mParagraphs.GetSize() - 1);
			}
		}
		mParagraphs.Unlock();

		// Mark all associated textures as dirty
		_MarkTexturesAsDirty();
	}
}

//============================================================================================================
// Set the scroll amount
//============================================================================================================

void TextArea::SetScroll (float val)
{
	val = Float::Clamp(val, 0.0f, 1.0f);

	if (mScroll != val && mLines.IsValid())
	{
		mScroll = val;
		_MarkTexturesAsDirty();
	}
}

//============================================================================================================
// Mark all associated textures as dirty
//============================================================================================================

void TextArea::_MarkTexturesAsDirty()
{
	mParagraphs.Lock();
	{
		for (uint i = 0; i < mParagraphs.GetSize(); ++i)
		{
			Paragraph& par = mParagraphs[i];

			if (par.mFont != 0)
			{
				const ITexture* tex = par.mFont->GetTexture();

				if (tex != 0)
				{
					OnDirty(tex);
				}
			}
		}
	}
	mParagraphs.Unlock();
}

//============================================================================================================
// INTERNAL: Rebuilds the drawn lines
//============================================================================================================

void TextArea::_Rebuild (uint offset)
{
	mNeedsRebuild = false;

	uint width = Float::FloorToUInt(mRegion.GetWidth());
	String currentText;

	// Run through the paragraphs and format the lines
	for (uint i = offset; i < mParagraphs.GetSize(); ++i)
	{
		Paragraph& par = mParagraphs[i];

		if (par.mFont != 0 && par.mFont->IsValid())
		{
			uint offset = 0, count = 0;
			Color4ub color (par.mColor);

			// Count the number of characters that would fit if we rendered this line
			while (count = par.mFont->CountChars(par.mText, width, offset))
			{
				// Read one line from the current paragraph
				uint blockEnd = par.mText.GetPhrase(currentText, offset, offset + count);

				// Add this line's height to the total
				mHeight += Float::RoundToUInt(1.15f * par.mFont->GetSize());

				// Create a new line
				TextLine* line = (TextLine*)TextLine::_CreateNew();

				// Set the root first
				line->_SetRootPtr(mRoot);

				// Update the line's properties
				line->SetText(currentText);
				line->SetFont(par.mFont);
				line->SetColor(color);
				line->SetShadow(par.mShadow);
				line->SetLayer(mLayer);

				// Add the line, without ever setting the parent (on purpose)
				mLines.Expand() = line;

				// Update the font's final color
				par.mFont->UpdateColor(par.mText, color, offset, blockEnd);

				// Update the offset
				offset = blockEnd;
			}
		}
	}
}

//============================================================================================================
// Not only do textures get marked, but the lines have to be rebuilt
//============================================================================================================

void TextArea::SetDirty()
{
	mNeedsRebuild = true;
	_MarkTexturesAsDirty();
}

//============================================================================================================
// Rebuild the lines if necessary and fill the draw queues
//============================================================================================================

void TextArea::OnFill (Queue* queue)
{
	if (queue->mLayer != mLayer || queue->mArea != 0) return;

	mParagraphs.Lock();
	{
		if (mNeedsRebuild)
		{
			mLines.Clear();
			_Rebuild(0);
		}

		if (mLines.IsValid())
		{
			// Visible region's height
			uint maxHeight = Float::FloorToUInt(mRegion.GetHeight());
			uint offset = 0;

			// How many lines to skip
			uint skip = Float::RoundToUInt(mScroll * mLines.GetSize());
			uint max = mLines.GetSize() - 1;

			// Don't go past the last line
			if (skip > max) skip = max;

			// Go through visible lines starting at the offset
			if (mStyle == Style::Chat)
			{
				for (uint i = skip; i < mLines.GetSize(); ++i)
				{
					TextLine* line = mLines[mLines.GetSize() - 1 - i];
					uint lineHeight = Float::RoundToUInt(1.15f * line->GetFont()->GetSize());
					uint nextOffset = offset + lineHeight;

					if (line->GetTexture() == queue->mTex)
					{
						if (nextOffset < maxHeight)
						{
							Region& rgn = line->GetRegion();

							// Calculate the line's regional dimensions
							rgn.SetTop(1.0f, -(float)nextOffset);
							rgn.SetBottom(1.0f, -(float)offset);

							// Update the region
							rgn.Update(mRegion);

							// Override the alpha to make it consistent
							rgn.OverrideAlpha(mRegion.GetAlpha());

							// Fill the line
							line->OnFill(queue);
						}
						else break;
					}

					// Adjust the offset
					offset = nextOffset;
				}
			}
			else
			{
				for (uint i = skip; i < mLines.GetSize(); ++i)
				{
					TextLine* line = mLines[i];
					uint lineHeight = Float::RoundToUInt(1.15f * line->GetFont()->GetSize());
					uint nextOffset = offset + lineHeight;

					if (line->GetTexture() == queue->mTex)
					{
						if (nextOffset < maxHeight)
						{
							Region& rgn = line->GetRegion();

							// Calculate the line's regional dimensions
							rgn.SetTop(0.0f, (float)offset);
							rgn.SetBottom(0.0f, (float)nextOffset);

							// Update the region
							rgn.Update(mRegion);

							// Override the alpha to make it consistent
							rgn.OverrideAlpha(mRegion.GetAlpha());

							// Fill the line
							line->OnFill(queue);
						}
						else break;
					}

					// Adjust the offset
					offset = nextOffset;
				}
			}
		}
	}
	mParagraphs.Unlock();
}

//============================================================================================================
// Scrolling should affect the text view
//============================================================================================================

bool TextArea::OnScroll (const Vector2i& pos, float delta)
{
	if (Area::OnScroll(pos, delta)) return true;

	if (mLines.IsValid())
	{
		// Number of skipped lines
		float skip = mScroll * mLines.GetSize();

		// Chat style scrolls backwards
		if (mStyle == Style::Chat)	skip += delta;
		else						skip -= delta;

		// Don't go past the last line
		skip = Float::Clamp(skip, 0.0f, mLines.GetSize() - 1.0f);

		// Set the new scrolling value
		SetScroll( skip / mLines.GetSize() );
	}
	return true;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool TextArea::CustomSerializeFrom(const TreeNode& root)
{
	if (root.mTag == "Style" && root.mValue.IsString())
	{
		if (root.mValue.AsString() == "Chat") mStyle = Style::Chat;
		else mStyle = Style::Normal;
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void TextArea::CustomSerializeTo(TreeNode& root) const
{
	root.AddChild("Style", (mStyle == Style::Chat) ? "Chat" : "Normal");
}