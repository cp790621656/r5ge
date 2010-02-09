#include "../../../Engine/Serialization/Include/_All.h"
using namespace R5;

//============================================================================================================
// Note to self: Reading filenames and directories:
// http://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1046380353&id=1044780608
//============================================================================================================

TreeNode* FindNode (TreeNode& root, const String& tag)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		TreeNode& node = root.mChildren[i];
		if (node.mTag == tag) return &node;
	}

	if (root.mChildren.IsValid())
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			TreeNode& node = root.mChildren[i];
			TreeNode* child = FindNode(node, tag);
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

void Process (const TreeNode& root)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];

		if (node.mTag == "Mesh")
		{
			String name ( node.mValue.IsString() ? node.mValue.AsString() : node.mValue.GetString() );

			TreeNode outRoot ("Root");
			AddMaterial( outRoot.AddChild("Graphics") );
			TreeNode& mesh = outRoot.AddChild("Core").AddChild("Mesh", name);

			for (uint b = 0; b < node.mChildren.GetSize(); ++b)
			{
				const TreeNode& child = node.mChildren[b];

				if (child.mTag == "Vertices")
				{
					// Copy vertex information as-is
					mesh.AddChild("Vertices", child.mValue);
				}
				else if (child.mTag == "TexCoords 0")
				{
					// Convert texture coordinates into a lightmap
					const Array<Vector2f>& texCoord = child.mValue.AsVector2fArray();
					Array<Color4ub>& colors = mesh.AddChild("Colors").mValue.ToColor4ubArray();

					for (uint c = 0; c < texCoord.GetSize(); ++c)
					{
						byte brightness = Float::ToRangeByte(texCoord[c].x);
						colors.Expand() = Color4ub(brightness, brightness, brightness, 255);
					}
				}
				else if (child.mTag == "Triangles")
				{
					mesh.AddChild("Triangles", child.mValue);
				}
			}

			// Add the template section
			TreeNode& temp = outRoot.AddChild("Template");
			temp.AddChild("Serializable" , false);
			TreeNode& limb = temp.AddChild("Limb", name);
			limb.AddChild("Mesh", name);
			limb.AddChild("Material", "Terrain");

			// Save the model
			name << ".r5a";

			if (outRoot.Save(name.GetBuffer(), false))
			{
				printf("...saved: '%s'\n", name.GetBuffer());
			}
			else
			{
				printf("Failed to save '%s'!\n", name.GetBuffer());
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
	if (argc > 1)
	{
		String source;

		for (int i = 1; i < argc; ++i)
		{
			TreeNode tree;

			if (tree.Load(argv[i]))
			{
				printf("Loaded '%s'...\n", argv[i]);
				TreeNode* core = FindNode(tree, "Core");
				
				if (core != 0)
				{
					Process(*core);
					printf("Processed '%s'\n", argv[i]);
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