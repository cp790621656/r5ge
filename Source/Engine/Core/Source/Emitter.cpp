#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Particle emitter class constructor
//============================================================================================================

Emitter::Emitter() :
	mTex			(0),
	mTech			(0),
	mLifetime		(1000),
	mActiveParts	(0),
	mMaxParticles	(10),
	mFrequency		(100),
	mActiveTime		(0),
	mAccumulated	(0),
	mLastVisible	(0),
	mUpdated		(false)
{
	mFlags.Set(Flag::FlipU | Flag::FlipV, true);
}

//============================================================================================================
// Runs through all particles and respawns them as necessary
//============================================================================================================

void Emitter::OnUpdate()
{
	// If the emitter is inactive, do nothing
	if ( (mActiveTime == 0 && mActiveParts == 0) || mMaxParticles == 0 || mTex == 0 ) return;

	// If the particle emitter is endless, hasn't moved, and hasn't been visible in a while, don't update it
	if (mActiveTime == -1 && !mIsDirty && (Time::GetMilliseconds() - mLastVisible > 500) ) return;

	// Delay since last frame
	ulong delta = Time::GetDelta();

	// Only consider updating particles if the delta is not zero
	if (delta == 0) return;

	// Recalculate the number of active particles
	mActiveParts = 0;
	mUpdated = true;
	mBounds.Reset();

	// Update our accumulated spawn time
	mAccumulated += delta;

	// Update the active time if we can
	if (mActiveTime != -1)
	{
		mActiveTime = (mActiveTime > delta) ? mActiveTime - delta : 0;
	}

	// Update the particle and the bounding volume
	for (uint i = mParticles.GetSize(); i > 0; )
	{
		Particle& particle = mParticles[--i];

		// Update the particle's time
		particle.mRemaining = (particle.mRemaining > delta) ? particle.mRemaining - delta : 0;

		// If this is an inactive particle, see if we can reactivate it
		if ( particle.mRemaining == 0 && mActiveTime != 0 && mActiveParts < mMaxParticles )
		{
			// If it's time to spawn a new particle, let's do that now
			if (mAccumulated >= mFrequency)
			{
				mAccumulated -= mFrequency;
				particle.mRemaining = (mLifetime > mAccumulated) ? mLifetime - mAccumulated : 0;
				InitParticle(particle);
			}
		}

		// If the particle has some time remaining, update it and include it in the bounds
		if (particle.mRemaining != 0)
		{
			UpdateParticle(particle);

			// One final check, as UpdateParticle might set the particle's remaining time to be 0 as well
			if (particle.mRemaining != 0)
			{
				++mActiveParts;
				mBounds.Include(particle.mPos, particle.mRadius);
			}
		}
	}

	// If the number of particles hasn't reached maximum yet, see if we can add new ones
	for ( ; (mParticles.GetSize() < mMaxParticles) && (mAccumulated >= mFrequency); ++mActiveParts )
	{
		mAccumulated -= mFrequency;
		Particle& particle = mParticles.Expand();
		particle.mRemaining = (mLifetime > mAccumulated) ? mLifetime - mAccumulated : 0;

		InitParticle(particle);
		UpdateParticle(particle);
		mBounds.Include(particle.mPos, particle.mRadius);
	}

	// Automatically shrink the trailing particles that are no longer used
	while (mParticles.IsValid() && mParticles.Back().mRemaining == 0)
		mParticles.Shrink();

	// The accumulated amount should never carry over more than a frequency's worth
	if (mAccumulated > mFrequency)
		mAccumulated = mFrequency;
}

//============================================================================================================
// Culls the particle emitter based on whether it's visible
//============================================================================================================

Object::CullResult Emitter::OnCull (CullParams &params, bool isParentVisible, bool render)
{
	if (mTex == 0 || mParticles.IsEmpty())
	{
		mLastVisible = Time::GetMilliseconds();
		return false;
	}
	else if ( params.mFrustum.IsVisible(mBounds) )
	{
		mLastVisible = Time::GetMilliseconds();

		if (render)
		{
			Drawable& obj		= params.mObjects.Expand();
			obj.mObject			= this;
			obj.mLayer			= 0;
			obj.mGroup			= 0;
			obj.mDistSquared	= (params.mCamPos - mBounds.GetCenter()).Dot();

			if (mTech == 0)
			{
				// If no special technique was specified, assume the default value
				mTech = mCore->GetGraphics()->GetTechnique("Particle", true);
			}
		}
		return true;
	}
	return true;
}

//============================================================================================================
// Draws all particles
//============================================================================================================

