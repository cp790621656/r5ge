#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// All codecs are kept global as they are shared across all models
//============================================================================================================

struct RegisteredCodec
{
	typedef ModelTemplate::CodecDelegate Delegate;

	String		mName;		// Name of the registered codec specified on Prop::RegisterCodec
	Delegate	mRead;	// Delegate function that gets triggered
};

Array<RegisteredCodec> g_codecs;

//============================================================================================================
// Only one codec is supported out-of-the-box: R5 TreeNode file format
//============================================================================================================

bool LoadR5 (const byte*		buffer,
			  uint				size,
			  const String&		extension,
			  ModelTemplate*	model,
			  bool				forceUpdate)
{
	Core* core = model->GetCore();
	if (core == 0) return false;

	TreeNode root;

	if (root.Load(buffer, size))
	{
		String temp ("Template");
		core->SerializeFrom(root, forceUpdate);

		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			TreeNode& node = root.mChildren[i];

			if ( node.mTag == temp )
			{
				model->SerializeFrom(node, forceUpdate);
			}
		}
		return true;
	}
	return false;
}

//============================================================================================================
// STATIC: Registers a new model codec
//============================================================================================================

void ModelTemplate::RegisterCodec (const String& name, const CodecDelegate& fnct)
{
	g_codecs.Lock();
	{
		for (uint i = 0; i < g_codecs.GetSize(); ++i)
		{
			if (g_codecs[i].mName == name)
			{
				g_codecs[i].mRead = fnct;
				g_codecs.Unlock();
				return;
			}
		}
		RegisteredCodec& codec = g_codecs.Expand();
		codec.mName = name;
		codec.mRead = fnct;
	}
	g_codecs.Unlock();
}

//============================================================================================================
// STATIC: Retrieves the names of all registered codecs
//============================================================================================================

void ModelTemplate::GetRegisteredCodecs (Array<String>& list)
{
	list.Clear();
	
	g_codecs.Lock();
	{
		for (uint i = 0; i < g_codecs.GetSize(); ++i)
		{
			RegisteredCodec& codec = g_codecs[i];
			
			if (codec.mName.IsValid() && codec.mRead != 0)
			{
				list.Expand() = codec.mName;
			}
		}
	}
	g_codecs.Unlock();
}

//============================================================================================================
// Constructor registers the default codecs
//============================================================================================================

ModelTemplate::ModelTemplate (const String& name) :
	mUID			(GenerateUID()),
	mName			(name),
	mTemplate		(0),
	mSkeleton		(0),
	mCore			(0),
	mSerializable	(false),
	mIsDirty		(true),
	mMask			(0)
{
	static bool doOnce = true;
	
	if (doOnce)
	{
		doOnce = false;
		RegisterCodec("R5", &LoadR5);
	}
}

//============================================================================================================
// Copies all limb information from the specified template
//============================================================================================================

void ModelTemplate::SetSource (ModelTemplate* temp)
{
	if (mTemplate != temp)
	{
		Lock();
		{
			// Release all current model data
			_OnRelease();
			mLimbs.Clear();
			mFilename.Clear();

			mSkeleton = 0;
			mTemplate = temp;
			mIsDirty = true;

			// If we were given a valid template, we need to copy limb information over
			if ( mSerializable = (mTemplate != 0) )
			{
				// Lock the passed template as well
				mTemplate->Lock();
				{
					ModelTemplate::Limbs& limbs = mTemplate->GetAllLimbs();

					for (uint i = 0; i < limbs.GetSize(); ++i)
					{
						Limb* limb = limbs[i];

						if (limb != 0)
						{
							// Copy the limb mesh and material information
							Limb* myLimb = mLimbs.Add( limb->GetName() );
							myLimb->SetMesh( limb->GetMesh() );
							myLimb->SetMesh( limb->GetCloud() );
							myLimb->SetMaterial( limb->GetMaterial() );
						}
					}

					// Copy over other parameters
					mSkeleton		= temp->GetSkeleton();
					mFilename		= temp->GetFilename();
					mOnSerialize	= temp->GetOnSerialize();

					// Update the skeletal information
					_OnSkeletonChanged();
				}
				mTemplate->Unlock();
			}
		}
		Unlock();
	}
}

//============================================================================================================
// Changes the current skeleton
//============================================================================================================

