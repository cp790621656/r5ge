#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Common matrix functionality
//============================================================================================================

void GLTransform::CommonMatrix::Reset()
{
	if (set)
	{
		set		= false;	// The matrix is no longer set manually
		update	= true;		// The matrix needs to be updated to its default value
		changed = true;		// The matrix has changed since last time
	}
}

//============================================================================================================
// Model matrix
//============================================================================================================

void GLTransform::ModelMatrix::Override (const Matrix43& m)
{
	mat		= m;
	set		= true;
	changed = true;
}

//============================================================================================================

const Matrix43& GLTransform::ModelMatrix::Get()
{
	if (!set && update)
	{
		mat.SetToIdentity();
		update  = false;
		changed = true;
	}
	return mat;
}

//============================================================================================================
// View matrix
//============================================================================================================

void GLTransform::ViewMatrix::Set (const Vector3f& p, const Vector3f& d, const Vector3f& u)
{
	if (pos != p) { pos = p; update = true; }
	if (dir != d) { dir = d; update = true; }
	if (up  != u) { up  = u; update = true; }
}

//============================================================================================================

const Matrix43& GLTransform::ViewMatrix::Get()
{
	if (!set && update)
	{
		mat.SetToView(pos, dir, up);
		update  = false;
		changed = true;
	}
	return mat;
}

//============================================================================================================
// Projection matrix
//============================================================================================================

void GLTransform::ProjMatrix::Override (const Matrix44& m)
{
	mat		= m;
	set		= true;
	changed = true;

	// Overwritten matrices get modified right away
	if (offset) mat[10] *= 1.0f - 0.001f * offset;
}

//============================================================================================================

void GLTransform::ProjMatrix::SetOffset (int val)
{
	if (offset != val)
	{
		// Overwritten matrices get modified right away
		if (set) mat[10] /= (1.0f - 0.001f * offset);

		offset = val;
		update = true;

		if (offset) mat[10] *= (1.0f - 0.001f * offset);
	}
}

//============================================================================================================

void GLTransform::ProjMatrix::SetRange (const Vector3f& inRange)
{
	if (Float::IsNotEqual(range.x, inRange.x))	{ range.x = inRange.x;	update = true; }
	if (Float::IsNotEqual(range.y, inRange.y))	{ range.y = inRange.y;	update = true; }
	if (Float::IsNotEqual(range.z, inRange.z))	{ range.z = inRange.z;	update = true; }
}

//============================================================================================================

void GLTransform::ProjMatrix::Set (float fov, float asp, float near, float far)
{
	SetRange(Vector3f(near, far, fov));
	SetAspect(aspect);
}

//============================================================================================================

const Matrix44& GLTransform::ProjMatrix::Get()
{
	if (!set && update)
	{
		mat.SetToProjection(range.z, aspect, range.x, range.y);
		if (offset) mat[10] *= 1.0f - 0.001f * offset;
		update  = false;
		changed = true;
	}
	return mat;
}

//============================================================================================================
// Activates or deactivates 2D mode
//============================================================================================================

void GLTransform::Set2DMode (bool active)
{
	if (mIs2D != active)
	{
		mIs2D = active;
		mReset = true;
	}
}

//============================================================================================================
// Sets the target size (which also affects aspect ratio)
//============================================================================================================

void GLTransform::SetTargetSize (Vector2i size)
{
	if (size.x < 1) size.x = 1;
	if (size.y < 1) size.y = 1;

	if (mSize != size)
	{
		mSize = size;
		mProj.SetAspect((float)size.x / size.y);

		// Update the OpenGL viewport
		glViewport(0, 0, size.x, size.y);

		// If we're in 2D mode, reset the matrices next time we draw
		if (mIs2D) mReset = true;
	}
}

//============================================================================================================
// Matrix retrieval functions
//============================================================================================================

const Matrix43& GLTransform::GetModelMatrix()
{
	const Matrix43& mat = mModel.Get();

	if (mModel.changed)
	{
		mMV.isValid		= false;
		mIMV.isValid	= false;
		mMVP.isValid	= false;
		mIMVP.isValid	= false;
	}
	return mat;
}

//============================================================================================================

const Matrix43& GLTransform::GetViewMatrix()
{
	const Matrix43& mat = mView.Get();

	if (mView.changed)
	{
		mMV.isValid		= false;
		mIMV.isValid	= false;
		mMVP.isValid	= false;
		mIMVP.isValid	= false;
	}
	return mat;
}

//============================================================================================================

