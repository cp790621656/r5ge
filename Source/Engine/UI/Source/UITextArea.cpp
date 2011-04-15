#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Clear all text
//============================================================================================================

void UITextArea::Clear()
{
	SetDirty();
	mParagraphs.Clear();
	mLines.Lock();
	mLines.Clear();
	mLines.Unlock();
	mHeight = 0;
}

//============================================================================================================
// Add a single paragraph
//============================================================================================================

void UITextArea::AddParagraph (const String& text, const Color4ub& textColor, const Color4ub& shadowColor, const IFont* font)
{
	if (font == 0) font = mUI->GetDefaultFont();

	if (font != 0)
	{
		mLines.Lock();
		{
			while (mParagraphs.GetSize() > mMaxParagraphs)
			{
				Paragraph& para = mParagraphs[0];

				const IFont* font = para.mFont;
				const ITexture* tex = (font == 0) ? 0 : font->GetTexture();

				if (tex != 0)
				{
					for (uint i = mLines.GetSize(); i > 0; )
					{
						// If this texture is currently in use, invalidate its draw queue
						if (mLines[--i].mLine->GetTexture() == tex)
						{
							mNeedsRebuild = true;
							OnDirty(tex);
							break;
						}
					}
				}
				
				// NOTE: This is a dangerous operation. Array::RemoveAt actually moves memory over,
				// without doing any copying. The advantage is that it's quick. The obvious glaring
				// disadvantage is being able to screw up *very* easily. Be *very* careful whenever
				// modifying this part of the code in the future. Strings must be released manually.

				para.Release();
				mParagraphs.RemoveAt(0);
			}

			Paragraph& par	 = mParagraphs.Expand();
			par.mText		 = text;
			par.mFont		 = font;
			par.mTextColor   = textColor;
			par.mShadowColor = shadowColor;
			par.mTime		 = Time::GetMilliseconds();

			// Only rebuild the very last paragraph if a full rebuild is not already needed
			if (!mNeedsRebuild)
			{
				_MarkVisibleTexturesAsDirty();
				_Rebuild(mParagraphs.GetSize() - 1);
				OnDirty(font->GetTexture());
			}
		}
		mLines.Unlock();
	}
}

//============================================================================================================
// Set the scroll amount
//============================================================================================================

void UITextArea::SetScroll (float val)
{
	val = Float::Clamp(val, 0.0f, 1.0f);

	if (mScroll != val && mLines.IsValid())
	{
		mScroll = val;
		mNeedsRebuild = true;
	}
}

//============================================================================================================
// Mark all associated textures as dirty
//============================================================================================================

