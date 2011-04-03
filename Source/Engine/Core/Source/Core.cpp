#include "../Include/_All.h"
using namespace R5;

#ifdef _DEBUG
#define ASSERT_IF_SELF_IS_UNLOCKED ASSERT(IsLocked() || \
	(GetNumberOfThreads() == 0 && GetThreadID() == Thread::GetID()), \
	"You must lock the core before you work with objects!");
#else
#define ASSERT_IF_SELF_IS_UNLOCKED
#endif

//============================================================================================================
// Thread for resource serialization
//============================================================================================================

R5_THREAD_FUNCTION(SerializeResourceThread, ptr)
{
	Resource* resource = (Resource*)ptr;

#ifdef _DEBUG
	Thread::IDType threadId = Thread::GetID();
	ulong timestamp = Time::GetMilliseconds();
	System::Log("[THREAD]  Executing '%s' [ID: %u]", resource->GetName().GetBuffer(), threadId);
#endif

	Core* core = resource->GetCore();
	core->Lock();
	core->SerializeFrom( resource->GetName(), false, false );
	core->Unlock();
	core->DecrementThreadCount();

#ifdef _DEBUG
	System::Log("[THREAD]  Finished executing '%s' in %u ms [ID: %u]",
		resource->GetName().GetBuffer(), Time::GetMilliseconds() - timestamp, threadId);
#endif
	return 0;
}
//============================================================================================================
// Core constructor and destructor
//============================================================================================================

Core::Core (IWindow* window, IGraphics* graphics, IUI* gui, IAudio* audio) :
	mWin(window), mGraphics(graphics), mUI(gui), mAudio(audio), mFullDraw(0)
{
	Init();
}

//============================================================================================================

Core::Core (IWindow* window, IGraphics* graphics, IUI* gui, IAudio* audio, Scene& scene) :
	mWin(window), mGraphics(graphics), mUI(gui), mAudio(audio), mFullDraw(0)
{
	Init();
	scene.SetRoot(&mRoot);
}

//============================================================================================================

Core::~Core()
{
	// Ensure that the core won't be deleted until all worker threads finish
	while (mThreadCount > 0) Thread::Sleep(1);
	if (mWin != 0) mWin->SetGraphics(0);

	// We no longer need the improved timer frequency
	Thread::ImproveTimerFrequency(false);

	// Root has to be released explicitly as it has to be cleared before meshes
	Lock();
	mRoot.Release();
}

//============================================================================================================
// Default initialization function
//============================================================================================================

void Core::Init()
{
#ifdef _DEBUG
	mSleepDelay = 1;
#else
	mSleepDelay = 0;
#endif
	mUISleepDelay = 10;

	// First update should update the scene regardless of time delta
	mIsDirty = true;

	// Remember the thread ID
	mThreadID = Thread::GetID();
	mThreadCount = 0;

	// Root of the scene needs to know who owns it
	mRoot.mCore = this;
	mRoot.mGraphics = mGraphics;
	mRoot.SetName("Root");

	// All keys start as inactive
	memset(mIsKeyDown, 0, sizeof(bool) * 256);

	// If a window was specified, ensure that R5::Core is the target of all of its callbacks
	if (mWin) mWin->SetEventHandler(this);

	// Ensure that the timer frequency is 1 millisecond
	Thread::ImproveTimerFrequency(true);

	// Automatically assign the graphics controller to the window
	if (mWin != 0) mWin->SetGraphics(mGraphics);
}

//============================================================================================================
// Releases the meshes and the scene graph
//============================================================================================================

void Core::Release()
{
	Lock();
	mRoot.Release();
	Unlock();

	mMeshes.Lock();
	mMeshes.Release();
	mMeshes.Unlock();

	mSkeletons.Lock();
	mSkeletons.Release();
	mSkeletons.Unlock();
}

//============================================================================================================
// Starting a new frame
//============================================================================================================

