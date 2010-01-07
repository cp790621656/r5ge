#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Basic anchored 2D region with a depth component
//============================================================================================================

class UIRegion
{
public:

	template <typename Type>
	struct Rectangle
	{
		Type mLeft;
		Type mRight;
		Type mTop;
		Type mBottom;

		Rectangle() : mLeft(0), mRight(0), mTop(0), mBottom(0) {}

		inline Type GetWidth()  const { return mRight - mLeft; }
		inline Type GetHeight() const { return mBottom - mTop; }

		inline void Set (Type left, Type right, Type top, Type bottom)
		{
			mLeft   = left;
			mRight  = right;
			mTop	= top;
			mBottom = bottom;
		}
	};

private:

	UIAnchor	mRelativeLeft;		// Anchored left side
	UIAnchor	mRelativeTop;		// Anchored top side
	UIAnchor	mRelativeRight;		// Anchored right side
	UIAnchor	mRelativeBottom;	// Anchored bottom side
	float		mRelativeAlpha;		// Alpha is cumulative, parent*this
	float		mParentAlpha;		// Saved parent alpha, used in animation
	bool		mDimsChanged;		// Whether the region's dimensions has been modified and will need to be updated

private:

	Rectangle<float> mRect;			// Calculated dimensions with floating-point precision
	float			 mAlpha;		// Calculated alpha
	bool			 mIsVisible;	// Whether the region is visible according to dimensions

public:

	UIRegion() :  mRelativeLeft	(0.0f, 0),
				mRelativeTop	(0.0f, 0),
				mRelativeRight	(1.0f, 0),
				mRelativeBottom	(1.0f, 0),
				mRelativeAlpha	(1.0f),
				mParentAlpha	(1.0f),
				mDimsChanged	(true),
				mAlpha			(1.0f),
				mIsVisible		(false) {}

public:

	// Set the relative-to-parent coordinates using the specified absolute offset values
	void SetRect (float x, float y, float w, float h);

	// Write access to relative values
	void SetLeft	(float relative, float absolute)	{   mRelativeLeft.Set(relative, absolute);  mDimsChanged  = true; }
	void SetTop		(float relative, float absolute)	{    mRelativeTop.Set(relative, absolute);  mDimsChanged  = true; }
	void SetRight	(float relative, float absolute)	{  mRelativeRight.Set(relative, absolute);  mDimsChanged  = true; }
	void SetBottom	(float relative, float absolute)	{ mRelativeBottom.Set(relative, absolute);  mDimsChanged  = true; }
	void SetAlpha	(float relative)					{ mRelativeAlpha = relative; }

	// Read access to relative values
	const UIAnchor& GetLeft()	const	{ return mRelativeLeft;		}
	const UIAnchor& GetRight()	const	{ return mRelativeRight;	}
	const UIAnchor& GetTop()	const	{ return mRelativeTop;		}
	const UIAnchor& GetBottom()	const	{ return mRelativeBottom;	}
	float			GetAlpha()	const	{ return mRelativeAlpha;	}

	// Retrieves the calculated absolute rectangle
	const Rectangle<float>& GetCalculatedRect() const { return mRect; }

	// Access to calculated absolute values
	float GetCalculatedLeft()	const	{ return mRect.mLeft;		}
	float GetCalculatedRight()	const	{ return mRect.mRight;		}
	float GetCalculatedTop()	const	{ return mRect.mTop;		}
	float GetCalculatedBottom()	const	{ return mRect.mBottom;		}
	float GetCalculatedWidth()	const	{ return mRect.GetWidth();	}
	float GetCalculatedHeight()	const	{ return mRect.GetHeight();	}
	float GetCalculatedAlpha()	const	{ return mAlpha;			}

	// Used in animation
	float GetParentAlpha()		const	{ return mParentAlpha;		}

	// Changes the absolute alpha until the next Update()
	void OverrideAlpha (float val) { mAlpha = val; }

	// Check to see if the position lies inside the region
	bool Contains (const Vector2i& pos) const;

	// Whether the region is actually visible (lies at least partially within the parent's)
	bool IsVisible() const { return (mIsVisible && mAlpha > 0.001f); }

	// Allows to adjust relative coordinates by the specified amount
	void Adjust (float left, float top, float right, float bottom);

private:

	friend class UIArea;

	// Serialization
	bool OnSerializeFrom (const TreeNode& node);
	void OnSerializeTo (TreeNode& node) const;

public:

	// Convenience function for the top-level areas
	bool Update (const Vector2i& size, bool forceUpdate);

	// Updates the region's position and size based on the provided parent's values
	// Returns 'true' if the region has changed
	bool Update (const UIRegion& parent, bool forceUpdate = false);
};