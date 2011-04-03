#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Fire emitter
//============================================================================================================

class FireEmitter : public Emitter
{
public:

	R5_DECLARE_INHERITED_CLASS("Fire Emitter", FireEmitter, Emitter, Object);

	FireEmitter();

protected:

	// Virtual functionality allows custom particle behavior
	virtual void InitParticle   (Particle& particle);
	virtual void UpdateParticle (Particle& particle);
	virtual void SetRenderStates(IGraphics* graphics);
};