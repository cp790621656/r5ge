#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Default values include 3% per millisecond friction for movement/rotation and 1% for dollying
//============================================================================================================

DebugCamera::DebugCamera() :
	mDolly			(0.0f, 0.0f, 50.0f),
	mFriction		(0.97f, 0.97f, 0.99f),
	mDollyMovement	(0.0f),
	mHasMovement	(false) {}

//============================================================================================================
// Stops the camera in its tracks
//============================================================================================================

void DebugCamera::Stop (bool animation)
{
	mHasMovement	= false;
	mPosMovement	= 0.0f;
	mRotMovement	= 0.0f;
	mDollyMovement	= 0.0f;

	if (animation)
	{
		mSplineV.Clear();
		mSplineQ.Clear();
		mSplineF.Clear();
	}
}

//============================================================================================================
// Animates the camera to the specified destination over the specified amount of seconds
//============================================================================================================

void DebugCamera::AnimateTo (const Vector3f& pos, const Quaternion& rot, float dolly, float delay)
{
	Stop(false);

	if (mSplineV.IsEmpty())
	{
		float time = Time::GetTime();
		mSplineV.AddKey(time, mRelativePos);
		mSplineQ.AddKey(time, mRelativeRot.GetForward());
		mSplineF.AddKey(time, mDolly.y);
	}
	mSplineV.AddKey(mSplineV.EndTime() + delay, pos);
	mSplineQ.AddKey(mSplineQ.EndTime() + delay, rot);
	mSplineF.AddKey(mSplineF.EndTime() + delay, dolly);
}

//============================================================================================================
// Updates the position and rotation based on the movement and returns the dolly-offset position
//============================================================================================================

Vector3f DebugCamera::_UpdateOffsetPosition()
{
	if (mSplineV.IsValid())
	{
		float time		= Time::GetTime();
		mRelativePos	= mSplineV.Sample(time);
		mRelativeRot	= mSplineQ.Sample(time);
		mDolly.y		= mSplineF.Sample(time);
		mDolly.y		= Float::Clamp(mDolly.y, mDolly.x, mDolly.z );
		mIsDirty		= true;

		// Stop the animation once the animation reaches the end
		if (time > mSplineV.EndTime()) Stop();
	}
	else if (mHasMovement)
	{
		mHasMovement = false;
		uint delta = (uint)Time::GetDeltaMS();

		Vector3f mov;
		Vector2f rot;
		float dolly(0.0f);

		for (uint i = 0; i < delta; ++i)
		{
			mov += mPosMovement;
			mPosMovement *= mFriction.x;

			rot += mRotMovement;
			mRotMovement *= mFriction.y;

			dolly += mDollyMovement;
			mDollyMovement *= mFriction.z;
		}

		// XYZ movement
		if (mov)
		{
			mHasMovement = true;
			mRelativePos += mov;
		}

		// Horizontal rotation
		if ( Float::IsNotZero(rot.x) )
		{
			mHasMovement = true;
			Quaternion rotQuat ( Vector3f(0.0f, 0.0f, 1.0f), rot.x );
			mRelativeRot = rotQuat * mRelativeRot;
			mRelativeRot.Normalize();
		}
		
		// Vertical rotation
		if ( Float::IsNotZero(rot.y) )
		{
			mHasMovement = true;
			Quaternion rotQuat ( mRelativeRot.GetRight(), rot.y );
			mRelativeRot = rotQuat * mRelativeRot;
			mRelativeRot.Normalize();
		}

		// Dolly factor
		if ( Float::IsNotZero(dolly) || mHasMovement )
		{
			float newDolly = Float::Clamp(mDolly.y + dolly, mDolly.x, mDolly.z);

			if ( Float::IsNotZero(newDolly - mDolly.y) )
			{
				mHasMovement = true;
				mDolly.y = newDolly;
			}
		}

		// If there is movement, the absolute coordinates should be recalculated
		if (mHasMovement) mIsDirty = true;
	}
	// Return the offset position
	return mRelativePos - mRelativeRot.GetForward() * mDolly.y;
}

//============================================================================================================
// Respond to mouse movement
//============================================================================================================

bool DebugCamera::OnMouseMove (const Vector2i& pos, const Vector2i& delta)
{
	if (mSplineV.IsEmpty())
	{
		if (mCore->IsKeyDown(Key::MouseLeft))
		{
			mHasMovement = true;
			mRotMovement.x -= delta.x * 0.0002f;
			mRotMovement.y -= delta.y * 0.0002f;
			return true;
		}
		else if (mCore->IsKeyDown(Key::MouseRight))
		{
			mHasMovement = true;
			Vector3f delta3 (delta.x * 0.0025f, -delta.y * 0.0025f, 0.0f);
			mPosMovement += delta3 * mRelativeRot;
			return true;
		}
		else if (mCore->IsKeyDown(Key::MouseMiddle))
		{
			mHasMovement = true;
			Vector3f delta3 (delta.x * 0.0025f, 0.0f, -delta.y * 0.0025f);
			mPosMovement += delta3 * mRelativeRot;
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Respond to scrolling events
//============================================================================================================

bool DebugCamera::OnScroll (const Vector2i& pos, float delta)
{
	if (mSplineV.IsEmpty() && Float::IsNotZero(delta))
	{
		mHasMovement = true;
		mDollyMovement -= delta * 0.001f * Float::Clamp(mDolly.y, 1.0f, 10.0f);
		return true;
	}
	return false;
}

//============================================================================================================
// PreUpdate function has to calculate the dolly-offset position
//============================================================================================================

void DebugCamera::OnPreUpdate()
{
	if (mIsDirty || mHasMovement || mSplineV.IsValid())
	{
		// Update the offset position
		Vector3f offsetPos = _UpdateOffsetPosition();

		// Remember the relative position
		mSavedPos = mRelativePos;

		// Override the relative position with the offset position so that the absolute position
		// gets calculated using the offset value. We will restore it to the saved value later.
		mRelativePos = offsetPos;
		mIsDirty = true;
	}
}

//============================================================================================================
// Restore the relative position for the next update
//============================================================================================================

void DebugCamera::OnPostUpdate()
{
	mRelativePos = mSavedPos;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void DebugCamera::OnSerializeTo (TreeNode& node) const
{
	Camera::OnSerializeTo (node);
	node.AddChild("Dolly", mDolly);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool DebugCamera::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Dolly") node.mValue >> mDolly;
	else return Camera::OnSerializeFrom (node);
	return true;
}