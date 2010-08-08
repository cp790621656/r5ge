#include "_All.h"

#undef CopyMemory

//============================================================================================================
// External configuration
//============================================================================================================

struct Config
{
	float		mScale;
	float		mPrecision;
	bool		mCleanKeys;
	bool		mBruteForce;
	uint		mPosMethod;
	uint		mRotMethod;
	TreeNode	mSkeleton;
	TreeNode	mGraphics;

	Config() : mScale(1.0f), mPrecision(0.00001f), mCleanKeys(true), mBruteForce(false), mPosMethod(2), mRotMethod(2) {}

	bool Load (const char* filename);
};

Config g_config;

//============================================================================================================
// Helper function
//============================================================================================================

inline byte GetInterpolationMethod (const String& s)
{
	if (s == "Spline") return 2;
	if (s == "Linear") return 1;
	return 0;
}

//============================================================================================================
// Load an external configuration file, if present
//============================================================================================================

bool Config::Load (const char* filename)
{
	String full (filename);
	String ext (System::GetExtensionFromFilename(full));
	String file;

	// +1 to account for the dot
	full.GetString(file, 0, full.GetLength() - (ext.GetLength() + 1));
	file << "_config.r5a";

	TreeNode root;

	if (root.Load(file.GetBuffer()))
	{
		FOREACH(i, root.mChildren)
		{
			const TreeNode& node = root.mChildren[i];

			if		(node.mTag == "Scale")					node.mValue >> mScale;
			else if (node.mTag == "Precision")				node.mValue >> mPrecision;
			else if (node.mTag == "Clean Keys")				node.mValue >> mCleanKeys;
			else if (node.mTag == "Brute Force")			node.mValue >> mBruteForce;
			else if (node.mTag == "Position Interpolation") mPosMethod = GetInterpolationMethod(node.mValue.AsString());
			else if (node.mTag == "Rotation Interpolation") mRotMethod = GetInterpolationMethod(node.mValue.AsString());
			else if (node.mTag == "Skeleton")				mSkeleton = node;
			else if (node.mTag == "Graphics")				mGraphics = node;
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Converts Max' matrix into R5's matrix. It's a straight copy since they match. There is something
// different about Max' quaternions or matrix-to-quaternion conversion however, so it's best to work with
// R5 types. Working with Max types then converting to R5 at a later point gives unexpected results.
//============================================================================================================

Matrix43 GetMatrix43 (::Matrix3& mat)
{
	mat.Orthogonalize();
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

	PointerArray<SubMesh> m_array;

	uint mCount;
	bool mHasTexCoords0;
	bool mHasTexCoords1;
	bool mHasColors;
	bool mHasWeights;

public:

	MultiMesh() : mCount(0), mHasTexCoords0(false), mHasTexCoords1(false), mHasColors(false), mHasWeights(false) {}

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
    else if ((normalCount = (rVert.rFlags & NORCT_MASK)) && face.smGroup)
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
					break;
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
	return (object != 0 && (object->ClassID() == BONE_OBJ_CLASSID ||
							object->ClassID() == BONE_CLASSID ||
							object->ClassID() == DUMMY_CLASSID));
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

inline bool IsAMatch (const Vector3f& v0, const Vector3f& v1)
{
	return	Float::Abs(v0.x - v1.x) <= g_config.mPrecision &&
			Float::Abs(v0.y - v1.y) <= g_config.mPrecision &&
			Float::Abs(v0.z - v1.z) <= g_config.mPrecision;
}

//============================================================================================================

inline bool IsAMatch (const Quaternion& q0, const Quaternion& q1)
{
	return (Float::Abs(q0.x - q1.x) <= g_config.mPrecision &&
			Float::Abs(q0.y - q1.y) <= g_config.mPrecision &&
			Float::Abs(q0.z - q1.z) <= g_config.mPrecision &&
			Float::Abs(q0.w - q1.w) <= g_config.mPrecision) ||
		   (Float::Abs(q0.x + q1.x) <= g_config.mPrecision &&
			Float::Abs(q0.y + q1.y) <= g_config.mPrecision &&
			Float::Abs(q0.z + q1.z) <= g_config.mPrecision &&
			Float::Abs(q0.w + q1.w) <= g_config.mPrecision);
}

//============================================================================================================
// Removes all duplicate or useless keys from the array
//============================================================================================================

void CleanKeys (R5MaxExporter::PosKeys& keys, const Vector3f& pos)
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
			if ( IsAMatch(current.mPos, next.mPos) && IsAMatch(current.mPos, previous.mPos) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
				continue;
			}

			float range = (float)(next.mTime - previous.mTime);
			float time  = (float)(current.mTime - previous.mTime);

			if ( IsAMatch(current.mPos, Interpolation::Linear(previous.mPos, next.mPos, time / range)) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
			}
		}
	}

	if (keys.GetSize() > 1)
	{
		if ( IsAMatch(keys[0].mPos, keys[1].mPos) )
		{
			if (keys.GetSize() == 2)
			{
				keys.Shrink();
			}
			else if ( IsAMatch(keys[0].mPos, pos) )
			{
				keys.RemoveAt(0);
			}
		}
	}

	if (keys.GetSize() > 1)
	{
		uint back = keys.GetSize() - 1;
		uint prev = back - 1;

		if ( IsAMatch(keys[back].mPos, keys[prev].mPos) )
		{
			keys.Shrink();
		}
	}

	if (keys.GetSize() == 1)
	{
		if ( IsAMatch(keys[0].mPos, pos) )
		{
			keys.Shrink();
		}
	}
}

//============================================================================================================

void CleanKeys (R5MaxExporter::RotKeys& keys, const Quaternion& rot)
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
			if ( IsAMatch(current.mRot, next.mRot) && IsAMatch(current.mRot, previous.mRot) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
				continue;
			}

			float range = (float)(next.mTime - previous.mTime);
			float time  = (float)(current.mTime - previous.mTime);

			if ( IsAMatch(current.mRot, Interpolation::Slerp(previous.mRot, next.mRot, time / range)) )
			{
				keys.RemoveAt(i+1);
				eliminated = true;
			}
		}
	}

	if (keys.GetSize() > 1)
	{
		if ( IsAMatch(keys[0].mRot, keys[1].mRot) )
		{
			if (keys.GetSize() == 2)
			{
				keys.Shrink();
			}
			else if ( IsAMatch(keys[0].mRot, rot) )
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

		if ( IsAMatch(keys[back].mRot, keys[prev].mRot) )
		{
			keys.Shrink();
		}
	}

	// If the first rotation matches the bind pose, it's safe to get rid of it
	if (keys.GetSize() == 1)
	{
		if ( IsAMatch(keys[0].mRot, rot) )
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

R5MaxExporter::Bone* R5MaxExporter::GetBone (const String& name, bool createIfMissing)
{
	ASSERT(name.IsValid(), "Missing bone name?");

	for (uint i = 0; i < mBones.GetSize(); ++i)
		if (mBones[i].mName == name)
			return &mBones[i];

	if (createIfMissing)
	{
		Bone& bone			= mBones.Expand();
		bone.mName			= name;
		bone.mParent		= -1;
		bone.mInterpolation	= (name.Contains("Linear") ? 1 : (name.Contains("None") ? 0 : 2));
		bone.mIsUsed		= false;
		return &bone;
	}
	return 0;
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
	{
		if (mBones[i].mName == name)
		{
			mBones[i].mIsUsed = true;
			return i;
		}
	}

	// We should already have the bone struct *before* exporting the skin
	ASSERT(mBones.IsValid(), "Missing bones?");
	return -1;
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
	Vector2f t0, t1;
	Color4ub col;

	// Create an array of cross-indices: an index of R5 bone for every Max bone present in the skin
	int boneCount = (onlyVertices || skin == 0 ? 0 : skin->GetNumBones());
	Array<uint> maxToMyBoneIndex (boneCount);

	for (int i = 0; i < boneCount; ++i)
	{
		// GetBoneName() sometimes returns completely different names! wtf?
		//TCHAR* name = skin->GetBoneName(i);
		INode* boneNode = skin->GetBone(i);
		String nodeName (boneNode->GetName());
		maxToMyBoneIndex.Expand() = nodeName.Contains("Ignore Me") ? -1 : GetBoneIndex(nodeName);
	}

	// Apparently parity determines whether the indices are clockwise or counter-clockwise
	bool reverseIndices = (tm.Parity() == 1);

	// Vertex colors, texCoords0, texCoords1
	::TVFace* mapFaces0 = maxMesh.mapFaces(0);
	::TVFace* mapFaces1 = maxMesh.mapFaces(1);
	::TVFace* mapFaces2 = maxMesh.mapFaces(2);
	::UVVert* mapVerts0 = maxMesh.mapVerts(0);
	::UVVert* mapVerts1 = maxMesh.mapVerts(1);
	::UVVert* mapVerts2 = maxMesh.mapVerts(2);

	// Remember whether the mesh has texture coordinates and bone weights
	myMultiMesh.mHasTexCoords0	= !onlyVertices && (mapVerts1 != 0);
	myMultiMesh.mHasTexCoords1	= !onlyVertices && (mapVerts2 != 0);
	myMultiMesh.mHasColors		= !onlyVertices && (mapVerts0 != 0);
	myMultiMesh.mHasWeights		= !onlyVertices && (boneCount > 0);

	// Run through all faces and collect vertex information into a multi-mesh
	for (uint iFace = 0; iFace < faceCount; ++iFace)
	{
		if (maxMesh.faces == 0) continue;

		::Face& face = maxMesh.faces[iFace];
		::TVFace* mapFace0 = (mapFaces0) == 0 ? 0 : &mapFaces0[iFace];
		::TVFace* mapFace1 = (mapFaces1) == 0 ? 0 : &mapFaces1[iFace];
		::TVFace* mapFace2 = (mapFaces2) == 0 ? 0 : &mapFaces2[iFace];

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

				// Vertex colors
				if (mapFace0 != 0)
				{
					DWORD index = mapFace0->t[iVert];
					::UVVert& vert = mapVerts0[index];
					col.Set(Float::ToRangeByte(vert.x), Float::ToRangeByte(vert.y), Float::ToRangeByte(vert.z), 255);
				}

				// Texture coordinates 0
				if (mapFace1 != 0)
				{
					DWORD index = mapFace1->t[iVert];
					t0 = mapVerts1[index];
				}

				// Texture coordinates 1
				if (mapFace2 != 0)
				{
					DWORD index = mapFace2->t[iVert];
					t1 = mapVerts2[index];
				}
			}

			// Adjust by position scale
			v *= g_config.mScale;

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
				else if (myVertices[c].Matches(v, n, t0, t1, col))
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
				vertex.Set(v, n, t0, t1, col);
				vertex.ClearBoneWeights();

				// If there is a skin present, run through all influences of this vertex and assign the bone weights
				if (!onlyVertices && boneCount > 0)
				{
					int influences = skinData->GetNumAssignedBones(maxIndex);

					for (int i = 0; i < influences; ++i)
					{
						int maxBoneIndex = skinData->GetAssignedBone(maxIndex, i);
						float weight = skinData->GetBoneWeight(maxIndex, i);

						uint myBoneIndex = maxToMyBoneIndex[maxBoneIndex];
						if (myBoneIndex != -1) vertex.AddBoneWeight(myBoneIndex, weight);
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
			myMesh->mHasTexCoords0	= myMultiMesh.mHasTexCoords0;
			myMesh->mHasTexCoords1	= myMultiMesh.mHasTexCoords1;
			myMesh->mHasColors		= myMultiMesh.mHasColors;
			myMesh->mHasWeights		= myMultiMesh.mHasWeights;

			myMesh->mVertices.CopyMemory( mySub->mVertices );
			myMesh->mIndices.CopyMemory ( mySub->mIndices  );

			// Get the limb and assign its mesh
			Limb* myLimb	= GetLimb(myName);
			myLimb->mMesh	= myMesh;
			myLimb->mMat	= _ConvertMaterial(maxMtl, i);

			// Special case: "faceted" marked meshes get exported as billboard clouds instead
			myLimb->mMesh->mClouds = (myLimb->mMat != 0) && myLimb->mMat->mClouds;
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

void R5MaxExporter::_ExportKeys (Bone* bone, ::INode* node, ::INode* parent, ::Interval interval)
{
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
		kf.mPos		 = myMat;
		kf.mPos		*= g_config.mScale;
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
		kf.mRot		= myMat;
	}
}

//============================================================================================================
// Exports keys the brute force way, ignoring Max and simply calculating bone positions at every single frame
//============================================================================================================

void R5MaxExporter::_ExportFull( Bone* bone, ::INode* node, ::INode* parent, ::Interval interval )
{
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
		pk.mPos		*= g_config.mScale;

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

	String name (node->GetName());

	if (name.EndsWith("Thigh"))
	{
		// Thighs should be parented to the biped's root instead of the spine
		if (parent->GetParentNode() != 0) parent = parent->GetParentNode();
		if (parent->GetParentNode() != 0) parent = parent->GetParentNode();
	}
	else if (name.EndsWith("Spine") || name.EndsWith("Clavicle"))
	{
		// Spine should be attached to the root node, and shoulders should be attached to spine, not neck
		if (parent->GetParentNode() != 0) parent = parent->GetParentNode();
	}

	// Get the bone we're working with
	Bone* bone = GetBone(name);

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
		bone->mPos *= g_config.mScale;
	}

	bool precise = isBiped || g_config.mBruteForce;

	// If this bone can be exported with low precision, do so now
	if (!precise)
	{
		_ExportKeys(bone, node, parent, interval);

		if (g_config.mCleanKeys)
		{
			CleanKeys (bone->mPosKeys, bone->mPos);
			CleanKeys (bone->mRotKeys, bone->mRot);
		}

		// If we happen to get no keys out of the process, let's try the high precision next
		precise = (bone->mPosKeys.IsEmpty() && bone->mRotKeys.IsEmpty());
	}

	// High precision key extraction runs through every single frame
	if (precise)
	{
		_ExportFull(bone, node, parent, interval);

		if (g_config.mCleanKeys)
		{
			CleanKeys (bone->mPosKeys, bone->mPos);
			CleanKeys (bone->mRotKeys, bone->mRot);
		}
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
				(maxMtl != 0) && (maxMtl->Requirements(0) & MTLREQ_FACETED) != 0 );

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
// Ensures that the specified bone index is valid
//============================================================================================================

void BoneIndexCheck (byte& current, float& weight, const byte& original)
{
	if (current == 0xFF)
	{
		if (Float::IsNotZero(weight))
		{
			// This should never happen, but just in case
			Thread::MessageWindow("WARNING! Bone %u got cut, but it's actually used", original);
		}
		current = 0;
		weight  = 0.0f;
	}
}

//============================================================================================================
// Save the geometry in R5 format
//============================================================================================================

bool R5MaxExporter::SaveR5 (const String& filename)
{
	if (mLimbs.IsValid())
	{
		TreeNode root ("Root");
		TreeNode& graphics	= root.AddChild("Graphics");
		TreeNode& core		= root.AddChild("Core");
		TreeNode& model		= root.AddChild("Template");

		// None of this information should be serialized out on app save
		core.AddChild ("Serializable", false);
		model.AddChild("Serializable", false);

		// Use the external configuration if present
		if (g_config.mGraphics.IsValid()) graphics = g_config.mGraphics;
		else graphics.AddChild("Serializable", false);

		// Ensure tags are proper
		graphics.mTag	= "Graphics";
		core.mTag		= "Core";
		model.mTag		= "Template";

		// Not all bones may end up getting saved out, and we need to keep track of that
		Array<byte> boneIndices;
		uint lastBone = 0;

		// Run through all bones and create a re-indexing array
		if (mBones.IsValid())
		{
			FOREACH(i, mBones)
			{
				Bone& bone = mBones[i];

				// If this bone is not used, there is no point in exporting it
				if (!bone.mIsUsed)
				{
					boneIndices.Expand() = 0xFF;
					continue;
				}

				// This is a valid bone
				boneIndices.Expand() = lastBone++;
			}
		}

		// Save the skeleton
		if (mBones.IsValid())
		{
			TreeNode& skel = core.AddChild("Skeleton");

			// Use the external configuration if present
			if (g_config.mSkeleton.IsValid()) skel = g_config.mSkeleton;

			// Go through all available bones
			for (uint i = 0; i < mBones.GetSize(); ++i)
			{
				Bone& bone = mBones[i];

				// If this bone should not be saved out, skip it
				if (boneIndices[i] == 0xFF) continue;

				// Add a new bone entry
				TreeNode& node = skel.AddChild("Bone");

				// Bone's index goes into the value slot
				(boneIndices[i] >> node.mValue);

				// If the node has a parent, save its index
				if (bone.mParent != -1)
				{
					// Child bone -- save the name of the parent
					node.AddChild("Parent", boneIndices[bone.mParent]);
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
					uint method = bone.mInterpolation;
					if (g_config.mPosMethod < method) method = g_config.mPosMethod;
					posNode.AddChild("Interpolation", (method == 2 ? "Spline" : (method == 1 ? "Linear" : "None")));

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
					uint method = bone.mInterpolation;
					if (g_config.mRotMethod < method) method = g_config.mRotMethod;
					rotNode.AddChild("Interpolation", (method == 2 ? "Spline" : (method == 1 ? "Linear" : "None")));

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
			Material* mat = mMaterials[i];

			// Check to ensure that this material tag doesn't already exist (loaded from an external file)
			{
				bool exists = false;
				
				FOREACH(b, graphics.mChildren)
				{
					TreeNode& child = graphics.mChildren[b];

					if (child.mTag == "Material" &&
						child.mValue.IsString() &&
						child.mValue.AsString() == mat->mName)
					{
						exists = true;
						break;
					}
				}
				if (exists) continue;
			}

			// Add a new material entry
			TreeNode& node	= graphics.AddChild("Material", mat->mName);

			node.AddChild("Diffuse", mat->mDiffuse);
			node.AddChild("Specular", mat->mSpecular);
			node.AddChild("Glow", mat->mGlow);
			node.AddChild("Technique", "Depth");

			// Add the deferred technique first
			TreeNode& tech = node.AddChild("Technique");

			// Wireframe if the material is wireframe, deferred if it's solid
			tech.mValue = (mat->mWireframe) ? "Wireframe" : "Deferred";

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

				Vector3f origin;
				Bounds bounds;
				
				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v = mesh->mVertices[b];
					origin += v.mPos;
					bounds.Include(v.mPos);
				}
				
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
				radius *= 12.5f;

				Array<Quaternion>& verts = node.AddChild("Instances").mValue.ToQuaternionArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v = mesh->mVertices[b];
					verts.Expand().Set(v.mPos.x, v.mPos.y, v.mPos.z, radius +
						radius * 0.25f * (origin.z - v.mPos.z) / bounds.GetRadius());
				}

				// Billboard clouds don't need to save normals, texture coordinates, or bone information
				mesh->mHasTexCoords0	= false;
				mesh->mHasTexCoords1	= false;
				mesh->mHasColors		= false;
				mesh->mHasWeights		= false;
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
			if (mesh->mHasTexCoords0)
			{
				Array<Vector2f>& tc = node.AddChild("TexCoords 0").mValue.ToVector2fArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v = mesh->mVertices[b];
					tc.Expand() = v.mTc0;
				}
			}

			// Second set of texture coordinates
			if (mesh->mHasTexCoords1)
			{
				Array<Vector2f>& tc = node.AddChild("TexCoords 1").mValue.ToVector2fArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v = mesh->mVertices[b];
					tc.Expand() = v.mTc1;
				}
			}

			// Vertex colors
			if (mesh->mHasColors)
			{
				bool saveColors = false;

				FOREACH(b, mesh->mVertices)
				{
					const Vertex& v = mesh->mVertices[b];

					// If all colors are just plain white, there is no point in saving them
					if (v.mColor != -1)
					{
						saveColors = true;
						break;
					}
				}

				if (saveColors)
				{
					Array<Color4ub>& colors = node.AddChild("Colors").mValue.ToColor4ubArray();

					for (uint b = 0; b < vertices; ++b)
					{
						const Vertex& v = mesh->mVertices[b];
						colors.Expand() = v.mColor;
					}
				}
			}

			// If the mesh contains skinning information, save it
			if (mesh->mHasWeights && mBones.IsValid())
			{
				Array<Color4f>&		weights = node.AddChild("Bone Weights").mValue.ToColor4fArray();
				Array<Color4ub>&	indices = node.AddChild("Bone Indices").mValue.ToColor4ubArray();

				for (uint b = 0; b < vertices; ++b)
				{
					const Vertex& v	 = mesh->mVertices[b];
					indices.Expand() = v.mBoneIndices;
					weights.Expand() = v.mBoneWeights;

					// Re-index the bone indices, since not all bones may have been saved out
					{
						Color4ub& idx = indices.Back();
						Color4f&  wt  = weights.Back();

						idx.r = boneIndices[idx.r];
						idx.g = boneIndices[idx.g];
						idx.b = boneIndices[idx.b];
						idx.a = boneIndices[idx.a];

						BoneIndexCheck(idx.r, wt.r, v.mBoneIndices.r);
						BoneIndexCheck(idx.g, wt.g, v.mBoneIndices.g);
						BoneIndexCheck(idx.b, wt.b, v.mBoneIndices.b);
						BoneIndexCheck(idx.a, wt.a, v.mBoneIndices.a);
					}
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
			if (limb->mMat != 0) node.AddChild("Material", limb->mMat->mName);
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

		// Load the external configuration, if present
		g_config.Load(filename);

		// Enum all bones first
		mStage = 0;
		ei->theScene->EnumTree(this);

		// Actual geometry comes next
		mStage = 1;
		ei->theScene->EnumTree(this);

		if (mLimbs.IsValid() || mBones.IsValid())
		{
			if (!SaveR5(filename))
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

		// Evaluate the object at its present state
		::Object* object = node->EvalWorldState( skin == 0 ? time : 0 ).obj;
		::INode*  parent = node->GetParentNode();

		if (object != 0)
		{
			::Class_ID objectType = object->ClassID();

			String name (node->GetName());

			if (!name.Contains("Ignore Me"))
			{
				if ( objectType == BONE_OBJ_CLASSID || objectType == BONE_CLASSID || objectType == DUMMY_CLASSID )
				{
					// Regular bone
					if (mStage == 0) ExportBone(node, interval, false, false);
				}
				else if (nodeController != 0)
				{
					// We need to know the object's class ID so we can decide what to do with it
					::Class_ID controllerType = nodeController->ClassID();

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
					else if ( objectType != DUMMY_CLASSID && IsGeometric(object) )
					{
						// If the object can be considered geometric, export it as geometry
						if (mStage == 1) ExportGeometry(node, object, time, skin);
					}
				}
			}
		}
		return TREE_CONTINUE;
	}
	return TREE_ABORT;
}