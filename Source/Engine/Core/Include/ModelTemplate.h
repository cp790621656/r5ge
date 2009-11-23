#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// All models are created from templates, which are usually individual files
//============================================================================================================

class ModelTemplate
{
public:

	// Codec functions accept the input data buffer, its size, optional file's extension,
	// and a pointer to the model that's loading the said data
	typedef FastDelegate<bool ( const byte*		data,
								uint			size,
								const String&	extension,
							    ModelTemplate*	model,
								bool			forceUpdate )> CodecDelegate;
	
	// STATIC: Registers a new model codec
	static void RegisterCodec (const String& name, const CodecDelegate& fnct);

	// STATIC: Retrieves the names of all registered codecs
	static void GetRegisteredCodecs (Array<String>& out);

	// Limbs are stored as an array of managed pointers
	typedef ResourceArray<Limb> Limbs;

protected:

	String				mName;			// Every template needs a unique name
	mutable String		mFilename;		// Source filename from which this template was loaded
	Limbs				mLimbs;			// List of model's limbs
	ModelTemplate*		mTemplate;		// Model templates can be based on other templates
	Skeleton*			mSkeleton;		// Skeleton used by this model
	Thread::Lockable	mLock;			// Thread-safety lock
	bool				mSerializable;	// Whether this template is serializable
	Core*				mCore;			// Pointer back to the core that contains this template
	int					mLayer;			// Drawing layer on which this model resides

public:

	ModelTemplate (const String& name);

	// Object creation
	R5_DECLARE_INTERFACE_CLASS("Model Template");

	// Thread safety locking/unlocking
	void Lock()		const { mLock.Lock();	}
	void Unlock()	const { mLock.Unlock(); }

	bool					IsValid()			const	{ return mLimbs.IsValid();	}
	bool					IsSerializable()	const	{ return mSerializable;		}
	const String&			GetName()			const	{ return mName;				}
	const String&			GetFilename()		const	{ return mFilename;			}
	const Limbs&			GetAllLimbs()		const	{ return mLimbs;			}
	const ModelTemplate*	GetSource()			const	{ return mTemplate;			}
	const Skeleton*			GetSkeleton()		const	{ return mSkeleton;			}
	int						GetLayer()			const	{ return mLayer;			}
	Limbs&					GetAllLimbs()				{ return mLimbs;			}
	ModelTemplate*			GetSource()					{ return mTemplate;			}
	Skeleton*				GetSkeleton()				{ return mSkeleton;			}
	Core*					GetCore()					{ return mCore;				}

	void SetName		 (const String& name)			{ mName = name;				}
	void SetSource		 (ModelTemplate* temp, bool forceUpdate = true);
	void SetSkeleton	 (Skeleton* skel);
	void SetSkeleton	 (const String& name);
	void SetSerializable (bool val)						{ mSerializable = val;		}
	void SetLayer		 (int layer)					{ mLayer = layer;			}

	// Limb retrieval
	Limb* GetLimb (const String& name, bool createIfMissing = true)
	{
		return (createIfMissing ? mLimbs.AddUnique(name) : mLimbs.Find(name));
	}

	// Releases all associated meshes and materials
	// WARNING: ALL resources used by this model will become empty!
	void Release (bool meshes = false, bool materials = false, bool skeleton = false);

	// Serialization
	virtual bool SerializeFrom	( const TreeNode& root, bool forceUpdate );
	virtual void SerializeTo	( TreeNode& root ) const;

protected:

	// Limb Serialization
	bool _LoadLimb ( const TreeNode& root, bool forceUpdate );
	void _SaveLimbs ( TreeNode& node, bool forceSave ) const;

	// Triggered when the skeleton has changed
	virtual void _OnSkeletonChanged() {}

	// Should release everything -- called by Release()
	virtual void _OnRelease() {}

public:

	// Loads a model from the specified file or memory buffer
	bool Load (const String& file, bool forceUpdate = false);
	bool Load (const byte* buffer, uint size, const String& extension, bool forceUpdate = false);

	// Saves the entire model (complete with meshes and materials) into the specified
	// file or R5 tree (regular serialization only references meshes and materials)
	bool Save (const String& file) const;
	bool Save (TreeNode& root) const;

private:

	// Allow Core class to set this value
	friend class Core;
	void _SetCore (Core* ptr) { mCore = ptr; }
};