#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Random number generator
//============================================================================================================

Random g_rand;

//============================================================================================================
// Immediately plays a random idle animation
//============================================================================================================

void OSPlayIdleAnimations::Play()
{
	if (mIdleAnims.IsValid())
	{
		g_rand.SetSeed(g_rand.GenerateUint() + (uint)Time::GetMilliseconds());
		uint index = g_rand.GenerateUint() % mIdleAnims.GetSize();

		if (mIdleLoop)
		{
			// If we have a looping animation playing in the background, we don't need a callback
			mModel->PlayAnimation(mIdleAnims[index]);
		}
		else
		{
			// If we don't have a looping animation, we will need this callback
			mModel->PlayAnimation(mIdleAnims[index], 1.0f, bind(&OSPlayIdleAnimations::OnIdleFinished, this));
		}
	}
}

//============================================================================================================
// Callback triggered when the idle animation finishes playing
//============================================================================================================

void OSPlayIdleAnimations::OnIdleFinished (Model* model, const Animation* anim, float timeToEnd)
{
	// This callback gets triggered twice. Once when animation starts fading, and once when it finishes.
	if (timeToEnd > 0.0f) Play();
}

//============================================================================================================
// When the script initializes, gather all idle animations
//============================================================================================================

void OSPlayIdleAnimations::OnInit()
{
	ModelInstance* inst = R5_CAST(ModelInstance, mObject);

	if (inst != 0)
	{
		mModel = inst->GetModel();

		if (mModel != 0)
		{
			const Skeleton* skel = mModel->GetSkeleton();

			if (skel != 0)
			{
				const Skeleton::Animations& anims = skel->GetAllAnimations();

				if (anims.IsValid())
				{
					// Collect all idle animations
					for (uint i = anims.GetSize(); i > 0; )
					{
						Animation* anim = anims[--i];

						if (anim->GetName().BeginsWith("Idle"))
						{
							// If it's a looping animation, start playing it immediately
							if (anim->IsLooping())
							{
								if (!mIdleLoop)
								{
									mIdleLoop = true;
									mModel->PlayAnimation(anim);
								}
							}
							else
							{
								// It's a secondary idle animation -- add it to the list
								mIdleAnims.Expand() = anim;
							}
						}
					}

					// If we have one or more animations to work with, all is well
					if (mIdleAnims.IsValid()) return;
				}
			}
		}
	}
	// If anything goes wrong, destroy this script
	DestroySelf();
}

//============================================================================================================
// Keeps track of when to play the next animation
//============================================================================================================

void OSPlayIdleAnimations::OnUpdate()
{
	if (mIdleLoop)
	{
		ulong delta = Time::GetDeltaMS();

		if (mTimeToPlay > delta)
		{
			mTimeToPlay -= delta;
		}
		else
		{
			mTimeToPlay = 4000 + Float::RoundToUInt(10000.0f * g_rand.GenerateFloat());
			Play();
		}
	}
	else
	{
		// Calling Script's OnUpdate will effectively disable the update notification.
		// We want to do this if we don't have a looping animation, eliminating the need for Update().
		Play();
		Script::OnUpdate();
	}
}