bool Core::Update()
{
	Time::Update();

	if (mAudio != 0) mAudio->Update();

	if (mWin != 0)
	{
		// Window update function can return 'false' if the window has been closed
		if (!mWin->IsValid() || !mWin->Update()) return false;
		bool minimized = mWin->IsMinimized();

		// Handle mouse movement
		if (mMouseDelta != 0)
		{
			if (mUI == 0 || !mUI->OnMouseMove(mMousePos, mMouseDelta))
			{
				HandleOnMouseMove(mMousePos, mMouseDelta);
			}
			mMouseDelta = 0;
		}

		// Size update is queued rather than executed in Core::OnResize() because that call needs to update
		// the graphics controller, and the controller may be created in a different thread than the window.
		if (mUpdatedSize)
		{
			if (mGraphics)	mGraphics->SetViewport(mUpdatedSize);
			if (mUI)		mUI->OnResize(mUpdatedSize);

			// The size no longer needs to be updated
			mUpdatedSize = 0;
		}

		// If some time has passed, update the scene
		if (Time::GetDeltaMS() != 0) mIsDirty = true;

		// Do not update anything unless some time has passed
		if (mIsDirty)
		{
			if (mFullDraw < 10) ++mFullDraw;
			mIsDirty = false;

			// If the root object is enabled, update all models
			if (mRoot.GetFlag(Object::Flag::Enabled))
			{
				// Update all props and models
				Lock();
				{
					mModels.Lock();
					{
						for (uint i = mModels.GetSize(); i > 0; )
						{
							Model* model = mModels[--i];
							if (model != 0) model->Update();
						}
					}
					mModels.Unlock();
				}
				Unlock();
			}
			else
			{
				mFullDraw = 0;
			}

			// Pre-update callbacks
			mPreList.Execute();

			// Update the entire scene
			if (mRoot.GetFlag(Object::Flag::Enabled))
			{
				Lock();
				mRoot.Update(Vector3f(), Quaternion(), 1.0f, false);
				Unlock();
			}
			else
			{
				mFullDraw = 0;
			}

			// Post-update callbacks
			mPostList.Execute();

			// The UI should only be updated if the window is not minimized
			if (mUI != 0 && !minimized) mUI->Update();

			// Post-GUI updates
			mLateList.Execute();
		}

		// If the window is not minimized, start the drawing process
		if (!minimized)
		{
			// Start the drawing process
			mWin->BeginFrame();
			if (mGraphics != 0)	mGraphics->BeginFrame();

			bool drawn = false;

			// Trigger all registered draw callbacks
			if (mRoot.GetFlag(Object::Flag::Enabled))
			{
				Lock();
				drawn = HandleOnDraw();
				Unlock();
			}

			// If we didn't draw anything, clear the screen
			if (!drawn)
			{
				mFullDraw = 0;
				mGraphics->SetActiveRenderTarget(0);
				mGraphics->Clear();
			}

			// Draw the UI after everything else
			if (mUI != 0) mUI->Draw();

			// Finish the drawing process
			if (mGraphics != 0)	mGraphics->EndFrame();
			mWin->EndFrame();

			// Increment the framerate
			Time::IncrementFPS();
		}

		// Sleep the thread, letting others run in the background
		Thread::Sleep(mFullDraw > 0 ? mSleepDelay : mUISleepDelay);
		return true;
	}
	return true;
}

//============================================================================================================
// Properly shuts down everything
//============================================================================================================

void Core::Shutdown()
{
	if (mWin != 0) mWin->Close();
}

//============================================================================================================
// Retrieves a mesh with the specified name
//============================================================================================================

Mesh* Core::GetMesh (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mMeshes.AddUnique(name) : mMeshes.Find(name));
}

//============================================================================================================
// Retrieves a mesh with the specified name
//============================================================================================================

Cloud* Core::GetCloud (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mClouds.AddUnique(name) : mClouds.Find(name));
}

//============================================================================================================
// Creates a new model or retrieves an existing one
//============================================================================================================

Model* Core::GetModel (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;

	Model* model = mModels.Find(name);
	
	if (model == 0 && createIfMissing)
	{
		model = mModels.AddUnique(name);
		model->mCore = this;
		if (model->Load(name)) model->SetSerializable(false);
	}
	return model;
}

//============================================================================================================
// Retrieves a resource with the specified name
//============================================================================================================

Resource* Core::GetResource (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	mResources.Lock();
	Resource* res = (createIfMissing ? mResources.AddUnique(name, false) : mResources.Find(name, false));
	if (res != 0) res->mCore = this;
	mResources.Unlock();
	return res;
}

//============================================================================================================
// Retrieves a skeleton with the specified name
//============================================================================================================

Skeleton* Core::GetSkeleton (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mSkeletons.AddUnique(name) : mSkeletons.Find(name));
}

//============================================================================================================
// Creates a new model template or retrieves an existing one
//============================================================================================================

ModelTemplate* Core::GetModelTemplate (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;

	if (createIfMissing)
	{
		ModelTemplate* temp = mModelTemplates.AddUnique(name);

		// If it's a brand-new model, try to load it
		if (temp->GetCore() == 0)
		{
			temp->mCore = this;

			if ( !temp->IsValid() )
			{
				if ( temp->Load(name, false) )
				{
					temp->SetSerializable(false);
				}
			}
		}
		return temp;
	}
	return mModelTemplates.Find(name);
}

