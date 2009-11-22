#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Smoke Emitter
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