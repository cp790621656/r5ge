#include "../Include/_All.h"
#include "../Include/_Filters.h"

//============================================================================================================
// List of registered filters is global for easy access across this file
//============================================================================================================

namespace R5
{
namespace Filter
{
struct RegisteredFilter
{
	String							mName;		// Name of the registered filter
	Noise::OnApplyFilterDelegate	mDelegate;	// Delegate function that will apply this filter
};

Array<RegisteredFilter> g_allFilters;

//============================================================================================================
// Registers a filter if it's not already present (used in the _RegisterAll() function below)
//============================================================================================================

inline void _Register (const String& name, const Noise::OnApplyFilterDelegate& fnct)
{
	RegisteredFilter& entry = g_allFilters.Expand();
	entry.mName = name;
	entry.mDelegate = fnct;
}

//============================================================================================================
// Registers a bunch of common filters
//============================================================================================================

void _RegisterAll()
{
	static bool doOnce = true;

	if (doOnce)
	{
		doOnce = false;

		_Register("Simple",		&Simple);
		_Register("Fractal",	&Fractal);
		_Register("Perlin",		&Perlin);
		_Register("Normalize",	&Normalize);
		_Register("Blur",		&Blur);
		_Register("Power",		&Power);
		_Register("Sqrt",		&Sqrt);
		_Register("Add",		&Add);
		_Register("Multiply",	&Multiply);
		_Register("Round",		&Round);
		_Register("Clamp",		&Clamp);
		_Register("Mirror",		&Mirror);
	}
}

//============================================================================================================
// Retrieves a registered filter callback
//============================================================================================================

Noise::OnApplyFilterDelegate _GetFilter (const String& name)
{
	Noise::OnApplyFilterDelegate fnct;

	g_allFilters.Lock();
	{
		_RegisterAll();

		for (uint b = 0; b < g_allFilters.GetSize(); ++b)
		{
			RegisteredFilter& rf = g_allFilters[b];

			// Match the names
			if (rf.mName == name)
			{
				fnct = rf.mDelegate;
				break;
			}
		}
	}
	g_allFilters.Unlock();
	return fnct;
}
}; // namespace Filter
}; // namespace R5

using namespace R5;
using namespace Filter;

//============================================================================================================
// STATIC: Registeres a new filter entry
//============================================================================================================

void Noise::RegisterFilter (const String& name, const R5::Noise::OnApplyFilterDelegate& fnct)
{
	g_allFilters.Lock();
	{
		_RegisterAll();

		for (uint i = 0; i < g_allFilters.GetSize(); ++i)
		{
			if (g_allFilters[i].mName == name)
			{
				g_allFilters[i].mDelegate = fnct;
				g_allFilters.Unlock();
				return;
			}
		}
		RegisteredFilter& filter = g_allFilters.Expand();
		filter.mName			 = name;
		filter.mDelegate		 = fnct;
	}
	g_allFilters.Unlock();
}

//============================================================================================================
// STATIC: Retrieves the names of all registered filters
//============================================================================================================

void Noise::GetRegisteredFilters (Array<String>& list)
{
	list.Clear();

	g_allFilters.Lock();
	{
		_RegisterAll();

		for (uint i = 0; i < g_allFilters.GetSize(); ++i)
		{
			RegisteredFilter& rf = g_allFilters[i];

			if (rf.mName.IsValid() && rf.mDelegate != 0)
			{
				list.Expand() = rf.mName;
			}
		}
	}
	g_allFilters.Unlock();
}

//============================================================================================================
// Noise constructor calls the common filter registration function which will automatically register
// basic known filters if they haven't been registered already.
//============================================================================================================

Noise::Noise()		 : mSeed(0), mData(0), mAux(0), mTemp(0), mBufferSize(0), mSeamless(true), mIsDirty(false) {}
Noise::Noise(uint s) : mSeed(s), mData(0), mAux(0), mTemp(0), mBufferSize(0), mSeamless(true), mIsDirty(false) {}

//============================================================================================================
// Both allocated buffers must be freed
//============================================================================================================

Noise::~Noise()
{
	if (mTemp != 0) delete [] mTemp;
	if (mAux  != 0) delete [] mAux;
	if (mData != 0) delete [] mData;
}

//============================================================================================================
// Release the allocated buffers
//============================================================================================================

void Noise::Release(bool clearFilters)
{
	mFilters.Lock();
	{
		if (mTemp != 0) { delete [] mTemp; mTemp = 0; }
		if (mAux  != 0) { delete [] mAux;  mAux  = 0; }
		if (mData != 0) { delete [] mData; mData = 0; }
		mBufferSize = 0;
		mIsDirty = true;
		if (clearFilters) mFilters.Clear();
	}
	mFilters.Unlock();
}

