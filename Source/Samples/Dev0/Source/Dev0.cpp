//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================
// Note to self: Reading filenames and directories:
// http://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1046380353&id=1044780608
//============================================================================================================

const TreeNode* FindNode (const TreeNode& root, const String& tag)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];
		if (node.mTag == tag) return &node;
	}

	if (root.mChildren.IsValid())
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node = root.mChildren[i];
			const TreeNode* child = FindNode(node, tag);
			if (child != 0) return child;
		}
	}
	return 0;
}

//============================================================================================================
// Adds the material section to the TreeNode
//============================================================================================================

void AddMaterial (TreeNode& root)
{
	Array<String> textures;
	textures.Expand() = "Textures/Grass/green.jpg";
	textures.Expand() = "Textures/Grass/green_ns.png";
	textures.Expand() = "Textures/Stone/grey.jpg";
	textures.Expand() = "Textures/Stone/grey_ns.png";

	root.AddChild("Serializable", false);
	TreeNode& node = root.AddChild("Material", "Terrain");
	node.AddChild("Diffuse", Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	node.AddChild("Specular", Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	
	TreeNode& child = node.AddChild("Technique", "Deferred");
	child.AddChild("Shader", "Deferred/TiledTerrain");
	child.AddChild("Textures", textures);
}

//============================================================================================================
// Adjust the specified vertex' normal if it lies on the edge of the bounds
//============================================================================================================

Vector3f AdjustNormal (const Vector3f& min, const Vector3f& max, const Vector3f& vertex, Vector3f normal)
{
	float mag = normal.Magnitude();
	if (Float::Abs(vertex.x - min.x) < 0.0001f || Float::Abs(vertex.x - max.x) < 0.0001f) normal.x = 0.0f;
	if (Float::Abs(vertex.y - min.y) < 0.0001f || Float::Abs(vertex.y - max.y) < 0.0001f) normal.y = 0.0f;
	normal.Normalize();
	return normal * mag;
}

//============================================================================================================
// Add a new unique vertex to the lists, provided it's not already present
//============================================================================================================

void AddUnique (Array<Vector3f>&	vertices,
				Array<Vector3f>&	normals,
				Array<Color4ub>&	colors,
				Array<ushort>&		indices,
				const Vector3f&		vertex,
				const Vector3f&		normal,
				const Color4ub&		color)
{
	uint index = 0;

	for (;;)
	{
		// Find the matching vertex
		index = vertices.Find(vertex, index);

		// If the vertex was found, check its normal and colors
		if (index < vertices.GetSize())
		{
			// If the normal and colors match, use this vertex
			if (normals[index] == normal && colors[index] == color)
			{
				indices.Expand() = (ushort)index;
				return;
			}

			// Find the next matching vertex
			++index;
			continue;
		}

		// This is a new vertex -- add it to the lists
		indices.Expand()	= vertices.GetSize();
		vertices.Expand()	= vertex;
		normals.Expand()	= normal;
		colors.Expand()		= color;
		break;
	}
}

//============================================================================================================
// Helper function that returns a 'true' if the vertex lies too close to the X boundary
//============================================================================================================

inline bool IsOnXBorder (const Vector3f& v, const Vector3f& min, const Vector3f& max)
{
	return (Float::Abs(v.x - min.x) < 0.001f) || (Float::Abs(v.x - max.x) < 0.001f);
}

//============================================================================================================
// Helper function that returns a 'true' if the vertex lies too close to the Y boundary
//============================================================================================================

inline bool IsOnYBorder (const Vector3f& v, const Vector3f& min, const Vector3f& max)
{
	return (Float::Abs(v.y - min.y) < 0.001f) || (Float::Abs(v.y - max.y) < 0.001f);
}

//============================================================================================================
// In order to eliminate all visible seams, the normals need to be aligned on the edges of the bounds.
//============================================================================================================

Vector3f GetAdjustedNormal (const Vector3f& v0,
							const Vector3f& v1,
							const Vector3f& v2,
							const Vector3f& min,
							const Vector3f& max)
{
	Vector3f v21 (v2 - v1);
	Vector3f v01 (v0 - v1);

	// Default normal
	Vector3f normal (Cross(v01, v21));

	// Check to see if the vertex pies on the edge of the bounding box.
	// If it does, we should ignore the axis the boundary of which the vertex got too close to.
	bool ignoreX = IsOnXBorder(v1, min, max);
	bool ignoreY = IsOnYBorder(v1, min, max);

	if (ignoreX && ignoreY)
	{
		// If both X and Y lie on the edge, the normal should be pointing straight up
		normal.Set(0.0f, 0.0f, normal.Magnitude());
	}
	else if (ignoreX)
	{
		if (IsOnXBorder(v0, min, max))
		{
			// v1 and v0 both lie on the edge. Replace v2 with an X-axis vector
			v21.Set(1.0f, 0.0f, 0.0f);
			Vector3f newNormal (Cross(v01, v21));
			normal = (newNormal.Dot(normal) > 0.0f) ? newNormal : -newNormal;
		}
		else if (IsOnXBorder(v2, min, max))
		{
			// v1 and v2 lie on the edge
			v01.Set(1.0f, 0.0f, 0.0f);
			Vector3f newNormal (Cross(v01, v21));
			normal = (newNormal.Dot(normal) > 0.0f) ? newNormal : -newNormal;
		}
		else return Vector3f();
	}
	else if (ignoreY)
	{
		if (IsOnYBorder(v0, min, max))
		{
			// v1 and v0 both lie on the edge. Replace v2 with a Y-axis vector
			v21.Set(0.0f, 1.0f, 0.0f);
			Vector3f newNormal (Cross(v21, v01));
			normal = (newNormal.Dot(normal) > 0.0f) ? newNormal : -newNormal;
		}
		else if (IsOnYBorder(v2, min, max))
		{
			// v1 and v2 lie on the edge
			v01.Set(0.0f, 1.0f, 0.0f);
			Vector3f newNormal (Cross(v21, v01));
			normal = (newNormal.Dot(normal) > 0.0f) ? newNormal : -newNormal;
		}
		else return Vector3f();
	}
	// Taking the angle into account ensures that the normals come out equal even if one
	// side has 2 triangles connected to this vertex while the other one has 50.
	return normal * GetAngle(v21, v01);
}

//============================================================================================================
// Save the specified TreeNode after parsing and processing its information
//============================================================================================================

void Save (TreeNode& node, const String& name, uint pieces)
{
	TreeNode*		vNode = (TreeNode*)FindNode(node, "Vertices");
	const TreeNode* iNode = FindNode(node, "Triangles");
	const TreeNode* tNode = FindNode(node, "TexCoords 0");

	if (vNode == 0 || iNode == 0 || tNode == 0 || pieces == 0) return;

	Array<Vector3f>&		vertices	= vNode->mValue.ToVector3fArray();
	const Array<Vector2f>&	texCoords	= tNode->mValue.AsVector2fArray();
	const Array<ushort>&	indices		= iNode->mValue.AsUShortArray();

	// We calculate piece indices and normals
	Array<uint>		pieceIndex;
	Array<Vector3f>	normals;

	// Calculate the bounds of this mesh
	Bounds bounds;
	for (uint i = vertices.GetSize(); i > 0; ) bounds.Include(vertices[--i]);

	// Calculate the size of each piece
	Vector3f min (bounds.GetMin());
	Vector3f max (bounds.GetMax());
	Vector3f size (max - min);

	// For horizontal offset, we want to the piece's center to be the origin point
	Vector3f offset (min + size * 0.5f);

	// For the vertical offset, we always want our piece to begin at 0
	offset.z = min.z;

	// Run through all vertices and make them centered at the origin
	if (Float::Abs(offset.x) + Float::Abs(offset.y) + Float::Abs(offset.z) > 0.0001f)
	{
		for (uint i = vertices.GetSize(); i > 0; ) vertices[--i] -= offset;
		min -= offset;
		max -= offset;
	}

	// The reason behind the numerical operations below is as follows:
	// index = ((v0.z + v1.z + v2.z) / 3.0f) / size.z
	// index * size.z = v / 3;
	// index * size.z * 3 = v
	// index = v / (size.z * 3)

	size /= (float)pieces;
	float heightMultiplier = 1.0f / (size.z * 3.0f);
	offset.z = 0.0f;

	// Reserve the normals
	normals.ExpandTo(vertices.GetSize());

	// Calculate the normals
	for (uint i = 0; i + 2 < indices.GetSize(); )
	{
		uint i0 = indices[i++];
		uint i1 = indices[i++];
		uint i2 = indices[i++];

		const Vector3f& v0 ( vertices[i0] );
		const Vector3f& v1 ( vertices[i1] );
		const Vector3f& v2 ( vertices[i2] );

		normals[i0] += GetAdjustedNormal(v1, v0, v2, min, max);
		normals[i1] += GetAdjustedNormal(v2, v1, v0, min, max);
		normals[i2] += GetAdjustedNormal(v0, v2, v1, min, max);

		if (pieces > 1)
		{
			// Calculate which section the piece belongs to
			float height = (v0.z + v1.z + v2.z);
			height *= heightMultiplier;

			// Don't allow the height to drop below zero
			if (height < 0.0f) height = 0.0f;

			uint index = Float::FloorToUInt(height);
			if (index >= pieces) index = pieces - 1;

			pieceIndex.Expand() = index;
			pieceIndex.Expand() = index;
			pieceIndex.Expand() = index;

			// If the height is on the threshold, the point should be added to both sides
			if (Float::Abs(height) < 0.0001f && height > 1.0f)
			{
				index = Float::FloorToUInt(height + 1.0f);
				pieceIndex.Expand() = index;
				pieceIndex.Expand() = index;
				pieceIndex.Expand() = index;
			}
		}
	}

	// Normalize all normals
	for (uint i = normals.GetSize(); i > 0; ) normals[--i].Normalize();

	// Separate each piece into a new model file
	for (uint p = 0; p < pieces; ++p)
	{
		String filename (name);

		if (pieces > 3)
		{
			filename << " ";
			filename << pieces;
		}
		else if (pieces == 3)
		{
			if		(p == 0) filename << " - Bottom";
			else if (p == 1) filename << " - Middle";
			else if (p == 2) filename << " - Top";
		}
		else if (pieces == 2)
		{
			if		(p == 0) filename << " - Bottom";
			else if (p == 1) filename << " - Top";
		}

		// R5 model file structure
		TreeNode root ("Root");
		AddMaterial( root.AddChild("Graphics") );
		TreeNode& mesh = root.AddChild("Core").AddChild("Mesh", filename);

		Array<Vector3f>&	outV	= mesh.AddChild("Vertices").mValue.ToVector3fArray();
		Array<Vector3f>&	outN	= mesh.AddChild("Normals").mValue.ToVector3fArray();
		Array<Color4ub>&	outC	= mesh.AddChild("Colors").mValue.ToColor4ubArray();
		Array<ushort>&		outI	= mesh.AddChild("Triangles").mValue.ToUShortArray();

		// Run through all triangles and save those that lie below the 'size' threshold
		for (uint i = 0, imax = indices.GetSize(); i < imax; ++i)
		{
			if (pieces < 2 || pieceIndex[i] == p)
			{
				uint index = indices[i];
				Vector3f vertex ( vertices[index] );
				Vector3f normal ( normals[index] );
				byte color = Float::ToRangeByte(texCoords[index].x);

				// Adjust by verticalOffset
				if (p > 0) vertex.z -= offset.z;

				// Add this vertex if it's unique
				AddUnique(outV, outN, outC, outI, vertex, normal, Color4ub(color, color, color, 255));
			}
		}

		// Add the template section
		TreeNode& temp = root.AddChild("Template");
		temp.AddChild("Serializable" , false);
		TreeNode& limb = temp.AddChild("Limb", filename);
		limb.AddChild("Mesh", filename);
		limb.AddChild("Material", "Terrain");

		// Append the extension
		filename << ".r5a";

		// Save the file in ASCII format
		if (root.Save(filename.GetBuffer(), false))
		{
			printf("...saved: '%s'\n", filename.GetBuffer());
		}
		else
		{
			printf("Failed to save '%s'!\n", filename.GetBuffer());
		}

		// Next piece starts where the previous one left off
		offset.z += size.z;
	}
}

//============================================================================================================
// Process the specified TreeNode containing model information with multiple meshes
//============================================================================================================

void Process (TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		TreeNode& node = root.mChildren[i];

		if (node.mTag == "Mesh")
		{
			String name ( node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString() );

			uint index = name.Find(" - Split");

			if (index < name.GetLength())
			{
				String temp;
				uint count = 0;
				name.GetWord(temp, index + 9);

				if (temp >> count)
				{
					name.GetString(temp, 0, index);
					Save(node, temp, count);
				}
			}
			else
			{
				Save(node, name, 1);
			}
		}
	}
}

//============================================================================================================
// Application entry point
//============================================================================================================

int main (int argc, char* argv[])
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif

#ifndef _DEBUG
	if (argc > 1)
#else
	System::SetCurrentPath("../../../../DotC/Resources/Models/Terrain/");
#endif
	{
		String source;

#ifndef _DEBUG
		for (int i = 1; i < argc; ++i)
#endif
		{
			TreeNode tree;

#ifndef _DEBUG
			if (tree.Load(argv[i]))
			{
				printf("Loaded '%s'...\n", argv[i]);
#else
			if (tree.Load("terrain.r5a"))
			{
#endif
				TreeNode* core = (TreeNode*)FindNode(tree, "Core");
				
				if (core != 0)
				{
					Process(*core);
#ifndef _DEBUG
					printf("Processed '%s'\n", argv[i]);
#endif
				}
				else
				{
					printf("Core section was not found!\n");
				}
			}
		}
	}
	getchar();
	return 0;
}