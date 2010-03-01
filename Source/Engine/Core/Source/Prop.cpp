#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Returns the number of triangles used by the model
//============================================================================================================

uint Prop::GetNumberOfTriangles()
{
	uint triangleCount (0);

	Lock();
	{
		for (Limb** start = mLimbs.GetStart(), **end = mLimbs.GetEnd(); start != end; ++start)
		{
			Limb* limb (*start);

			if ( limb != 0 && limb->mMesh != 0 && limb->mMat != 0 )
			{
				triangleCount += limb->mMesh->GetNumberOfTriangles();
			}
		}
	}
	Unlock();
	return triangleCount;
}

//============================================================================================================
// Helper functions that allows checking whether the model
//============================================================================================================

bool Prop::IsUsingMaterial (const IMaterial* mat) const
{
	Lock();
	{
		for (uint i = 0; i < mLimbs.GetSize(); ++i)
		{
			const Limb* limb = mLimbs[i];

			if (limb != 0 && limb->mMat == mat)
			{
				Unlock();
				return true;
			}
		}
	}
	Unlock();
	return false;
}

//============================================================================================================
// Helper function that checks to see if the prop is using the specified texture
//============================================================================================================

bool Prop::IsUsingTexture (const ITexture* ptr) const
{
	Lock();
	{
		for (uint i = 0; i < mLimbs.GetSize(); ++i)
		{
			const Limb* limb = mLimbs[i];

			if (limb != 0 && limb->mMat != 0)
			{
				const IMaterial::DrawMethods& methods = limb->mMat->GetAllDrawMethods();

				methods.Lock();
				{
					for (uint b = 0; b < methods.GetSize(); ++b)
					{
						const IMaterial::Textures& textures = methods[b].GetAllTextures();

						for (uint c = 0; c < textures.GetSize(); ++c)
						{
							const ITexture* tex = textures[c];

							if (tex != 0 && tex == ptr)
							{
								methods.Unlock();
								Unlock();
								return true;
							}
						}
					}
				}
				methods.Unlock();
			}
		}
	}
	Unlock();
	return false;
}

//============================================================================================================
// Draw the object, called by the rendering queue this object was added to
//============================================================================================================

uint Prop::_Draw (uint group, IGraphics* graphics, const ITechnique* tech)
{
	uint mask = tech->GetMask();

	// Go through every limb and render it
	Lock();
	{
		for (Limb** start = mLimbs.GetStart(), **end = mLimbs.GetEnd(); start != end; ++start)
		{
			Limb* limb (*start);

			// Only draw limbs as long as they are visible with the current technique
			if (limb->IsVisibleWith(mask))
			{
				if (group == GetUID() || group == limb->GetMaterial()->GetUID())
				{
					if ( graphics->SetActiveMaterial(limb->mMat) )
					{
						if (limb->mMesh != 0)
						{
							limb->mMesh->Draw(graphics);
						}
						else if (limb->mCloud != 0)
						{
							limb->mCloud->Draw(graphics);
						}
					}
				}
			}
		}
	}
	Unlock();
	return 1;
}