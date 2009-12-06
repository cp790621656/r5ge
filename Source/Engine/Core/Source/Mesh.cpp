#include "../Include/_All.h"
using namespace R5;

// Defined in Model.cpp
extern bool g_skinToVBO;

//============================================================================================================
// Helpful macros that shorten the code below
//============================================================================================================

#define IF_VERTEX				if (mFormat.mVertex		!= 0xFFFFFFFF)
#define IF_NORMAL				if (mFormat.mNormal		!= 0xFFFFFFFF)
#define IF_TANGENT				if (mFormat.mTangent	!= 0xFFFFFFFF)
#define IF_TEXCOORD				if (mFormat.mTexCoord	!= 0xFFFFFFFF)
#define IF_BONEINDEX			if (mFormat.mBoneIndex	!= 0xFFFFFFFF)
#define IF_BONEWEIGHT			if (mFormat.mBoneWeight	!= 0xFFFFFFFF)
#define IF_TANGENT_AND_NORMAL	if (mFormat.mNormal		!= 0xFFFFFFFF && \
									mFormat.mTangent	!= 0xFFFFFFFF)

#define CURRENT_VERTEX		(*((Vector3f*)(current + mFormat.mVertex)))
#define CURRENT_NORMAL		(*((Vector3f*)(current + mFormat.mNormal)))
#define CURRENT_TANGENT		(*((Vector3f*)(current + mFormat.mTangent)))
#define CURRENT_TEXCOORD	(*((Vector2f*)(current + mFormat.mTexCoord)))
#define CURRENT_BONEINDEX	(*((Color4ub*)(current + mFormat.mBoneIndex)))
#define CURRENT_BONEWEIGHT	(*((Color4f*)(current + mFormat.mBoneWeight)))

//============================================================================================================
// Returns the number of triangles in the index buffer
//============================================================================================================
namespace
{
uint CountTriangles (uint indices, uint primitive)
{
	switch (primitive)
	{
		case IGraphics::Primitive::Triangle:		return indices / 3;
		case IGraphics::Primitive::TriangleStrip:	return indices > 1 ? indices - 2 : 0;
		case IGraphics::Primitive::Quad:			return indices / 2;
		case IGraphics::Primitive::QuadStrip:		return indices > 1 ? indices - 2 : 0;
		case IGraphics::Primitive::TriangleFan:		return indices > 1 ? indices - 2 : 0;
		case IGraphics::Primitive::Line:			return indices / 2;
		case IGraphics::Primitive::LineStrip:		return indices > 0 ? indices - 1 : 0;
		case IGraphics::Primitive::Point:			return indices;
	}
	return 0;
}

//============================================================================================================
// Converts primitive to text
//============================================================================================================

const char* GetType (uint primitive)
{
	switch (primitive)
	{
		case IGraphics::Primitive::Triangle:		return "Triangles";
		case IGraphics::Primitive::TriangleStrip:	return "Triangle Strip";
		case IGraphics::Primitive::Quad:			return "Quads";
		case IGraphics::Primitive::QuadStrip:		return "Quad Strip";
		case IGraphics::Primitive::TriangleFan:		return "Triangle Fan";
		case IGraphics::Primitive::Line:			return "Lines";
		case IGraphics::Primitive::LineStrip:		return "Line Strip";
		case IGraphics::Primitive::Point:			return "Points";
	}
	return "Unknown";
}

//============================================================================================================
// Converts text to primitive
//============================================================================================================

uint GetPrimitive (const String& type)
{
	if (type == "Triangles")		return IGraphics::Primitive::Triangle;
	if (type == "Triangle Strip")	return IGraphics::Primitive::TriangleStrip;
	if (type == "Quads")			return IGraphics::Primitive::Quad;
	if (type == "Quad Strip")		return IGraphics::Primitive::QuadStrip;
	if (type == "Triangle Fan")		return IGraphics::Primitive::TriangleFan;
	if (type == "Lines")			return IGraphics::Primitive::Line;
	if (type == "Line Strip")		return IGraphics::Primitive::LineStrip;
	if (type == "Points")			return IGraphics::Primitive::Point;
									return -1;
}

//============================================================================================================
// Counts the maximum number of bones per vertex
//============================================================================================================

uint CountBonesPerVertex (const Mesh::BoneWeights& bw)
{
	uint max = 0;

	for (uint i = bw.GetSize(); i > 0; )
	{
		const Color4f& boneWeight = bw[--i];

		if		(max < 4 && boneWeight.a != 0.0f) { max = 4; break; }
		else if (max < 3 && boneWeight.b != 0.0f) max = 3;
		else if (max < 2 && boneWeight.g != 0.0f) max = 2;
		else if (max < 1 && boneWeight.r != 0.0f) max = 1;
	}
	return max;
}
} // anonymous namespace

