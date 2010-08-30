#include "../Include/_All.h"
using namespace R5;

extern R5::Random randomGen;

//============================================================================================================
// Fire emitter constructor
//============================================================================================================

FireEmitter::FireEmitter()
{
	mLifetime		= 1000;
	mMaxParticles	= 15;
	mFrequency		= mLifetime / mMaxParticles;
	mActiveTime		= -1;

	// We don't want to allow random flipping the flame texture vertically (by default it randomly flips both)
	mFlags.Set(Flag::FlipV, false);
}

//============================================================================================================
// Initializes a particle, setting it to default (spawn time) values
//============================================================================================================

void FireEmitter::InitParticle (Particle& particle)
{
	particle.mSpawnPos		= mAbsolutePos;
	particle.mSpawnPos.x   += mAbsoluteScale.x * (randomGen.GenerateRangeFloat() * 0.15f);
	particle.mSpawnPos.y   += mAbsoluteScale.y * (randomGen.GenerateRangeFloat() * 0.15f);
	particle.mSpawnDir.x	= mAbsoluteScale.x * (randomGen.GenerateRangeFloat());
	particle.mSpawnDir.y	= mAbsoluteScale.y * (randomGen.GenerateRangeFloat());
	particle.mSpawnDir.z	= mAbsoluteScale.z * (randomGen.GenerateRangeFloat() * 0.5f + 4.5f);
	particle.mRotation		= 0.0f;
}

//============================================================================================================
// Updates the particle
//============================================================================================================

void FireEmitter::UpdateParticle (Particle& particle)
{
	float time			= (float)(mLifetime - particle.mRemaining);
	float progress		= Interpolation::Linear( 0.0f, 1.0f, Float::Clamp(time / mLifetime, 0.0f, 1.0f ) );
	float movement		= pow(progress, 1.5f);
	particle.mPos		= particle.mSpawnPos + particle.mSpawnDir * movement;
	particle.mRadius	= mAbsoluteScale.y * (1.0f - movement);

	// Make the flame get narrower as it gets higher
	Vector3f pos ( particle.mSpawnPos.x, particle.mSpawnPos.y, particle.mPos.z );
	particle.mPos = Interpolation::Linear( particle.mPos, pos, progress * 0.95f );

	// Flame should spawn as transparent, reach 1.0 alpha by 25% lifetime, then fade out as it gets to 0%
	float alpha = (progress < 0.25f) ? progress * 4.0f : 1.0f - (progress - 0.25f) * 1.333333f;

	// Color is based on the same factor value
	particle.mColor.r	= Float::ToRangeByte(alpha);
	particle.mColor.g	= particle.mColor.r;
	particle.mColor.b	= particle.mColor.r;
	particle.mColor.a	= 255;
}

//============================================================================================================
// Set up the optional render states
//============================================================================================================

void FireEmitter::SetRenderStates (IGraphics* graphics)
{
	graphics->SetBlending	( IGraphics::Blending::Add );
	graphics->SetAlphaTest	( false );
	graphics->SetFog		( false );
}