void ModelTemplate::SetSkeleton (Skeleton* skel)
{
	if (mSkeleton != skel)
	{
		Lock();
		{
			mSkeleton = skel;
			_OnSkeletonChanged();
		}
		Unlock();
	}
}

//============================================================================================================

void ModelTemplate::SetSkeleton (const String& name)
{
	SetSkeleton( mCore->GetSkeleton(name, true) );
}

//============================================================================================================
// Releases all associated meshes and materials
//============================================================================================================

void ModelTemplate::Release (bool meshes, bool materials, bool skeleton)
{
	Lock();
	{
		_OnRelease();

		if (meshes || materials)
		{
			for (Limb** start = mLimbs.GetStart(), **end = mLimbs.GetEnd(); start != end; ++start)
			{
				Limb* limb (*start);

				if (limb != 0)
				{
					if (meshes    && limb->mMat		!= 0)  limb->mMat->Release();
					if (materials && limb->mMesh	!= 0)  limb->mMesh->Release();
					if (materials && limb->mCloud	!= 0)  limb->mCloud->Release();
				}
			}
		}

		mLimbs.Clear();

		if (skeleton && mSkeleton != 0)
		{
			mSkeleton->Release();
		}

		mSkeleton	= 0;
		mTemplate	= 0;
		mIsDirty	= false;
		mBounds.Clear();
	}
	Unlock();
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool ModelTemplate::SerializeFrom ( const TreeNode& root, bool forceUpdate )
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if (tag == Limb::ClassID())
		{
			_LoadLimb( node, true );
		}
		else if (tag == Skeleton::ClassID())
		{
			SetSkeleton( mCore->GetSkeleton(value.AsString(), true) );
		}
		else if (tag == "Source")
		{
			ModelTemplate* temp = mCore->GetModelTemplate(value.AsString(), true);
			SetSource(temp);
		}
		else if (tag == "OnSerialize")
		{
			mOnSerialize = node;
		}
	}
	return true;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void ModelTemplate::SerializeTo	(TreeNode& root, bool forceSave) const
{
	if (!forceSave && !mSerializable) return;

	if ( forceSave || mFilename.IsValid() || (mTemplate != 0 && mTemplate->GetName() != GetName()) )
	{
		TreeNode& node = root.AddChild( GetClassID() );
		GetName() >> node.mValue;

		if (!mSerializable) node.AddChild("Serializable", false);

		if (mTemplate != 0)
		{
			node.AddChild("Source", mTemplate->GetName());
		}
		else if (mFilename.IsValid())
		{
			node.AddChild("Source", mFilename);
		}

		// Save the limbs
		_SaveLimbs(node, forceSave);

		// Save the OnSerialize section
		if (forceSave && mOnSerialize.HasChildren()) node.mChildren.Expand() = mOnSerialize;

		// If we don't have anything to save, don't bother leaving an empty entry
		if (node.mChildren.GetSize() == 1 &&
			node.mChildren[0].mTag == "Serializable") root.mChildren.Shrink();
	}
}

//============================================================================================================
// Loads a single limb
//============================================================================================================

bool ModelTemplate::_LoadLimb (const TreeNode& root, bool forceUpdate)
{
	IGraphics* graphics = mCore->GetGraphics();
	if (graphics == 0) return false;

	Mesh*		mesh	= 0;
	Cloud*		bm		= 0;
	IMaterial*	mat		= 0;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	( root.mChildren[i] );
		const String&	tag		( node.mTag	);
		const Variable&	value	( node.mValue );

		if ( tag == Mesh::ClassID() )
		{
			mesh = mCore->GetMesh(value.AsString());
		}
		else if (tag == Cloud::ClassID())
		{
			bm = mCore->GetCloud(value.AsString());
		}
		else if ( tag == IMaterial::ClassID() )
		{
			mat = graphics->GetMaterial(value.AsString());
		}
	}

	if ((mesh != 0 || bm != 0) && mat != 0)
	{
		Lock();
		{
			// First get the limb by its name
			String name (root.mValue.GetString());
			Limb* limb = GetLimb(name, false);

			// If the limb is missing, see if we can match it by mesh
			if (limb == 0)
			{
				// Run through the current limbs and see if there is already a limb using this mesh
				for (uint i = 0; i < mLimbs.GetSize(); ++i)
				{
					Limb* ptr = mLimbs[i];

					// Remember the matching limb
					if ( ptr  != 0 &&
						(mesh != 0 && ptr->mMesh == mesh) ||
						(bm   != 0 && ptr->mCloud == bm) )
					{
						limb = ptr;
						break;
					}
				}
			}

			// If the limb is still missing, time to add a new entry
			if (limb == 0)
			{
				limb = mLimbs.Add(name);
				forceUpdate = true;
			}

			if (forceUpdate)
			{
				// Update the limb's name, mesh, and material
				limb->SetName(name);
				if (mesh != 0) limb->Set(mesh, mat);
				else limb->Set(bm, mat);
				SetDirty();
			}
		}
		Unlock();
	}
	return true;
}

