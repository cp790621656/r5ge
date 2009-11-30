#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// All codecs are kept global as they are shared across all models
//============================================================================================================

struct RegisteredCodec
{
	typedef ModelTemplate::CodecDelegate Delegate;

	String		mName;		// Name of the registered codec specified on Prop::RegisterCodec
	Delegate	mDelegate;	// Delegate function that gets triggered
};

Array<RegisteredCodec> g_codecs;

//============================================================================================================
// Helper function that locates a prop
//============================================================================================================

String FindModel (const String& file, const char* extension)
{
	String path (file);

	if (System::FileExists(path)) return path;

	String filename (System::GetFilenameFromPath(file));
	{
		path = "Models/" + filename;
		if (System::FileExists(path)) return path;

		path << extension;
		if (System::FileExists(path)) return path;

		path = "Resources/" + path;
		if (System::FileExists(path)) return path;
	}
	return "";
}

//============================================================================================================
// Only one codec is supported out-of-the-box: R5 Ascii file format
//============================================================================================================

bool LoadTXT (const byte*		buffer,
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
				g_codecs[i].mDelegate = fnct;
				g_codecs.Unlock();
				return;
			}
		}
		RegisteredCodec& codec = g_codecs.Expand();
		codec.mName = name;
		codec.mDelegate = fnct;
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
			
			if (codec.mName.IsValid() && codec.mDelegate != 0)
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
	mName			(name),
	mTemplate		(0),
	mSkeleton		(0),
	mSerializable	(false),
	mCore			(0),
	mLayer			(0)
{
	static bool doOnce = true;
	
	if (doOnce)
	{
		doOnce = false;
		RegisterCodec("TXT", &LoadTXT);
	}
}

//============================================================================================================
// Copies all limb information from the specified template
//============================================================================================================

void ModelTemplate::SetSource (ModelTemplate* temp, bool forceUpdate)
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
							myLimb->SetMaterial( limb->GetMaterial() );
						}
					}

					// Copy over other parameters
					mSkeleton	= temp->GetSkeleton();
					mFilename	= temp->GetFilename();
					mLayer		= temp->GetLayer();

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
					if (meshes    && limb->mMat  != 0)  limb->mMat->Release();
					if (materials && limb->mMesh != 0)  limb->mMesh->Release();
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
		mLayer		= 0;
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

		if ( tag == Limb::ClassID() )
		{
			_LoadLimb( node, true );
		}
		else if ( tag == Skeleton::ClassID() )
		{
			SetSkeleton( mCore->GetSkeleton(value.IsString() ? value.AsString() : value.GetString(), true) );
		}
		else if ( tag == "Source" )
		{
			ModelTemplate* temp = mCore->GetModelTemplate(value.IsString() ? value.AsString() : value.GetString(), true);
			SetSource( temp, forceUpdate );
		}
		else if ( tag == "Layer" )
		{
			value >> mLayer;
		}
	}
	return true;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void ModelTemplate::SerializeTo	 (TreeNode& root) const
{
	if (!mSerializable) return;

	if ( mFilename.IsValid() || (mTemplate != 0 && mTemplate->GetName() != GetName()) )
	{
		TreeNode& node = root.AddChild( GetClassID() );
		GetName() >> node.mValue;

		if (mTemplate != 0)
		{
			node.AddChild("Source", mTemplate->GetName());
		}
		else if (mFilename.IsValid())
		{
			node.AddChild("Source", mFilename);
		}

		// Add the drawing layer the template belongs to
		node.AddChild("Layer", mLayer);

		// Save the limbs
		_SaveLimbs(node, false);
	}
}

//============================================================================================================
// Loads a single limb
//============================================================================================================

