#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
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

		inline float GetWidth()  const { return mRight - mLeft; }
		inline float GetHeight() const { return mBottom - mTop; }

		inline void Set (Type left, Type right, Type top, Type bottom)
		{
			mLeft   = left;
			mRight  = right;
			mTop	 = top;
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

	inline void SetLeft		(float relative, float absolute)	{   mRelativeLeft.Set(relative, absolute);  mDimsChanged  = true; }
	inline void SetTop		(float relative, float absolute)	{    mRelativeTop.Set(relative, absolute);  mDimsChanged  = true; }
	inline void SetRight	(float relative, float absolute)	{  mRelativeRight.Set(relative, absolute);  mDimsChanged  = true; }
	inline void SetBottom	(float relative, float absolute)	{ mRelativeBottom.Set(relative, absolute);  mDimsChanged  = true; }
	inline void SetAlpha	(float relative)					{ mRelativeAlpha = relative; }

	void SetRect (float x, float y, float w, float h);

	inline const Rectangle<float>& GetRect() const { return mRect; }
	
	// Convenience functions
	inline float GetLeft()	 const	{ return mRect.mLeft;		}
	inline float GetRight()	 const	{ return mRect.mRight;		}
	inline float GetTop()	 const	{ return mRect.mTop;		}
	inline float GetBottom() const	{ return mRect.mBottom;		}
	inline float GetWidth()	 const	{ return mRect.GetWidth();	}
	inline float GetHeight() const	{ return mRect.GetHeight();	}
	inline float GetAlpha()	 const	{ return mAlpha;			}

	// Relative anchors
	inline UIAnchor& GetRelativeLeft()		{ return mRelativeLeft;		}
	inline UIAnchor& GetRelativeRight()		{ return mRelativeRight;	}
	inline UIAnchor& GetRelativeTop()		{ return mRelativeTop;		}
	inline UIAnchor& GetRelativeBottom()	{ return mRelativeBottom;	}

	inline const UIAnchor& GetRelativeLeft()	const	{ return mRelativeLeft;		}
	inline const UIAnchor& GetRelativeRight()	const	{ return mRelativeRight;	}
	inline const UIAnchor& GetRelativeTop()		const	{ return mRelativeTop;		}
	inline const UIAnchor& GetRelativeBottom()	const	{ return mRelativeBottom;	}

	// Used in animation
	inline float GetParentAlpha()	 const	{ return mParentAlpha;	 }
	inline float GetRelativeAlpha()  const	{ return mRelativeAlpha; }

	// Changes the absolute alpha until the next Update()
	inline void OverrideAlpha (float val) { mAlpha = val; }

	// Check to see if the position lies inside the region
	bool Contains (const Vector2i& pos) const;

	// Whether the region is actually visible (lies at least partially within the parent's)
	inline bool IsVisible() const { return (mIsVisible && mAlpha > 0.001f); }

	// Allows to adjust relative coordinates by the specified amount
	void Adjust (float left, float top, float right, float bottom);

private:

	friend class UIArea;

	// Serialization
	bool Load (const String& tag, const Variable& value);
	void SerializeTo (TreeNode& root) const;

public:

	// Convenience function for the top-level areas
	bool Update (const Vector2i& size, bool forceUpdate);

	// Updates the region's position and size based on the provided parent's values
	// Returns 'true' if the region has changed
	bool Update (const UIRegion& parent, bool forceUpdate = false);
};