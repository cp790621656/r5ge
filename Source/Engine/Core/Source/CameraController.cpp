#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Returns the most recently added camera that's not currently fading out
//============================================================================================================

const Camera* CameraController::GetActiveCamera() const
{
	const Camera* cam = 0;

	if (mCameras.IsValid())
	{
		mCameras.Lock();
		{
			for (uint i = mCameras.GetSize(); i > 0; )
			{
				const Entry& ent = mCameras[--i];

				if (!ent.mIsFading)
				{
					cam = ent.mCamera;
					break;
				}
			}
		}
		mCameras.Unlock();
	}
	return cam;
}

//============================================================================================================
// Adds a new camera to the controller. The new camera will be smoothly faded in.
//============================================================================================================

void CameraController::Activate (Camera* cam)
{
	if (cam != 0)
	{
		mCameras.Lock();
		{
			// Ensure that the camera isn't already on the list
			for (uint i = mCameras.GetSize(); i > 0; )
			{
				Entry& ent = mCameras[--i];

				if (ent.mCamera == cam)
				{
					// If it is, simply fade it in
					ent.mIsFading = false;
					ent.mRemove = false;

					// Make all higher level cameras fade out
					for (uint b = i + 1; b < mCameras.GetSize(); ++b)
					{
						// Fade out, but don't change its 'remove' status
						mCameras[b].mIsFading = true;
					}
					mCameras.Unlock();
					return;
				}
			}

			// Add a new camera entry
			Entry& ent		= mCameras.Expand();
			ent.mCamera		= cam;
			ent.mAlpha		= 0.0f;
			ent.mIsFading	= false;
			ent.mRemove		= false;
		}
		mCameras.Unlock();
	}
}

//============================================================================================================
// Deactivates an existing camera, starting the fading process. The camera will be removed at the end.
//============================================================================================================

void CameraController::Deactivate (const Camera* cam)
{
	if (cam != 0)
	{
		mCameras.Lock();
		{
			for (uint i = mCameras.GetSize(); i > 0; )
			{
				Entry& ent = mCameras[--i];

				if (ent.mCamera == cam)
				{
					// Fade this camera out and remove it when done
					ent.mIsFading = true;
					ent.mRemove = true;
					break;
				}
			}
		}
		mCameras.Unlock();
	}
}

//============================================================================================================
// Immediately removes the specified camera
//============================================================================================================

void CameraController::Remove (const Camera* cam)
{
	if (cam != 0)
	{
		mCameras.Lock();
		{
			for (uint i = mCameras.GetSize(); i > 0; )
			{
				Entry& ent = mCameras[--i];

				if (ent.mCamera == cam)
				{
					mCameras.RemoveAt(i);
					break;
				}
			}
		}
		mCameras.Unlock();
	}
}

//============================================================================================================
// Updates the pos/rot/range, blending the cameras together
//============================================================================================================

void CameraController::Update()
{
	if (mCameras.IsEmpty()) return;

	ulong delta = Time::GetDelta();
	if (delta == 0) return;

	float remain = 1.0f;
	float change = (mFadeTime > 0.0f) ? (0.001f * delta) / mFadeTime : 1.0f;

	mCameras.Lock();
	{
		// Start at the end of the list as whatever is at the back was added last and has the highest priority
		for (uint i = mCameras.GetSize(); i > 0; )
		{
			Entry& ent = mCameras[--i];

			if (remain > 0.0f)
			{
				if (ent.mIsFading)
				{
					ent.mAlpha -= change;
					
					if (ent.mAlpha < 0.0f)
					{
						ent.mAlpha = 0.0f;

						if (ent.mRemove) mCameras.RemoveAt(i);
						continue;
					}
				}
				else
				{
					ent.mAlpha += change;
					if (ent.mAlpha > 1.0f) ent.mAlpha = 1.0f;
				}

				// Update the controller's orientation
				mPos = Interpolation::Linear(mPos, ent.mCamera->GetAbsolutePosition(), remain);
				mRot = Interpolation::Slerp (mRot, ent.mCamera->GetAbsoluteRotation(), remain);
				mRan = Interpolation::Linear(mRan, ent.mCamera->GetAbsoluteRange(), remain);

				// The remaining alpha should be decreased by the entry's alpha
				remain -= Interpolation::Cosine(ent.mAlpha);
			}
			else if (ent.mIsFading && ent.mRemove)
			{
				mCameras.RemoveAt(i);
			}
		}
	}
	mCameras.Unlock();
}