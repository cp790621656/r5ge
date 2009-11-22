#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Delayed callback executed by the GLGraphics manager (buffers should only be released on the graphics thread)
//============================================================================================================

void DeleteBuffer(void* ptr)
{
	uint vbo = (uint)(ulong)ptr;
	glDeleteBuffers(1, &vbo);
}

//============================================================================================================
// STATIC: Activates the specified vertex buffer
//============================================================================================================

uint g_vertexBuffer = 0;
uint g_indexBuffer  = 0;

void VBO::Activate(uint id, uint type)
{
	if (type == IVBO::Type::Vertex)
	{
		if (g_vertexBuffer != id)
		{
			glBindBuffer( GL_ARRAY_BUFFER, g_vertexBuffer = id );
			CHECK_GL_ERROR;
		}
	}
	else if (type == IVBO::Type::Index)
	{
		if (g_indexBuffer != id)
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, g_indexBuffer = id );
			CHECK_GL_ERROR;
		}
	}
	else
	{
		ASSERT(false, "Invalid VBO type!");
	}
}

//============================================================================================================

VBO::VBO(IGraphics* graphics) : mGraphics(graphics), mVbo(0), mSize(0), mType(Type::Invalid), mGlType(0), mData(0) {}

//============================================================================================================
// INTERNAL: Releases the VBO
//============================================================================================================

void VBO::_InternalRelease(bool delayExecution)
{
	if (mVbo != 0)
	{
		g_caps.DecreaseBufferMemory(mSize);
		
		mSize	= 0;
		mType	= Type::Invalid;
		mGlType = 0;
		mData	= 0;

		if (g_vertexBuffer == mVbo) g_vertexBuffer = -1;
		if (g_indexBuffer  == mVbo) g_indexBuffer  = -1;

		if (mVbo != 0)
		{
			if (delayExecution)
			{
				mGraphics->AddDelayedDelegate(DeleteBuffer, (void*)mVbo);
			}
			else
			{
				// Optimized version for when we are inside the graphics thread
				glDeleteBuffers(1, &mVbo);
			}
			mVbo = 0;
		}
	}
}

//============================================================================================================
// PRIVATE: Changes the VBO's type if necessary, and activates the VBO
//============================================================================================================

void VBO::_ActivateAs(uint type)
{
	if ( mType != type && mVbo != 0 )
	{
		glDeleteBuffers(1, &mVbo);
		mVbo = 0;
		WARNING( "Warning: Buffer type changed. Is this intentional?" );
	}

	mType = type;
	mGlType = (mType == Type::Index) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

	// If the buffer is new, it should be static and it needs to be created
	if (mVbo == 0) glGenBuffers(1, &mVbo);
	ASSERT( mVbo != 0, "Failed to create the VBO!" );

	// Activate the VBO
	Activate(mVbo, mType);
}

//============================================================================================================
// Retrieves the buffer's contents from the video memory
//============================================================================================================

void VBO::Get (VoidPtr& data, uint& size, uint& type)
{
	// Don't allow more than one VBO operation per lock/unlock session
	if (mSize > 0 && mData == 0)
	{
		_ActivateAs(type);

		mData = glMapBuffer(mGlType, GL_READ_ONLY);
		ASSERT( mData != 0, glGetErrorString() );

		data = mData;
		size = mSize;
		type = mType;
	}
	else
	{
		data = 0;
		size = 0;
		type = IVBO::Type::Invalid;
	}
}

//============================================================================================================
// Updates the vertex buffer's data, creating a new VBO if necessary
//============================================================================================================

void VBO::Set (const void* data, uint size, uint type, bool dynamic)
{
	if (data != 0 && mData == 0)
	{
		_ActivateAs(type);
		g_caps.DecreaseBufferMemory(mSize);
		glBufferData( mGlType, mSize = size, data, dynamic ? GL_STREAM_DRAW : GL_STATIC_DRAW );
		CHECK_GL_ERROR;
		g_caps.IncreaseBufferMemory(mSize);
	}
}

//============================================================================================================
// Reserves the specified size chunk of memory on the videocard and returns a pointer to the editable data
//============================================================================================================

void* VBO::Reserve (uint size, uint type, bool dynamic)
{
	// Don't allow more than one VBO operation per lock/unlock session
	if (mData != 0) return 0;

	// Activate the VBO
	_ActivateAs(type);

	// If data was specified or the requested size was increased, update the buffer
	if ( size > mSize )
	{
		g_caps.DecreaseBufferMemory(mSize);
		glBufferData( mGlType, mSize = size, 0, dynamic ? GL_STREAM_DRAW : GL_STATIC_DRAW );
		CHECK_GL_ERROR;
		g_caps.IncreaseBufferMemory(mSize);
	}

	mData = glMapBuffer( mGlType, GL_WRITE_ONLY);
	ASSERT( mData != 0, glGetErrorString() );
	return mData;
}

//============================================================================================================
// Submits the data to the videocard
//============================================================================================================

void VBO::Unlock()
{
	if (mData != 0)
	{
		Activate(mVbo, mType);
		glUnmapBuffer( mGlType );
		mData = 0;
	}
	mLock.Unlock();
}