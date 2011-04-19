#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Frame statistics struct -- used by both UI as well as Graphics
// Author: Michael Lyashenko
//============================================================================================================

struct FrameStats
{
	uint mTriangles;
	uint mDrawCalls;
	uint mMatSwitches;
	uint mTexSwitches;
	uint mBufferBinds;
	uint mShaderSwitches;
	uint mLightSwitches;
	uint mTechSwitches;

	FrameStats() { Clear(); }

	void Clear()
	{
		mTriangles		= 0;
		mDrawCalls		= 0;
		mMatSwitches	= 0;
		mTexSwitches	= 0;
		mBufferBinds	= 0;
		mShaderSwitches	= 0;
		mLightSwitches	= 0;
		mTechSwitches	= 0;
	}

	void operator -= (const FrameStats& stats)
	{
		mTriangles		-= stats.mTriangles;
		mDrawCalls		-= stats.mDrawCalls;
		mMatSwitches	-= stats.mMatSwitches;
		mTexSwitches	-= stats.mTexSwitches;
		mBufferBinds	-= stats.mBufferBinds;
		mShaderSwitches	-= stats.mShaderSwitches;
		mLightSwitches	-= stats.mLightSwitches;
		mTechSwitches	-= stats.mTechSwitches;
	}
};