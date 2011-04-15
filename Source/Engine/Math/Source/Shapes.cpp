#include "../Include/_All.h"
using namespace R5;

namespace Icosa
{
//============================================================================================================
// Icosahedron's edge information -- index of the two vertices plus the midpoint vertex
//============================================================================================================

struct Edge
{
	uint	i0;
	uint	i1;
	uint	mp;

	Edge() : i0(-1), i1(-1), mp(-1) {}
};

//============================================================================================================
// List of edges gets some additional functionality
//============================================================================================================

class EdgeList
{
	Array<Edge>	mList;

public:

	void Add (uint i0, uint i1, uint& edge, bool& flipped)
	{
		if ((flipped = (i0 > i1)))
			Swap(i0, i1);

		for (uint i = 0; i < mList.GetSize(); ++i)
		{
			if (mList[i].i0 == i0 && mList[i].i1 == i1)
			{
				edge = i;
				return;
			}
		}

		edge = mList.GetSize();
		Edge& entry (mList.Expand());
		entry.i0 = i0;
		entry.i1 = i1;
	}

	uint GetIndex (uint edge, bool flip) const
	{
		if (edge < mList.GetSize())
		{
			return flip ? mList[edge].i0 : mList[edge].i1;
		}
		ASSERT(false, "Invalid edge");
		return 0;
	}

	ushort GetMidpoint (uint edge, Array<Vector3d>& vertices)
	{
		ASSERT(edge < mList.GetSize(), "Invalid edge");
		uint& mp (mList[edge].mp);
		
		if (mp == INVALID_VAL)
		{
			uint i0 (mList[edge].i0);
			uint i1 (mList[edge].i1);

			ASSERT(i0 < vertices.GetSize() && i1 < vertices.GetSize(), "Index out of bounds!");

			mp = vertices.GetSize();
			Vector3d& v = vertices.Expand();
			v = (vertices[i0] + vertices[i1]) * 0.5;
			v.Normalize();
		}
		return mp;
	}
};

//============================================================================================================
// Icosahedron's triangle information
//============================================================================================================

class Triangle
{
	uint	mEdge0;
	uint	mEdge1;
	uint	mEdge2;
	bool	mFlip0;
	bool	mFlip1;
	bool	mFlip2;

public:

	Triangle() :	mEdge0(-1),
					mEdge1(-1),
					mEdge2(-1),
					mFlip0(false),
					mFlip1(false),
					mFlip2(false) {}

	void Set (EdgeList& edges, ushort i0, ushort i1, ushort i2)
	{
		edges.Add(i0, i1, mEdge0, mFlip0);
		edges.Add(i1, i2, mEdge1, mFlip1);
		edges.Add(i2, i0, mEdge2, mFlip2);
	}

	void Fill (const EdgeList& edges, Array<ushort>& indices) const
	{
		indices.Expand() = (ushort)edges.GetIndex(mEdge0, mFlip0);
		indices.Expand() = (ushort)edges.GetIndex(mEdge1, mFlip1);
		indices.Expand() = (ushort)edges.GetIndex(mEdge2, mFlip2);
	}

	void Subdivide (Array<Triangle>& triangles, EdgeList& edges, Array<Vector3d>& vertices)
	{
		uint i0 = edges.GetIndex(mEdge0, mFlip0);
		uint i1 = edges.GetIndex(mEdge1, mFlip1);
		uint i2 = edges.GetIndex(mEdge2, mFlip2);

		uint m0 = edges.GetMidpoint(mEdge0, vertices);
		uint m1 = edges.GetMidpoint(mEdge1, vertices);
		uint m2 = edges.GetMidpoint(mEdge2, vertices);

		// Change this triangle to different indices
		Set(edges, m0, m1, m2);
		
		// Add the 3 new triangles
		triangles.Expand().Set(edges, m0, i0, m1);
		triangles.Expand().Set(edges, m1, i1, m2);
		triangles.Expand().Set(edges, m2, i2, m0);
	}
};
}; // namespace Icosa

//============================================================================================================
// Main function that generates the icosahedron
//============================================================================================================

