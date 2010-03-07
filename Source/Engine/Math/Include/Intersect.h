#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

namespace Intersect
{
	bool RaySphere (const Vector3f& origin, const Vector3f& dir, const Vector3f& center, float radius);
	bool RayBounds (const Vector3f& origin, const Vector3f& dir, const Bounds& bounds);
};