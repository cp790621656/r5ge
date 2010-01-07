#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Up to 4 parameters can be passed to each noise filter
//============================================================================================================

class Parameters
{
	uint	mCount;
	float	mF[4];
	
public:

	Parameters ()										: mCount(0) { memset(mF, 0, sizeof(float) * 4); }
	Parameters (float f0)								: mCount(1) { mF[0] = f0;  mF[1] = 0.0f; mF[2] = 0.0f; mF[3] = 0.0f; }
	Parameters (const Vector2f& v)						: mCount(2) { mF[0] = v.x; mF[1] = v.y;  mF[2] = 0.0f; mF[3] = 0.0f; }
	Parameters (const Vector3f& v)						: mCount(3) { mF[0] = v.x; mF[1] = v.y;  mF[2] = v.z;  mF[3] = 0.0f; }
	Parameters (const Quaternion& v)					: mCount(4) { mF[0] = v.x; mF[1] = v.y;  mF[2] = v.z;  mF[3] = v.w;  }
	Parameters (float f0, float f1)						: mCount(2) { mF[0] = f0;  mF[1] = f1;   mF[2] = 0.0f; mF[3] = 0.0f; }
	Parameters (float f0, float f1, float f2)			: mCount(3) { mF[0] = f0;  mF[1] = f1;   mF[2] = f2;   mF[3] = 0.0f; }
	Parameters (float f0, float f1, float f2, float f3)	: mCount(4) { mF[0] = f0;  mF[1] = f1;   mF[2] = f2;   mF[3] = f3;   }
	Parameters (const String& text)						{ memset(mF, 0, sizeof(float) * 4); *this = text; }
	Parameters (const char* text)						{ memset(mF, 0, sizeof(float) * 4); *this = text; }
	
	uint GetCount() const								{ return mCount; }
	float operator [] (uint val) const					{ return val < mCount ? mF[val] : 0.0f; }
	void operator = (const String& text)				{ return (*this = text.GetBuffer()); }
	void operator = (float f)							{ mCount = 1; mF[0] = f; }
	void operator = (const Vector2f& v)					{ mCount = 2; mF[0] = v.x; mF[1] = v.y; }
	void operator = (const Vector3f& v)					{ mCount = 3; mF[0] = v.x; mF[1] = v.y; mF[2] = v.z; }
	void operator = (const Quaternion& v)				{ mCount = 4; mF[0] = v.x; mF[1] = v.y; mF[2] = v.z; mF[3] = v.w; }
	void operator = (const char* text)
	{
		mCount = (text != 0 && text[0] != 0) ?
			sscanf(text, "%f %f %f %f", &mF[0], &mF[1], &mF[2], &mF[3]) : 0;
	}
};