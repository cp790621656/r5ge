#include "../Include/_All.h"

#define POSITION_SCALE 0.2f
#undef CopyMemory

//============================================================================================================
// Required R5 Libraries
//============================================================================================================

#pragma comment(lib, "R5_Basic.lib")
#pragma comment(lib, "R5_Math.lib")
#pragma comment(lib, "R5_Serialization.lib")

//============================================================================================================
// Converts Max' matrix into R5's matrix. It's a straight copy since they match. There is something
// different about Max' quaternions or matrix-to-quaternion conversion however, so it's best to work with
// R5 types. Working with Max types then converting to R5 at a later point gives unexpected results.
//============================================================================================================

Matrix43 GetMatrix43 (::Matrix3& mat)
{
	mat.NoScale();
	float* f = (float*)&mat;
	Matrix43 m43;

	for (uint i = 0, b = 0; i < 12; ++b)
	{
		m43[b++] = f[i++];
		m43[b++] = f[i++];
		m43[b++] = f[i++];
	}
	return m43;
}

//============================================================================================================
// Max sub-materials mean we have to split mesh into sub-meshes, one per sub-material
//============================================================================================================

struct SubMesh
{
	typedef R5MaxExporter::VertexArray	VertexArray;
	typedef R5MaxExporter::IndexArray	IndexArray;

	VertexArray		mVertices;		// Array of vertices
	IndexArray		mIndices;		// Index array in triangles
};

//============================================================================================================
// 3ds Max-like mesh containing multiple sub-meshes. Sub-meshes is what R5 needs.
//============================================================================================================

struct MultiMesh
{
public:

	typedef SubMesh* SubMeshPtr;

	PointerArray<SubMesh>	m_array;
	uint					mCount;
	bool					mHasTexCoords;
	bool					mHasWeights;

public:

	MultiMesh() : mCount(0), mHasTexCoords(false), mHasWeights(false) {}

	SubMeshPtr& GetSubMesh(uint id)
	{
		m_array.ExpandTo(id+1);

		if (m_array[id] == 0)
		{
			m_array[id] = new SubMesh();
			++mCount;
		}
		return m_array[id];
	}

	uint				GetSize()			 const	{ return m_array.GetSize(); }
	uint				GetValidCount()		 const	{ return mCount; }
	const SubMeshPtr&	operator [] (uint i) const	{ return m_array[i]; }
};

//============================================================================================================
// Returns the correct vertex normal
//============================================================================================================

::Point3 GetVertexNormal (::Mesh& maxMesh, int faceNum, int vertNum)
{
	int normalCount;

	::Point3	normal;
	::Face&		face  = maxMesh.faces[faceNum];
	::RVertex&	rVert = maxMesh.getRVert(face.getVert(vertNum));
    
    // Is normal specified, use it
    if (rVert.rFlags & SPECIFIED_NORMAL)
    {
        normal = rVert.rn.getNormal();
    }
    // If normal is not specified it's only available if the face belongs to a smoothing group
    else if ((normalCount = rVert.rFlags & NORCT_MASK) && face.smGroup)
    {
        // If there is only one vertex, the normal is found in the rn member
        if (normalCount == 1)
        {
            normal = rVert.rn.getNormal();
        }
        else
        {
            // If there are two or more vertices, we need to step through them
            // and find the vertex with the same smoothing group as the current face.
            for (int j = 0; j < normalCount; j++)
            {
                if (rVert.ern[j].getSmGroup() & face.smGroup)
                {
                    normal = rVert.ern[j].getNormal();
                }
            }
        }
    }
    else
    {
        normal = maxMesh.getFaceNormal(faceNum);
    }
	return normal;
}

//============================================================================================================
// Quick check to see whether the object can be exported
//============================================================================================================

inline bool IsGeometric ( ::Object* obj )
{
	return ( obj != 0 && obj->IsRenderable() && obj->CanConvertToType(TRIOBJ_CLASSID) );
}

//============================================================================================================
// Whether the specified node is a bone
//============================================================================================================

bool IsBone (::INode* node)
{
	::Control* control = node->GetTMController();
	if (control == 0) return false;

	::Class_ID ctrlType = control->ClassID();
	if (ctrlType == BIPED_ROOT_CLASSID || ctrlType == BIPED_NODE_CLASSID) return true;

	::Object* object = node->EvalWorldState(0).obj;
	return (object != 0 && object->ClassID() == BONE_OBJ_CLASSID);
}

//============================================================================================================
// Retrieves a modifier from the list associated with the specified node
//============================================================================================================

