#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// R5 exporter plugin for Autodesk's 3ds Max
// Author: Michael Lyashenko
//============================================================================================================

struct MultiMesh;
class R5MaxExporter : public ::SceneExport, public ::ITreeEnumProc
{
public:

	struct Vertex
	{
		Vector3f	mPos;				// 12 bytes - 12
		Vector3f	mNormal;			// 12 bytes - 24
		Vector2f	mTc0;				//  8 bytes - 32
		Vector2f	mTc1;				//  8 bytes - 40
		Color4ub	mColor;				//  4 bytes - 44
		Color4f		mBoneWeights;		// 16 bytes - 60
		Color4ub	mBoneIndices;		//  4 bytes - 64

		operator const float*() { return &mPos.x; }

		void Set (const Vector3f& v, const Vector3f& n, const Vector2f& t0, const Vector2f& t1, const Color4ub& c)
		{
			mPos	= v;
			mNormal = n;
			mTc0	= t0;
			mTc1	= t1;
			mColor	= c;
		}

		bool Matches (const Vector3f& v, const Vector3f& n, const Vector2f& t0, const Vector2f& t1, const Color4ub& c)
		{
			return (mPos == v && mNormal == n && mTc0 == t0 && mTc1 == t1 && mColor == c);
		}

		void AddBoneWeight (unsigned char boneIndex, float weight);
		void ClearBoneWeights() { mBoneIndices = 0; mBoneWeights = 0.0f; }
		void NormalizeBoneWeights();
	};


	struct PosKey
	{
		unsigned int	mTime;
		Vector3f		mPos;
	};

	struct RotKey
	{
		unsigned int	mTime;
		Quaternion		mRot;
	};

	typedef Array<PosKey>			PosKeys;
	typedef Array<RotKey>			RotKeys;
	typedef Array<Vertex>			VertexArray;
	typedef Array<unsigned short>	IndexArray;

	struct Bone
	{
		unsigned int	mParent;
		String			mName;
		Vector3f		mPos;
		Quaternion		mRot;
		PosKeys			mPosKeys;
		RotKeys			mRotKeys;
		byte			mInterpolation;
		bool			mIsUsed;
	};

	struct Mesh
	{
		String			mName;
		VertexArray		mVertices;
		IndexArray		mIndices;
		bool			mHasTexCoords0;
		bool			mHasTexCoords1;
		bool			mHasColors;
		bool			mHasWeights;
		bool			mClouds;

		Mesh (const String& name) : mName(name), mHasTexCoords0(false), mHasTexCoords1(false),
			mHasColors(false), mHasWeights(false), mClouds(false) {}

		const String& GetName() const { return mName; }
	};

	struct Material
	{
		String		mName;
		Color4f		mDiffuse;
		float		mGlow;
		float		mSpecularity;
		float		mShininess;
		bool		mWireframe;
		bool		mTwosided;
		bool		mClouds;

		Material (const String& name) : mName(name), mGlow(0.0f), mSpecularity(0.0f), mShininess(0.2f),
			mWireframe(false), mTwosided(false), mClouds(false) {}

		const String& GetName() const { return mName; }
	};

	struct Limb
	{
		String		mName;
		Mesh*		mMesh;
		Material*	mMat;

		Limb (const String& name) : mName(name), mMesh(0), mMat(0) {}
		const String& GetName() const { return mName; }
	};

	typedef Array<Bone>				Bones;
	typedef ResourceArray<Limb>		Limbs;
	typedef ResourceArray<Mesh>		Meshes;
	typedef ResourceArray<Material>	Materials;

protected:

	Thread::Lockable	mLock;
	Bones				mBones;
	Limbs				mLimbs;
	Meshes				mMeshes;
	Materials			mMaterials;
	unsigned int		mStage;

public:

	R5MaxExporter() : mStage(0) {}

protected:

	void Release();

	Bone*			GetBone		(unsigned int index);
	Bone*			GetBone		(const String& name, bool createIfMissing = true);
	unsigned int	GetBoneIndex(const String& name);
	Limb*			GetLimb		(const String& name)	{ return mLimbs.AddUnique(name); }
	Mesh*			GetMesh		(const String& name)	{ return mMeshes.AddUnique(name); }
	Material*		GetMaterial	(const String& name)	{ return mMaterials.AddUnique(name); }

private:

	void		_FillMultiMesh	( MultiMesh& myMultiMesh, ::Mesh& maxMesh, ::Mtl* maxMtl, ::Matrix3& tm, ::ISkin* skin = 0, ::ISkinContextData* skinData = 0, bool onlyVertices = false );
	Material*	_ConvertMaterial( ::Mtl* mtl, unsigned int subMatIndex = 0 );
	void		_CreateLimbs	( MultiMesh& myMultiMesh, ::Mtl* maxMtl, ::TimeValue time, const String& meshName );
	void		_ExportKeys		( Bone* bone, ::INode* node, ::INode* parent, ::Interval interval );
	void		_ExportFull		( Bone* bone, ::INode* node, ::INode* parent, ::Interval interval );

protected:

	void ExportBone		( ::INode* node, ::Interval interval, bool isBipedRoot, bool isBiped );
	void ExportGeometry	( ::INode* node, ::Object* object, ::TimeValue time, ::ISkin* skin = 0 );
	bool SaveR5			( const String& filename );

public:

	// Functions from ::SceneExport
	virtual int				ExtCount()				{ return 3; }
	virtual const char*		Ext(int n)				{ return (n == 0) ? "r5a" : (n == 1 ? "r5b" : "r5c"); }
	virtual const char*		LongDesc()				{ return "R5 Engine Model"; }
	virtual const char*		ShortDesc()				{ return "R5 Model"; }
	virtual const char*		AuthorName()			{ return "Michael Lyashenko"; }
	virtual const char*		CopyrightMessage()		{ return "Copyright (c) 2007-2010 Michael Lyashenko"; }
	virtual const char*		OtherMessage1()			{ return "";  }
	virtual const char*		OtherMessage2()			{ return "";  }
	virtual unsigned int	Version()				{ return 280; }
	virtual void			ShowAbout(HWND hWnd)	{ Thread::MessageWindow("R5 Engine 3ds Max Exporter: Please visit [www.nextrevision.com] for more information."); }

	virtual int SupportsOptions (int ext, unsigned long options) { return 0; }

	virtual int DoExport (	const char*		filename,
							::ExpInterface*	ei,
							::Interface*	i,
							int				suppressPrompts = 0,
							unsigned long	options			= 0 );

	// Callback function from ::ITreeEnumProc for scene traversal
	virtual int callback(INode* nodeP);
};