//============================================================================================================
// Constructor
//============================================================================================================

Mesh::Mesh (const String& name) :
	mName				(name),
	mIsLocked			(false),
	mBones				(0),
	mGeneratedNormals	(true),
	mVbo				(0),
	mTbo				(0),
	mVboSize			(0),
	mTboSize			(0),
	mPrimitive			(IGraphics::Primitive::Triangle),
	mIbo				(0),
	mIboSize			(0),
	mGraphics			(0) {}

//============================================================================================================
// VBOs should be released when the class is destroyed, as they are created in Draw()
//============================================================================================================

Mesh::~Mesh()
{
#ifdef _DEBUG
	System::Log("[MESH]    Releasing '%s'", mName.GetBuffer());
#endif

	if (mGraphics)
	{
		if (mIbo != 0)	mGraphics->DeleteVBO(mIbo);
		if (mTbo != 0)	mGraphics->DeleteVBO(mTbo);
		if (mVbo != 0)	mGraphics->DeleteVBO(mVbo);
	}
}

//============================================================================================================
// Releases all memory used by the mesh
//============================================================================================================

void Mesh::Release()
{
	Lock();
	{
#ifdef _DEBUG
		if (IsValid()) System::Log("[MESH]    Releasing '%s'", mName.GetBuffer());
#endif
		_Clear();

		mVboSize	= 0;
		mTboSize	= 0;
		mIboSize	= 0;

		if (mIbo) mIbo->Release();
		if (mTbo) mTbo->Release();
		if (mVbo) mVbo->Release();
	}
	Unlock();
}

//============================================================================================================
// Clears all arrays and resets all flags
//============================================================================================================

void Mesh::_Clear()
{
	mFormat.Clear();
	mV.Clear();
	mN.Clear();
	mT.Clear();
	mTc.Clear();
	mBi.Clear();
	mBw.Clear();
	mTv.Clear();
	mTn.Clear();
	mTt.Clear();
	mIndices.Clear();
	mBounds.Reset();
}

//============================================================================================================
// Recalculate min/max/center/radius bounds
//============================================================================================================

void Mesh::_RecalculateBounds()
{
	mBounds.Reset();

	if (mTv.IsValid())
	{
		for (uint i = 0; i < mTv.GetSize(); ++i)
			mBounds.Include(mTv[i]);
	}
	else
	{
		for (uint i = 0; i < mV.GetSize(); ++i)
			mBounds.Include(mV[i]);
	}
}

//============================================================================================================
// Recalculates normals and tangents as requested
//============================================================================================================