::Modifier* GetModifier (::INode* node, const ::Class_ID& classID)
{
	if (node != 0)
	{
		::Object* ref = node->GetObjectRef();

		if (ref != 0 && ref->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		{
			::IDerivedObject* derived = (IDerivedObject*)ref;

			int count = derived->NumModifiers();

			for (int i = 0; i < count; ++i)
			{
				::Modifier* mod = derived->GetModifier(i);
				if (mod != 0 && mod->ClassID() == classID) return mod;
			}
		}
	}
	return 0;
}

//============================================================================================================
// Matching functions
//============================================================================================================

inline bool IsAMatch (const Vector3f& v0, const Vector3f& v1, bool precise)
{
	float threshold = (precise ? 0.0001f : 0.001f);

	return	Float::Abs(v0.x - v1.x) < threshold &&
			Float::Abs(v0.y - v1.y) < threshold &&
			Float::Abs(v0.z - v1.z) < threshold;
}

//============================================================================================================

inline bool IsAMatch (const Quaternion& q0, const Quaternion& q1, bool precise)
{
	if (!precise) return GetAngle(q0, q1) < 0.0017f;

	return (Float::Abs(q0.x - q1.x) < 0.0001f &&
			Float::Abs(q0.y - q1.y) < 0.0001f &&
			Float::Abs(q0.z - q1.z) < 0.0001f &&
			Float::Abs(q0.w - q1.w) < 0.0001f) ||
		   (Float::Abs(q0.x + q1.x) < 0.0001f &&
			Float::Abs(q0.y + q1.y) < 0.0001f &&
			Float::Abs(q0.z + q1.z) < 0.0001f &&
			Float::Abs(q0.w + q1.w) < 0.0001f);
}

//============================================================================================================
// Removes all duplicate or useless keys from the array
//============================================================================================================

void CleanKeys (R5MaxExporter::PosKeys& keys, const Vector3f& pos, bool precise)
{
	typedef R5MaxExporter::PosKey PosKey;

	bool eliminated = true;

	while (eliminated)
	{
		eliminated = false;

		for (uint i = keys.GetSize(); i > 2; ++i)
		{
			PosKey& next     = keys[--i];
			PosKey& current  = keys[--i];
			PosKey& previous = keys[--i];

			// If the center frame matches the other two, just drop it
			if ( IsAMatch(current.mPos, next.mPos, precise) && IsAMatch(current.mPos, previous.mPos, precise) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
				continue;
			}

			float range = (float)(next.mTime - previous.mTime);
			float time  = (float)(current.mTime - previous.mTime);

			if ( IsAMatch(current.mPos, Interpolation::Linear(previous.mPos, next.mPos, time / range), precise) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
			}
		}
	}

	if (keys.GetSize() > 1)
	{
		if ( IsAMatch(keys[0].mPos, keys[1].mPos, precise) )
		{
			if (keys.GetSize() == 2)
			{
				keys.Shrink();
			}
			else if ( IsAMatch(keys[0].mPos, pos, precise) )
			{
				keys.RemoveAt(0);
			}
		}
	}

	if (keys.GetSize() > 1)
	{
		uint back = keys.GetSize() - 1;
		uint prev = back - 1;

		if ( IsAMatch(keys[back].mPos, keys[prev].mPos, precise) )
		{
			keys.Shrink();
		}
	}

	if (keys.GetSize() == 1)
	{
		if ( IsAMatch(keys[0].mPos, pos, precise) )
		{
			keys.Shrink();
		}
	}
}

//============================================================================================================

void CleanKeys (R5MaxExporter::RotKeys& keys, const Quaternion& rot, bool precise)
{
	typedef R5MaxExporter::RotKey RotKey;

	bool eliminated = true;

	while (eliminated)
	{
		eliminated = false;

		for (uint i = keys.GetSize(); i > 2; ++i)
		{
			RotKey& next     = keys[--i];
			RotKey& current  = keys[--i];
			RotKey& previous = keys[--i];

			// If the center frame matches the other two, just drop it
			if ( IsAMatch(current.mRot, next.mRot, precise) && IsAMatch(current.mRot, previous.mRot, precise) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
				continue;
			}

			float range = (float)(next.mTime - previous.mTime);
			float time  = (float)(current.mTime - previous.mTime);

			if ( IsAMatch(current.mRot, Interpolation::Slerp(previous.mRot, next.mRot, time / range), precise) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
			}
		}
	}

	if (keys.GetSize() > 1)
	{
		if ( IsAMatch(keys[0].mRot, keys[1].mRot, precise) )
		{
			if (keys.GetSize() == 2)
			{
				keys.Shrink();
			}
			else if ( IsAMatch(keys[0].mRot, rot, precise) )
			{
				keys.RemoveAt(0);
			}
		}
	}

	// If the last two keys match, remove the very last one
	if (keys.GetSize() > 1)
	{
		uint back = keys.GetSize() - 1;
		uint prev = back - 1;

		if ( IsAMatch(keys[back].mRot, keys[prev].mRot, precise) )
		{
			keys.Shrink();
		}
	}

	// If the first rotation matches the bind pose, it's safe to get rid of it
	if (keys.GetSize() == 1)
	{
		if ( IsAMatch(keys[0].mRot, rot, precise) )
		{
			keys.Shrink();
		}
	}

	// Ensure that all rotation keys take the shortest path
	for (uint i = 1; i < keys.GetSize(); ++i)
	{
		RotKey& prev = keys[i-1];
		RotKey& curr = keys[i];

		if (prev.mRot.Dot(curr.mRot) < 0.0f)
		{
			curr.mRot.Flip();
		}
	}
}

//============================================================================================================
// Adds the specified weight to the list, assuming it has a greater effect than existing ones
//============================================================================================================

void R5MaxExporter::Vertex::AddBoneWeight (unsigned char boneIndex, float weight)
{
	unsigned char*	bi	= &mBoneIndices.r;
	float*			bw	= &mBoneWeights.r;

	for (uint i = 0; i < 4; ++i)
	{
		if (weight > bw[i])
		{
			// Shift all entries toward the back
			for (uint b = 3; b > i; --b)
			{
				bw[b] = bw[b-1];
				bi[b] = bi[b-1];
			}

			// Overwrite the current
			bw[i] = weight;
			bi[i] = boneIndex;
			break;
		}
	}
}

//============================================================================================================
// Normalizes all bone weights, making them equal to a combined weight value of 1.0
//============================================================================================================

void R5MaxExporter::Vertex::NormalizeBoneWeights()
{
	float total = mBoneWeights.r + mBoneWeights.g + mBoneWeights.b + mBoneWeights.a;
	if (total > 0.0f) mBoneWeights *= 1.0f / total;
}

//============================================================================================================
// Releases all existing scene information
//============================================================================================================

void R5MaxExporter::Release()
{
	mLock.Lock();
	{
		mStage = 0;
		mBones.Release();
		mLimbs.Release();
		mMeshes.Release();
		mMaterials.Release();
	}
	mLock.Unlock();
}

//============================================================================================================
// Retrieves a bone by its name
//============================================================================================================

R5MaxExporter::Bone* R5MaxExporter::GetBone (const String& name)
{
	ASSERT(name.IsValid(), "Missing bone name?");

	for (uint i = 0; i < mBones.GetSize(); ++i)
		if (mBones[i].mName == name)
			return &mBones[i];

	Bone& bone		= mBones.Expand();
	bone.mName		= name;
	bone.mParent	= -1;
	return &bone;
}

//============================================================================================================
// Retrieves a bone by its index
//============================================================================================================

R5MaxExporter::Bone* R5MaxExporter::GetBone (uint index)
{
	return (index < mBones.GetSize() ? &mBones[index] : 0);
}

//============================================================================================================
// Retrieves a bone by its name
//============================================================================================================

uint R5MaxExporter::GetBoneIndex (const String& name)
{
	ASSERT(name.IsValid(), "Missing bone name?");

	for (uint i = 0; i < mBones.GetSize(); ++i)
		if (mBones[i].mName == name)
			return i;

	// We should already have the bone struct *before* exporting the skin
	ASSERT(mBones.IsValid(), "Missing bones?");
	return 0;
}

//============================================================================================================
// Parse 3ds max mesh information into a format easier understood by R5
//============================================================================================================

void R5MaxExporter::_FillMultiMesh ( MultiMesh&			myMultiMesh,
									::Mesh&				maxMesh,
									::Mtl*				maxMtl,
									::Matrix3&			tm,
									::ISkin*			skin,
									::ISkinContextData*	skinData,
									bool				onlyVertices)
{
	uint	vertexCount = maxMesh.getNumVerts();
	uint	tcCount		= maxMesh.getNumTVerts();
	uint	faceCount	= maxMesh.getNumFaces();
	ushort	matIndex	= maxMesh.getMtlIndex();
	bool	multiMat	= (maxMtl != 0 && maxMtl->IsMultiMtl() == TRUE);

	Vector3f v, n;
	Vector2f t;

	// Create an array of cross-indices: an index of R5 bone for every Max bone present in the skin
	int boneCount = (onlyVertices || skin == 0 ? 0 : skin->GetNumBones());
	Array<uint> boneIndices ( boneCount );

	// Remember whether the mesh has texture coordinates and bone weights
	myMultiMesh.mHasTexCoords	= !onlyVertices && (maxMesh.tvFace != 0);
	myMultiMesh.mHasWeights		= !onlyVertices && (boneCount > 0);

	for (int i = 0; i < boneCount; ++i)
	{
		// GetBoneName() sometimes returns completely different names! wtf?
		//TCHAR* name = skin->GetBoneName(i);
		INode* boneNode = skin->GetBone(i);
		TCHAR* nodeName = boneNode->GetName();
		boneIndices.Expand() = GetBoneIndex( nodeName );
	}

	// Apparently parity determines whether the indices are clockwise or counter-clockwise
	bool reverseIndices = (tm.Parity() == 1);

	// Run through all faces and collect vertex information into a multi-mesh
	for (uint iFace = 0; iFace < faceCount; ++iFace)
	{
		if (maxMesh.faces == 0) continue;

		::Face& face = maxMesh.faces[iFace];

		unsigned short	mtlId	(multiMat ? maxMesh.getFaceMtlIndex(iFace) : 0);
		SubMesh*		myMesh	(myMultiMesh.GetSubMesh(mtlId));

		SubMesh::VertexArray&	myVertices	(myMesh->mVertices);
		SubMesh::IndexArray&	myIndices	(myMesh->mIndices);

		unsigned short myTriangle[3];

		// There are always 3 vertices per tri-mesh face
		for (uint iVert = 0; iVert < 3; ++iVert)
		{
			int maxIndex = face.v[iVert];

			v = tm.PointTransform ( maxMesh.getVert(maxIndex) );

			if (!onlyVertices)
			{
				// Matrix3::VectorTransform() actually applies the scaling as well, so Normalize() is required
				n = tm.VectorTransform( GetVertexNormal(maxMesh, iFace, iVert) );
				n.Normalize();

				// If texture coordinates are present, use them
				if (maxMesh.tvFace != 0)
				{
					TVFace& tvFace = maxMesh.tvFace[iFace];
					t = maxMesh.getTVert( tvFace.t[iVert] );
				}
			}

			// Adjust by position scale
			v *= POSITION_SCALE;

			// We need to check to see if the vertex/normal/texcoord combo already exists in the list
			uint myIndex = -1;

			// Check to see if this vertex is already present
			for (uint c = 0; c < myVertices.GetSize(); ++c)
			{
				if (onlyVertices)
				{
					if (myVertices[c].mPos == v)
					{
						// Existing entry
						myIndex = c;
						break;
					}
				}
				else if (myVertices[c].Matches(v, n, t))
				{
					// Existing entry -- remember the index
					myIndex = c;
					break;
				}
			}

			if (myIndex == -1)
			{
				// If it's a new entry, simply add it to the end of the vertex list
				myIndex = myVertices.GetSize();
				Vertex& vertex = myVertices.Expand();

				// Set the vertex position, normal, and texture coordinates, then clear all bone weights
				vertex.Set(v, n, t);
				vertex.ClearBoneWeights();

				// If there is a skin present, run through all influences of this vertex and assign the bone weights
				if (!onlyVertices && boneCount > 0)
				{
					int influences = skinData->GetNumAssignedBones(maxIndex);

					for (int i = 0; i < influences; ++i)
					{
						int assigned = skinData->GetAssignedBone (maxIndex, i);
						float weight = skinData->GetBoneWeight   (maxIndex, i);

						uint boneIndex = boneIndices[assigned];
						vertex.AddBoneWeight(boneIndex, weight);
					}

					// Normalize all bone weights, just in case they don't equal a combined value of 1
					vertex.NormalizeBoneWeights();
				}
			}

			if (!onlyVertices)
			{
				// For safety purposes limit the maximum number of vertices to 65535
				ASSERT(myIndex < 65535, "Indices exceed the limit of an unsigned short");

				// Remember the index
				myTriangle[iVert] = (unsigned short)myIndex;
			}
		}

		// Update the myTriangle array
		if (!onlyVertices)
		{
			if (reverseIndices)
			{
				myIndices.Expand() = myTriangle[2];
				myIndices.Expand() = myTriangle[1];
				myIndices.Expand() = myTriangle[0];
			}
			else
			{
				myIndices.Expand() = myTriangle[0];
				myIndices.Expand() = myTriangle[1];
				myIndices.Expand() = myTriangle[2];
			}
		}
	}
}

//============================================================================================================
// Creates a material recognized by R5 based on the Max material
//============================================================================================================

R5MaxExporter::Material*  R5MaxExporter::_ConvertMaterial (::Mtl* mtl, uint subMatIndex)
{
	Material* myMat (0);

	// If the mesh has an assigned material, use it
	if (mtl != 0)
	{
		String matName (mtl->GetName().data());

		// For some reason the materials need to be updated before colors are retrieved
		mtl->Update(0, ::Interval());

		// If this is a multi-material, move on to the sub-material
		if (mtl->IsMultiMtl())
		{
			mtl = mtl->GetSubMtl(subMatIndex);
			if (mtl) matName.Append(" - %s", mtl->GetName().data());
		}

		if (mtl != 0)
		{
			Color3f ambient		(mtl->GetAmbient());
			Color3f diffuse		(mtl->GetDiffuse());
			Color3f specular	(mtl->GetSpecular() * mtl->GetShinStr());
			Color3f emission	(mtl->GetSelfIllumColor() * mtl->GetSelfIllum());
			float	shininess	(mtl->GetShininess());
			float	alpha		(1.0f - mtl->GetXParency());
			bool	twosided	((mtl->Requirements(0) & MTLREQ_2SIDE)	 != 0);
			bool	wireframe	((mtl->Requirements(0) & MTLREQ_WIRE)	 != 0);
			bool	faceted		((mtl->Requirements(0) & MTLREQ_FACETED) != 0);

			// Retrieve the material and set its properties
			myMat				= GetMaterial(matName);
			myMat->mDiffuse		= diffuse;
			myMat->mSpecular	= specular;
			myMat->mDiffuse.a	= alpha;
			myMat->mSpecular.a	= shininess;
			myMat->mGlow		= (emission.r + emission.g + emission.b) / 3.0f;
			myMat->mWireframe	= wireframe;
			myMat->mTwosided	= twosided;
			myMat->mClouds		= faceted;

			// Leaving for future reference -- texture retrieval:
			//::BitmapTex* diffuseTex		= dynamic_cast<BitmapTex*>(mtl->GetSubTexmap(ID_DI));
			//::BitmapTex* specularTex		= dynamic_cast<BitmapTex*>(mtl->GetSubTexmap(ID_SP));
			//::BitmapTex* opacityTex		= dynamic_cast<BitmapTex*>(mtl->GetSubTexmap(ID_OP));
			//::BitmapTex* bumpTex			= dynamic_cast<BitmapTex*>(mtl->GetSubTexmap(ID_BU));
			//::BitmapTex* reflectionTex	= dynamic_cast<BitmapTex*>(mtl->GetSubTexmap(ID_RL));
		}
	}

	// If the material wasn't assigned, just use the default material
	if (myMat == 0)
	{
		myMat = GetMaterial("Default Material");
		myMat->mDiffuse.Set (1.0f, 1.0f, 1.0f, 1.0f);
		myMat->mSpecular.Set(0.0f, 0.0f, 0.0f, 0.2f);
		myMat->mGlow		= 0.0f;
		myMat->mWireframe	= false;
		myMat->mTwosided	= false;
		myMat->mClouds		= false;
	}
	return myMat;
}

//============================================================================================================
// Create limbs recognized by R5 (mesh+material pairings)
//============================================================================================================

void R5MaxExporter::_CreateLimbs (  MultiMesh&		myMultiMesh,
									::Mtl*			maxMtl,
									::TimeValue		time,
									const String&	meshName )
{
	// Run through all multi-mesh entries
	for (uint i = 0; i < myMultiMesh.GetSize(); ++i)
	{
		const SubMesh* mySub (myMultiMesh[i]);

		if (mySub)
		{
			String myName (meshName);

			// If the compound mesh has more than 1 sub-mesh, each one should become a separate mesh
			if (myMultiMesh.GetValidCount() > 1)
			{
				myName.Append(" [%d]", i);
			}

			// Get the mesh and copy over the list of vertices and indices
			Mesh* myMesh			= GetMesh(myName);
			myMesh->mHasTexCoords	= myMultiMesh.mHasTexCoords;
			myMesh->mHasWeights		= myMultiMesh.mHasWeights;

			myMesh->mVertices.CopyMemory( mySub->mVertices );
			myMesh->mIndices.CopyMemory ( mySub->mIndices  );

			// Get the limb and assign its mesh
			Limb* myLimb	= GetLimb(myName);
			myLimb->mMesh	= myMesh;
			myLimb->mMat	= _ConvertMaterial(maxMtl, i);

			// Special case: "faceted" marked meshes get exported as billboard clouds instead
			myLimb->mMesh->mClouds = myLimb->mMat->mClouds;
		}
	}
}

//============================================================================================================
// Appends the specified control's keys to the list of key times
//============================================================================================================

void AppendKeys (Array<int>& keyTimes, ::Animatable* ctrl)
{
	if (ctrl == 0) return;
	int numKeys = ctrl->NumKeys();
	if (ctrl->NumKeys() == NOT_KEYFRAMEABLE) return;

	for (int i = 0; i < numKeys; ++i)
	{
		int timeVal = ctrl->GetKeyTime(i);
		keyTimes.AddUnique(timeVal);
	}
}

//============================================================================================================
// Exports keys relying on Max to tell us when those keys should be
//============================================================================================================

void R5MaxExporter::_ExportKeys( Bone* bone, ::INode* node, ::INode* parent, ::Interval interval )
{
	bone->mSpline = true;

	Array<int> posTimes;
	Array<int> rotTimes;

	::Control* control = node->GetTMController();
	AppendKeys(posTimes, control->SubAnim(VERTICAL_SUBANIM));
	AppendKeys(posTimes, control->SubAnim(HORIZONTAL_SUBANIM));
	AppendKeys(rotTimes, control->SubAnim(ROTATION_SUBANIM));
	AppendKeys(posTimes, control->GetPositionController());
	AppendKeys(rotTimes, control->GetRotationController());

	posTimes.Sort();
	rotTimes.Sort();

	// Run through all position keyframes and calculate the relative coordinates
	for (uint i = 0; i < posTimes.GetSize(); ++i)
	{
		int time = posTimes[i];

		Matrix43 myMat (GetMatrix43(node->GetNodeTM(time)));

		if (bone->mParent != -1)
		{
			Matrix43 myParent (GetMatrix43(parent->GetNodeTM(time)));
			myParent.Invert();
			myMat *= myParent;
		}

		PosKey& kf	 = bone->mPosKeys.Expand();
		kf.mTime	 = (uint)(time / GetTicksPerFrame());
		kf.mPos	 = myMat;
		kf.mPos	*= POSITION_SCALE;
	}

	// Same thing with the rotation keyframes
	for (uint i = 0; i < rotTimes.GetSize(); ++i)
	{
		int time = rotTimes[i];

		Matrix43 myMat (GetMatrix43(node->GetNodeTM(time)));

		if (bone->mParent != -1)
		{
			Matrix43 myParent (GetMatrix43(parent->GetNodeTM(time)));
			myParent.Invert();
			myMat *= myParent;
		}

		RotKey& kf	= bone->mRotKeys.Expand();
		kf.mTime	= (uint)(time / GetTicksPerFrame());
		kf.mRot	= myMat;
	}
}

//============================================================================================================
// Exports keys the brute force way, ignoring Max and simply calculating bone positions at every single frame
//============================================================================================================

void R5MaxExporter::_ExportFull( Bone* bone, ::INode* node, ::INode* parent, ::Interval interval )
{
	// Note that this should be 'false' if bipeds with interpolated keyframes are used
	bone->mSpline = true;

	int frameSize  = GetTicksPerFrame();
	int firstFrame = interval.Start() / frameSize;
	int lastFrame  = interval.End()	/ frameSize;

	// Reserve the size in the array so the memory is not re-allocated
	bone->mPosKeys.Reserve( (uint)(lastFrame - firstFrame) );
	bone->mRotKeys.Reserve( (uint)(lastFrame - firstFrame) );

	// Run through every single frame and figure out each position and rotation relative to the parent
	for (int i = firstFrame; i <= lastFrame; ++i)
	{
		Matrix43 myMat (GetMatrix43(node->GetNodeTM(i * frameSize)));

		if (bone->mParent != -1)
		{
			Matrix43 myParent (GetMatrix43(parent->GetNodeTM(i * frameSize)));
			myParent.Invert();
			myMat *= myParent;
		}

		PosKey& pk	= bone->mPosKeys.Expand();
		pk.mTime	= (uint)i;
		pk.mPos		= myMat;
		pk.mPos		*= POSITION_SCALE;

		RotKey& rk	= bone->mRotKeys.Expand();
		rk.mTime	= (uint)i;
		rk.mRot		= myMat;
	}
}

//============================================================================================================
// Export the specified bone
//============================================================================================================

void R5MaxExporter::ExportBone ( ::INode* node, ::Interval interval, bool isBipedRoot, bool isBiped )
{
	::INode* parent = node->GetParentNode();
	if (parent == 0) return;

	::Control* control = node->GetTMController();
	if (control == 0) return;

	// Get the bone we're working with
	Bone* bone = GetBone( node->GetName() );

	// Save the bone's parent
	bone->mParent = (!isBipedRoot && IsBone(parent) ? GetBoneIndex( parent->GetName() ) : -1);

	// Bind pose
	{
		// Get the initial transformation matrix
		Matrix43 myMat (GetMatrix43(node->GetNodeTM(0)));

		// Get the parent's transformation matrix and make this node's matrix relative to the parent
		if (bone->mParent != -1)
		{
			Matrix43 myParent (GetMatrix43(parent->GetNodeTM(0)));
			myParent.Invert();
			myMat *= myParent;
		}

		// Set the bone's bind pose orientation
		bone->mPos = myMat;
		bone->mRot = myMat;
		bone->mPos *= POSITION_SCALE;
	}

	// Unfortunately bipeds require insane precision in order to export them out properly
	bool precise = isBiped;/* && (isBipedRoot		||
				   (bone->mName.BeginsWith("Bip")	&&
				   (bone->mName.Contains("Spine")	||
					bone->mName.Contains("Pelvis")	||
					bone->mName.Contains("L Thigh")	||
					bone->mName.Contains("R Thigh")	||
					bone->mName.Contains("L Calf")	||
					bone->mName.Contains("R Calf"))));*/

	// If this bone can be exported low precision, do so now
	if (!precise)
	{
		_ExportKeys(bone, node, parent, interval);

		CleanKeys (bone->mPosKeys, bone->mPos, precise);
		CleanKeys (bone->mRotKeys, bone->mRot, precise);

		// If we happen to get no keys out of the process, let's try the high precision next
		precise = (bone->mPosKeys.IsEmpty() && bone->mRotKeys.IsEmpty());
	}

	// High precision key extraction runs through every single frame
	if (precise)
	{
		_ExportFull(bone, node, parent, interval);

		CleanKeys (bone->mPosKeys, bone->mPos, precise);
		CleanKeys (bone->mRotKeys, bone->mRot, precise);
	}
}

//============================================================================================================
// Exports the specified node's geometry
//============================================================================================================

void R5MaxExporter::ExportGeometry (::INode* node, ::Object* object, ::TimeValue time, ::ISkin* skin)
{
	::TriObject* triObj = static_cast<TriObject*>(object->ConvertToType(time, TRIOBJ_CLASSID));

	if (triObj != 0)
	{
		if (triObj->CheckObjectIntegrity())
		{
			// If the skin information is present, we only want to export geometry at its original state
			if (skin != 0) time = 0;

			// We need the Max mesh, material, transformation matrix
			::Mesh&				maxMesh  ( triObj->GetMesh() );
			::Mtl*				maxMtl	 ( node->GetMtl() );
			::Matrix3			tm		 ( node->GetObjectTM(time) );
			::ISkinContextData* skinData ( skin == 0 ? 0 : skin->GetContextInterface(node) );

			// ::Mesh::checkNormals(1) function actually creates normals if they are not present
			maxMesh.checkNormals(1);

			MultiMesh myMultiMesh;

			// Every Max mesh can support sub-materials, essentially making us split the meshes up into sub-meshes
			_FillMultiMesh( myMultiMesh, maxMesh, maxMtl, tm, skin, skinData,
				(maxMtl->Requirements(0) & MTLREQ_FACETED) != 0 );

			// Convert submeshes to actual R5 meshes
			_CreateLimbs( myMultiMesh, maxMtl, time, node->GetName() );
		}

		if (triObj != object)
		{
			// TriObject needs to be deleted if pointers don't match
			triObj->DeleteMe();
		}
	}
}

//============================================================================================================
// Save the geometry in Ascii format
//============================================================================================================

bool R5MaxExporter::SaveAscii ( const String& filename )
{
	if (mLimbs.IsValid())
	{
		TreeNode root ("Root");
		TreeNode& graphics	= root.AddChild("Graphics");
		TreeNode& core		= root.AddChild("Core");
		TreeNode& model		= root.AddChild("Template");

		graphics.AddChild ("Serializable", false);
		core.AddChild     ("Serializable", false);
		model.AddChild	  ("Serializable", false);

		// Save the skeleton
		if (mBones.IsValid())
		{
			TreeNode& skel = core.AddChild("Skeleton");

			// Go through all available bones
			for (uint i = 0; i < mBones.GetSize(); ++i)
			{
				Bone& bone		= mBones[i];
				TreeNode& node	= skel.AddChild("Bone");

				// Bone's index goes into the value slot
				(i >> node.mValue);

				// If the node has a parent, save its index
				if (bone.mParent != -1)
				{
					// Child bone -- save the name of the parent
					node.AddChild("Parent", bone.mParent);
				}
				else
				{
					// Root node -- set the name of the skeleton
					skel.mValue = bone.mName;
					model.AddChild("Skeleton", bone.mName);
				}

				// Save the bone's name so it can be identifier by something other than a number
				node.AddChild("Name", bone.mName);

				// Save the bone's starting position and rotation
				TreeNode& posNode = node.AddChild("Position", bone.mPos);
				TreeNode& rotNode = node.AddChild("Rotation", bone.mRot);

				// Save all position keyframes if they are present
				if (bone.mPosKeys.IsValid())
				{
					posNode.AddChild("Smooth", bone.mSpline);

					Array<ushort>& frames = posNode.AddChild("Frames").mValue.ToUShortArray();
					Array<Vector3f>& values = posNode.AddChild("Values").mValue.ToVector3fArray();

					for (uint k = 0; k < bone.mPosKeys.GetSize(); ++k)
					{
						PosKey& key	= bone.mPosKeys[k];
						frames.Expand() = (ushort)key.mTime;
						values.Expand() = key.mPos;
					}
				}

				// Same with the rotation keyframes
				if (bone.mRotKeys.IsValid())
				{
					rotNode.AddChild("Smooth", bone.mSpline);

					Array<ushort>& frames = rotNode.AddChild("Frames").mValue.ToUShortArray();
					Array<Quaternion>& values = rotNode.AddChild("Values").mValue.ToQuaternionArray();

					for (uint k = 0; k < bone.mRotKeys.GetSize(); ++k)
					{
						RotKey& key	= bone.mRotKeys[k];
						frames.Expand() = (ushort)key.mTime;
						values.Expand() = key.mRot;
					}
				}
			}
		}

		// Save all materials
		for (uint i = 0; i < mMaterials.GetSize(); ++i)
		{
			Material* mat	= mMaterials[i];
			TreeNode& node	= graphics.AddChild("Material", mat->mName);

			node.AddChild("Diffuse", mat->mDiffuse);
			node.AddChild("Specular", mat->mSpecular);
			node.AddChild("Glow", mat->mGlow);

			// Add the deferred technique first
			TreeNode& tech = node.AddChild("Technique");

			// Wireframe if the material is wireframe, deferred if it's solid, and post-deferred otherwise
			if		(mat->mWireframe)			tech.mValue = "Wireframe";
			else if (mat->mDiffuse.a == 1.0f)	tech.mValue = "Deferred";
			else								tech.mValue = "Post-Deferred";

			// Use the default material shader
			tech.AddChild("Shader", "Deferred/Material");

			// As a fallback option, add a non-deferred technique without any other options
			node.AddChild("Technique", (mat->mDiffuse.a == 1.0f ? "Opaque" : "Transparent"));
		}

		// Save all meshes
		for (uint i = 0; i < mMeshes.GetSize(); ++i)
		{
			Mesh* mesh		= mMeshes[i];
			TreeNode& node	= core.AddChild(mesh->mClouds ? "Cloud" : "Mesh", mesh->mName);
			uint vertices	= mesh->mVertices.GetSize();

			// Vertex information comes first
			if (mesh->mClouds)
			{
				float size = 1.0f;

				// Find the center
				Vector3f origin;
				for (uint b = 0; b < vertices; ++b) origin += mesh->mVertices[b].mPos;
				origin *= 1.0f / vertices;
				node.AddChild("Origin", origin);

				// Find the distance to the farthest point
				float radius = 0.0f;
				for (uint b = 0; b < vertices; ++b)
				{
					float current = origin.GetDistanceTo(mesh->mVertices[b].mPos);
					if (current > radius) radius = current;
				}

				// Starting radius of each billboard
				radius /= vertices;
				radius *= 10.0f;

				Array<Quaternion>& verts = node.AddChild(mesh->mClouds ? "Instances" : "Vertices").mValue.ToQuaternionArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v = mesh->mVertices[b];
					verts.Expand().Set(v.mPos.x, v.mPos.y, v.mPos.z, radius);
				}

				// Billboard clouds don't need to save normals, texture coordinates, or bone information
				mesh->mHasTexCoords = false;
				mesh->mHasWeights	= false;
			}
			else
			{
				Array<Vector3f>& verts		= node.AddChild("Vertices").mValue.ToVector3fArray();
				Array<Vector3f>& normals	= node.AddChild("Normals").mValue.ToVector3fArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v		= mesh->mVertices[b];
					verts.Expand()		= v.mPos;
					normals.Expand()	= v.mNormal;
				}
			}

			// Save the texture coordinates if they are present
			if (mesh->mHasTexCoords)
			{
				Array<Vector2f>& tc = node.AddChild("TexCoords 0").mValue.ToVector2fArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v = mesh->mVertices[b];
					tc.Expand() = v.mTc;
				}
			}

			// If the mesh contains skinning information, save it
			if (mesh->mHasWeights)
			{
				Array<Color4f>&		weights = node.AddChild("Bone Weights").mValue.ToColor4fArray();
				Array<Color4ub>&	indices = node.AddChild("Bone Indices").mValue.ToColor4ubArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v	 = mesh->mVertices[b];
					indices.Expand() = v.mBoneIndices;
					weights.Expand() = v.mBoneWeights;
				}
			}

			// Indices follow
			if (!mesh->mClouds)
			{
				TreeNode& indices = node.AddChild("Triangles");
				indices.mValue = mesh->mIndices;
			}
		}

		// Save all limbs
		for (uint i = 0; i < mLimbs.GetSize(); ++i)
		{
			Limb* limb		= mLimbs[i];
			TreeNode& node	= model.AddChild("Limb", limb->mName);

			node.AddChild(limb->mMesh->mClouds ? "Cloud" : "Mesh", limb->mMesh->mName);
			node.AddChild("Material", limb->mMat->mName);
		}

		// Save the file
		return root.Save(filename);
	}
	return true;
}

