#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

Glow::Glow() :	mType		(Type::Point),
				mColor		(1.0f, 1.0f, 1.0f, 1.0f),
				mMag		(4.0f),
				mBackground (0),
				mForeground (0),
				mTimeStamp	(0) {}

//============================================================================================================
// Changes the alpha value that the glow animates to
//============================================================================================================

void Glow::_SetTargetAlpha (float target)
{
	if ( target != mIntensity.z )
	{
		mIntensity.x = mIntensity.y;
		mIntensity.z = target;
		mTimeStamp = Time::GetTime();
	}
}

//============================================================================================================
// Updates the glow's alpha
//============================================================================================================

void Glow::OnUpdate()
{
	// If the alpha is changing, interpolate the value
	if ( Float::IsNotEqual(mIntensity.y, mIntensity.z) )
	{
		float current = Time::GetTime();
		float factor = (current - mTimeStamp) * 8.0f;
		factor = Float::Clamp(factor, 0.0f, 1.0f);
		mIntensity.y = mIntensity.x * (1.0f - factor) + mIntensity.z * factor;
	}
}

//============================================================================================================
// Fills the render queues
//============================================================================================================

bool Glow::OnFill (FillParams& params)
{
	float radius = mAbsoluteScale * mMag;
	float maxRadius = mAbsoluteScale > radius ? mAbsoluteScale : radius;

	bool isVisible (false);

	if (mType == Type::Point)
	{
		// Point lights are checked against the frustum as a sphere
		isVisible = params.mFrustum.IsVisible(mAbsolutePos, maxRadius);
	}
	else
	{
		// Directional and spot lights are checked against the camera's directional vector
		isVisible = params.mCamDir.Dot(mAbsoluteRot.GetForward()) < 0.0f;
	}

	if (isVisible)
	{
		uint myMask = 0;
		IGraphics* graphics = mCore->GetGraphics();

		if (graphics != 0)
		{
			static uint glowMask  = graphics->GetTechnique("Glow")->GetMask();
			static uint glareMask = graphics->GetTechnique("Glare")->GetMask();

			if (mBackground != 0 && mAbsoluteScale > 0.0f)	myMask |= glowMask;
			if (mForeground != 0 && radius > 0.0f)			myMask |= glareMask;
		}

		if (myMask != 0)
		{
			float dist = (mType == Type::Directional) ? 0.0f : (mAbsolutePos - params.mCamPos).Dot();
			const void* group = (mBackground == 0 ? mForeground : mBackground);
			params.mDrawQueue.Add(mLayer, this, myMask, group, dist);
		}
		else
		{
			_SetTargetAlpha(0.0f);
		}
	}
	return true;
}

//============================================================================================================
// Draws visible objects in the scene graph
//============================================================================================================