//============================================================================================================
// Save all current limbs
//============================================================================================================

void ModelTemplate::_SaveLimbs (TreeNode& node, bool forceSave) const
{
	Lock();
	{
		for (uint i = 0; i < mLimbs.GetSize(); ++i)
		{
			const Limb* limb ( mLimbs[i] );

			if ( limb != 0 && (forceSave || limb->mSerializable) && limb->IsValid() )
			{
				TreeNode& child = node.AddChild( Limb::ClassID(), limb->GetName() );

				if (limb->mMesh != 0) 
				{
					child.AddChild( Mesh::ClassID(), limb->mMesh->GetName() );
				}
				else if (limb->mCloud != 0)
				{
					child.AddChild( Cloud::ClassID(), limb->mCloud->GetName() );
				}
				child.AddChild( IMaterial::ClassID(), limb->mMat->GetName() );
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Updates the technique mask and the bounding volume
//============================================================================================================

void ModelTemplate::_OnUpdate()
{
	mIsDirty = false;
	mMask = 0;
	mBounds.Clear();

	if (mLimbs.IsValid())
	{
		Lock();
		{
			for (uint i = mLimbs.GetSize(); i > 0; )
			{
				const Limb* limb = mLimbs[--i];

				if (limb->IsValid())
				{
					if (limb->mMesh != 0) mBounds.Include(limb->mMesh->GetBounds());
					else if (limb->mCloud != 0) mBounds.Include(limb->mCloud->GetBounds());
					mMask |= limb->mMat->GetTechniqueMask();
				}
			}
		}
		Unlock();
	}
}

//============================================================================================================
// Tries to load model information from the specified file
//============================================================================================================

bool ModelTemplate::Load (const String& file, bool forceUpdate)
{
	// Don't attempt to load anything unless forced to do so
	if (!forceUpdate && IsValid()) return true;

	// Find the model
	Array<String> files;

	// If the file was found, load it
	if (System::GetFiles(file, files))
	{
		Memory mem;

		// Load the entire file into memory
		if (mem.Load(files[0]))
		{
			String extension ( System::GetExtensionFromFilename(files[0]) );

			if ( Load(mem.GetBuffer(), mem.GetSize(), extension, forceUpdate) )
			{
				mFilename = files[0];
				mSerializable = true;
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================
// Tries to load model information from a memory buffer
//============================================================================================================
// NOTE: This function is not thread-safe on purpose, since all codecs should be registered at the very
// beginning of the program's life cycle, implying that the list should be never resized mid-run.
//============================================================================================================

bool ModelTemplate::Load (const byte* buffer, uint size, const String& extension, bool forceUpdate)
{
	for (uint i = 0; i < g_codecs.GetSize(); ++i)
	{
		CodecDelegate& callback (g_codecs[i].mRead);
			
		if (callback != 0 && callback(buffer, size, extension, this, forceUpdate))
		{
			mFilename.Clear();
			mTemplate = 0;
			return true;
		}
	}
	return false;
}

//============================================================================================================
// Helper function -- runs through the specified TreeNode and finds all referenced models
//============================================================================================================

void GatherModels (const TreeNode& root, Array<String>& models)
{
	bool retVal = false;

	if (root.mTag == "Model")
	{
		models.AddUnique( root.mValue.IsString() ? root.mValue.AsString() : root.mValue.GetString() );
	}

	FOREACH(i, root.mChildren)
	{
		GatherModels(root.mChildren[i], models);
	}
}

//============================================================================================================
// Complete serialization -- save to an R5 tree
//============================================================================================================

bool ModelTemplate::Save (TreeNode& root) const
{
	Array<Mesh*> meshes;
	Array<Cloud*> mBMs;
	Array<IMaterial*> materials;

	Lock();
	{
		TreeNode& graphics	= root.AddUnique(IGraphics::ClassID());
		TreeNode& core		= root.AddUnique(Core::ClassID());

		bool isFirst = true;

		FOREACH(i, root.mChildren)
		{
			if (root.mChildren[i].mTag == "Template")
			{
				isFirst = false;
				break;
			}
		}

		// First model template gets saved into an unnamed "Template" tag, others into their own
		TreeNode& model = isFirst ? root.AddChild("Template") : core.AddChild(ModelTemplate::ClassID(), mName);

		// Disable serialization for all sections
		graphics.AddUnique("Serializable").mValue = false;
		model.AddUnique   ("Serializable").mValue = false;

		// Save the skeleton if it's present
		if (mSkeleton != 0)
		{
			mSkeleton->SerializeTo(core);
			model.AddChild( Skeleton::ClassID(), mSkeleton->GetName() );
		}

		// Collect all unique meshes and materials
		for (uint i = 0; i < mLimbs.GetSize(); ++i)
		{
			const Limb* limb ( mLimbs[i] );

			if (limb != 0)
			{
				if (limb->mMat		!= 0) materials.AddUnique(limb->mMat);
				if (limb->mMesh		!= 0) meshes.AddUnique(limb->mMesh);
				if (limb->mCloud	!= 0) mBMs.AddUnique(limb->mCloud);

				// If the limb is valid, save it right away
				if (limb->IsValid())
				{
					TreeNode& child ( model.AddChild( Limb::ClassID(), limb->GetName() ) );

					if (limb->mMesh != 0)
					{
						child.AddChild( Mesh::ClassID(), limb->mMesh->GetName() );
					}
					else if (limb->mCloud != 0)
					{
						child.AddChild( Cloud::ClassID(), limb->mCloud->GetName() );
					}
					child.AddChild( IMaterial::ClassID(), limb->mMat->GetName() );
				}
			}
		}

		// Save materials
		for (uint i = 0; i < materials.GetSize(); ++i)
		{
			IMaterial* mat = materials[i];
			if (mat == 0) continue;

			FOREACH(b, graphics.mChildren)
			{
				TreeNode& child = graphics.mChildren[b];

				if (child.mTag == IMaterial::ClassID() &&
					child.mValue.IsString() &&
					child.mValue.AsString() == mat->GetName())
				{
					mat = 0;
					break;
				}
			}

			if (mat != 0)
			{
				bool ser = mat->IsSerializable();
				mat->SetSerializable(true);
				mat->SerializeTo(graphics);
				mat->SetSerializable(ser);
			}
		}

		// Save all unique meshes
		for (uint i = 0; i < meshes.GetSize(); ++i)
		{
			Mesh* mesh = meshes[i];

			FOREACH(b, core.mChildren)
			{
				TreeNode& child = core.mChildren[b];

				if (child.mTag == Mesh::ClassID() &&
					child.mValue.IsString() &&
					child.mValue.AsString() == mesh->GetName())
				{
					mesh = 0;
					break;
				}
			}
			if (mesh != 0) mesh->SerializeTo(core);
		}

		// Save all unique billboard clouds
		for (uint i = 0; i < mBMs.GetSize(); ++i)
		{
			Cloud* mesh = mBMs[i];

			FOREACH(b, core.mChildren)
			{
				TreeNode& child = core.mChildren[b];

				if (child.mTag == Mesh::ClassID() &&
					child.mValue.IsString() &&
					child.mValue.AsString() == mesh->GetName())
				{
					mesh = 0;
					break;
				}
			}
			if (mesh != 0) mBMs[i]->SerializeTo(core);
		}

		// Save all templates this model depends on
		if (mOnSerialize.HasChildren())
		{
			Array<String> models;
			GatherModels(mOnSerialize, models);

			FOREACH(i, models)
			{
				const String& m = models[i];
				ModelTemplate* temp = mCore->GetModelTemplate(m, false);
				if (temp != 0 && temp != this) temp->Save(root);
			}

			// Save the OnSerialize section itself
			model.mChildren.Expand() = mOnSerialize;
			model.mChildren.Back().mTag = "OnSerialize";
		}
	}
	Unlock();
	return true;
}

//============================================================================================================
// Complete serialization -- save to file
//============================================================================================================

bool ModelTemplate::Save (const String& file) const
{
	TreeNode root;
	root.mTag = "Root";

	if ( Save(root) && root.Save(file) )
	{
		mFilename = file;
		return true;
	}
	return false;
}