#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Geometric shapes generated using mathematical algorithms
// Author: Michael Lyashenko
//============================================================================================================

namespace Shape
{
	void Icosahedron (Array<Vector3f>& vertices, Array<ushort>& indices, uint iterations = 1);
	void Box (Array<Vector3f>& vertices, Array<ushort>& indices, float size = 1.0f, bool inverted = false);
};