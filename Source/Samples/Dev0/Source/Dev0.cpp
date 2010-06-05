//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Dev0 is a temporary testing application. Its source code and purpose change frequently.
//============================================================================================================

#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================

struct Vertex
{
	Vector3f pos;
	Vector3f normal;
	Vector2f texCoord;

	bool Matches (const Vector3f& p, const Vector3f& n, const Vector2f& tc)
	{
		return (p == pos && n == normal && tc == texCoord);
	}

	void Set (const Vector3f& p, const Vector3f& n, const Vector2f& tc)
	{
		pos = p;
		normal = n;
		texCoord = tc;
	}
};

//============================================================================================================

struct Index
{
	uint pi;
	uint ni;
	uint ti;

	Index() : pi(0), ni(0), ti(0) {}
};

//============================================================================================================

ushort Add (Array<Vertex>& verts, const Vector3f& p, const Vector3f& n, const Vector2f& tc)
{
	FOREACH(i, verts)
	{
		if (verts[i].Matches(p, n, tc))
		{
			return (ushort)i;
		}
	}
	verts.Expand().Set(p, n, tc);
	return (ushort)(verts.GetSize() - 1);
}

//============================================================================================================

void Finish (Array<Vertex>& verts, Array<ushort>& idx, bool save)
{
	static uint counter (0);
	static TreeNode root ("Root");
	static TreeNode& core = root.AddChild("Core");

	TreeNode& mesh = core.AddChild("Mesh", String("Weapon %u", counter++));

	Array<Vector3f>& vout = mesh.AddChild("Vertices").mValue.ToVector3fArray();
	Array<Vector3f>& nout = mesh.AddChild("Normals").mValue.ToVector3fArray();
	Array<Vector2f>& tout = mesh.AddChild("TexCoords 0").mValue.ToVector2fArray();

	mesh.AddChild("Triangles", idx);

	FOREACH(i, verts)
	{
		Vertex& v = verts[i];
		vout.Expand() = v.pos;
		nout.Expand() = v.normal;
		tout.Expand() = v.texCoord;
	}

	verts.Clear();
	idx.Clear();

	if (save) root.Save("weapons.r5a");
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
	System::SetCurrentPath("../../../Resources/");

	String pack;

	if (pack.Load("c:/temp/weaponpack/weapons.obj"))
	{
		uint offset(0), stage(0);
		String line, word;

		// Raw data
		Array<Vector3f> rawV, rawN;
		Array<Vector2f> rawTC;
		Array<Index> rawI;

		// Final data
		Array<Vertex> verts;
		Array<ushort> idx;
		
		for (;;)
		{
			offset = pack.GetLine(line, offset);

			if (line.IsEmpty())
			{
				if (stage == 1)
				{
					stage = 0;
					Finish(verts, idx, true);
				}
				break;
			}

			if (line[0] == 'v')
			{
				if (stage == 1)
				{
					stage = 0;
					Finish(verts, idx, false);
				}

				if (line[1] == ' ')
				{
					Vector3f& v = rawV.Expand();

					if (sscanf(line.GetBuffer(), "v %f %f %f", &v.x, &v.y, &v.z) != 3)
					{
						printf("Parsing error\n");
					}
				}
				else if (line[1] == 't')
				{
					Vector2f& v = rawTC.Expand();

					if (sscanf(line.GetBuffer(), "vt %f %f", &v.x, &v.y) != 2)
					{
						printf("Parsing error\n");
					}
				}
				else if (line[1] == 'n')
				{
					Vector3f& v = rawN.Expand();

					if (sscanf(line.GetBuffer(), "vn %f %f %f", &v.x, &v.y, &v.z) != 3)
					{
						printf("Parsing error\n");
					}
				}
			}
			else if (line[0] == 'f' && line[1] == ' ')
			{
				stage = 1;
				rawI.Clear();

				for (uint wordOffset = 2;;)
				{
					wordOffset = line.GetWord(word, wordOffset);
					if (word.IsEmpty()) break;

					Index& index = rawI.Expand();

					if (sscanf(word.GetBuffer(), "%u/%u/%u", &index.pi, &index.ti, &index.ni) != 3)
					{
						printf("Parsing error\n");
					}
					else
					{
						// OBJ's index is 1-based
						--index.pi;
						--index.ti;
						--index.ni;
					}
				}

				if (rawI.GetSize() == 3 || rawI.GetSize() == 4)
				{
					idx.Expand() = Add(verts, rawV[rawI[0].pi], rawN[rawI[0].ni], rawTC[rawI[0].ti]);
					idx.Expand() = Add(verts, rawV[rawI[1].pi], rawN[rawI[1].ni], rawTC[rawI[1].ti]);
					idx.Expand() = Add(verts, rawV[rawI[2].pi], rawN[rawI[2].ni], rawTC[rawI[2].ti]);
				
					if (rawI.GetSize() == 4)
					{
						idx.Expand() = Add(verts, rawV[rawI[2].pi], rawN[rawI[2].ni], rawTC[rawI[2].ti]);
						idx.Expand() = Add(verts, rawV[rawI[3].pi], rawN[rawI[3].ni], rawTC[rawI[3].ti]);
						idx.Expand() = Add(verts, rawV[rawI[0].pi], rawN[rawI[0].ni], rawTC[rawI[0].ti]);
					}
				}
				else
				{
					printf("Unexpected number of vertex entries\n");
				}
			}
		}
	}
	printf("Done!\n");
	getchar();
	return 0;
}