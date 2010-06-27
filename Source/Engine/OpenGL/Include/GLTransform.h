#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Transform matrices bundled together into one place
//============================================================================================================

class GLTransform
{
	// Common component shared between all matrix structs below
	struct CommonMatrix
	{
		bool update;	// Whether the default matrix needs to be updated
		bool set;		// Whether the matrix has been set manually
		bool changed;	// Whether the matrix has changed since the last time it was activated

		CommonMatrix() : update(true), set(false), changed(true) {}

		// Reset the matrix to its default non-overridden values
		void Reset();
	};

	// Model matrix is straightforward
	struct ModelMatrix : public CommonMatrix
	{
		Matrix43 mat;

		// Manually override the matrix
		void Override (const Matrix43& m);

		// Retrieves the set or default (identity) model matrix
		const Matrix43& Get();
	};

	// View matrix adds position, direction and up vectors
	struct ViewMatrix : public ModelMatrix
	{
		Vector3f pos;
		Vector3f dir;
		Vector3f up;

		// Sets the properties used to calculate the view matrix
		void Set (const Vector3f& p, const Vector3f& d, const Vector3f& u);

		// Retrieves the set or calculated view matrix
		const Matrix43& Get();
	};

	// Projection matrix has field of view, aspect ratio, as well as near and far planes
	struct ProjMatrix : public CommonMatrix
	{
		Matrix44 mat;
		Vector3f range;
		float aspect;

		ProjMatrix() : range(1.0f, 100.0f, 90.0f), aspect(1.33333f) {}

		// Manually overrides the projection matrix
		void Override (const Matrix44& m);

		// Sets the aspect ratio
		void Set (float asp) { if (Float::IsNotEqual(aspect, asp)) { aspect = asp; update = true; } }

		// Sets the projection matrix using the specified near, far, and field of view
		void Set (const Vector3f& inRange);

		// Sets the properties used to calculate the projection matrix
		void Set (float fov, float asp, float near, float far);

		// Retrieves the set or calculated projection matrix
		const Matrix44& Get();
	};

	template <typename Matrix>
	struct CachedEntry
	{
		Matrix	mat;		// Actual matrix
		bool	isValid;	// Whether the matrix is valid or needs to be recalculated

		CachedEntry() : isValid(false) {}
	};

	typedef CachedEntry<Matrix43> Mat43;
	typedef CachedEntry<Matrix44> Mat44;

private:

	// Allow the controller class to access private members for simplicity's sake
	friend class GLController;

	bool		mInst;	// Whether instancing is enabled
	bool		mIs2D;	// Overriding 2D projection mode
	bool		mReset;	// Manual matrix reset, used by 2D mode
	Vector2i	mSize;	// Viewport size
	ModelMatrix	mModel;	// Model matrix
	ViewMatrix	mView;	// View matrix
	ProjMatrix	mProj;	// Projection matrix
	Mat43		mMV;	// World * View matrix
	Mat43		mIMV;	// Inverse ModelView matrix
	Mat44		mMVP;	// ModelView*Projection matrix
	Mat44		mIP;	// Inverse Projection matrix
	Mat44		mIMVP;	// Inverse ModelView*Projection matrix

public:

	GLTransform() : mInst(false), mIs2D(false), mReset(false) {}

	// Activates or deactivates 2D mode
	void Set2DMode (bool active);

	// Sets the target size (which also affects the aspect ratio)
	void SetTargetSize (Vector2i size);

	// Sets the projection matrix
	void SetProjectionRange (const Vector3f& range) { mProj.Set(range); }

	// Sets the projection matrix
	void SetProjectionMatrix (float fov, float aspect, float near, float far) { mProj.Set(fov, aspect, near, far); }

	// Sets the view matrix
	void SetViewMatrix (const Vector3f& eyePos, const Vector3f& dir, const Vector3f& up) { mView.Set(eyePos, dir, up); }

	// Model matrix manipulation
	void OverrideModelMatrix (const Matrix43& mat) { mModel.Override(mat); }
	void CancelModelMatrixOverride() { mModel.Reset(); }

	// View matrix manipulation
	void OverrideViewMatrix (const Matrix43& mat) { mView.Override(mat); }
	void CancelViewMatrixOverride() { mView.Reset(); }

	// Projection matrix manipulation
	void OverrideProjectionMatrix (const Matrix44& mat) { mProj.Override(mat); }
	void CancelProjectionMatrixOverride() { mProj.Reset(); }

	// Matrix retrieval functions
	const Matrix43& GetModelMatrix();
	const Matrix43& GetViewMatrix();
	const Matrix44& GetProjectionMatrix();
	const Matrix43&	GetModelViewMatrix();
	const Matrix44&	GetModelViewProjMatrix();
	const Matrix43&	GetInverseModelViewMatrix();
	const Matrix44&	GetInverseProjMatrix();
	const Matrix44& GetInverseMVPMatrix();

	// Activates all proper matrices
	uint Activate (const IShader* shader);
};