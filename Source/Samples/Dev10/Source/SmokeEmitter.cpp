#include "../Include/_All.h"
using namespace R5;

extern R5::Random randomGen;

//============================================================================================================
// Smoke emitter constructor
//============================================================================================================

SmokeEmitter::SmokeEmitter()
{
	mLifetime		= 3000;
	mMaxParticles	= 10;
	mFrequency		= mLifetime / mMaxParticles;
	mActiveTime		= -1;
}

//============================================================================================================
// Initializes a particle, setting it to default (spawn time) values
//============================================================================================================

void SmokeEmitter::InitParticle (Particle& particle)
{
	particle.mSpawnPos		= mAbsolutePos;
	particle.mSpawnPos.x   += mAbsoluteScale * (randomGen.GenerateRangeFloat() * 0.15f);
	particle.mSpawnPos.y   += mAbsoluteScale * (randomGen.GenerateRangeFloat() * 0.15f);
	particle.mSpawnDir.x	= mAbsoluteScale * (randomGen.GenerateRangeFloat() * 0.35f);
	particle.mSpawnDir.y	= mAbsoluteScale * (randomGen.GenerateRangeFloat() * 0.35f);
	particle.mSpawnDir.z	= mAbsoluteScale * (randomGen.GenerateRangeFloat() + 10.0f);
	particle.mParam			= (randomGen.GenerateRangeFloat() < 0.0f) ? 0 : 1;
}

//============================================================================================================
// Updates the particle
//============================================================================================================

void SmokeEmitter::UpdateParticle (Particle& particle)
{
	float time			= (float)(mLifetime - particle.mRemaining);
	float progress		= Interpolation::Linear( 0.0f, 1.0f, Float::Clamp(time / mLifetime, 0.0f, 1.0f ) );
	float movement		= pow(progress, 0.75f);
	particle.mPos		= particle.mSpawnPos + particle.mSpawnDir * movement;
	particle.mRadius	= mAbsoluteScale * (1.0f + movement * 2.0f);
	particle.mRotation	= (particle.mParam == 0 ? PI : -PI) * Float::Sin( Float::Sqrt(progress) );
	particle.mColor.r	= 0;
	particle.mColor.g	= 0;
	particle.mColor.b	= 0;
	particle.mColor.a	= Float::ToRangeByte( 1.0f - Float::Abs((progress - 0.5f) * 2.0f) );
}

//============================================================================================================
// Set up the optional render states
//============================================================================================================

void SmokeEmitter::SetRenderStates(IGraphics* graphics)
{
	graphics->SetBlending	( IGraphics::Blending::Normal );
	graphics->SetAlphaTest	( true );
	graphics->SetFog		( true );
}