void Mesh::_CalculateNormalsAndTangents()
{
	if (mT.IsValid() || mV.IsEmpty() || mV.GetSize() != mTc.GetSize()) return;

	// Don't bother with points or lines
	if (mPrimitive == IGraphics::Primitive::Point		||
		mPrimitive == IGraphics::Primitive::Line		||
		mPrimitive == IGraphics::Primitive::LineStrip) return;

	// Allocate a temporary buffer to store binormals
	Array<Vector3f> binormals (mV.GetSize());

	// Remember whether we already have normals to work with
	bool calculateNormals = ( mN.GetSize() != mV.GetSize() );

	// If we should calculate normals, clear the existing normal array
	if (calculateNormals)
	{
		mN.Clear();
		mN.ExpandTo(mV.GetSize());
	}

	// Expand the tangent array
	mT.ExpandTo(mV.GetSize());

	// Triangles
	if (mPrimitive == IGraphics::Primitive::Triangle && mIndices.GetSize() > 2)
	{
		for (uint i = 0; i < mIndices.GetSize(); i+=3)
		{
			uint i0 ( mIndices[i  ] );
			uint i1 ( mIndices[i+1] );
			uint i2 ( mIndices[i+2] );

			ASSERT(i0 < mV.GetSize(), "Index out of bounds!");
			ASSERT(i1 < mV.GetSize(), "Index out of bounds!");
			ASSERT(i2 < mV.GetSize(), "Index out of bounds!");

			const Vector3f& v0 ( mV[i0] );
			const Vector3f& v1 ( mV[i1] );
			const Vector3f& v2 ( mV[i2] );

			const Vector2f& t0 ( mTc[i0] );
			const Vector2f& t1 ( mTc[i1] );
			const Vector2f& t2 ( mTc[i2] );

			Vector3f v10 (v1 - v0);
			Vector3f v20 (v2 - v0);
			Vector2f t10 (t1 - t0);
			Vector2f t20 (t2 - t0);

			if (calculateNormals)
			{
				Vector3f normal (Cross(v10, v20));

				mN[i0] += normal;
				mN[i1] += normal;
				mN[i2] += normal;
			}

			float denominator = t10.x * t20.y - t20.x * t10.y;

			if ( Float::IsNotZero(denominator) )
			{
				float scale = 1.0f / denominator;

				Vector3f tangent ((v10.x * t20.y - v20.x * t10.y) * scale,
								  (v10.y * t20.y - v20.y * t10.y) * scale,
								  (v10.z * t20.y - v20.z * t10.y) * scale);

				Vector3f binormal((v20.x * t10.x - v10.x * t20.x) * scale,
								  (v20.y * t10.x - v10.y * t20.x) * scale,
								  (v20.z * t10.x - v10.z * t20.x) * scale);

				mT[i0] += tangent;
				mT[i1] += tangent;
				mT[i2] += tangent;

				binormals[i0] += binormal;
				binormals[i1] += binormal;
				binormals[i2] += binormal;
			}
		}

		// If we're calculating normals we need to match all normals with identical vertices
		if (calculateNormals)
		{
			Array<uint> matches;

			for (uint i = 0; i < mV.GetSize(); ++i)
			{
				matches.Clear();
				matches.Expand() = i;
				Vector3f N = mN[i];

				const Vector3f& V (mV[i]);

				for (uint b = 0; b < mV.GetSize(); ++b)
				{
					if (i != b && V == mV[b])
					{
						matches.Expand() = b;
						N += mN[b];
					}
				}

				N.Normalize();

				for (uint b = 0; b < matches.GetSize(); ++b)
				{
					mN[ matches[b] ] = N;
				}
			}
		}
	}

	// Normalize all tangents
	for (uint i = 0; i < mV.GetSize(); ++i)
	{
		Vector3f&  T (mT[i]);
		Vector3f&  B (binormals[i]);
		Vector3f&  N (mN[i]);

		// In order to avoid visible seams, the tangent should be 90 degrees to the normal
		// Note to self: Gram-Schmidt formula for the cross product below: T = T - N * Dot(N, T);
		T = Cross(B, N);
		T.Normalize();

		// Flip the tangent if the handedness is incorrect
		if (Dot(Cross(T, B), N) < 0.0f) T.Flip();
	}
}

//============================================================================================================
// Rebuilds the mesh arrays
//============================================================================================================

void Mesh::Update ( bool rebuildBuffers,
					bool rebuildNormals,
					bool rebuildTangents,
					bool texCoordsChanged,
					bool bonesChanged,
					bool indicesChanged )
{
	if (indicesChanged)
	{
		mIboSize = 0;
		rebuildBuffers = true;
	}

	if (texCoordsChanged)
	{
		rebuildTangents = true;
		rebuildBuffers  = true;
	}

	if (rebuildNormals)
	{
		mN.Clear();
		mTn.Clear();
		mGeneratedNormals	= true;
		rebuildTangents		= true;
		rebuildBuffers		= true;
	}

	if (rebuildTangents)
	{
		mT.Clear();
		mTt.Clear();
		_CalculateNormalsAndTangents();
		rebuildBuffers = true;
	}

	if (bonesChanged)
	{
		mBones = CountBonesPerVertex(mBw);
		rebuildBuffers = true;
	}

	// Reset the VBO sizes so they are updated next frame
	if (rebuildBuffers)
	{
		mVboSize = 0;
		mTboSize = 0;

		// Transformed vertices should not exist beyond this point
		mTv.Clear();
		mTn.Clear();
		mTt.Clear();

		// Recalculate the vertex format
		mFormat.Clear();

		if (mV.IsValid()) 
		{
			mFormat.mVertex	    = mFormat.mFullSize;
			mFormat.mFullSize  += mV.GetElementSize();
			mFormat.mTransSize += mV.GetElementSize();
		}

		if (mN.IsValid()) 
		{
			mFormat.mNormal	    = mFormat.mFullSize;
			mFormat.mFullSize  += mN.GetElementSize();
			mFormat.mTransSize += mN.GetElementSize();
		}

		if (mT.IsValid()) 
		{
			mFormat.mTangent    = mFormat.mFullSize;
			mFormat.mFullSize  += mT.GetElementSize();
			mFormat.mTransSize += mT.GetElementSize();
		}

		if (mTc.IsValid()) 
		{
			mFormat.mTexCoord  = mFormat.mFullSize;
			mFormat.mFullSize += mTc.GetElementSize();
		}

		if (mBi.IsValid())
		{
			mFormat.mBoneIndex = mFormat.mFullSize;
			mFormat.mFullSize += mBi.GetElementSize();
		}

		if (mBw.IsValid())
		{
			mFormat.mBoneWeight = mFormat.mFullSize;
			mFormat.mFullSize  += mBw.GetElementSize();
		}

		// Recalculate the bounds
		_RecalculateBounds();
	}

#ifdef _DEBUG
	if (mV.IsValid() && mIndices.IsValid())
	{
		System::Log("[MESH]    Updated '%s'", mName.GetBuffer());
		System::Log("          - %u vertices (%u: V%c%c%c%c%c)", mV.GetSize(), mFormat.mFullSize,
			(mN.IsValid()  ? 'N' : '-'),
			(mT.IsValid()  ? 'T' : '-'),
			(mTc.IsValid() ? 'X' : '-'),
			(mBw.IsValid() ? 'B' : '-'),
			(mBi.IsValid() ? 'I' : '-'));
		System::Log("          - %u indices (%s)", mIndices.GetSize(), GetType(mPrimitive));
		System::Log("          - %u triangles", CountTriangles(mIndices.GetSize(), mPrimitive));
		System::Log("          - %u bone weights per vertex", GetNumberOfWeights());
		System::Log("          - %s bytes", String::GetFormattedSize( GetSizeInMemory() ).GetBuffer());
	}
#endif
}

