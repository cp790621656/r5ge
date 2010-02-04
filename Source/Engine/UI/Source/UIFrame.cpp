#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Hides the frame
//============================================================================================================

void UIFrame::Hide (float animTime)
{
	SetFocus(false);
	SetAlpha(0.0f, animTime);
}

//============================================================================================================
// Marks a rendering queue associated with this texture as being dirty
//============================================================================================================

void UIFrame::OnDirty (const ITexture* tex, int layer, const UIWidget* widget)
{
	UIQueue* ptr (0);

	for (uint i = 0; i < mQs.GetSize(); ++i)
	{
		UIQueue* queue (mQs[i]);

		if (queue			!= 0 &&
			queue->mTex		== tex &&
			queue->mLayer	== layer &&
			queue->mWidget	== widget)
		{
			ptr = queue;
			break;
		}
	}

	if (ptr == 0)
	{
		ptr = mUI->CreateQueue();
		ptr->mTex	= tex;
		ptr->mLayer = layer;
		ptr->mWidget	= widget;
		ASSERT(ptr != 0, "Failed to create the queue?");
		mQs.Expand() = ptr;
	}

	ptr->mIsDirty = true;
	mUI->SetDirty();
}

//============================================================================================================
// Marks all rendering queues as needing to be rebuilt
//============================================================================================================

void UIFrame::SetDirty()
{
	mUI->SetDirty();

	for (uint i = 0; i < mQs.GetSize(); ++i)
	{
		if (mQs[i] != 0)
		{
			mQs[i]->mIsDirty = true;
		}
	}
}

//============================================================================================================
// Updates the rendering queues as necessary, then draws them to the screen
//============================================================================================================

uint UIFrame::OnDraw()
{
	uint triangles = 0;

	if (mRegion.IsVisible())
	{
		bool sorted (true);
		uint size = mQs.GetSize();

		// Update the queues
		for (uint i = 0; i < size; ++i)
		{
			UIQueue* queue (mQs[i]);

			if (queue != 0 && queue->mIsDirty)
			{
				const ITexture* tex (queue->mTex);

				if (tex == 0 || tex->IsValid())
				{
					sorted = false;
					Fill(queue);

#ifdef _DEBUG
	//System::Log("[UI]      [Layer %u] Frame '%s' is updating queue for texture '%s'", queue->mLayer,
	//	mName.GetBuffer(), (queue->mTex ? queue->mTex->GetName().GetBuffer() : "<None>"));
	//System::Log("          - %u quads", queue->mVertices.GetSize() / 4);
	//System::Log("          - %s bytes", String::GetFormattedSize(queue->mVertices.GetSize() * sizeof(IUI::Vertex)).GetBuffer() );
#endif

					// Update the queue
					mUI->UpdateBuffer( queue );
					queue->mVertices.Clear();

					// The queue's buffer has been updated -- reset the flags
					queue->mIsDirty = false;
					queue->mDynamic = false;
				}
			}
		}

		if (!sorted)
		{
			// Sorting
			for (uint last = size; last > 1 && !sorted; )
			{
				--last;
				sorted = true;

				for (uint i = 0; i < last; ++i)
				{
					if (mQs[i]->mLayer > mQs[i+1]->mLayer)
					{
						Swap(mQs[i], mQs[i+1]);
						sorted = false;
					}
				}
			}
		}

		// Cleanup
		for (uint i = size; i > 1; )
		{
			UIQueue* queue (mQs[--i]);

			if (queue == 0 || !queue->IsValid())
			{
				mQs.DeleteAt(i);
				--size;
			}
		}

		// Rendering
		for (uint i = 0; i < size; ++i)
		{
			UIQueue* queue (mQs[i]);

			if (queue != 0)
			{
				const ITexture* tex (queue->mTex);

				if (tex == 0 || tex->IsValid())
				{
					// Draw the queue
					triangles += mUI->DrawQueue( queue );
				}
			}
		}
	}
	return triangles;
}