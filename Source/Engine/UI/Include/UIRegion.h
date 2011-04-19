#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic anchored 2D region with an alpha
// Author: Michael Lyashenko
//============================================================================================================

class UIRegion
{
private:

	UIAnchor	mRelativeLeft;		// Anchored left side
	UIAnchor	mRelativeTop;		// Anchored top side
	UIAnchor	mRelativeRight;		// Anchored right side
	UIAnchor	mRelativeBottom;	// Anchored bottom side
	float		mRelativeAlpha;		// Alpha is cumulative, parent*this
	float		mParentAlpha;		// Saved parent alpha, used in animation
	bool		mDimsChanged;		// Whether the region's dimensions has been modified and will need to be updated
	bool		mUnscheduled;		// Whether our last update was unscheduled

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
				mUnscheduled	(false),
				mAlpha			(1.0f),
				mIsVisible		(false) {}

public:

	// Set the relative-to-parent coordinates using the specified absolute offset values
	void SetRect (float x, float y, float w, float h);

	// Write access to relative values
	void SetLeft	(float relative, float absolute)	{ mDimsChanged |= mRelativeLeft.Set(relative, absolute); }
	void SetTop		(float relative, float absolute)	{ mDimsChanged |= mRelativeTop.Set(relative, absolute); }
	void SetRight	(float relative, float absolute)	{ mDimsChanged |= mRelativeRight.Set(relative, absolute); }
	void SetBottom	(float relative, float absolute)	{ mDimsChanged |= mRelativeBottom.Set(relative, absolute); }
	void SetAlpha	(float relative)					{ mRelativeAlpha = Float::Clamp(relative, 0.0f, 1.0f); }
	void SetDirty() { mDimsChanged = true; }

	// Read access to relative values
	const UIAnchor& GetLeft()	const	{ return mRelativeLeft;		}
	const UIAnchor& GetRight()	const	{ return mRelativeRight;	}
	const UIAnchor& GetTop()	const	{ return mRelativeTop;		}
	const UIAnchor& GetBottom()	const	{ return mRelativeBottom;	}
	float			GetAlpha()	const	{ return mRelativeAlpha;	}

	// Retrieves the calculated absolute rectangle
	const Rectangle<float>& GetCalculatedRect() const { return mRect; }

	// Access to calculated absolute values
	float GetCalculatedLeft()	const	{ return mRect.left;		}
	float GetCalculatedRight()	const	{ return mRect.right;		}
	float GetCalculatedTop()	const	{ return mRect.top;			}
	float GetCalculatedBottom()	const	{ return mRect.bottom;		}
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
	bool IsVisible() const { return mIsVisible; }

	// Allows to adjust relative coordinates by the specified amount
	void Adjust (float left, float top, float right, float bottom);

private:

	friend class UIWidget;

	// Serialization
	bool OnSerializeFrom (const TreeNode& node);
	void OnSerializeTo (TreeNode& node) const;

public:

	// Perform an unscheduled update
	void Update (const UIRegion& parent) { Update(parent, true, false); }

	// Convenience function for the top-level areas
	bool Update (const Vector2i& size, bool forceUpdate);

	// Updates the region's position and size based on the provided parent's values
	// Returns 'true' if the region has changed
	bool Update (const UIRegion& parent, bool forceUpdate, bool scheduledUpdate);
};
