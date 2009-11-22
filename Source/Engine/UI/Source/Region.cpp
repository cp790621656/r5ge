#include "../Include/_All.h"
using namespace R5;

//====================================================================================================
// Convenience function
//====================================================================================================

inline bool ToAnchor (const Variable& value, Anchor& a)
{
	Vector2f v;
	
	if (value >> v)
	{
		a.mRelative = v.x;
		a.mAbsolute = v.y;
		return true;
	}
	return false;
}

//====================================================================================================
// Mostly a convenience function to quickly set absolute position and size
//====================================================================================================

void Region::SetRect (float x, float y, float w, float h)
{
	mRelativeLeft.Set(0.0f, x);
	mRelativeTop.Set(0.0f, y);
	mRelativeRight.Set(0.0f, x + w);
	mRelativeBottom.Set(0.0f, y + h);
	mDimsChanged = true;
}

//====================================================================================================
// Check to see if the position lies inside the region
//====================================================================================================

bool Region::Contains (const Vector2i& pos) const
{
	if ( pos.x < mRect.mLeft   ) return false;
	if ( pos.y < mRect.mTop    ) return false;
	if ( pos.x > mRect.mRight  ) return false;
	if ( pos.y > mRect.mBottom ) return false;
	return true;
}

//====================================================================================================
// Allows to adjust relative coordinates by the specified amount
//====================================================================================================

void Region::Adjust (float left, float top, float right, float bottom)
{
	mRelativeLeft.mAbsolute	+= left;
	mRelativeTop.mAbsolute	+= top;
	mRelativeRight.mAbsolute	+= right;
	mRelativeBottom.mAbsolute += bottom;
	mDimsChanged = true;
}

//====================================================================================================
// Serialization -- Load
//====================================================================================================

bool Region::Load (const String& tag, const Variable& value)
{
	if		( tag == "Left"	  && ToAnchor(value, mRelativeLeft)		)	return mDimsChanged  = true;
	else if ( tag == "Right"  && ToAnchor(value, mRelativeRight)	)	return mDimsChanged  = true;
	else if ( tag == "Top"	  && ToAnchor(value, mRelativeTop)		)	return mDimsChanged  = true;
	else if ( tag == "Bottom" && ToAnchor(value, mRelativeBottom)	)	return mDimsChanged  = true;
	else if ( tag == "Alpha"  && value >> mRelativeAlpha			)	return true;
	return false;
}

//====================================================================================================
// Serialization -- Save
//====================================================================================================

void Region::SerializeTo (TreeNode& root) const
{
	root.AddChild("Left",	Vector2f(mRelativeLeft.mRelative,	mRelativeLeft.mAbsolute));
	root.AddChild("Right",	Vector2f(mRelativeRight.mRelative,	mRelativeRight.mAbsolute));
	root.AddChild("Top",	Vector2f(mRelativeTop.mRelative,	mRelativeTop.mAbsolute));
	root.AddChild("Bottom", Vector2f(mRelativeBottom.mRelative,	mRelativeBottom.mAbsolute));
	root.AddChild("Alpha",	mRelativeAlpha);
}

//====================================================================================================
// Convenience function for the top-level areas
//====================================================================================================

bool Region::Update (const Vector2i& size, bool forceUpdate)
{
	static Region screen;
	screen.mRect.Set(0.0f, size.x, 0.0f, size.y);
	return Update(screen, forceUpdate);
}

//====================================================================================================
// Updates the region's position and size based on the provided parent's values
//====================================================================================================

bool Region::Update (const Region& parent, bool forceUpdate)
{
	bool changed (forceUpdate);
	bool wasVisible ( mIsVisible && mAlpha > 0.001f );

	// Calculate the current alpha
	mParentAlpha = parent.GetAlpha();
	float alpha = mParentAlpha * mRelativeAlpha;

	// Update the alpha
	if ( Float::IsNotEqual (mAlpha, alpha) )
	{
		changed = true;
		mAlpha	= alpha;
	}

	// Update the dimensions
	if (forceUpdate || mDimsChanged)
	{
		const Rectangle<float>& pr (parent.GetRect());

		float width  = pr.GetWidth();
		float height = pr.GetHeight();
		float left	 = pr.mLeft +   mRelativeLeft.mAbsolute +   mRelativeLeft.mRelative * width;
		float right  = pr.mLeft +  mRelativeRight.mAbsolute +  mRelativeRight.mRelative * width;
		float top	 = pr.mTop  +    mRelativeTop.mAbsolute +    mRelativeTop.mRelative * height;
		float bottom = pr.mTop  + mRelativeBottom.mAbsolute + mRelativeBottom.mRelative * height;

		if (right < left) right = left;
		if (bottom < top) bottom = top;

		// Update the dimension change flag
		mDimsChanged =	Float::IsNotEqual(left,   mRect.mLeft)	||
						Float::IsNotEqual(right,  mRect.mRight)	||
						Float::IsNotEqual(top,	  mRect.mTop)		||
						Float::IsNotEqual(bottom, mRect.mBottom);

		// Check to see if the area is really visible according to dimensions
		bool isVisible = (!(left	> pr.mRight	||
							right	< pr.mLeft		||
							top		> pr.mBottom	||
							bottom	< pr.mTop))	&&
							right	> left			&&
							bottom	> top;

		// If dimensions or visibility changed, 
		if (mDimsChanged || (wasVisible != (isVisible && mAlpha > 0.001f)))
		{
			changed = true;
			mIsVisible = isVisible;
			mRect.Set(left, right, top, bottom);
		}
	}

	// Set the new visibility flag and reset the change flags
	mDimsChanged = false;
	return changed;
}