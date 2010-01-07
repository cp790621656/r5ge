#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Interface for the videocard memory buffer
//============================================================================================================

struct IVBO
{
	struct Type
	{
		enum
		{
			Invalid = 0,
			Vertex  = 1,
			Index	= 2
		};
	};

	virtual	~IVBO() {};

	virtual uint	GetID()		const=0;
	virtual uint	GetType()	const=0;
	virtual uint	GetSize()	const=0;
	virtual bool	IsValid()	const=0;
	virtual void	Release()=0;
	virtual void	Lock()=0;
	virtual void	Get	(VoidPtr& data, uint& size, uint& type)=0;
	virtual void	Set	(const void* data, uint size, uint type = Type::Vertex, bool dynamic = false)=0;
	virtual void	Unlock()=0;

	template <typename T>
	void Set(const Array<T>& arr, uint type = Type::Vertex, bool dynamic = false)
	{
		Set(&arr[0], arr.GetSizeInMemory(), type, dynamic);
	}
};