const Matrix44& GLTransform::GetProjectionMatrix()
{
	const Matrix44& mat = mProj.Get();

	if (mProj.changed)
	{
		mIP.isValid		= false;
		mMVP.isValid	= false;
		mIMVP.isValid	= false;
	}
	return mat;
}

//============================================================================================================

const Matrix43&	GLTransform::GetModelViewMatrix()
{
	if (mModel.set)
	{
		const Matrix43& m = GetModelMatrix();
		const Matrix43& v = GetViewMatrix();

		if (!mMV.isValid)
		{
			mMV.isValid = true;
			mMV.mat = m * v;
		}
		return mMV.mat;
	}
	return GetViewMatrix();
}

//============================================================================================================

const Matrix44&	GLTransform::GetModelViewProjMatrix()
{
	const Matrix44& p = GetProjectionMatrix();
	const Matrix43& mv = GetModelViewMatrix();

	if (!mMVP.isValid)
	{
		mMVP.isValid = true;
		mMVP.mat = mv * p;
	}
	return mMVP.mat;
}

//============================================================================================================

const Matrix43&	GLTransform::GetInverseModelViewMatrix()
{
	const Matrix43& mv = GetModelViewMatrix();

	if (!mIMV.isValid)
	{
		mIMV.isValid = true;
		mIMV.mat = mv;
		mIMV.mat.Invert();
	}
	return mIMV.mat;
}

//============================================================================================================

const Matrix44&	GLTransform::GetInverseProjMatrix()
{
	const Matrix44& p = GetProjectionMatrix();

	if (!mIP.isValid)
	{
		mIP.isValid = true;
		mIP.mat = p;
		mIP.mat.Invert();
	}
	return mIP.mat;
}

//============================================================================================================

const Matrix44& GLTransform::GetInverseMVPMatrix()
{
	const Matrix44& mvp = GetModelViewProjMatrix();

	if (!mIMVP.isValid)
	{
		mIMVP.isValid = true;
		mIMVP.mat = mvp;
		mIMVP.mat.Invert();
	}
	return mIMVP.mat;
}

//============================================================================================================
// Activates all proper matrices
//============================================================================================================

uint GLTransform::Activate (const IShader* shader)
{
	uint switches (0);

	if (mIs2D)
	{
		if (mReset)
		{
			Matrix43 mat (mSize.x, mSize.y);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(mat.mF);
			++switches;
		}
	}
	else
	{
		// If the mode has been recently changed we should consider all matrices changed
		if (mReset)
		{
			mModel.changed  = true;
			mView.changed   = true;
			mProj.changed   = true;
		}

		const Matrix43& m = GetModelMatrix();
		const Matrix43& v = GetViewMatrix();
		const Matrix44& p = GetProjectionMatrix();

		if (mProj.changed)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(p.mF);
			++switches;
		}

		if (mModel.changed || mView.changed)
		{
			// Scale parameter needs to be updated each time the model matrix changes
			if (shader != 0 && shader->GetFlag(IShader::Flag::WorldScale))
			{
				Vector3f scale (m.GetScale());
				shader->SetUniform("R5_worldScale", scale);
			}

			// If the active shader supports pseudo-instancing, take advantage of that
			if (shader != 0 && shader->GetFlag(IShader::Flag::Instanced))
			{
				// When a shader supporting instancing is enabled, feed it the world matrix
				glMultiTexCoord4fv(GL_TEXTURE2, m.mColumn0);
				glMultiTexCoord4fv(GL_TEXTURE3, m.mColumn1);
				glMultiTexCoord4fv(GL_TEXTURE4, m.mColumn2);
				glMultiTexCoord4fv(GL_TEXTURE5, m.mColumn3);

				if (mView.changed || !mInst)
				{
					// If the view has changed, or we haven't been using instancing before, set the view matrix
					glMatrixMode(GL_MODELVIEW);
					glLoadMatrixf(v.mF);

					++switches;
					mInst = true;
				}
			}
			else
			{
				// No pseudo-instancing support -- just use the ModelView matrix
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf(GetModelViewMatrix().mF);

				++switches;
				mInst = false;
			}
		}
		else if (mProj.changed)
		{
			// Always end with ModelView
			glMatrixMode(GL_MODELVIEW);
		}
	}

	// Reset the 'changed' flags on all matrices
	mModel.changed  = false;
	mView.changed   = false;
	mProj.changed   = false;
	mReset			= false;

	// Return the number of matrix switches
	return switches;
}