uint Emitter::OnDraw (IGraphics* graphics, const ITechnique* tech, bool insideOut)
{
	if (mTech == tech && mFlags.Get(Flag::Enabled))
	{
		// If we need to update the vertex array, let's do that now
		if (mUpdated)
		{
			mUpdated = false;

			uint flipU = mFlags.Get(Flag::FlipU) ? 1 : 0;
			uint flipV = mFlags.Get(Flag::FlipV) ? 2 : 0;

			// Rotate the 4 corners using the inverse projection matrix
			const Matrix43& invView = graphics->GetInverseViewMatrix();

			// 4 corners of the particle
			Vector3f v0 (-1.0f,  1.0f, 0.5f);
			Vector3f v1 (-1.0f, -1.0f, 0.5f);
			Vector3f v2 ( 1.0f, -1.0f, 0.5f);
			Vector3f v3 ( 1.0f,  1.0f, 0.5f);

			// Whether we need to transform the corners by the inverse view matrix
			bool transform = true;

			// UV coordinates can be randomly flipped in order to add non-uniformness to the particle textures
			float left, right, top, bottom, last = 0.0f;

			// Time to re-fill the vertex information
			mPositions.Clear();
			mTexCoords.Clear();
			mColors.Clear();

			// Run through all particles and update them one at a time
			for (uint i = mParticles.GetSize(); i > 0; )
			{
				Particle& particle = mParticles[--i];
				if (particle.mRemaining == 0) continue;

				// Add non-uniformness by flipping the texture coordinates, if allowed
				if ( (i & flipU) == 1 )	{ left   = 1.0f;  right	= 0.0f; }
				else					{ left   = 0.0f;  right	= 1.0f; }
				if ( (i & flipV) == 2 )	{ bottom = 1.0f;  top	= 0.0f; }
				else					{ bottom = 0.0f;  top	= 1.0f; }

				// Colors are identical across all 4 vertices
				mColors.Expand() = particle.mColor;
				mColors.Expand() = particle.mColor;
				mColors.Expand() = particle.mColor;
				mColors.Expand() = particle.mColor;

				// Texture coordinates are also simple
				mTexCoords.Expand().Set(left, top);
				mTexCoords.Expand().Set(left, bottom);
				mTexCoords.Expand().Set(right, bottom);
				mTexCoords.Expand().Set(right, top);

				// Rotate the particle if the rotation has changed
				if (last != particle.mRotation)
				{
					last = particle.mRotation;

					float z =  Float::Sin(last);
					float w = -Float::Cos(last);

					float zw = z * w;
					float zz = z * z;

					// Optimized rotation of the 4 points based on the starting values above
					// rotated by the quaternion created using the rotation around the Z axis.

					v3.Set(1.0f - (zw + zz) * 2.0f, 1.0f + (zw - zz) * 2.0f, 0.5f);
					v0.Set(-v3.y,  v3.x, 0.5f);
					v1.Set(-v3.x, -v3.y, 0.5f);
					v2.Set( v3.y, -v3.x, 0.5f);

					transform = true;
				}

				// Transform the vertices by the inverse view matrix if needed
				if (transform)
				{
					transform = false;
					v0 %= invView;
					v1 %= invView;
					v2 %= invView;
					v3 %= invView;
				}

				// Vertex positions are based on the offset particle position
				mPositions.Expand() = particle.mPos + v0 * particle.mRadius;
				mPositions.Expand() = particle.mPos + v1 * particle.mRadius;
				mPositions.Expand() = particle.mPos + v2 * particle.mRadius;
				mPositions.Expand() = particle.mPos + v3 * particle.mRadius;
			}
		}

		if ( mPositions.IsValid() )
		{
			// Set up initial drawing states
			graphics->ResetViewMatrix();
			graphics->SetActiveShader(0);
			graphics->SetADT(0.003921568627451f);
			graphics->SetActiveMaterial(mTex);

			// Trigger the custom functionality
			SetRenderStates(graphics);

			// Particles don't use normals, tangents, or secondary texture coordinates
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,	 0 );
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,	 0 );
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1, 0 );

			// Texture coordinate array
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0,
				&mTexCoords[0], IGraphics::DataType::Float, 2, sizeof(Vector2f) );

			// Color array
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,
				&mColors[0], IGraphics::DataType::Byte, 4, sizeof(Color4ub) );

			// Position array
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,
				&mPositions[0], IGraphics::DataType::Float, 3, sizeof(Vector3f) );

			// Draw the particles
			return graphics->DrawVertices (IGraphics::Primitive::Quad, mPositions.GetSize());
		}
	}
	return 0;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Emitter::OnSerializeTo (TreeNode& root) const
{
	if (mTex  != 0) root.AddChild(   ITexture::ClassID(),  mTex->GetName() );
	if (mTech != 0) root.AddChild( ITechnique::ClassID(), mTech->GetName() );
}

//============================================================================================================
// Serialiation -- Load
//============================================================================================================

bool Emitter::OnSerializeFrom (const TreeNode& root)
{
	if (root.mTag == ITexture::ClassID())
	{
		IGraphics* graphics = mCore->GetGraphics();
		if (graphics != 0) SetTexture( graphics->GetTexture(root.mValue.GetString()) );
		return true;
	}
	else if (root.mTag == ITechnique::ClassID())
	{
		IGraphics* graphics = mCore->GetGraphics();
		mTech = graphics->GetTechnique( root.mValue.IsString() ? root.mValue.AsString() :
			root.mValue.GetString(), true );
	}
	return false;
}