void UITextArea::_MarkAllTexturesAsDirty()
{
	for (uint i = mParagraphs.GetSize(); i > 0; )
	{
		const Paragraph& par = mParagraphs[--i];

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

//============================================================================================================
// Marks all visible textures as dirty
//============================================================================================================

void UITextArea::_MarkVisibleTexturesAsDirty()
{
	const ITexture* last = 0;

	for (uint i = mLines.GetSize(); i > 0; )
	{
		const Line& entry = mLines[--i];

		if (entry.mLine->GetFont() != 0)
		{
			const ITexture* tex = entry.mLine->GetTexture();

			if (tex != 0 && tex != last)
			{
				last = tex;
				OnDirty(tex);
			}
		}
	}
}

//============================================================================================================
// INTERNAL: Rebuilds the drawn lines
//============================================================================================================

void UITextArea::_Rebuild (uint offset)
{
	uint width = Float::FloorToUInt(mRegion.GetCalculatedWidth());
	String currentText;

	// Run through the paragraphs and format the lines
	for (uint i = offset; i < mParagraphs.GetSize(); ++i)
	{
		Paragraph& par = mParagraphs[i];

		if (par.mFont != 0 && par.mFont->IsValid())
		{
			uint offset = 0, count = 0;
			Color4ub textColor (par.mTextColor);

			// Count the number of characters that would fit if we rendered this line
			while ((count = par.mFont->CountChars(par.mText, width, offset)))
			{
				// Read one line from the current paragraph
				uint blockEnd = par.mText.GetPhrase(currentText, offset, offset + count);

				// Add this line's height to the total
				mHeight += Float::RoundToUInt(1.15f * par.mFont->GetSize());

				Line& entry = mLines.Expand();
				
				if (entry.mLine == 0)
				{
					entry.mLine = (UITextLine*)UITextLine::_CreateNew();
					entry.mLine->_SetRootPtr(mUI);
				}

				// Set the time when the line was created
				entry.mTime = par.mTime;
				entry.mAlpha = 1.0f;

				// Update the line's properties
				entry.mLine->SetText(currentText);
				entry.mLine->SetFont(par.mFont);
				entry.mLine->SetTextColor(textColor);
				entry.mLine->SetShadowColor(par.mShadowColor);
				entry.mLine->SetLayer(mLayer);

				// Update the font's final color
				par.mFont->UpdateColor(par.mText, textColor, offset, blockEnd);

				// Update the offset
				offset = blockEnd;
			}
		}
	}
}

//============================================================================================================
// Runs through all visible lines and fades them out as necessary
//============================================================================================================

bool UITextArea::OnUpdate (bool dimensionsChanged)
{
	// If dimensions have changed we need to rebuild all lines
	if (dimensionsChanged) mNeedsRebuild = true;

	// If we have a scheduled rebuild, clear all lines and re-create them
	if (mNeedsRebuild)
	{
		mMinTime = Time::GetMilliseconds();
		mNeedsRebuild = false;

		mLines.Lock();
		{
			mLines.Clear();
			_MarkAllTexturesAsDirty();
			_Rebuild(0);
		}
		mLines.Unlock();
	}

	// If the fade delay has been set, we need to run through all lines and fade them as needed
	if (mFadeDelay != 0)
	{
		ulong ms = Time::GetMilliseconds();

		mLines.Lock();
		{
			for (uint i = mLines.GetSize(); i > 0; )
			{
				Line& line = mLines[--i];
				ulong time = line.mTime;

				// Adjust the time by the minimum and the delay
				if (mMinTime > time) time = mMinTime;
				time += mFadeDelay;

				// Calculate the alpha
				float alpha = (time > ms) ? 1.0f : 1.0f - (0.001f * (ms - time)) / mFadeDuration;
				if (alpha < 0.0f) alpha = 0.0f;

				// If dimensions or alpha have changed, mark the queue as dirty
				if (dimensionsChanged || alpha != line.mAlpha)
				{
					line.mAlpha = alpha;
					OnDirty(line.mLine->GetTexture());
				}
			}
		}
		mLines.Unlock();
	}
	// We handle our own SetDirty() calls above
	return false;
}

//============================================================================================================
// Rebuild the lines if necessary and fill the draw queues
//============================================================================================================

void UITextArea::OnFill (UIQueue* queue)
{
	if (queue->mLayer != mLayer || queue->mWidget != 0) return;

	if (mLines.IsValid())
	{
		mLines.Lock();
		{
			// Visible region's height
			uint maxHeight = Float::FloorToUInt(mRegion.GetCalculatedHeight());
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
					Line& entry = mLines[mLines.GetSize() - 1 - i];
					UITextLine* line = entry.mLine;
					ASSERT(line != 0, "Null line?");

					uint lineHeight = Float::RoundToUInt(1.15f * line->GetFont()->GetSize());
					uint nextOffset = offset + lineHeight;

					if (line->GetTexture() == queue->mTex && entry.mAlpha > 0.0f)
					{
						if (nextOffset < maxHeight)
						{
							UIRegion& rgn = line->GetRegion();

							// Calculate the line's regional dimensions
							rgn.SetTop(1.0f, -(float)nextOffset);
							rgn.SetBottom(1.0f, -(float)offset);

							// Update the region
							rgn.Update(mRegion, true, true);

							// Override the alpha to make it consistent
							rgn.OverrideAlpha(mRegion.GetCalculatedAlpha() * entry.mAlpha);

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
					Line& entry = mLines[i];
					UITextLine* line = entry.mLine;

					uint lineHeight = Float::RoundToUInt(1.15f * line->GetFont()->GetSize());
					uint nextOffset = offset + lineHeight;

					if (line->GetTexture() == queue->mTex && entry.mAlpha > 0.0f)
					{
						if (nextOffset < maxHeight)
						{
							UIRegion& rgn = line->GetRegion();

							// Calculate the line's regional dimensions
							rgn.SetTop(0.0f, (float)offset);
							rgn.SetBottom(0.0f, (float)nextOffset);

							// Update the region
							rgn.Update(mRegion, false, true);

							// Override the alpha to make it consistent
							rgn.OverrideAlpha(mRegion.GetCalculatedAlpha() * entry.mAlpha);

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
		mLines.Unlock();
	}
}

//============================================================================================================
// Scrolling should affect the text view
//============================================================================================================

void UITextArea::OnScroll (const Vector2i& pos, float delta)
{
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
	UIWidget::OnScroll(pos, delta);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UITextArea::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Style")
	{
		if (node.mValue.IsString())
		{
			if (node.mValue.AsString() == "Chat") mStyle = Style::Chat;
			else mStyle = Style::Normal;
		}
	}
	else if (node.mTag == "Fade Delay")
	{
		if (node.mValue.IsFloat())
		{
			SetFadeDelay(node.mValue.AsFloat());
		}
	}
	else if (node.mTag == "Fade Duration")
	{
		if (node.mValue.IsFloat())
		{
			SetFadeDuration(node.mValue.AsFloat());
		}
	}
	else return false;
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UITextArea::OnSerializeTo (TreeNode& node) const
{
	node.AddChild("Style", (mStyle == Style::Chat) ? "Chat" : "Normal");
	
	if (mFadeDelay != 0)
	{
		node.AddChild("Fade Delay", 0.001f * mFadeDelay);
		node.AddChild("Fade Duration", mFadeDuration);
	}
}
