#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
//  2D short-based vector -- an alternative to using LOWORD() / HIWORD()
// Author: Michael Lyashenko
//============================================================================================================

struct Vector2f;
struct Vector2i
{
	union
	{
		struct { short x, y; };
		uint mVal;
	};

	Vector2i (uint iValue = 0)		: mVal(iValue) {}
	Vector2i (const Vector2i& v)	: mVal(v.mVal) {}
	Vector2i (const Vector2f& v);
	Vector2i (short X, short Y)		: x(X), y(Y) {}
	void Set (short X, short Y)		{ x = X;  y = Y; }

				operator uint&()						{ return mVal;							}
				operator bool()					const	{ return mVal != 0;						}
	bool		operator == (uint val)			const	{ return mVal == val;					}
	bool		operator == (const Vector2i& v)	const	{ return mVal == v.mVal;				}
	bool		operator == (int val)			const	{ return mVal == (uint)val;				}
	bool		operator != (uint val)			const	{ return mVal != val;					}
	bool		operator != (const Vector2i& v) const	{ return mVal != v.mVal;				}
	bool		operator != (int val)			const	{ return mVal != (uint)val;				}
	Vector2i&	operator  = (uint val)					{ mVal  = val;		return *this;		}
	Vector2i&	operator  = (const Vector2i& v)			{ mVal  = v.mVal;	return *this;		}
	Vector2i&	operator  = (const Vector2f& v);
	Vector2i	operator  + (const Vector2i& v) const	{ return Vector2i(x + v.x, y + v.y);	}
	Vector2i	operator  - (const Vector2i& v) const	{ return Vector2i(x - v.x, y - v.y);	}
	Vector2i	operator  + (int val)			const	{ return Vector2i(x + val, y + val);	}
	Vector2i	operator  - (int val)			const	{ return Vector2i(x - val, y - val);	}
	Vector2i	operator  * (const Vector2i& v) const	{ return Vector2i(x * v.x, y * v.y);	}
	Vector2i	operator  / (const Vector2i& v) const	{ return Vector2i(x / v.x, y / v.y);	}
	Vector2i	operator  * (int val)			const	{ return Vector2i(x * val, y * val);	}
	Vector2i	operator  / (int val)			const	{ return Vector2i(x / val, y / val);	}
	Vector2f	operator  * (float val)			const;
	void		operator += (const Vector2i& v)			{ x += v.x;  y += v.y;					}
	void		operator -= (const Vector2i& v)			{ x -= v.x;  y -= v.y;					}

	void		operator /= (int val)					{ x /= val;  y /= val;					}
	void		operator /= (uint val)					{ x /= val;  y /= val;					}
	void		operator /= (short val)					{ x /= val;  y /= val;					}
	void		operator /= (ushort val)				{ x /= val;  y /= val;					}

	void		operator *= (int val)					{ x *= val;  y *= val;					}
	void		operator *= (uint val)					{ x *= val;  y *= val;					}
	void		operator *= (short val)					{ x *= val;  y *= val;					}
	void		operator *= (ushort val)				{ x *= val;  y *= val;					}
};