bool ModelTemplate::_LoadLimb ( const TreeNode& root, bool forceUpdate )
{
	IGraphics* graphics = mCore->GetGraphics();
	if (graphics == 0) return false;

	Mesh* mesh(0);
	IMaterial* mat (0);

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	( root.mChildren[i] );
		const String&	tag		( node.mTag	);
		const Variable&	value	( node.mValue );

		if ( tag == Mesh::ClassID() )
		{
			mesh = mCore->GetMesh(value.IsString() ? value.AsString() : value.GetString());
		}
		else if ( tag == IMaterial::ClassID() )
		{
			mat = graphics->GetMaterial(value.IsString() ? value.AsString() : value.GetString());
		}
	}
	
	if (mesh != 0 && mat != 0)
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
					if (ptr != 0 && ptr->mMesh == mesh)
					{
						limb = ptr;
						break;
					}
				}
			}

			// If the limb is still missing, time to add a new entry
			if (limb == 0)
			{
				// This is a new limb -- add a new entry
				limb = mLimbs.Add(name);

				// Update the limb's name, mesh, and material
				limb->SetName(name);
				limb->Set(mesh, mat);
			}
			else if (forceUpdate)
			{
				// Update the limb's name, mesh, and material
				limb->SetName(name);
				limb->Set(mesh, mat);
			}
		}
		Unlock();
	}
	return true;
}

//============================================================================================================
// Save all current limbs
//============================================================================================================

void ModelTemplate::_SaveLimbs ( TreeNode& node, bool forceSave ) const
{
	Lock();
	{
		for (uint i = 0; i < mLimbs.GetSize(); ++i)
		{
			const Limb* limb ( mLimbs[i] );

			if ( limb != 0 && (forceSave || limb->mSerializable) && limb->mMat != 0 && limb->mMesh != 0 )
			{
				TreeNode& child = node.AddChild( Limb::ClassID(), limb->GetName() );
				child.AddChild( Mesh::ClassID(), limb->mMesh->GetName() );
				child.AddChild( IMaterial::ClassID(), limb->mMat->GetName() );
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Tries to load model information from the specified file
//============================================================================================================

bool ModelTemplate::Load (const String& file, bool forceUpdate)
{
	// Don't attempt to load anything unless forced to do so
	if (!forceUpdate && IsValid()) return true;

	// For convenience sake
	String source = FindModel(file, ".r5a");

	// If the file was found, load it
	if (source.IsValid())
	{
		Memory mem;

		// Load the entire file into memory
		if (mem.Load(source))
		{
			String extension ( System::GetExtensionFromFilename(source) );
			mFilename = source;

			if ( Load(mem.GetBuffer(), mem.GetSize(), extension, forceUpdate) )
			{
				mFilename = source;
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
// beginning of the program's lifecycle, implying that the list should be never resized mid-run.
//============================================================================================================

bool ModelTemplate::Load (const byte* buffer, uint size, const String& extension, bool forceUpdate)
{
	for (uint i = 0; i < g_codecs.GetSize(); ++i)
	{
		CodecDelegate& callback (g_codecs[i].mDelegate);
			
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
// Complete serialization -- save to an R5 tree
//============================================================================================================

bool ModelTemplate::Save (TreeNode& root) const
{
	Array<Mesh*>		meshes;
	Array<IMaterial*>	materials;

	Lock();
	{
		TreeNode& graphics	= root.AddChild(IGraphics::ClassID());
		TreeNode& core		= root.AddChild(Core::ClassID());
		TreeNode& model		= root.AddChild("Template");

		// Disable serialization for all sections
		graphics.AddChild("Serializable", false);
		model.AddChild   ("Serializable", false);

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
				if (limb->mMat  != 0) materials.AddUnique(limb->mMat);
				if (limb->mMesh != 0) meshes.AddUnique(limb->mMesh);

				// If the limb is valid, save it right away
				if (limb->IsValid())
				{
					TreeNode& child ( model.AddChild( Limb::ClassID(), limb->GetName() ) );
					child.AddChild( Mesh::ClassID(), limb->mMesh->GetName() );
					child.AddChild( IMaterial::ClassID(), limb->mMat->GetName() );
				}
			}
		}

		// Save materials
		for (uint i = 0; i < materials.GetSize(); ++i)
		{
			IMaterial* mat = materials[i];

			if (mat != 0)
			{
				bool ser = mat->IsSerializable();
				mat->SetSerializable(true);
				mat->SerializeTo(graphics);
				mat->SetSerializable(ser);
			}
		}

		// Save meshes
		for (uint i = 0; i < meshes.GetSize(); ++i)
			meshes[i]->SerializeTo(core);
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