//============================================================================================================
// Returns the memory size used by the mesh
//============================================================================================================

uint Mesh::GetSizeInMemory() const
{
	uint size = mIndices.GetSizeInMemory();

	size += mV.GetSizeInMemory();
	size += mN.GetSizeInMemory();
	size += mT.GetSizeInMemory();
	size += mTc.GetSizeInMemory();
	size += mBw.GetSizeInMemory();
	size += mBi.GetSizeInMemory();

	return size;
}

//============================================================================================================
// Returns the number of triangles that would be rendered if Draw() was called
//============================================================================================================

uint Mesh::GetNumberOfTriangles() const
{
	return CountTriangles( (mIboSize == 0) ? mIndices.GetSize() : mIboSize, mPrimitive );
}

//============================================================================================================
// Software skinning on the CPU
//============================================================================================================

bool Mesh::ApplyTransforms (const Array<Matrix43>& transforms, uint instances)
{
	if (mV.IsValid() && mFormat.mTransSize != 0)
	{
		uint maxWeights = GetNumberOfWeights();

		if (maxWeights > 0)
		{
			Lock();
			{
				// If the model is referenced only once, we don't need to cache the result in a VBO.
				// However if the VBO has already been created, we might as well reuse it.
				if ( g_skinToVBO && (mGraphics != 0) && (mTbo != 0 || instances > 1) )
				{
					// Try to transform directly into the VBO
					if ( _TransformToVBO(transforms) )
					{
						// From here on all transforms will be using VBOs, so let's release the VAs
						mTv.Release();
						mTn.Release();
						mTt.Release();
					}
					else
					{
						// If fails, revert to Vertex Arrays
						_TransformToVAs(transforms);
					}
				}
				else
				{
					// If there is only one instance, it's faster to use Vertex Arrays
					_TransformToVAs(transforms);
				}
			}
			Unlock();
		}
	}
	return false;
}

//============================================================================================================
// Common loop element, macro-defined to shorten the code as it repeats
//============================================================================================================

#define COMMON_LOOP										\
	--i;												\
														\
	const Color4ub& bi	= mBi[i];						\
	const Color4f& bw	= mBw[i];						\
														\
	uint myWeights = maxWeights;						\
														\
	if (myWeights == 4 && bw[3] == 0.0f) --myWeights;	\
	if (myWeights == 3 && bw[2] == 0.0f) --myWeights;	\
	if (myWeights == 2 && bw[1] == 0.0f) --myWeights;	\
	if (myWeights == 1 && bw[0] == 0.0f) --myWeights;

//============================================================================================================

#define VBO_VERTEX	byte* current = (ptr + mFormat.mTransSize * i);	\
					const Vector3f& v	= mV [i];	\
					Vector3f&		tv	= CURRENT_VERTEX;

//============================================================================================================

#define VBO_NORMAL	const Vector3f& n  = mN [i];	\
					Vector3f&		tn = CURRENT_NORMAL;

//============================================================================================================

#define VBO_TANGENT	const Vector3f& t  = mT [i]; \
					Vector3f&		tt = CURRENT_TANGENT;

//============================================================================================================

#define VA_VERTEX	const Vector3f& v	= mV [i];	\
					Vector3f&		tv	= mTv[i];

//============================================================================================================

#define VA_NORMAL	const Vector3f& n  = mN [i];	\
					Vector3f&		tn = mTn[i];

//============================================================================================================

#define VA_TANGENT	const Vector3f& t  = mT [i]; \
					Vector3f&		tt = mTt[i];

