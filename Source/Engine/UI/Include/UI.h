#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// R5 engine-based implementation of the rendering queue
//============================================================================================================

struct R5Queue : public Queue
{
	IGraphics*	mGraphics;		// In order to release the VBOs properly, graphics controller must be remembered
	IVBO*		mVbo;			// Vertex buffer object, created as needed
	uint		mVertexCount;	// Number of vertices in the VBO

	R5Queue (IGraphics* graphics) : Queue(), mGraphics(graphics), mVbo(0), mVertexCount(0) {}

	virtual bool IsValid() const { return mVertices.IsValid() || mVertexCount > 0; }
};

//============================================================================================================
// R5 engine-based implementation of the Root
//============================================================================================================

class UI : public Root
{
private:

	IGraphics*  mGraphics;

public:

	UI (IGraphics* graphics);

public:

	virtual float		GetCurrentTime() const				{ return Time::GetTime();	}
	virtual ITexture*	GetTexture  (const String& name)	{ return mGraphics->GetTexture(name);	}
	virtual IFont*		GetFont		(const String& name)	{ return mGraphics->GetFont(name);		}
	virtual IShader*	GetShader   (const String& name)	{ return mGraphics->GetShader(name);	}

protected:

	virtual Queue*	CreateQueue()				{ return new R5Queue(mGraphics); }
	virtual void	UpdateBuffer(Queue* queue);	// Updates the buffer associated with the rendering queue
	virtual void	OnPreRender() const;		// Prepares to render
	virtual uint	RenderQueue(Queue* queue);	// Renders a single queue, returning the number of triangles drawn
	virtual void	OnPostRender() const;		// Post-render cleanup
};