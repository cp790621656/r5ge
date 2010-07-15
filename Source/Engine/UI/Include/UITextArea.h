#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Text Area containing paragraphs of text
//============================================================================================================

class UITextArea : public UIWidget
{
public:

	// Text drawing style
	struct Style
	{
		enum
		{
			Normal	= 0,
			Chat	= 1,
		};
	};

protected:

	// Single paragraph entry
	struct Paragraph
	{
		Color4ub		mColor;		// Text can be colored
		const IFont*	mFont;		// Pointer to the font being used
		String			mText;		// It would be quite odd if the text line was missing actual text
		bool			mShadow;	// Whether the text has a shadow outline
		ulong			mTime;		// Time when this paragraph was added

		Paragraph() : mColor(0xFFFFFFFF), mFont(0), mShadow(true), mTime(0) {}
		void Release() { mText.Release(); }
	};

	struct Line
	{
		UITextLine*	mLine;	// Generated line
		ulong		mTime;	// Time when this line was created
		float		mAlpha;	// Line's alpha

		Line() : mLine(0), mTime(0), mAlpha(0.0f) {}

		~Line()
		{
			if (mLine != 0)
			{
				delete mLine;
				mLine = 0;
			}
		}
	};

protected:

	Array<Paragraph>	mParagraphs;	// Paragraph entries
	Array<Line>			mLines;			// Generated lines based on the paragraphs above

	bool	mNeedsRebuild;	// Whether the paragraph lines need to be rebuilt
	float	mScroll;		// Current scrolling amount determining visible text
	uint	mStyle;			// Whether chat style paragraphs are used (bottom to top)
	uint	mHeight;		// Combined height of lines, in pixels
	uint	mMaxParagraphs;	// Maximum allowed number of paragraphs before old ones get removed
	ulong	mFadeDelay;		// Delay until text lines begin to fade
	float	mFadeDuration;	// How long it takes lines to fade out
	ulong	mMinTime;		// Minimum timestamp used for line fading (scrolling resets this)

public:

	UITextArea() :
		mScroll			(0.0f),
		mNeedsRebuild	(false),
		mStyle			(Style::Normal),
		mHeight			(0),
		mMaxParagraphs	(255),
		mFadeDelay		(0),
		mFadeDuration	(3.0f),
		mMinTime		(0) {}

	uint	GetMaxParagraphs()	const { return mMaxParagraphs;	}
	uint	GetStyle()			const { return mStyle;			}
	float	GetScroll()			const { return mScroll;			}

	// Clears all text
	void Clear();

	// Maximum number of allowed paragraphs
	void SetMaxParagraphs (uint val) { mMaxParagraphs = val; }

	// Adds a new paragraph of specified font and color
	void AddParagraph (const String& text, const Color4ub& color = 0xFFFFFFFF, bool shadow = false, const IFont* font = 0);

	// Paragraph drawing style (normal being top-down, chatbox being bottom up, etc)
	void SetStyle (uint style) { mStyle = style; _MarkAllTexturesAsDirty(); }

	// Scroll amount
	void SetScroll (float val);

	// Delay until the text starts to fade out (by default this feature is turned off)
	void SetFadeDelay (float delay) { mFadeDelay = Float::RoundToUInt(delay * 1000.0f); }

	// How long it takes for lines to fade out
	void SetFadeDuration (float seconds) { mFadeDuration = seconds; if (mFadeDuration < 0.01f) mFadeDuration = 0.01f; }

	// Resets the fade timer of all visible lines
	void ResetFadeTimer() { mMinTime = Time::GetMilliseconds(); }

private:

	// INTERNAL: Marks all associated textures as dirty
	void _MarkAllTexturesAsDirty();
	void _MarkVisibleTexturesAsDirty();

	// INTERNAL: Rebuilds the printed lines
	void _Rebuild (uint offset);

public:

	// Area creation
	R5_DECLARE_INHERITED_CLASS("UITextArea", UITextArea, UIWidget, UIWidget);

	// Area functions
	virtual void SetDirty() { _MarkVisibleTexturesAsDirty(); }
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);
	virtual void OnScroll (const Vector2i& pos, float delta);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};