//============================================================================================================
// Applies a new filter to the noise
//============================================================================================================

void Noise::ApplyFilter (const String& filterName, const Parameters& params)
{
	mFilters.Lock();
	{
		mIsDirty = true;
		AppliedFilter& filter = mFilters.Expand();
		filter.mName = filterName;
		filter.mParams = params;
	}
	mFilters.Unlock();
}

//============================================================================================================
// Returns a pointer to the noise buffer (and generates it if necessary)
//============================================================================================================
	
float* Noise::GetBuffer(const Vector2i& size)
{
	if (mIsDirty)
	{
		if (mSize.x < 1 || mSize.y < 1) return 0;
	
		mFilters.Lock();
		{
			mIsDirty = false;
			mRand.SetSeed(mSeed);
			
			uint needed = (uint)mSize.x * mSize.y;

			// Not enough allocated memory? Grab some more.
			if (mBufferSize < needed)
			{
				if (mAux  != 0) delete [] mAux;
				if (mData != 0) delete [] mData;

				mBufferSize = needed;

				mData = new float[mBufferSize];
				mAux  = new float[mBufferSize];

				memset(mData, 0, sizeof(float) * mBufferSize);
				memset(mAux,  0, sizeof(float) * mBufferSize);
			}

			// Go through all applied filters
			for (uint i = 0; i < mFilters.GetSize(); ++i)
			{
				AppliedFilter& mf = mFilters[i];
				OnApplyFilterDelegate fnc = _GetFilter(mf.mName);
				if (fnc) fnc(mRand, mData, mAux, mSize, mf.mParams, mSeamless);
			}
		}
		mFilters.Unlock();

		// Downsample or upsample the noise, depending on the requested dimensions
		if (size != mSize && size.x > 0 && size.y > 0)
		{
			uint width	 = size.x;
			uint height	 = size.y;
			uint outSize = (uint)width * height;

			if (mTemp) delete [] mTemp;
			mTemp = new float[outSize];

			for (uint y = 0; y < height; ++y)
			{
				uint yw = y * width;
				float fy = (float)y / height;

				for (uint x = 0; x < width; ++x)
				{
					uint index = yw + x;
					float fx = (float)x / width;

					mTemp[index] = Interpolation::BilinearTile(mData, mSize.x, mSize.y, fx, fy);
				}
			}
			return mTemp;
		}
		else if (mTemp != 0)
		{
			delete [] mTemp;
			mTemp = 0;
		}
	}
	return (mTemp == 0) ? mData : mTemp;
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool Noise::SerializeFrom (const TreeNode& root)
{
	Release();

	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node  = root.mChildren[i];
		const String&	tag   = node.mTag;
		const Variable&	value = node.mValue;

		if		(tag == "Seed")		value >> mSeed;
		else if (tag == "Size")		value >> mSize;
		else if (tag == "Seamless")	value >> mSeamless;
		else if (tag == "Filters")
		{
			mFilters.Lock();
			{
				for (uint b = 0; b < node.mChildren.GetSize(); ++b)
				{
					const TreeNode& child = node.mChildren[b];
					AppliedFilter& f = mFilters.Expand();
					f.mName = child.mTag;

					if (child.mValue.IsQuaternion())
					{
						f.mParams = child.mValue.AsQuaternion();
					}
					else if (child.mValue.IsVector3f())
					{
						f.mParams = child.mValue.AsVector3f();
					}
					else if (child.mValue.IsVector2f())
					{
						f.mParams = child.mValue.AsVector2f();
					}
					else if (child.mValue.IsFloat())
					{
						f.mParams = child.mValue.AsFloat();
					}
				}
			}
			mFilters.Unlock();
		}
	}
	return true;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Noise::SerializeTo (TreeNode& root) const
{
	TreeNode& node = root.AddChild("Noise");

	node.AddChild("Seed", mSeed);
	node.AddChild("Size", mSize);
	node.AddChild("Seamless", mSeamless);
	
	TreeNode& child = node.AddChild("Filters");

	mFilters.Lock();
	{
		for (uint i = 0; i < mFilters.GetSize(); ++i)
		{
			const AppliedFilter& f = mFilters[i];
			
			TreeNode& filter = child.AddChild(f.mName);
			uint count = f.mParams.GetCount();
			const Parameters& p = f.mParams;
			
			switch (count)
			{
				case 0:		break;
				case 1:		filter.mValue = p[0];								break;
				case 2:		filter.mValue = Vector2f(p[0], p[1]);				break;
				case 3:		filter.mValue = Vector3f(p[0], p[1], p[2]);			break;
				default:	filter.mValue = Quaternion(p[0], p[1], p[2], p[3]); break;
			};			
		}
	}
	mFilters.Unlock();
	return true;
}