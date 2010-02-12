#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================
// Certain points are used as "edge points", and their normals need to be adjusted to identical values
// in order to eliminate any and all visible seams.
//============================================================================================================

void FillAdjustmentArrays (const char* filename, Array<Vector3f>& vertices, Array<Vector3f>& normals)
{
	TreeNode root;
	
	if (!root.Load(filename))
	{
		printf("Adjustment file not found (%s)\n", filename);
	}
	else
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node = root.mChildren[i];

			if (node.mValue.IsVector3fArray())
			{
				if		(node.mTag == "Vertices")	vertices	= node.mValue.AsVector3fArray();
				else if (node.mTag == "Normals")	normals		= node.mValue.AsVector3fArray();
			}
		}

		if (vertices.IsValid() && vertices.GetSize() == normals.GetSize())
		{
			// Normalize all normals
			for (uint i = normals.GetSize(); i > 0; ) normals[--i].Normalize();

			// We should now run through all vertices in this group
			for (uint i = vertices.GetSize(); i > 0; )
			{
				Vector3f vertex (vertices[--i]);
				Vector3f normal (normals[i]);

				for (uint c = 0; c < 2; ++c)
				{
					// We want to take the specified vertex and add its
					// 90, 180, and 270 degree rotated versions
					for (uint b = 0; b < 3; ++b)
					{
						// Rotate the vertex 90 degrees
						vertex.Set(-vertex.y, vertex.x, vertex.z);
						normal.Set(-normal.y, normal.x, normal.z);

						// If this vertex doesn't exist yet, add it
						if (!vertices.Contains(vertex))
						{
							vertices.Expand() = vertex;
							normals.Expand()  = normal;
						}
					}

					// We now want to flip the X, mirroring this coordinate and repeat the process
					vertex	 = vertices[i];
					normal	 = normals[i];
					vertex.x = -vertex.x;
					normal.x = -normal.x;

					// Don't forget the current vertex
					if (!vertices.Contains(vertex))
					{
						vertices.Expand() = vertex;
						normals.Expand()  = normal;
					}
				}
			}
			return;
		}
		else
		{
			printf("Error: Invalid or mis-matched vertex/normal set in '%s'!\n", filename);
		}
	}
	vertices.Clear();
	normals.Clear();
}

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
// Adjust the specified vertex' normal, if the vertex is present within the specified list
//============================================================================================================

void AdjustNormal (Vector3f& vertex, Vector3f& normal, const Array<Vector3f>& vertices, const Array<Vector3f>& normals)
{
	for (uint i = vertices.GetSize(); i > 0; )
	{
		const Vector3f& match = vertices[--i];

		if ((vertex - match).Dot() < 0.00001f && normal.Dot(normals[i]) > 0.5f)
		{
			vertex = match;
			normal = normals[i];
		}
	}
}

//============================================================================================================
// Add a new unique vertex to the lists, provided it's not already present
//============================================================================================================

void AddUnique (Array<Vector3f>& vertices, Array<Vector3f>& normals, Array<Color4ub>& colors, Array<ushort>& indices,
				const Vector3f& vertex, const Vector3f& normal, const Color4ub& color)
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
// Save the specified TreeNode after parsing and processing its information
//============================================================================================================

void Save (const TreeNode& node, const String& name, uint pieces, const Array<Vector3f>& av, const Array<Vector3f>& an)
{
	const TreeNode* vNode = FindNode(node, "Vertices");
	const TreeNode* iNode = FindNode(node, "Triangles");
	const TreeNode* tNode = FindNode(node, "TexCoords 0");

	if (vNode == 0 || iNode == 0 || tNode == 0 || pieces == 0) return;

	const Array<Vector3f>&	vertices	= vNode->mValue.AsVector3fArray();
	const Array<Vector2f>&	texCoords	= tNode->mValue.AsVector2fArray();
	const Array<ushort>&	indices		= iNode->mValue.AsUShortArray();

	// We calculate piece indices and normals
	Array<uint>		pieceIndex;
	Array<Vector3f>	normals;

	// Calculate the bounds of this mesh
	Bounds bounds;
	for (uint i = vertices.GetSize(); i > 0; ) bounds.Include(vertices[--i]);

	// Calculate the size of each piece
	Vector3f size (bounds.GetMax() - bounds.GetMin());
	size /= (float)pieces;
	float offset = 0.0f;

	// index = ((v0.z + v1.z + v2.z) / 3.0f) / size.z
	// index * size.z = v / 3;
	// index * size.z * 3 = v
	// index = v / (size.z * 3)
	float heightMultiplier = 1.0f / (size.z * 3.0f);

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

		Vector3f v10 (v1 - v0);
		Vector3f v20 (v2 - v0);

		Vector3f normal (Cross(v10, v20));

		normals[i0] += normal;
		normals[i1] += normal;
		normals[i2] += normal;

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

				// Adjust by offset
				if (p > 0) vertex.z -= offset;

				// Adjust the normal and add a unique vertex
				AdjustNormal(vertex, normal, av, an);
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
		offset += size.z;
	}
}

//============================================================================================================

void Process (const TreeNode& root, const Array<Vector3f>& av, const Array<Vector3f>& an)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];

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
					Save(node, temp, count, av, an);
				}
			}
			else
			{
				Save(node, name, 1, av, an);
			}
		}
	}
}

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

		Array<Vector3f> vertices, normals;
		FillAdjustmentArrays("_Adjustments.txt", vertices, normals);

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
				const TreeNode* core = FindNode(tree, "Core");
				
				if (core != 0)
				{
					Process(*core, vertices, normals);
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