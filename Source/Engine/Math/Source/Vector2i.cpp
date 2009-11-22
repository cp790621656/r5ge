#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Vector2i::Vector2i(const Vector2f& v)
{
	x = (short)Float::RoundToInt(v.x);
	y = (short)Float::RoundToInt(v.y);
}

//============================================================================================================

Vector2i& Vector2i::operator =(const Vector2f& v)
{
	x = (short)Float::RoundToInt(v.x);
	y = (short)Float::RoundToInt(v.y);
	return *this;
}