void Shape::Icosahedron (Array<Vector3f>& vertices, Array<ushort>& indices, uint iterations)
{
	double pi = 3.141592653589793238462643383279;
	double horizontal = (pi / 5.0);	// 5 horizontal slices per 180 degrees
	double vertical = pi / 3.0;		// 3 vertical slices per 180 degrees
	double f = sin(vertical);		// Sine of 60 degrees equals to 0.86 with change
	double z = cos(vertical);		// Cosine is 0.5 (leaving the formulas here for reference)

	// Initial set of points that make up the icosahedron
	Array<Vector3d> points;
	points.Expand().Set(0.0, 0.0, 1.0);
	for (uint i = 0; i < 10; ++i)
		points.Expand().Set(sin(horizontal*i) * f, cos(horizontal*i) * f, (i % 2) ? -z : z);
	points.Expand().Set(0.0, 0.0, -1.0);

	// Initial set of indices is hard-coded based on the created points above
	ushort list[60] =
	{
		1,  0, 3,   3,  0, 5,   5,  0, 7,    7,  0, 9,    9,  0,  1, // Top side
		3,  2, 1,   2,  3, 4,   5,  4, 3,    4,  5, 6,    7,  6,  5, // Central ring
		6,  7, 8,   9,  8, 7,   8,  9, 10,   1, 10, 9,   10,  1,  2,
		4, 11, 2,   6, 11, 4,   8, 11, 6,   10, 11, 8,    2, 11, 10, // Bottom side
	};

	Icosa::EdgeList edges;				// List of all shared edges
	Array<Icosa::Triangle> triangles;	// Array of unique triangles

	// Create the initial set of triangles, also filling the edges
	for (uint i = 0; i < 60; i+=3)
		triangles.Expand().Set(edges, list[i], list[i+1], list[i+2]);

	// For every iteration, split each triangle into 4
	for (uint iteration = 0; iteration < iterations; ++iteration)
	{
		// Subdivide every single triangle (NOTE: important to iterate only up to the old size!)
		for (uint i = 0, size = triangles.GetSize(); i < size; ++i)
			triangles[i].Subdivide(triangles, edges, points);
	}

	// Fill the vertices
	vertices.Clear();
	for (uint i = 0; i < points.GetSize(); ++i)
	{
		Vector3d& vd = points[i];
		Vector3f& vf = vertices.Expand();
		vf.Set((float)vd.x, (float)vd.y, (float)vd.z);
	}

	// Create the final set of indices
	indices.Clear();
	for (uint i = 0; i < triangles.GetSize(); ++i)
		triangles[i].Fill(edges, indices);
};

//============================================================================================================
// Generates a box
//============================================================================================================

void Shape::Box (Array<Vector3f>& vertices, Array<ushort>& indices, float size, bool inverted)
{
	vertices.Clear();
	vertices.Expand().Set( 1.0f,  1.0f,  1.0f);
	vertices.Expand().Set( 1.0f,  1.0f, -1.0f);
	vertices.Expand().Set( 1.0f, -1.0f,  1.0f);
	vertices.Expand().Set( 1.0f, -1.0f, -1.0f);
	vertices.Expand().Set(-1.0f,  1.0f,  1.0f);
	vertices.Expand().Set(-1.0f,  1.0f, -1.0f);
	vertices.Expand().Set(-1.0f, -1.0f,  1.0f);
	vertices.Expand().Set(-1.0f, -1.0f, -1.0f);

	if (size != 1.0f)
	{
		for (uint i = vertices.GetSize(); i > 0; ) vertices[--i] *= size;
	}

	if (inverted)
	{
		ushort idx[] = 
		{
			0, 1, 3, 0, 3, 2, 4, 7, 5, 4, 6, 7, 0, 2, 6, 0, 6, 4,
			1, 7, 3, 1, 5, 7, 0, 5, 1, 0, 4, 5, 2, 3, 7, 2, 7, 6
		};
		indices.Reserve(36);
		for (uint i = 0; i < 36; ++i) indices.Expand() = idx[i];
	}
	else
	{
		ushort idx[] = 
		{
			1, 0, 3, 3, 0, 2, 7, 4, 5, 6, 4, 7, 2, 0, 6, 6, 0, 4,
			7, 1, 3, 5, 1, 7, 5, 0, 1, 4, 0, 5, 3, 2, 7, 7, 2, 6
		};
		indices.Reserve(36);
		for (uint i = 0; i < 36; ++i) indices.Expand() = idx[i];
	}
}
