#include "../Include/_All.h"

namespace R5
{

//============================================================================================================
// Helper function that inverts an integer. It's used in order to store the color in text format properly.
// RRGGBBAA color is stored as AABBGGRR by default when written using printf(%x).
//============================================================================================================

inline uint InvertColor (uint val)
{
	return	((val >> 24) & 0x000000FF) |
			((val >>  8) & 0x0000FF00) |
			((val <<  8) & 0x00FF0000) |
			((val << 24) & 0xFF000000);
}

//============================================================================================================
// 'sprintf' leaves spaces behind. For hex numbers to load properly they must be replaced with zeroes.
//============================================================================================================

inline void CleanupHexString(String& s)
{
	char* buffer = s.GetBuffer();
	
	for (uint i = 0, imax = s.GetLength(); i < imax; ++i)
	{
		if (buffer[i] == ' ')
		{
			buffer[i] = '0';
		}
	}
}

//============================================================================================================

String&	operator << (String& str, const Color4ub& v)
{
	String color ("0x%2X%2X%2X%2X", v.r, v.g, v.b, v.a);
	CleanupHexString(color);
	return str << color;
}

//============================================================================================================

String&	operator >> (const Color4ub& v, String& str)
{
	str.Set("0x%2X%2X%2X%2X", v.r, v.g, v.b, v.a);
	CleanupHexString(str);
	return str;
}

//============================================================================================================

String&	operator << (String& str, const Matrix43& m)
{
	return str.Append("%f %f %f %f %f %f %f %f %f %f %f %f",
		m[0], m[4], m[8],  m[12],
		m[1], m[5], m[9],  m[13],
		m[2], m[6], m[10], m[14]);
}

//============================================================================================================

String&	operator << (String& str, const Matrix44& m)
{
	return str.Append("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		m[0], m[4], m[8],  m[12],
		m[1], m[5], m[9],  m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]);
}

//============================================================================================================

String&	operator >> (const Matrix43& m, String& str)
{
	return str.Set("%f %f %f %f %f %f %f %f %f %f %f %f",
		m[0], m[4], m[8],  m[12],
		m[1], m[5], m[9],  m[13],
		m[2], m[6], m[10], m[14]);
}

//============================================================================================================

String&	operator >> (const Matrix44& m, String& str)
{
	return str.Set("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		m[0], m[4], m[8],  m[12],
		m[1], m[5], m[9],  m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]);
}

//============================================================================================================

bool operator >> (const String& s, Vector2f& v)
{
	return s.IsValid() && (sscanf(s, "%f %f", &v.x, &v.y) == 2);
}

//============================================================================================================

bool operator >> (const String& s, Vector3f& v)
{
	return s.IsValid() && (sscanf(s, "%f %f %f", &v.x, &v.y, &v.z) == 3);
}

//============================================================================================================

bool operator >> (const String& s, Color3f& c)
{
	if (s.IsValid())
	{
		if (sscanf(s, "%f %f %f", &c.r, &c.g, &c.b) == 3)
		{
			return true;
		}
		else
		{
			uint val (0);

			// RRGGBB hex format
			if (sscanf(s, "0x%x", &val) == 1)
			{
				c.r = (float)((val >> 16) & 0xFF) / 255.0f;
				c.g = (float)((val >> 8)  & 0xFF) / 255.0f;
				c.b = (float)((val)       & 0xFF) / 255.0f;
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================

bool operator >> (const String& s, Color4f& c)
{
	if (s.IsValid())
	{
		int count = sscanf(s, "%f %f %f %f", &c.r, &c.g, &c.b, &c.a);

		if (count == 4)
		{
			return true;
		}
		else if (count == 3)
		{
			c.a = 1.0f;
			return true;
		}
		else
		{
			uint val (0);

			// RRGGBBAA hex format
			if (sscanf(s, "0x%x", &val) == 1)
			{
				c = Color4ub(InvertColor(val));
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================

bool operator >> (const String& s, Color4ub& c)
{
	if (s.IsValid())
	{
		Color4f cf (1.0f);

		if (sscanf(s, "%f %f %f %f", &cf.r, &cf.g, &cf.b, &cf.a) > 2)
		{
			c = cf;
			return true;
		}
		else
		{
			uint val;

			if (sscanf(s, "0x%x", &val) == 1)
			{
				c.mVal = InvertColor(val);
				return true;
			}
		}
	}
	return false;
}

//============================================================================================================

bool operator >> (const String& s, Quaternion& q)
{
	if (s.IsValid())
	{
		if (sscanf(s, "%f %f %f %f", &q.x, &q.y, &q.z, &q.w) == 4)
		{
			return true;
		}

		Vector3f dir;

		if ( s >> dir )
		{
			float a = Float::Abs(dir.x);
			float b = Float::Abs(dir.y);
			float c = Float::Abs(dir.z);

			if (a + b + c > 3.0f)
			{
				dir.x = DEG2RAD(dir.x);
				dir.y = DEG2RAD(dir.y);
				dir.z = DEG2RAD(dir.z);
				q.SetFromEuler(dir);
			}
			else
			{
				q.SetFromDirection(dir);
			}
			return true;
		}
	}
	return false;
}

//============================================================================================================

bool operator >> (const String& s, Vector2i& v)
{
	int X, Y;
	if (s.IsValid() && sscanf(s, "%d %d", &X, &Y) == 2)
	{
		v.x = X;  v.y = Y;
		return true;
	}
	return false;
}

//============================================================================================================

bool operator >> (const String& s, Matrix43& m)
{
	return (s.IsValid() && (sscanf(s, "%f %f %f %f %f %f %f %f %f %f %f %f",
		&m[0], &m[4], &m[8],  &m[12],
		&m[1], &m[5], &m[9],  &m[13],
		&m[2], &m[6], &m[10], &m[14]) == 12));
}

//============================================================================================================

bool operator >> (const String& s, Matrix44& m)
{
	return (s.IsValid() && (sscanf(s, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		&m[0], &m[4], &m[8],  &m[12],
		&m[1], &m[5], &m[9],  &m[13],
		&m[2], &m[6], &m[10], &m[14],
		&m[3], &m[7], &m[11], &m[15]) == 16));
}
}; // namespace R5