//============================================================================================================
// INTERNAL: Transforms the vertices using the specified matrices directly into the VBO
//============================================================================================================
// NOTE: This code was designed for speed over size, so it has a fair bit of duplication.
//============================================================================================================

bool Mesh::_TransformToVBO (const Array<Matrix43> &transforms)
{
	// Try to create a VBO
	if (mTbo == 0) mTbo = mGraphics->CreateVBO();

	// If VBO couldn't be created, nothing we can do
	if (mTbo == 0) return false;

	// Get the number of vertices
	uint maxWeights = GetNumberOfWeights();
	uint vertices   = GetNumberOfVertices();
	ASSERT( vertices > 0, "Invalid number of vertices? How did this happen?" );

	// Transformed buffer size
	mTboSize = mFormat.mTransSize * vertices;
	ASSERT( mTboSize > 0, "Invalid VBO size" );

	mTbo->Lock();
	{
		// Reserve the required amount of space in the buffer
		byte* ptr = mMem.Resize(mTboSize);

		IF_TANGENT_AND_NORMAL
		{
			for (uint i = vertices; i > 0; )
			{
				COMMON_LOOP;
				VBO_VERTEX;
				VBO_NORMAL;
				VBO_TANGENT;

				if (myWeights != 0)
				{
					for (uint b = 0; b < myWeights; ++b)
					{
						const byte&	 boneIndex	= bi[b];
						const float& boneWeight	= bw[b];

						ASSERT(boneIndex < transforms.GetSize(), "Bone index is out of range");

						const Matrix43& m = transforms[boneIndex];

						if (b == 0)
						{
							tv = (v	* m) * boneWeight;
							tn = (n % m) * boneWeight;
							tt = (t % m) * boneWeight;
						}
						else
						{
							tv += (v * m) * boneWeight;
							tn += (n % m) * boneWeight;
							tt += (t % m) * boneWeight;
						}
					}

					if (myWeights > 1)
					{
						tn.Normalize();
						tt.Normalize();
					}
				}
				else
				{
					tv = v;
					tn = n;
					tt = t;
				}
			}
		}
		else IF_NORMAL
		{
			for (uint i = vertices; i > 0; )
			{
				COMMON_LOOP;
				VBO_VERTEX;
				VBO_NORMAL;

				if (myWeights != 0)
				{
					for (uint b = 0; b < myWeights; ++b)
					{
						const byte&	 boneIndex	= bi[b];
						const float& boneWeight	= bw[b];

						ASSERT(boneIndex < transforms.GetSize(), "Bone index is out of range");

						const Matrix43& m = transforms[boneIndex];

						if (b == 0)
						{
							tv = (v	* m) * boneWeight;
							tn = (n % m) * boneWeight;
						}
						else
						{
							tv += (v * m) * boneWeight;
							tn += (n % m) * boneWeight;
						}
					}

					if (myWeights > 1)
					{
						tn.Normalize();
					}
				}
				else
				{
					tv = v;
					tn = n;
				}
			}
		}
		else
		{
			for (uint i = vertices; i > 0; )
			{
				COMMON_LOOP;
				VBO_VERTEX;

				if (myWeights != 0)
				{
					for (uint b = 0; b < myWeights; ++b)
					{
						const byte&	 boneIndex	= bi[b];
						const float& boneWeight	= bw[b];

						ASSERT(boneIndex < transforms.GetSize(), "Bone index is out of range");

						const Matrix43& m = transforms[boneIndex];

						if (b == 0)
						{
							tv = (v	* m) * boneWeight;
						}
						else
						{
							tv += (v * m) * boneWeight;
						}
					}
				}
				else
				{
					tv = v;
				}
			}
		}

		// Copy the local memory buffer to the VBO
		mTbo->Set(mMem.GetBuffer(), mMem.GetSize(), IVBO::Type::Vertex);
	}
	mTbo->Unlock();
	return true;
}

//============================================================================================================
// INTERNAL: Transforms vertices, normals, and tangents into vertex arrays
//============================================================================================================
// NOTE: Just like its VBO counterpart above this function was designed for speed over size.
//============================================================================================================

