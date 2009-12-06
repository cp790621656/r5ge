#pragma once

//=============================================================================================
// OpenGL Vertex Buffer Object implementation
//=============================================================================================

class GLGraphics;

class GLVBO : public IVBO
{
protected:

	IGraphics*	mGraphics;	// GLGraphics manager that has created this VBO
	uint		mVbo;		// Actual bound vertex buffer object
	uint		mSize;		// Size of the allocated memory
	uint		mType;		// OpenGL has normal array buffers, as well as element buffers
	uint		mGlType;
	void*		mData;

	Thread::Lockable	mLock;

private:

	// Only the R5::GLGraphics class should be able to create VBOs
	friend class R5::GLGraphics;
	GLVBO(IGraphics* graphics);

public:

	virtual ~GLVBO() { _InternalRelease(false); }

private:

	void _InternalRelease(bool delayExecution);

public:
	
	virtual uint	GetID()		const { return mVbo; }
	virtual uint	GetType()	const { return mType; }
	virtual uint	GetSize()	const { return mSize; }
	virtual bool	IsValid()	const { return (mVbo != 0); }
	virtual void	Release()	{ mLock.Lock(); _InternalRelease(true); mLock.Unlock(); }
	virtual void	Lock()		{ mLock.Lock(); }
	virtual void	Get			(VoidPtr& data,		uint& size, uint& type);
	virtual void	Set			(const void* data,	uint  size, uint  type = Type::Vertex, bool dynamic = false);
	virtual void	Unlock();

private:

	friend class GLController;

	static void Activate(uint id, uint type);
	void _ActivateAs(uint type);
};