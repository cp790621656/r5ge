#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Unfinished array template used by Array and PointerArray
//============================================================================================================

namespace Unfinished
{
	template <typename Type>
	class BaseArray
	{
	protected:

		Type*	mArray;
		uint	mSize;
		uint	mAllocated;
	
	private:
		mutable Thread::Lockable	mLock;

	protected:

		BaseArray()				: mArray(0), mSize(0), mAllocated(0) {}
		BaseArray(uint reserve)	: mArray(0), mSize(0), mAllocated(0) { Reserve(reserve); }

	public:
	
		virtual ~BaseArray() { if (mArray != 0) { delete [] mArray; mArray = 0; } }

		inline void				Lock()				const	{ mLock.Lock();			}
		inline void				Unlock()			const	{ mLock.Unlock();		}
		inline uint				GetSize()			const	{ return mSize;			}
		inline uint				GetElementSize()	const	{ return (mSize != 0) ? sizeof(Type) : 0;	}
		inline uint				GetAllocatedSize()	const	{ return mAllocated;	}
		inline uint				GetSizeInMemory()	const	{ return mSize * sizeof(Type); }
		inline bool				IsValid()			const	{ return mSize != 0;	}
		inline bool				IsEmpty()			const	{ return mSize == 0;	}
		inline operator			bool()				const	{ return mSize != 0;	}
		inline operator			Type*()						{ return mArray;		}
		inline operator const	Type*()				const	{ return mArray;		}
		inline operator const	void*()				const	{ return mArray;		}

		inline const Type* GetBuffer()				const	{ return mArray;		}
		inline 		 Type* GetBuffer()						{ return mArray;		}

		inline const Type& operator[] (uint index) const	{ return mArray[index]; }
		inline 		 Type& operator[] (uint index)			{ return mArray[index]; }

		inline const Type& operator[] (int index) const		{ return mArray[index]; }
		inline 		 Type& operator[] (int index)			{ return mArray[index]; }

		inline const Type& operator[] (byte index) const	{ return mArray[index]; }
		inline 		 Type& operator[] (byte index)			{ return mArray[index]; }

		inline const Type& Front(uint offset = 0) const		{ return mArray[offset]; }
		inline		 Type& Front(uint offset = 0)			{ return mArray[offset]; }

		inline const Type& Back(uint offset = 0)  const		{ return mArray[mSize - 1 - offset]; }
		inline		 Type& Back(uint offset = 0)			{ return mArray[mSize - 1 - offset]; }

		inline void  MemsetZero()
		{
			if (mAllocated != 0)
			{
				memset(mArray, 0, mAllocated * sizeof(Type));
			}
		}

		inline Type& Expand()
		{
			if (mSize == mAllocated) Reserve(mSize + 8);
			return mArray[mSize++];
		}

		void Reserve (uint count)
		{
			if (count > mAllocated)
			{
				mAllocated = ((mAllocated << 1) > count) ? (mAllocated << 1) : count;
				
				if (mArray == 0)
				{
					mArray = new Type[mAllocated];
				}
				else
				{
					Type* newArray = new Type[mAllocated];
					uint current = mSize * sizeof(Type);
					memcpy(newArray, mArray, current);
					memset(mArray, 0, current);
					delete [] mArray;
					mArray = newArray;
				}
			}
		}

		BaseArray& operator << (const Type& in)
		{
			if (mSize == mAllocated) Reserve(mSize + 8);
			mArray[mSize++] = in;
			return *this;
		}

		// Returns whether the value already exists in the array
		bool Contains (const Type& val) const
		{
			if ( mSize != 0)
			{
				const Type* start = mArray;
				const Type* end = start + mSize;

				for ( ; start != end; ++start )
					if (*start == val)
						return true;
			}
			return false;
		}

		// Adds a unique entry to the array, or returns an existing one
		Type& AddUnique (const Type& val)
		{
			for (uint i = 0; i < mSize; ++i)
				if (mArray[i] == val)
					return mArray[i];

			Type& entry = Expand();
			entry = val;
			return entry;
		}

		// Removes an entry from the array
		// NOTE: YOU are responsible for freeing whatever memory is used by the item being removed!
		// This means that if you have anything allocated (Strings, etc), then you are going to be
		// responsible for releasing those strings prior to calling the Array::Remove function.
		
		bool Remove (const Type& val)
		{
			if ( mSize != 0)
			{
				Type* start = mArray;
				Type* end	= mArray + mSize;

				for ( ; start != end; ++start )
				{
					if (*start == val)
					{
						--end;
						--mSize;

						if (start < end)
						{
							uint size = (byte*)end - (byte*)start;
							memmove(start, start + 1, size);
							memset(end, 0, sizeof(Type));
						}

						return true;
					}
				}
			}
			return false;
		}

		// Removes an indexed entry from the array
		// NOTE: YOU are responsible for freeing whatever memory is used by the item being removed!
		// This means that if you have anything allocated (Strings, etc), then you are going to be
		// responsible for releasing those strings prior to calling the Array::RemoveAt function.
		
		bool RemoveAt (uint val)
		{
			if ( mSize != 0 && val < mSize )
			{
				Type* start = mArray + val;
				Type* end	= mArray + mSize;
				
				--end;
				--mSize;

				if (start < end)
				{
					uint size = (byte*)end - (byte*)start;
					memmove(start, start + 1, size);
					memset(end, 0, sizeof(Type));
				}

				return true;
			}
			return false;
		}
	};
};