//============================================================================================================
// 3ds Max Export function
//============================================================================================================

int R5MaxExporter::DoExport (	const char*		filename,
								ExpInterface*	ei,
								Interface*		i,
								int				suppressPrompts,
								unsigned long	options )
{
	int retVal = 1;

	mLock.Lock();
	{
		mLimbs.Release();
		mMeshes.Release();
		mMaterials.Release();

		// Enum all bones first
		mStage = 0;
		ei->theScene->EnumTree(this);

		// Actual geometry comes next
		mStage = 1;
		ei->theScene->EnumTree(this);

		if (mLimbs.IsValid() || mBones.IsValid())
		{
			if (!SaveAscii(filename))
			{
				Thread::MessageWindow("Save to '%s' failed.\nIs the file write-protected?", filename);
				retVal = 0;
			}
		}
		else
		{
			Thread::MessageWindow("There is nothing to export");
			retVal = 0;
		}
	}
	mLock.Unlock();

	return 1;
}

//============================================================================================================
// Scene traversal callback function
//============================================================================================================

int R5MaxExporter::callback (::INode* node)
{
	::Interface* pInterface ( ::GetCOREInterface() );

	if (node != 0 && pInterface != 0)
	{
		// Get the animation interval as well as current time from the interface
		::Interval	interval	= pInterface->GetAnimRange();
		::TimeValue	time		= pInterface->GetTime();

		// All nodes have some kind of a controller
		::Control* nodeController = node->GetTMController();

		// Try to retrieve the skin modifier
		::Modifier* mod = GetModifier(node, SKIN_CLASSID);
		::ISkin* skin = (mod == 0 ? 0 : (ISkin*)mod->GetInterface(I_SKIN));

		// Evalulate the object at its present state
		::Object* object = node->EvalWorldState( skin == 0 ? time : 0 ).obj;
		::INode*  parent = node->GetParentNode();

		// Safety reasons, although I believe neither of these can ever be null
		if (nodeController != 0 && object != 0)
		{
			// We need to know the object's class ID so we can decide what to do with it
			::Class_ID controllerType	= nodeController->ClassID();
			::Class_ID objectType		= object->ClassID();

			if ( objectType != DUMMY_CLASSID )
			{
				if ( controllerType == BIPED_ROOT_CLASSID )
				{
					// Biped root
					if (mStage == 0) ExportBone(node, interval, true, true);
				}
				else if ( controllerType == BIPED_NODE_CLASSID )
				{
					// Biped bone
					if (mStage == 0) ExportBone(node, interval, false, true);
				}
				else if ( objectType == BONE_OBJ_CLASSID )
				{
					// Regular bone
					if (mStage == 0) ExportBone(node, interval, false, false);
				}
				else if ( IsGeometric(object) )
				{
					// If the object can be considered geometric, export it as geometry
					if (mStage == 1) ExportGeometry(node, object, time, skin);
				}
			}
		}
		return TREE_CONTINUE;
	}
	return TREE_ABORT;
}