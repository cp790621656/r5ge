#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Fire emitter
// Author: Michael Lyashenko
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