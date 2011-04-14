#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Smoke Emitter
// Author: Michael Lyashenko
//============================================================================================================

class SmokeEmitter : public Emitter
{
public:

	R5_DECLARE_INHERITED_CLASS("Smoke Emitter", SmokeEmitter, Emitter, Object);

	SmokeEmitter();

protected:

	// Virtual functionality allows custom particle behavior
	virtual void InitParticle   (Particle& particle);
	virtual void UpdateParticle (Particle& particle);
	virtual void SetRenderStates(IGraphics* graphics);
};