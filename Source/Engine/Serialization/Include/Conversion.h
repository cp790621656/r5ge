#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// To-String conversion
//============================================================================================================

#define APPEND_STREAM(type) inline String&	operator << (String& str, const type& v)
#define SET_STREAM(type)	inline String&	operator >> (const type& v, String& str)

APPEND_STREAM(Vector2i)		{ return str.Append("%d %d", v.x, v.y);	}
APPEND_STREAM(Vector2f)		{ return str.Append("%f %f", v.x, v.y);	}
APPEND_STREAM(Vector3f)		{ return str.Append("%f %f %f", v.x, v.y, v.z);	}
APPEND_STREAM(Color3f)		{ return str.Append("%.3f %.3f %.3f", v.r, v.g, v.b); }
APPEND_STREAM(Color4f)		{ return str.Append("%.3f %.3f %.3f %.3f", v.r, v.g, v.b, v.a); }
APPEND_STREAM(Quaternion)	{ return str.Append("%f %f %f %f", v.x, v.y, v.z, v.w); }

SET_STREAM(Vector2i)		{ return str.Set("%d %d", v.x, v.y); }
SET_STREAM(Vector2f)		{ return str.Set("%f %f", v.x, v.y); }
SET_STREAM(Vector3f)		{ return str.Set("%f %f %f", v.x, v.y, v.z); }
SET_STREAM(Color3f)			{ return str.Set("%.3f %.3f %.3f", v.r, v.g, v.b); }
SET_STREAM(Color4f)			{ return str.Set("%.3f %.3f %.3f %.3f", v.r, v.g, v.b, v.a); }
SET_STREAM(Quaternion)		{ return str.Set("%f %f %f %f", v.x, v.y, v.z, v.w); }

String&	operator << (String& str, const Color4ub& v);
String&	operator << (String& str, const Color3f& v);
String&	operator << (String& str, const Color4f& v);

String&	operator >> (const Color4ub& v, String& str);
String&	operator >> (const Color3f& v, String& str);
String&	operator >> (const Color4f& v, String& str);

String&	operator << (String& str, const Matrix43& m);
String&	operator << (String& str, const Matrix44& m);

String&	operator >> (const Matrix43& m, String& str);
String&	operator >> (const Matrix44& m, String& str);

//============================================================================================================
// From-String conversion
//============================================================================================================

bool operator >> (const String& s, Vector2f& v);
bool operator >> (const String& s, Vector3f& v);
bool operator >> (const String& s, Color3f& c);
bool operator >> (const String& s, Color4f& c);
bool operator >> (const String& s, Quaternion& q);
bool operator >> (const String& s, Vector2i& v);
bool operator >> (const String& s, Color4ub& c);
bool operator >> (const String& s, Matrix43& m);
bool operator >> (const String& s, Matrix44& m);