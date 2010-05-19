#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Particle emitter system template
//============================================================================================================

class Emitter : public Object
{
public:

	struct Flag : public Object::Flag
	{
		enum
		{
			FlipU	= 1 << 4,		// Whether U texture coordinate can be randomly flipped
			FlipV	= 1 << 5,		// Whether V texture coordinate can be randomly flipped
		};
	};

	struct Particle
	{
		Vector3f	mSpawnPos;		// Position the particle was spawned at
		Vector3f	mSpawnDir;		// Particle's offset value -- can be used as velocity if need be
		Vector3f	mPos;			// Particle's current position
		Color4ub	mColor;			// Particle's current color
		float		mRadius;		// Particle's radius, used to calculate the 4 corners
		float		mRotation;		// Particle's clockwise rotation in radians
		uint		mRemaining;		// Current time remaining according to the particle
		uint		mParam;			// Any other additional flags used by the particle
	};

	typedef Array<Particle> Particles;

protected:

	const ITexture*		mTex;		// Texture used by the emitter
	const ITechnique*	mTech;		// Technique used to draw this emitter

	Particles		mParticles;		// Array of active particles
	uint			mLifetime;		// Lifetime of individual particles
	uint			mActiveParts;	// Current number of active particles
	uint			mMaxParticles;	// Maximum number of particles allowed to be active at any given time
	uint			mFrequency;		// Spawning frequency of particles in milliseconds
	uint			mActiveTime;	// Time left to actively spawn particles
	uint			mAccumulated;	// Calculated: Accumulated time since the last particle was spawned
	uint			mLastVisible;	// Calculated: Timestamp of when the emitter was last visible
	bool			mUpdated;		// Whether the particle positions need to be updated

private:

	Array<Vector3f>		mPositions;	// Array of positions sent to the videocard
	Array<Vector2f>		mTexCoords;	// Array of texture coordinates (used in the CPU-based approach)
	Array<Color4ub>		mColors;	// Array of colors sent to the GPU

public:

	R5_DECLARE_ABSTRACT_CLASS("Emitter", Object);

	Emitter();

	const ITexture*		GetTexture()	const	{ return mTex;		}
	const ITechnique*	GetTechnique()	const	{ return mTech;		}
	
	bool IsActive()				const	{ return mActiveTime != 0;	}
	uint GetParticleLifetime()	const	{ return mLifetime;			}
	uint GetMaxParticles()		const	{ return mMaxParticles;		}
	uint GetSpawnFrequency()	const	{ return mFrequency;		}

	void SetActive			 (uint ms = 1)			{ mActiveTime	= ms;		}
	void SetTexture			 (const ITexture* tex)	{ mTex			= tex;		}
	void SetTechnique		 (const ITechnique* t)	{ mTech			= t;		}
	void SetParticleLifetime (uint life)			{ mLifetime		= life;		}
	void SetMaxParticles	 (uint count)			{ mMaxParticles = count;	}
	void SetSpawnFrequency	 (uint ms)				{ mFrequency	= ms;		}

protected:

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Adds the emitter to the list of renderable objects
	virtual bool OnFill (FillParams& params);

	// Draw the object using the specified technique
	virtual uint OnDraw (uint group, const ITechnique* tech, bool insideOut);

	// Serialization to and from the scenegraph tree
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);

protected:

	// MUST OVERRIDE: Virtual functionality allows custom particle behavior
	virtual void InitParticle   (Particle& particle)=0;
	virtual void UpdateParticle (Particle& particle)=0;
	virtual void SetRenderStates(IGraphics* graphics)=0;
};