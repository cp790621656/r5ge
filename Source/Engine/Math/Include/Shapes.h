#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
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