void Mesh::_TransformToVAs (const Array<Matrix43>& transforms)
{
	// Get the number of vertices
	uint maxWeights = GetNumberOfWeights();

	// If the vertices changed, resize the transformed arrays
	{
		if (mV.GetSize() != mTv.GetSize())
		{
			mTv.Clear();
			mTv.ExpandTo(mV.GetSize());
		}

		if (mN.GetSize() != mTn.GetSize())
		{
			mTn.Clear();
			mTn.ExpandTo(mN.GetSize());
		}

		if (mT.GetSize() != mTt.GetSize())
		{
			mTt.Clear();
			mTt.ExpandTo(mT.GetSize());
		}
	}

	IF_TANGENT_AND_NORMAL
	{
		// Run through all vertices and calculate transformed values
		for (uint i = mV.GetSize(); i > 0; )
		{
			COMMON_LOOP;
			VA_VERTEX;
			VA_NORMAL;
			VA_TANGENT;

			if (myWeights != 0)
			{
				for (uint b = 0; b < myWeights; ++b)
				{
					const byte&	 boneIndex	= bi[b];
					const float& boneWeight	= bw[b];

					ASSERT(boneIndex < transforms.GetSize(), "Bone index is out of range");

					const Matrix43& m = transforms[boneIndex];

					if (b == 0)
					{
						tv = (v	* m) * boneWeight;
						tn = (n % m) * boneWeight;
						tt = (t % m) * boneWeight;
					}
					else
					{
						tv += (v * m) * boneWeight;
						tn += (n % m) * boneWeight;
						tt += (t % m) * boneWeight;
					}
				}

				if (myWeights > 1)
				{
					tn.Normalize();
					tt.Normalize();
				}
			}
			else
			{
				tv = v;
				tn = n;
				tt = t;
			}
		}
	}
	else IF_NORMAL
	{
		for (uint i = mV.GetSize(); i > 0; )
		{
			COMMON_LOOP;
			VA_VERTEX;
			VA_NORMAL;

			if (myWeights != 0)
			{
				for (uint b = 0; b < myWeights; ++b)
				{
					const byte&	 boneIndex	= bi[b];
					const float& boneWeight	= bw[b];

					ASSERT(boneIndex < transforms.GetSize(), "Bone index is out of range");

					const Matrix43& m = transforms[boneIndex];

					if (b == 0)
					{
						tv = (v	* m) * boneWeight;
						tn = (n % m) * boneWeight;
					}
					else
					{
						tv += (v * m) * boneWeight;
						tn += (n % m) * boneWeight;
					}
				}

				if (myWeights > 1)
				{
					tn.Normalize();
				}
			}
			else
			{
				tv = v;
				tn = n;
			}
		}
	}
	else
	{
		for (uint i = mV.GetSize(); i > 0; )
		{
			COMMON_LOOP;
			VA_VERTEX;

			if (myWeights != 0)
			{
				for (uint b = 0; b < myWeights; ++b)
				{
					const byte&	 boneIndex	= bi[b];
					const float& boneWeight	= bw[b];

					ASSERT(boneIndex < transforms.GetSize(), "Bone index is out of range");

					const Matrix43& m = transforms[boneIndex];

					if (b == 0)
					{
						tv = (v	* m) * boneWeight;
					}
					else
					{
						tv += (v * m) * boneWeight;
					}
				}
			}
			else
			{
				tv = v;
			}
		}
	}
}

//============================================================================================================
// Returns the number of vertex entries
//============================================================================================================

uint Mesh::GetNumberOfVertices() const
{
	uint vertices = mV.GetSize();

	if (vertices == 0)
	{
		vertices = mN.GetSize();

		if (vertices == 0)
		{
			vertices = mTc.GetSize();

			if (vertices == 0)
			{
				vertices = mTv.GetSize();
			}
		}
	}
	return vertices;
}

//============================================================================================================
// Draws the mesh, sending the data to the graphics controller
//============================================================================================================