//============================================================================================================
// Load the specified file
//============================================================================================================

bool Core::operator << (const char* file)
{
	TreeNode node;
	Lock();
	bool retVal = node.Load(file) ? SerializeFrom(node) : false;
	Unlock();
	return retVal;
}

//============================================================================================================
// Save everything into the specified file
//============================================================================================================

bool Core::operator >> (const char* file) const
{
	TreeNode node;
	node.mTag = "Root";
	Lock();
	bool retVal = SerializeTo(node) && node.Save(file);
	Unlock();
	return retVal;
}

//============================================================================================================
// Triggered by IWindow -- responds to char events
//============================================================================================================

bool Core::OnChar(byte key)
{
	if (mUI && mUI->OnChar(key)) return true;
	return false;
}

//============================================================================================================
// Triggered by IWindow -- responds to key events
//============================================================================================================

bool Core::OnKeyPress(const Vector2i& pos, byte key, bool isDown)
{
	mIsKeyDown[key] = isDown;

	// Let the UI handle this event first
	if (mUI && mUI->OnKeyPress(pos, key, isDown)) return true;

	// If the UI doesn't handle it, let the registered callbacks have at it
	if (HandleOnKey(pos, key, isDown)) return true;

	// Default behavior with no set listener
	if (mWin != 0 && !isDown)
	{
		if (key == Key::Escape)
		{
			Shutdown();
		}
		else if (key == Key::F5)
		{
			mWin->SetStyle( (mWin->GetStyle() & IWindow::Style::FullScreen) == 0 ?
				IWindow::Style::FullScreen : IWindow::Style::Normal );
		}
		else if (key == Key::F6)
		{
			mWin->SetPosition( Vector2i(100, 100) );
			mWin->SetSize( Vector2i(900, 600) );
			mWin->SetStyle(IWindow::Style::Normal);
		}
	}
	return false;
}

//============================================================================================================
// Triggered by IWindow -- responds to mouse movement
//============================================================================================================

bool Core::OnMouseMove(const Vector2i& pos, const Vector2i& delta)
{
	mMousePos = pos;
	mMouseDelta += delta;
	return true;
}

//============================================================================================================
// Triggered by IWindow -- responds to scrolling
//============================================================================================================

bool Core::OnScroll(const Vector2i& pos, float delta)
{
	if (mUI && mUI->OnScroll(pos, delta)) return true;
	return HandleOnScroll(pos, delta);
}

//============================================================================================================
// Triggered by IWindow -- responds to window resizing
//============================================================================================================

