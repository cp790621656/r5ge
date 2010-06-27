#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Initialize the storage
//============================================================================================================

void TemporaryStorage::Initialize (IGraphics* graphics)
{
	mGraphics = graphics;
	mTempTextures.ExpandTo(32, true);
	mTempTargets.ExpandTo(32, true);
	mTempVBOs.ExpandTo(32, true);
}

//============================================================================================================
// Release all temporary resources
//============================================================================================================

void TemporaryStorage::Release()
{
	FOREACH(i, mTempVBOs)
	{
		if (mTempVBOs[i] != 0)
		{
			mGraphics->DeleteVBO(mTempVBOs[i]);
			mTempVBOs[i] = 0;
		}
	}

	FOREACH(i, mTempTargets)
	{
		if (mTempTargets[i] != 0)
		{
			mGraphics->DeleteRenderTarget(mTempTargets[i]);
			mTempTargets[i] = 0;
		}
	}

	FOREACH(i, mTempTextures)
	{
		if (mTempTextures[i] != 0)
		{
			mGraphics->DeleteTexture(mTempTextures[i]);
			mTempTextures[i] = 0;
		}
	}
}

//============================================================================================================
// Retrieves the specified texture, creating a new one if necessary
//============================================================================================================

ITexture* TemporaryStorage::GetRenderTexture (uint index)
{
	typedef ITexture* ITexturePtr;
	mTempTextures.ExpandTo(index, true);
	ITexturePtr& ptr = mTempTextures[index];
	if (ptr == 0) ptr = mGraphics->CreateRenderTexture();
	return ptr;
}

//============================================================================================================
// Retrieves the specified render target, creating a new one if necessary
//============================================================================================================

IRenderTarget* TemporaryStorage::GetRenderTarget (uint index)
{
	typedef IRenderTarget* IRenderTargetPtr;
	mTempTargets.ExpandTo(index, true);
	IRenderTargetPtr& ptr = mTempTargets[index];
	if (ptr == 0) ptr = mGraphics->CreateRenderTarget();
	return ptr;
}

//============================================================================================================
// Retrieves the specified vertex buffer object, creating a new one if necessary
//============================================================================================================

IVBO* TemporaryStorage::GetVBO (uint index)
{
	typedef IVBO* IVBOPtr;
	mTempVBOs.ExpandTo(index, true);
	IVBOPtr& ptr = mTempVBOs[index];
	if (ptr == 0) ptr = mGraphics->CreateVBO();
	return ptr;
}