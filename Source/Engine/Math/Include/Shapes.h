#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Geometric shapes generated using mathematical algorithms
//============================================================================================================

namespace Shape
{
	void Icosahedron (Array<Vector3f>& vertices, Array<ushort>& indices, uint iterations = 1);
	void Box (Array<Vector3f>& vertices, Array<ushort>& indices, float size = 1.0f, bool inverted = false);
};