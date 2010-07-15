#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Retrieves the texture associated with the widget
//============================================================================================================

const ITexture* UISubPicture::GetTexture() const
{
	const UISkin* skin = GetSkin();
	return (skin != 0) ? skin->GetTexture() : 0;
}

//============================================================================================================
// Changes the skin and face for the image
//============================================================================================================

void UISubPicture::Set (const UISkin* skin, const String& face, bool setDirty)
{
	if (mSkin != skin || mFace == 0 || mFace->GetName() != face)
	{
		if (setDirty) SetDirty();
		mSkin = const_cast<UISkin*>(skin);

		if (mSkin != 0)
		{
			mFace = mSkin->GetFace(face);
			if (setDirty) SetDirty();
		}
		else
		{
			mFace = 0;
		}
	}
}

//============================================================================================================
// Changes the skin
//============================================================================================================

void UISubPicture::SetSkin (const UISkin* skin, bool setDirty)
{
	if (mSkin != skin)
	{
		if (setDirty) SetDirty();
		mSkin = const_cast<UISkin*>(skin);

		if (mSkin != 0 && mFace != 0)
		{
			mFace = mSkin->GetFace(mFace->GetName());
			if (setDirty) SetDirty();
		}
		else
		{
			mFace = 0;
		}
	}
}

//============================================================================================================
// Changes the face
//============================================================================================================

void UISubPicture::SetFace (const String& face, bool setDirty)
{
	if (mSkin == 0) mSkin = mUI->GetDefaultSkin();

	if (mSkin != 0)
	{
		if (face.IsEmpty())
		{
			if (setDirty) SetDirty();
			mFace = 0;
		}
		else if (mFace == 0 || mFace->GetName() != face)
		{
			mFace = mSkin->GetFace(face);
			if (setDirty) SetDirty();
		}
	}
}

//============================================================================================================
// Color tint
//============================================================================================================

void UISubPicture::SetBackColor (const Color4ub& c, bool setDirty)
{
	if (mColor != c)
	{
		mColor = c;
		if (setDirty) SetDirty();
	}
}

//============================================================================================================
// Marks the widget as needing to be rebuilt
//============================================================================================================

void UISubPicture::SetDirty()
{
	if (mFace != 0)
	{
		const ITexture* tex = GetTexture();
		if (tex != 0) OnDirty(tex);
	}
}

//============================================================================================================
// Called when something changes in the skin
//============================================================================================================

void UISubPicture::OnTextureChanged (const ITexture* ptr)
{
	if (mFace != 0)
	{
		const ITexture* tex = GetTexture();
		if (tex == ptr) OnDirty(tex);
	}
}

//============================================================================================================
// If the border size changes, the dimensions consider to be changed
//============================================================================================================

