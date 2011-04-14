#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// R5 engine-based implementation of the UIManager
// Author: Michael Lyashenko
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
	IWindow*	mWindow;

	// Stack of draw regions
	Array<IGraphics::Rect> mRects;

public:

	// Graphics is necessary as it's used to draw the UI.
	// The window is optional -- it's used to copy/paste to and from the clipboard.
	UI (IGraphics* graphics, IWindow* window = 0);

public:

	// Retrieves a string from the clipboard
	virtual String GetClipboardText() const { return (mWindow != 0) ? mWindow->GetClipboardText() : String(); }

	// Sets the system clipboard text
	virtual void SetClipboardText (const String& text) { if (mWindow != 0) mWindow->SetClipboardText(text); }

	virtual float		GetCurrentTime() const				{ return Time::GetTime();	}
	virtual ITexture*	GetTexture  (const String& name)	{ return mGraphics->GetTexture(name);	}
	virtual IFont*		GetFont		(const String& name);
	virtual IShader*	GetShader   (const String& name)	{ return mGraphics->GetShader(name);	}

protected:

	virtual UIQueue*	CreateQueue()						{ return new CustomQueue(mGraphics); }
	virtual void		UpdateBuffer(UIQueue* queue);		// Updates the buffer associated with the rendering queue
	virtual void		OnPreDraw() const;					// Prepares to render
	virtual void		SetClipRect (const Rect& rect);		// Sets the draw region
	virtual uint		DrawQueue(UIQueue* queue);			// Draws a single queue, returning the number of triangles drawn
	virtual void		RestoreClipRect();						// Restores the previous draw region
	virtual void		OnPostDraw() const;					// Post-render cleanup
};