uint Mesh::Draw (IGraphics* graphics)
{
	uint result(0);

	Lock();
	{
#ifdef _DEBUG
		if (mGraphics != 0) ASSERT(mGraphics == graphics, "Graphics controller doesn't match!");
#endif

		mGraphics = graphics;

		// Determine the number of vertices
		uint vertices = GetNumberOfVertices();

		// If the vertex count has been reset, and there is data present, recreate the VBO
		if ( mVboSize == 0 && vertices != 0 )
		{
#ifdef _DEBUG
			// All sizes must match up
			ASSERT(mV.IsEmpty()  || mV.GetSize()  == vertices, "Size mismatch!");
			ASSERT(mN.IsEmpty()  || mN.GetSize()  == vertices, "Size mismatch!");
			ASSERT(mT.IsEmpty()  || mT.GetSize()  == vertices, "Size mismatch!");
			ASSERT(mTc.IsEmpty() || mTc.GetSize() == vertices, "Size mismatch!");
			ASSERT(mBi.IsEmpty() || mBi.GetSize() == vertices, "Size mismatch!");
			ASSERT(mBw.IsEmpty() || mBw.GetSize() == vertices, "Size mismatch!");
#endif
			// Update the size of the VBO
			mVboSize = mFormat.mFullSize * vertices;
			ASSERT(mVboSize > 0, "Invalid VBO size");

			// If the VBO hasn't been created yet, create it
			if (mVbo == 0) mVbo = graphics->CreateVBO();

			if (mVbo != 0 && mVboSize > 0)
			{
				// Remember whether the buffer already had something in it or not
				bool clear = !mMem.IsValid();

				// Resize the buffer to fit our VBO's contents
				byte* ptr = mMem.Resize(mVboSize);

				// Fill in all the vertex information
				for (uint i = vertices; i > 0; )
				{
					--i;
					byte* current = (ptr + mFormat.mFullSize * i);

					IF_VERTEX		CURRENT_VERTEX		= mV[i];
					IF_NORMAL		CURRENT_NORMAL		= mN[i];
					IF_TANGENT		CURRENT_TANGENT		= mT[i];
					IF_TEXCOORD		CURRENT_TEXCOORD	= mTc[i];
					IF_BONEINDEX	CURRENT_BONEINDEX	= mBi[i];
					IF_BONEWEIGHT	CURRENT_BONEWEIGHT	= mBw[i];
				}

				mVbo->Lock();
				mVbo->Set(mMem.GetBuffer(), mMem.GetSize(), IVBO::Type::Vertex);
				mVbo->Unlock();

				// If the memory was not used before, clear it
				if (clear) mMem.Release();
			}
		}

		// Index buffer
		if ( mIboSize == 0 && mIndices.IsValid() )
		{
			mIboSize = mIndices.GetSize();

			if (mIbo == 0) mIbo = graphics->CreateVBO();

			if (mIbo != 0)
			{
				mIbo->Lock();
				mIbo->Set(mIndices, IVBO::Type::Index, false);
				mIbo->Unlock();
			}
		}

		// Only proceed if there is something to render
		if ( vertices != 0 && mIndices.IsValid() )
		{
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color, 0 );
			graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1, 0 );

			// Normals
			{
				if ( mTn.GetSize() == vertices )
				{
					// If there are transformed normals to use, use them instead
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal, mTn );
				}
				else IF_NORMAL
				{
					if (mTboSize != 0)
					{
						// Pre-transformed normals in the transformed VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal, mTbo,
							mFormat.mNormal, IGraphics::DataType::Float, 3, mFormat.mTransSize );
					}
					else if (mVbo != 0)
					{
						// No transformed normals, but there are normals present in the VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal, mVbo,
							mFormat.mNormal, IGraphics::DataType::Float, 3, mFormat.mFullSize );
					}
					else
					{
						// No VBO support
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal, mN );
					}
				}
				else
				{
					// No normals present
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal, 0 );
				}
			}

			// Tangents
			{
				if ( mTt.GetSize() == vertices )
				{
					// Transformed tangents present -- use them
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent, mTt );
				}
				else IF_TANGENT
				{
					if (mTboSize != 0)
					{
						// Pre-transformed tangents in the transformed VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent, mTbo,
							mFormat.mTangent, IGraphics::DataType::Float, 3, mFormat.mTransSize );
					}
					else if (mVbo != 0)
					{
						// Tangents are in the VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent, mVbo,
							mFormat.mTangent, IGraphics::DataType::Float, 3, mFormat.mFullSize );
					}
					else
					{
						// No VBO support
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent, mT );
					}
				}
				else
				{
					// No tangents present
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent, 0 );
				}
			}

			// Texture coordinates
			{
				IF_TEXCOORD
				{
					if (mVbo != 0)
					{
						// Texture coordinates are in the VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0, mVbo,
							mFormat.mTexCoord, IGraphics::DataType::Float, 2, mFormat.mFullSize );
					}
					else
					{
						// No VBO support
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0, mTc );
					}
				}
				else
				{
					// No texture coordinates
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0, 0 );
				}
			}

			// Bone Weights
			{
				IF_BONEWEIGHT
				{
					if (mVbo != 0)
					{
						// Texture coordinates are in the VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight, mVbo,
							mFormat.mBoneWeight, IGraphics::DataType::Float, 4, mFormat.mFullSize );
					}
					else
					{
						// No VBO support
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight, mBw );
					}
				}
				else
				{
					// No texture coordinates
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight, 0 );
				}
			}

			// Bone Indices
			{
				IF_BONEINDEX
				{
					if (mVbo != 0)
					{
						// Texture coordinates are in the VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex, mVbo,
							mFormat.mBoneIndex, IGraphics::DataType::Byte, 4, mFormat.mFullSize );
					}
					else
					{
						// No VBO support
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex, mBi );
					}
				}
				else
				{
					// No texture coordinates
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex, 0 );
				}
			}

			bool draw = false;

			// Vertex positions
			{
				if ( mTv.GetSize() == vertices )
				{
					// Transformed vertices present
					graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, mTv );
					draw = true;
				}
				else IF_VERTEX
				{
					if (mTboSize != 0)
					{
						// Vertices are in the transformed VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, mTbo,
							mFormat.mVertex, IGraphics::DataType::Float, 3, mFormat.mTransSize );
					}
					else if (mVbo != 0)
					{
						// Vertices are in the VBO
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, mVbo,
							mFormat.mVertex, IGraphics::DataType::Float, 3, mFormat.mFullSize );
					}
					else
					{
						// No VBO support
						graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position, mV );
					}
					draw = true;
				}
			}

			if (draw)
			{
				if (mIbo != 0)
				{
					// Indices are in the VBO
					result += graphics->DrawIndices( mIbo, mPrimitive, mIboSize );
				}
				else
				{
					// No VBO support
					result += graphics->DrawIndices( mIndices, mPrimitive, mIndices.GetSize() );
				}
			}
		}
	}
	Unlock();
	return result;
}

