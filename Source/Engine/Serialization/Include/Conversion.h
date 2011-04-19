#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// To-String conversion
// Author: Michael Lyashenko
//============================================================================================================

#define APPEND_STREAM(type) inline String&	operator << (String& str, const type& v)
#define SET_STREAM(type)	inline String&	operator >> (const type& v, String& str)

APPEND_STREAM(Vector2i)		{ return str.Append("%d %d", v.x, v.y);	}
APPEND_STREAM(Vector2f)		{ str << v.x; str << " ";
							  str << v.y; return str; }
APPEND_STREAM(Vector3f)		{ str << v.x; str << " ";
							  str << v.y; str << " ";
							  str << v.z; return str; }
APPEND_STREAM(Color3f)		{ str << v.r; str << " ";
							  str << v.g; str << " ";
							  str << v.b; return str; }
APPEND_STREAM(Color4f)		{ str << v.r; str << " ";
							  str << v.g; str << " ";
							  str << v.b; str << " ";
							  str << v.a; return str; }
APPEND_STREAM(Quaternion)	{ str << v.x; str << " ";
							  str << v.y; str << " ";
							  str << v.z; str << " ";
							  str << v.w; return str; }

SET_STREAM(Vector2i)		{ return str.Set("%d %d", v.x, v.y); }
SET_STREAM(Vector2f)		{ str.Clear(); str << v; return str; }
SET_STREAM(Vector3f)		{ str.Clear(); str << v; return str; }
SET_STREAM(Color3f)			{ str.Clear(); str << v; return str; }
SET_STREAM(Color4f)			{ str.Clear(); str << v; return str; }
SET_STREAM(Quaternion)		{ str.Clear(); str << v; return str; }

String&	operator << (String& str, const Color4ub& v);
String&	operator >> (const Color4ub& v, String& str);

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