void Core::OnResize(const Vector2i& size)
{
	// Size update is queued rather than executed in Core::OnResize() because this call needs to update
	// the graphics controller, and the controller may be created in a different thread than the window.
	mUpdatedSize = size;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Core::SerializeTo (TreeNode& root, bool window, bool graphics, bool ui) const
{
	ASSERT_IF_SELF_IS_UNLOCKED;

	// Window information should always be saved first as it *has* to come first
	if (mWin != 0 && window) mWin->SerializeTo(root);

	// Makes sense to have graphics second
	if (mGraphics != 0 && graphics) mGraphics->SerializeTo(root);

	// User interface comes next
	if (mUI != 0 && ui) mUI->SerializeTo(root);

	// Save all resources and models
	if (mResources.IsValid() || mFileResource.IsValid() || mModels.IsValid())
	{
		TreeNode& node = root.AddChild( Core::ClassID() );

		if (mFileResource.IsValid())
		{
			mFileResource.Lock();
			node.AddChild("Serialize From", mFileResource);
			mFileResource.Unlock();
		}

		mModelTemplates.Lock();
		{
			for (uint i = 0; i < mModelTemplates.GetSize(); ++i)
			{
				const ModelTemplate* temp = mModelTemplates[i];

				if (temp != 0)
				{
					temp->SerializeTo(node, false);
				}
			}
		}
		mModelTemplates.Unlock();

		mModels.Lock();
		{
			for (uint i = 0; i < mModels.GetSize(); ++i)
			{
				const Model* model = mModels[i];

				if (model != 0)
				{
					model->SerializeTo(node, false);
				}
			}
		}
		mModels.Unlock();

		// Remove this node if it's empty
		if (node.mChildren.IsEmpty()) root.mChildren.Shrink();
	}

	const Object::Children& children = mRoot.GetChildren();
	const Object::Scripts&  scripts  = mRoot.GetScripts();

	// Save out the scenegraph only if there is something to save
	if (children.IsValid() || scripts.IsValid())
	{
		TreeNode& node = root.AddChild(Scene::ClassID());

		for (uint i = 0; i < scripts.GetSize(); ++i)
			scripts[i]->SerializeTo(node);

		for (uint i = 0; i < children.GetSize(); ++i)
			children[i]->SerializeTo(node);

		// If nothing was saved, don't keep the node around
		if (node.mChildren.IsEmpty())
			root.mChildren.Shrink();
	}
	return true;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Core::SerializeFrom (const TreeNode& root, bool forceUpdate, bool createThreads)
{
	ASSERT_IF_SELF_IS_UNLOCKED;
	IncrementThreadCount();

	bool retVal = true;
	bool serializable = true;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	= root.mChildren[i];
		const String&	tag		= node.mTag;
		const Variable&	value	= node.mValue;

		if (tag == Core::ClassID())
		{
			// SerializeFrom only returns 'false' if something important failed
			if ( !SerializeFrom(node, forceUpdate, createThreads) )
			{
				retVal = false;
				break;
			}
		}
		else if (tag == IWindow::ClassID())
		{
			// If window creation fails, let the calling function know
			if (mWin != 0 && !mWin->SerializeFrom(node))
			{
				retVal = false;
				break;
			}
		}
		else if (tag == IGraphics::ClassID())
		{
			// If graphics init fails, let the calling function know
			if (mGraphics != 0 && !mGraphics->SerializeFrom(node, forceUpdate))
			{
				retVal = false;
				break;
			}
		}
		else if (tag == IUI::ClassID())
		{
			if (mUI != 0 && mGraphics != 0)
			{
				Unlock();
				mUI->SerializeFrom(node);
				Lock();
			}
		}
		else if (tag == Scene::ClassID())
		{
			mRoot.SerializeFrom(node, forceUpdate);
		}
		else if (tag == Mesh::ClassID())
		{
			Mesh* mesh = GetMesh(value.AsString(), true);
			if (mesh != 0) mesh->SerializeFrom(node, forceUpdate);
		}
		else if (tag == Cloud::ClassID())
		{
			Cloud* bm = GetCloud(value.AsString(), true);
			if (bm != 0) bm->SerializeFrom(node, forceUpdate);
		}
		else if (tag == Skeleton::ClassID())
		{
			Skeleton* skel = GetSkeleton(value.AsString(), true);
			if (skel != 0) skel->SerializeFrom(node, forceUpdate);
		}
		else if (tag == ModelTemplate::ClassID())
		{
			ModelTemplate* temp = GetModelTemplate(value.AsString(), true);

			if (temp != 0)
			{
				temp->SerializeFrom(node, forceUpdate);
				if (!serializable) temp->SetSerializable(false);
			}
		}
		else if (tag == Model::ClassID())
		{
			Model* model = GetModel(value.AsString(), true);

			if (model != 0)
			{
				model->SerializeFrom(node, forceUpdate);
				if (!serializable) model->SetSerializable(false);
			}
		}
		else if (tag == "Serializable")
		{
			value >> serializable;
		}
		else if (tag == "Sleep")
		{
			uint ms;
			if (value >> ms) Thread::Sleep( ms );
		}
		else if (tag == "Serialize From" || tag == "Execute")
		{
			if (value.IsStringArray())
			{
				const Array<String>& arr = value.AsStringArray();

				FOREACH(b, arr)
				{
					SerializeFrom(arr[b], serializable && createThreads, serializable);
				}
			}
			else if (value.IsString())
			{
				SerializeFrom(value.AsString(), serializable && createThreads, serializable);
			}
		}
	}
	// Something may have changed, update the scene
	mIsDirty = true;
	DecrementThreadCount();
	return retVal;
}

//============================================================================================================
// Serializes from the specified path
//============================================================================================================

bool Core::SerializeFrom (const String& path, bool separateThread, bool save)
{
	bool retVal (false);

#ifndef R5_MEMORY_TEST
	if (separateThread)
	{
		IncrementThreadCount();
		Thread::Create( SerializeResourceThread, GetResource(path) );
	}
	else
#endif
	{
		Array<String> files;

		if (System::GetFiles(path, files, true))
		{
			FOREACH(i, files)
			{
				TreeNode root;
				if (root.Load(files[i])) SerializeFrom(root);
			}
		}
	}

	// Remember this serialization request
	if (save)
	{
		mFileResource.Lock();
		mFileResource.AddUnique(path);
		mFileResource.Unlock();
	}
	return retVal;
}