bool UISubPicture::OnUpdate (bool dimensionsChanged)
{
	short border = ((mFace != 0) ? mFace->GetBorder() : 0);

	// If the border has changed, update the region
	if (mBorder != border)
	{
		mBorder = border;

		// Only non-negative border matters for the sub-region
		if (border < 0) border = 0;

		mSubRegion.SetLeft	(0.0f,  (float)border);
		mSubRegion.SetRight (1.0f, -(float)border);
		mSubRegion.SetTop	(0.0f,  (float)border);
		mSubRegion.SetBottom(1.0f, -(float)border);

		// Dimensions have been updated and the widget should be considered changed
		dimensionsChanged = true;
	}

	if (dimensionsChanged) mSubRegion.Update(mRegion, true, true);
	return dimensionsChanged;
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UISubPicture::OnFill (UIQueue* queue)
{
	if (mFace			!= 0 &&
		queue->mLayer	== mLayer &&
		queue->mTex		!= 0 &&
		queue->mTex		== GetTexture() &&
		queue->mWidget	== 0)
	{
		Array<IUI::Vertex>& v (queue->mVertices);

		float left	 ( mRegion.GetCalculatedLeft() );
		float top	 ( mRegion.GetCalculatedTop() );
		float right	 ( mRegion.GetCalculatedRight() );
		float bottom ( mRegion.GetCalculatedBottom() );

		Color4ub color (mColor, mRegion.GetCalculatedAlpha());
		const Vector2i& texSize (queue->mTex->GetSize());
		UIFace::Rectangle face (mFace->GetRectangle(texSize));

		if (mFace->GetBorder() == 0)
		{
			// No border? Must be a basic sub-image, a single textured quad
			v.Expand().Set( left,  top,		face.mLeft,  face.mTop,		color );
			v.Expand().Set( left,  bottom,	face.mLeft,  face.mBottom,	color );
			v.Expand().Set( right, bottom,	face.mRight, face.mBottom,	color );
			v.Expand().Set( right, top,		face.mRight, face.mTop,		color );
		}
		else
		{
			float border  = mFace->GetBorder();
			float borderX = border / texSize.x;
			float borderY = border / texSize.y;

			float cx[4];
			float cy[4];
			float tx[4];
			float ty[4];

			// Positive border -- border lies on the inside of the specified coordinates
			if ( border > 0 )
			{
				cx[0] = left;
				cy[0] = top;
				tx[0] = face.mLeft;
				ty[0] = face.mTop;

				cx[1] = left + border;
				cy[1] = top + border;
				tx[1] = face.mLeft + borderX;
				ty[1] = face.mTop - borderY;

				cx[2] = right - border;
				cy[2] = bottom - border;
				tx[2] = face.mRight - borderX;
				ty[2] = face.mBottom + borderY;

				cx[3] = right;
				cy[3] = bottom;
				tx[3] = face.mRight;
				ty[3] = face.mBottom;
				
			}
			// Negative border -- must be on the outside of the specified coordinates
			else
			{
				cx[0] = left + border;
				cy[0] = top + border;
				tx[0] = face.mLeft + borderX;
				ty[0] = face.mTop - borderY;

				cx[1] = left;
				cy[1] = top;
				tx[1] = face.mLeft;
				ty[1] = face.mTop;

				cx[2] = right;
				cy[2] = bottom;
				tx[2] = face.mRight;
				ty[2] = face.mBottom;
				
				cx[3] = right - border;
				cy[3] = bottom - border;
				tx[3] = face.mRight - borderX;
				ty[3] = face.mBottom + borderY;
			}

			// Run through all 9 quads and write them into the buffer
			for (uint y = 0; y < 3; ++y)
			{
				uint y2 = y + 1;

				for (uint x = 0; x < 3; ++x)
				{
					uint x2 = x + 1;

					v.Expand().Set( cx[x ], cy[y ], tx[x ], ty[y ], color );
					v.Expand().Set( cx[x ], cy[y2], tx[x ], ty[y2], color );
					v.Expand().Set( cx[x2], cy[y2], tx[x2], ty[y2], color );
					v.Expand().Set( cx[x2], cy[y ], tx[x2], ty[y ], color );
				}
			}
		}
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UISubPicture::OnSerializeFrom (const TreeNode& node)
{
	const Variable& value = node.mValue;
	
	if (node.mTag == "Skin")
	{
		SetSkin( mUI->GetSkin( value.IsString() ? value.AsString() : value.GetString() ) );
		return true;
	}
	else if (node.mTag == "Face")
	{
		SetFace( value.IsString() ? value.AsString() : value.GetString() );
		return true;
	}
	else if (node.mTag == "Color" || node.mTag == "Back Color")
	{
		Color4ub c;
		if (node.mValue >> c) SetBackColor(c);
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UISubPicture::OnSerializeTo (TreeNode& node) const
{
	if (mSkin != 0 && mSkin != mUI->GetDefaultSkin())
		node.AddChild("Skin", mSkin->GetName());

	TreeNode& faceNode = node.AddChild("Face");
	if (mFace != 0) faceNode.mValue = mFace->GetName();

	node.AddChild("Back Color", mColor);
}