uint Glow::OnDraw (const ITechnique* tech, bool insideOut)
{
	IGraphics* graphics = mCore->GetGraphics();

	static ITechnique* glow  = graphics->GetTechnique("Glow");
	static ITechnique* glare = graphics->GetTechnique("Glare");

	float radius (0.0f);
	const Vector2f& range  (graphics->GetCameraRange());
	const Vector3f& camPos (graphics->GetCameraPosition());

	// Disable any active shader
	graphics->SetActiveShader(0);
	graphics->SetADT(0.003921568627451f);

	// Use the assigned color by default, unless modified below
	Color4f color (mColor);

	const char* parentType = mParent->GetClassID();

	// Light's color is automatically inherited from the parent light, if the parent is indeed a light
	if (parentType == PointLight::ClassID())
	{
		color = ((PointLight*)mParent)->GetDiffuse();
		color.Normalize();
	}
	else if (parentType == DirectionalLight::ClassID())
	{
		color = ((DirectionalLight*)mParent)->GetDiffuse();
		color.Normalize();
	}

	// Transparent tech is always drawn first -- calculate the matrices
	if (tech == glow)
	{
		// Radius matches the absolute scale
		radius = mAbsoluteScale;

		if (radius > 0.0f)
		{
			// Directional object's position should be always relative to the camera's position,
			// while the positional orientation is more straightforward since it's in world space
			Vector3f offset = (mType == Type::Directional) ?
				camPos - mAbsoluteRot.GetForward() * range.y :
				mAbsolutePos;

			// Get the modelview matrix and remove the top-left 3x3 component, eliminating rotation and scaling
			mMat = graphics->GetViewMatrix();
			mMat.PreTranslate(offset);
			mMat.ClearRotationAndScaling();

			// Scale the matrix so the directional glow is of appropriate size
			if (mType == Type::Directional)
			{
				mMat.PreScale(range.y / 10.0f);
			}

			// Set transparent states
			graphics->SetActiveColor(color);
			graphics->SetActiveMaterial(mBackground);
		}
	}
	// Overlay pass is drawn second -- figure out if the lens flare should be visible based on occlusion
	else if (tech == glare)
	{
		// IGraphics::IsVisible() check is affected by the modelview matrix
		graphics->ResetViewMatrix();

		if (mType == Type::Directional)
		{
			Vector3f point ( camPos - mAbsoluteRot.GetForward() * (range.y * 0.99f) );
			_SetTargetAlpha( graphics->IsPointVisible(point) );
		}
		else if (mType == Type::Point)
		{
			_SetTargetAlpha( graphics->IsPointVisible(mAbsolutePos) );
		}

		// Radius is scaled by intensity and magnitude
		radius = (mIntensity.y > 0.0f) ? mAbsoluteScale * mMag * mIntensity.y : 0.0f;

		if (radius > 0.0f)
		{
			// Set overlay states
			color.a *= mIntensity.y;
			graphics->SetActiveColor(color);
			graphics->SetActiveMaterial(mForeground);
		}
	}

	if (radius > 0.0f)
	{
		// Set the active modelview matrix
		graphics->SetViewMatrix(mMat);

		float vertices[] = { -radius,  radius,
							 -radius, -radius,
							  radius, -radius,
							  radius,  radius };

		float texcoords[] = { 0.0f, 0.0f,
							  0.0f, 1.0f,
							  1.0f, 1.0f,
							  1.0f, 0.0f };

		// Set the buffers
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Color,	  0 );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Normal,	  0 );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Tangent,	  0 );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneIndex,  0 );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::BoneWeight, 0 );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord1,  0 );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::TexCoord0, (Vector2f*)texcoords );
		graphics->SetActiveVertexAttribute( IGraphics::Attribute::Position,	 (Vector2f*)vertices );

		// Draw the glow
		graphics->DrawVertices (IGraphics::Primitive::Quad, 4);
		return 1;
	}
	return 0;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void Glow::OnSerializeTo  (TreeNode& root) const
{
	root.AddChild("Type", (mType == Type::Directional) ? "Directional" : "Point");
	root.AddChild("Color", mColor);
	root.AddChild("Magnification", mMag);

	if (mBackground) root.AddChild("Background", mBackground->GetName());
	if (mForeground) root.AddChild("Foreground", mForeground->GetName());
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Glow::OnSerializeFrom (const TreeNode& root)
{
	const String&	tag   = root.mTag;
	const Variable&	value = root.mValue;

	if ( tag == "Type" )
	{
		if (value.IsString())
		{
			if (value.AsString() == "Directional")
			{
				mType = Type::Directional;
			}
			else
			{
				mType = Type::Point;
			}
		}
	}
	else if	( tag == "Color" )			value >> mColor;
	else if ( tag == "Magnification" )	value >> mMag;
	else if ( tag == "Background" )
	{
		IGraphics* graphics = mCore->GetGraphics();

		if (graphics != 0)
		{
			ITexture* tex = graphics->GetTexture(value.IsString() ? value.AsString() : value.GetString());
			SetBackgroundTexture(tex);
		}
	}
	else if ( tag == "Foreground" )
	{
		IGraphics* graphics = mCore->GetGraphics();

		if (graphics != 0)
		{
			ITexture* tex = graphics->GetTexture(value.IsString() ? value.AsString() : value.GetString());
			SetForegroundTexture(tex);
		}
	}
	else return false;
	return true;
}