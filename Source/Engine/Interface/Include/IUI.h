#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Basic interface for the GUI -- based on the event receiver interface
// Author: Michael Lyashenko
//============================================================================================================

struct IUI : public IEventReceiver, public Thread::Lockable
{
	R5_DECLARE_INTERFACE_CLASS(UI);

	virtual ~IUI() {};

	struct Vertex
	{
		Vector2f	mPos;
		Vector2f	mTc;
		Color4ub	mColor;

		void Set (const Vector2f& pos, const Vector2f& tc, const Color4ub& color)
		{
			mPos	= pos;
			mTc		= tc;
			mColor	= color;
		}

		void Set (float x, float y, float u, float v, const Color4ub& color)
		{
			mPos.Set(x, y);
			mTc.Set(u, v);
			mColor = color;
		}
	};

	virtual bool Update()=0;
	virtual uint Draw()=0;
	virtual void Release()=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool threadSafe = true)=0;
	virtual bool SerializeTo (TreeNode& root, bool threadSafe = true) const=0;
};