//============================================================================================================
// Executes a tree node containing mesh information
//============================================================================================================

bool Mesh::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	if (IsValid() && !forceUpdate) return true;

	Lock();
	{
		_Clear();

		bool vertices	= false,
			 normals	= false,
			 texCoords	= false,
			 boneInfo	= false,
			 indices	= false;

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			bool success = true;

			const TreeNode& node	= root.mChildren[i];
			const String&	tag		= node.mTag;
			const Variable&	value	= node.mValue;

			if ( tag == "Vertices" )
			{
				if (value.IsVector3fArray())
				{
					mV.CopyMemory(value.AsVector3fArray());
					success  = true;
					vertices = true;
				}
			}
			else if ( tag == "Normals" )
			{
				if (value.IsVector3fArray())
				{
					mN.CopyMemory(value.AsVector3fArray());
					success = true;
					normals = true;
					mGeneratedNormals = false;
				}
			}
			else if ( tag == "TexCoords 0" )
			{
				if (value.IsVector2fArray())
				{
					mTc.CopyMemory(value.AsVector2fArray());
					success = true;
					texCoords = true;
				}
			}
			else if ( tag == "Bone Weights" )
			{
				if (value.IsColor4fArray())
				{
					mBw.CopyMemory(value.AsColor4fArray());
					success = true;
					boneInfo = true;
				}
			}
			else if ( tag == "Bone Indices" )
			{
				if (value.IsColor4ubArray())
				{
					mBi.CopyMemory(value.AsColor4ubArray());
					success = true;
					boneInfo = true;
				}
			}
			else
			{
				uint primitive = ::GetPrimitive(tag);

				if (primitive != -1)
				{
					mPrimitive = primitive;

					if (value.IsUShortArray())
					{
						mIndices.CopyMemory(value.AsUShortArray());
						success = true;
						indices = true;
					}
				}
			}

			if (!success)
			{
				ASSERT(false, "Failed to parse the mesh");
				Unlock();
				return false;
			}
		}

		// Update everything
		Update(vertices, mGeneratedNormals, normals || texCoords, texCoords, boneInfo, indices);
	}
	Unlock();
	return true;
}

//============================================================================================================
// Serialization -- save
//============================================================================================================

bool Mesh::SerializeTo (TreeNode& root) const
{
	if (mV.GetSize() > 0 && mIndices.GetSize() > 0)
	{
		Lock();
		{
			TreeNode& node = root.AddChild("Mesh");
			node.mValue = mName;

			if (mV.IsValid())
			{
				TreeNode& child	= node.AddChild("Vertices");
				child.mValue.ToVector3fArray().CopyMemory(mV);
			}

			if (mN.IsValid() && !mGeneratedNormals)
			{
				TreeNode& child = node.AddChild("Normals");
				child.mValue.ToVector3fArray().CopyMemory(mN);
			}

			if (mT.IsValid())
			{
				TreeNode& child = node.AddChild("TexCoords 0");
				child.mValue.ToVector2fArray().CopyMemory(mTc);
			}

			if ( GetNumberOfWeights() > 0 )
			{
				TreeNode& bw = node.AddChild("Bone Weights");
				bw.mValue.ToColor4fArray().CopyMemory(mBw);

				TreeNode& bi = node.AddChild("Bone Indices");
				bi.mValue.ToColor4ubArray().CopyMemory(mBi);
			}

			if (HasIndices())
			{
				TreeNode& faceNode = node.AddChild(GetType(mPrimitive));
				faceNode.mValue.ToUShortArray().CopyMemory(mIndices);
			}
		}
		Unlock();
		return true;
	}
	return false;
}