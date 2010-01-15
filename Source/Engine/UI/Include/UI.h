#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// R5 engine-based implementation of the UIManager
//============================================================================================================

class UI : public UIManager
{
private:

	struct CustomQueue : public UIQueue
	{
		IGraphics*	mGraphics;		// In order to release the VBOs properly, graphics controller must be remembered
		IVBO*		mVbo;			// Vertex buffer object, created as needed
		uint		mVertexCount;	// Number of vertices in the VBO

		CustomQueue (IGraphics* graphics) : UIQueue(), mGraphics(graphics), mVbo(0), mVertexCount(0) {}

		virtual bool IsValid() const { return mVertices.IsValid() || mVertexCount > 0; }
	};

	IGraphics*  mGraphics;

public:

	UI (IGraphics* graphics);

public:

	virtual float		GetCurrentTime() const				{ return Time::GetTime();	}
	virtual ITexture*	GetTexture  (const String& name)	{ return mGraphics->GetTexture(name);	}
	virtual IFont*		GetFont		(const String& name)	{ return mGraphics->GetFont(name);		}
	virtual IShader*	GetShader   (const String& name)	{ return mGraphics->GetShader(name);	}

protected:

	virtual UIQueue*	CreateQueue()					{ return new CustomQueue(mGraphics); }
	virtual void		UpdateBuffer(UIQueue* queue);	// Updates the buffer associated with the rendering queue
	virtual void		OnPreDraw() const;				// Prepares to render
	virtual uint		DrawQueue(UIQueue* queue);		// Draws a single queue, returning the number of triangles drawn
	virtual void		OnPostDraw() const;				// Post-render cleanup
};