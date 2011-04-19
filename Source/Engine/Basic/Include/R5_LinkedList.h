#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Linked list template (FIFO)
// Author: Michael Lyashenko
//============================================================================================================

template <typename Type>
class LinkedList
{
public:

	template <typename ValType>
	struct Value
	{
		ValType		mVal;

	private:
		Value*		mPrev;
		Value*		mNext;

	public:
		Value() : mPrev(0), mNext(0) {}

		// Allow linked list to modify previous and next pointers
		friend class LinkedList<ValType>;
	};

	typedef Value<Type> Entry;

protected:

	Entry*	mFirst;
	Entry*	mUnused;

private:

	mutable Thread::ValType	mLock;

public:

	LinkedList() : mFirst(0), mUnused(0), mLock(0) {}
	~LinkedList() { Release(); }

	// Thread sync functions
	void Lock()		const	{ Thread::WaitFor(mLock); }
	void Unlock()	const	{ mLock = 0; }

	bool IsValid() const { return mFirst != 0; }
	bool IsEmpty() const { return mFirst == 0; }

	// Clear the linked list without releasing the memory
	void Clear()
	{
		if (mFirst != 0)
		{
			Entry* last = mFirst;
			while (last->mNext != 0) { last = last->mNext; }
			last->mNext = mUnused;
			if (mUnused != 0) mUnused->mPrev = last;
			mUnused = mFirst;
			mFirst = 0;
		}
	}

	// Release all memory used by the linked list
	void Release()
	{
		while (mFirst != 0)
		{
			Entry* next = mFirst->mNext;
			delete mFirst;
			mFirst = next;
		}

		while (mUnused != 0)
		{
			Entry* next = mUnused->mNext;
			delete mUnused;
			mUnused = next;
		}
	}

	// Navigation functions
	Entry* GetFirst()					{ return mFirst; }
	Entry* GetNext		(Entry* entry)	{ return entry->mNext; }
	Entry* GetPrevious	(Entry* entry)	{ return entry->mPrev; }

	// Uniform version of the functions above
	const Entry* GetFirst()						const 	{ return mFirst; }
	const Entry* GetNext		(Entry* entry)	const 	{ return entry->mNext; }
	const Entry* GetPrevious	(Entry* entry)	const 	{ return entry->mPrev; }

	// Returns an unused linked list entry
	Entry* GetUnused()
	{
		if (mUnused == 0) mUnused = new Entry();
		return mUnused;
	}

	// Adds this entry to the managed list
	void Push (Entry* entry)
	{
		if (entry != 0)
		{
			entry->mPrev = 0;
			entry->mNext = mFirst;

			if (mFirst != 0) mFirst->mPrev = entry;
			mFirst = entry;
		}
	}

	// Removes the first entry and 
	// NOTE: From this point on, you are responsible for releasing the removed entry
	Entry* Pop()
	{
		Entry* entry = mFirst;

		if (entry != 0)
		{
			mFirst = mFirst->mNext;

			if (mFirst != 0)
				mFirst->mPrev = 0;

			entry->mPrev = 0;
			entry->mNext = 0;
		}
		return entry;
	}

	// Remove the specified entry from the list
	// NOTE: From this point on, you are responsible for releasing the removed entry
	void Remove (Entry* entry)
	{
		if (entry != 0)
		{
			if (mUnused == entry) mUnused = mUnused->mNext;
			if (mFirst  == entry) mFirst  = mFirst->mNext;

			if (entry->mPrev != 0)
				entry->mPrev->mNext = entry->mNext;

			if (entry->mNext != 0)
				entry->mNext->mPrev = entry->mPrev;

			entry->mPrev = 0;
			entry->mNext = 0;
		}
	}

	// Move the specified entry into the managed unused list to be reused later
	void Recycle (Entry* entry)
	{
		if (mUnused != entry)
		{
			// Ensure that the entry was removed first
			Remove(entry);

			// Insert the removed entry into the unused list
			if (mUnused != 0) mUnused->mPrev = entry;
			entry->mNext = mUnused;
			mUnused = entry;
		}
	}

	// Adds a new entry to the list
	Type& Expand()
	{
		Entry* entry = GetUnused();

		if (mUnused->mNext != 0)
			mUnused->mNext->mPrev = 0;
		mUnused = mUnused->mNext;

		Push(entry);
		return entry->mVal;
	}

	// Inserts the specified entry into the linked list (sorted)
	void Insert (Entry* entry)
	{
		if (entry != 0)
		{
			// Ensure that the entry is not linked to anything else
			Remove(entry);

			// If there are no valid entries, just add it as first
			if (mFirst == 0)
			{
				mFirst = entry;
			}
			else
			{
				// Start at the beginning
				Entry* ptr = mFirst;

				// Keep advancing while the value is less than specified
				while (ptr->mVal < entry->mVal)
				{
					// If we've reached the end, add this node to the end
					if (ptr->mNext == 0)
					{
						ptr->mNext = entry;
						entry->mPrev = ptr;
						return;
					}

					// Otherwise move on to the next node and keep going
					ptr = ptr->mNext;
				}

				// If this is the first entry, then the new entry becomes first
				if (mFirst == ptr) mFirst = entry;

				// Insert this entry before the found one
				entry->mNext = ptr;
				entry->mPrev = ptr->mPrev;

				if (ptr->mPrev != 0) ptr->mPrev->mNext = entry;
				ptr->mPrev = entry;
			}
		}
	}

	// Convenience function
	void Insert (const Type& val)
	{
		Entry* entry = GetUnused();
		entry->mVal = val;
		Insert(entry);
	}

	// Insert this entry before the other one
	//void InsertBefore (Entry* entry, Entry* ptr)
	//{
	//	if (entry != ptr && entry != 0 && ptr != 0)
	//	{
	//		// Ensure that the entry was removed first
	//		Remove(entry);

	//		// If this is the first entry, then the new entry becomes first
	//		if (mFirst == ptr) mFirst = entry;

	//		entry->mNext = ptr;
	//		entry->mPrev = ptr->mPrev;

	//		if (ptr->mPrev != 0) ptr->mPrev->mNext = entry;
	//		ptr->mPrev = entry;
	//	}
	//}
};