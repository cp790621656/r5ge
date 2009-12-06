#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Rendering technique is used to batch objects of similar properties together
//============================================================================================================

struct ITechnique
{
	R5_DECLARE_INTERFACE_CLASS("Technique");

	struct Lighting
	{
		enum
		{
			None		= 0,
			OneSided	= 1,
			TwoSided	= 2,
		};
	};

	struct Blending
	{
		enum
		{
			None		= 0,
			Normal		= 1,
			Add			= 2,
			Subtract	= 3,
			Modulate	= 4,
		};
	};

	struct Culling
	{
		enum
		{
			None	= 0,
			Front	= 1,
			Back	= 2,
		};
	};

	struct Sorting
	{
		enum
		{
			None			= 0,
			FrontToBack		= 1,
			BackToFront		= 2,
		};
	};

protected:

	// Each newly created technique should increment this value by 1. As such, there should be
	// no more than 32 techniques total as this value is used to create a bit mask that's used
	// for quick elimination. This value is out here rather than in the derived class because
	// index and mask retrieval functions must remain identical in all implementations.

	uint mIndex;

public:

	ITechnique() : mIndex(0) {}
	virtual ~ITechnique() {}

	inline uint GetIndex() const { return mIndex; }
	inline uint GetMask() const { return (1 << mIndex); }

	virtual const String& GetName() const=0;
	virtual bool GetFog()			const=0;
	virtual bool GetDepthWrite()	const=0;
	virtual bool GetDepthTest()		const=0;
	virtual bool GetColorWrite()	const=0;
	virtual bool GetAlphaTest()		const=0;
	virtual bool GetWireframe()		const=0;
	virtual byte GetLighting()		const=0;
	virtual byte GetBlending()		const=0;
	virtual byte GetCulling()		const=0;
	virtual byte GetSorting()		const=0;

	virtual void SetFog			 (bool val)=0;
	virtual void SetDepthWrite	 (bool val)=0;
	virtual void SetDepthTest	 (bool val)=0;
	virtual void SetColorWrite	 (bool val)=0;
	virtual void SetAlphaTest	 (bool val)=0;
	virtual void SetWireframe	 (bool val)=0;
	virtual void SetLighting	 (byte val)=0;
	virtual void SetBlending	 (byte val)=0;
	virtual void SetCulling		 (byte val)=0;
	virtual void SetSorting		 (byte val)=0;

	// Serialization
	virtual bool IsSerializable() const=0;
	virtual void SetSerializable(bool val)=0;
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false)=0;
	virtual bool SerializeTo (TreeNode& root) const=0;
};