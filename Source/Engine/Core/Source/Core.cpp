#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Since R5::Core has worker threads, they need to have a pointer to the core
//============================================================================================================

Core*			g_core			= 0;
Thread::ValType	g_threadCount	= 0;

//============================================================================================================
// Resource executioner thread callback
//============================================================================================================

R5_THREAD_FUNCTION(WorkerThread, ptr)
{
	if (g_core)
	{
		Thread::Increment( g_threadCount );
		Resource* resource = reinterpret_cast<Resource*>(ptr);

#ifdef _DEBUG
		long threadId = g_threadCount;
		ulong timestamp = Time::GetMilliseconds();
		System::Log("[THREAD]  Executing '%s' [ID: %u]", resource->GetName().GetBuffer(), threadId);
#endif

		resource->Lock();
		g_core->SerializeFrom( resource->GetRoot() );
		resource->Unlock();

		Thread::Decrement( g_threadCount );

#ifdef _DEBUG
		System::Log("[THREAD]  Finished executing '%s' in %u ms [ID: %u]",
			resource->GetName().GetBuffer(), Time::GetMilliseconds() - timestamp, threadId);
#endif
	}
	return 0;
}

//============================================================================================================
// Core constructor and destructor
//============================================================================================================

Core::Core(	IWindow*		window,
			IGraphics*		graphics,
			IUI*			gui) :	mWin(window),
									mGraphics(graphics),
									mUI(gui),
									mScene(0)
{
	g_core = this;
	mScene = new Scene(this);

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

Core::~Core()
{
	// Ensure that the core won't be deleted until all worker threads finish
	while (g_threadCount > 0) Thread::Sleep(1);
	if (mWin != 0) mWin->SetGraphics(0);
	if (mScene != 0) delete mScene;
	g_core = 0;
	Thread::ImproveTimerFrequency(false);
}

//============================================================================================================
// Releases the meshes and the scene graph
//============================================================================================================

void Core::Release()
{
	mMeshes.Lock();
	{
		if (mScene != 0) mScene->Release();
		mMeshes.Release();

		mSkeletons.Lock();
		mSkeletons.Release();
		mSkeletons.Unlock();
	}
	mMeshes.Unlock();
}

//============================================================================================================
// Starting a new frame
//============================================================================================================

bool Core::Update ()
{
	Time::Update();
	Time::IncrementFPS();

	if (mWin != 0)
	{
		// Window update function can return 'false' if the window has been closed
		if (!mWin->Update()) return false;
		bool minimized = mWin->IsMinimized();

		// Size update is queued rather than executed in Core::OnResize() because that call needs to update
		// the graphics controller, and the controller may be created in a different thread than the window.
		if (mUpdatedSize)
		{
			if (mGraphics)	mGraphics->SetViewport(mUpdatedSize);
			if (mUI)		mUI->OnResize(mUpdatedSize);

			// The size no longer needs to be updated
			mUpdatedSize = 0;
		}

		// Do not update anything unless some time has passed
		if (Time::GetDelta() != 0)
		{
			// Update all props and models
			mModels.Lock();
			{
				for (uint i = mModels.GetSize(); i > 0; )
				{
					Model* model = mModels[--i];
					if (model != 0) model->Update();
				}
			}
			mModels.Unlock();

			// Pre-update callbacks
			mPre.Execute();

			// Update the scenegraph
			if (mScene != 0) mScene->Update();

			// Post-update callbacks
			mPost.Execute();

			// The UI should only be updated if the window is not minimized
			if (mUI != 0 && !minimized) mUI->Update();

			// Post-GUI updates
			mLate.Execute();
		}

		// If we have an OnDraw listener, call it
		if (mOnDraw && !minimized) mOnDraw();
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
// Begins the rendering process, locking all the necessary resources
//============================================================================================================

void Core::BeginFrame()
{
	if (mWin		!= 0)	mWin->BeginFrame();
	if (mGraphics	!= 0)	mGraphics->BeginFrame();
}

//============================================================================================================
// Prepares the scene for rendering onto the specified target
//============================================================================================================

void Core::PrepareScene (IRenderTarget* target)
{
	if ( mWin != 0 && !mWin->IsMinimized() )
	{
		// Activate the render target
		mGraphics->SetActiveRenderTarget(target);

		// Clear the screen
		mGraphics->Clear();
	}
}

//============================================================================================================
// Culls the scene's objects, given the specified camera's perspective
//============================================================================================================

void Core::CullScene (const Vector3f& pos, const Quaternion& rot, const Vector3f& range)
{
	if (mWin != 0 && !mWin->IsMinimized())
	{
		mGraphics->SetCameraRange(range);
		mGraphics->SetCameraOrientation( pos, rot.GetDirection(), rot.GetUp() );
		mGraphics->SetActiveProjection( IGraphics::Projection::Perspective );

		// Update the frustum
		mFrustum.Update( mGraphics->GetViewProjMatrix() ); // If world matrix is used: (world * ViewProj)

		// Cull the scene
		mScene->Cull(mFrustum);
	}
}

//============================================================================================================
// Convenience function
//============================================================================================================

void Core::CullScene (const Camera* cam)
{
	if (cam != 0)
	{
		CullScene(	cam->GetAbsolutePosition(),
					cam->GetAbsoluteRotation(),
					cam->GetAbsoluteRange() );
	}
}

//============================================================================================================
// Convenience function
//============================================================================================================

void Core::CullScene (const CameraController& cam)
{
	CullScene(	cam.GetPosition(),
				cam.GetRotation(),
				cam.GetRange() );
}

//============================================================================================================
// Draw the entire scene
//============================================================================================================

uint Core::DrawScene (const ITechnique* tech, bool insideOut)
{
	uint count (0);

	if ( mWin != 0 && !mWin->IsMinimized() && mGraphics != 0 )
	{
		if (tech != 0)
		{
			// Draw all objects from the specified technique
			count += mScene->Render(mGraphics, tech, insideOut);
		}
		else
		{
			// Draw the scene using all default forward rendering techniques
			ITechnique* opaque = mGraphics->GetTechnique("Opaque");
			ITechnique* trans  = mGraphics->GetTechnique("Transparent");
			ITechnique* part   = mGraphics->GetTechnique("Particle");
			ITechnique* glow   = mGraphics->GetTechnique("Glow");
			ITechnique* glare  = mGraphics->GetTechnique("Glare");

			count += mScene->Render(mGraphics, opaque, insideOut);
			count += mScene->Render(mGraphics, trans,  insideOut);
			count += mScene->Render(mGraphics, part,   insideOut);
			count += mScene->Render(mGraphics, glow,   insideOut);
			count += mScene->Render(mGraphics, glare,  insideOut);
		}

		// Restore the potentially changed properties
		mGraphics->ResetWorldMatrix();
		mGraphics->SetNormalize(false);
	}
	return count;
}

//============================================================================================================
// Draw the user interface
//============================================================================================================

uint Core::DrawUI()
{
	if (mWin != 0 && !mWin->IsMinimized() && mUI != 0)
	{
		return mUI->Render();
	}
	return 0;
}

//============================================================================================================
// Finish drawing, refreshing the screen
//============================================================================================================

void Core::EndFrame()
{
	if (mGraphics	!= 0)	mGraphics->EndFrame();
	if (mWin		!= 0)	mWin->EndFrame();
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
// Creates a new model or retrieves an existing one
//============================================================================================================

Model* Core::GetModel (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;

	if (createIfMissing)
	{
		Model* model = mModels.AddUnique(name);
		model->_SetCore(this);
		return model;
	}
	return mModels.Find(name);
}

//============================================================================================================
// Retrieves a resource with the specified name
//============================================================================================================

Resource* Core::GetResource (const String& name, bool createIfMissing)
{
	if (name.IsEmpty()) return 0;
	return (createIfMissing ? mResources.AddUnique(name)	: mResources.Find(name));
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
			temp->_SetCore(this);

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
	return node.Load(file) ? SerializeFrom(node) : false;
}

//============================================================================================================
// Save everything into the specified file
//============================================================================================================

bool Core::operator >> (const char* file) const
{
	TreeNode node;
	node.mTag = "Root";
	if (!SerializeTo(node)) return false;
	return node.Save(file);
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

bool Core::OnKey(const Vector2i& pos, byte key, bool isDown)
{
	mIsKeyDown[key] = isDown;
	if (mUI && mUI->OnKey(pos, key, isDown)) return true;

	// If we have a key event listener, let it respond
	if (mOnKey) return mOnKey(pos, key, isDown);

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
	if (mUI && mUI->OnMouseMove(pos, delta)) return true;
	return (mOnMouseMove) ? mOnMouseMove(pos, delta) : false;
}

//============================================================================================================
// Triggered by IWindow -- responds to scrolling
//============================================================================================================

bool Core::OnScroll(const Vector2i& pos, float delta)
{
	if (mUI && mUI->OnScroll(pos, delta)) return true;
	return (mOnScroll) ? mOnScroll(pos, delta) : false;
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
// Loads the resources node
//============================================================================================================

void Core::ParseResources(const TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;

		Resource* resource = GetResource(tag);
		resource->SerializeFrom(node);
	}
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Core::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	bool serializable = true;

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node	= root.mChildren[i];
		const String&	tag		= node.mTag;
		const Variable&	value	= node.mValue;

		if ( tag == Core::ClassID() )
		{
			// SerializeFrom only returns 'false' if something important failed
			if ( !SerializeFrom(node, forceUpdate) )
				return false;
		}
		else if ( tag == IWindow::ClassID() )
		{
			// If window creation fails, let the calling function know
			if (mWin != 0 && !mWin->SerializeFrom(node))
				return false;
		}
		else if ( tag == IGraphics::ClassID() )
		{
			// If graphics init fails, let the calling function know
			if (mGraphics != 0 && !mGraphics->SerializeFrom(node, forceUpdate))
				return false;
		}
		else if ( tag == IUI::ClassID() )
		{
			if (mUI != 0 && mGraphics != 0)
				mUI->SerializeFrom(node);
		}
		else if ( tag == Scene::ClassID() )
		{
			mScene->SerializeFrom(node, forceUpdate);
		}
		else if ( tag == Mesh::ClassID() )
		{
			Mesh* mesh = GetMesh(value.IsString() ? value.AsString() : value.GetString(), true);
			if (mesh != 0) mesh->SerializeFrom(node, forceUpdate);
		}
		else if ( tag == Skeleton::ClassID() )
		{
			Skeleton* skel = GetSkeleton(value.IsString() ? value.AsString() : value.GetString(), true);
			if (skel != 0) skel->SerializeFrom(node, forceUpdate);
		}
		else if ( tag == Resource::ClassID() )
		{
			Resource* res = GetResource(value.IsString() ? value.AsString() : value.GetString(), true);

			if (res != 0)
			{
				res->SerializeFrom(node, forceUpdate);
				if (!serializable) res->SetSerializable(false);
			}
		}
		else if ( tag == ModelTemplate::ClassID() )
		{
			ModelTemplate* temp = GetModelTemplate(value.IsString() ? value.AsString() : value.GetString(), true);

			if (temp != 0)
			{
				temp->SerializeFrom(node, forceUpdate);
				if (!serializable) temp->SetSerializable(false);
			}
		}
		else if ( tag == Model::ClassID() )
		{
			Model* model = GetModel(value.IsString() ? value.AsString() : value.GetString(), true);

			if (model != 0)
			{
				model->SerializeFrom(node, forceUpdate);
				if (!serializable) model->SetSerializable(false);
			}
		}
		else if ( tag == "Serializable" )
		{
			value >> serializable;
		}
		else if ( tag == "Sleep" )
		{
			uint ms;
			if (value >> ms) Thread::Sleep( ms );
		}
		else if ( tag == "Execute" )
		{
			// Find the resource
			String name (value.IsString() ? value.AsString() : value.GetString());
			Resource* res = GetResource(name);

			// If the resource is valid, create a worker thread for it
			if (res->IsValid())
			{
#ifndef R5_MEMORY_TEST
				Thread::Create( WorkerThread, res );
#else
				SerializeFrom( res->GetRoot() );
#endif
				if (serializable)
				{
					mExecuted.Lock();
					mExecuted.Expand() = name;
					mExecuted.Unlock();
				}
			}
		}
		// Registered serialization callback
		else if (mOnFrom) mOnFrom(node);
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Core::SerializeTo (TreeNode& root) const
{
	// Window information should always be saved first as it *has* to come first
	if (mWin) mWin->SerializeTo(root);

	// Makes sense to have graphics second
	if (mGraphics) mGraphics->SerializeTo(root);

	// User interface comes next
	if (mUI) mUI->SerializeTo(root);

	// Save all resources and models
	if (mResources.IsValid() || mExecuted.IsValid() || mModels.IsValid())
	{
		TreeNode& node = root.AddChild( Core::ClassID() );

		mResources.Lock();
		{
			for (uint i = 0; i < mResources.GetSize(); ++i)
				mResources[i]->SerializeTo(node);
		}
		mResources.Unlock();

		mExecuted.Lock();
		{
			for (uint i = 0; i < mExecuted.GetSize(); ++i)
				node.AddChild("Execute").mValue = mExecuted[i];
		}
		mExecuted.Unlock();

		mModelTemplates.Lock();
		{
			for (uint i = 0; i < mModelTemplates.GetSize(); ++i)
			{
				const ModelTemplate* temp = mModelTemplates[i];

				if (temp != 0)
				{
					temp->SerializeTo(node);
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
					model->SerializeTo(node);
				}
			}
		}
		mModels.Unlock();
	}

	// Scenegraph is loaded second last
	mScene->SerializeTo(root);

	// Registered serialization callback
	if (mOnTo) mOnTo(root);
	return true;
}

//============================================================================================================
// Executes an existing (loaded) resource in a different thread
//============================================================================================================

void Core::SerializeFrom (Resource* resource)
{
	if (resource != 0 && resource->IsValid())
	{
#ifndef R5_MEMORY_TEST
		Thread::Create( WorkerThread, resource );
#else
		SerializeFrom( resource->GetRoot() );
#endif
	}
}