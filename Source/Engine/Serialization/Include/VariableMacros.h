#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// This file contains inlined macroed functionality used by the Variable class.
// Author: Michael Lyashenko
//============================================================================================================
// Common implementation macro
//============================================================================================================

#define VAR_COMMON_IMPL(varType, funcType, typeName)						\
	explicit Variable(const Array<varType>& arr)							\
		: mType(Type::typeName | Type::Array)								\
	{																		\
		Array<varType>* ptr = new Array<varType>();							\
		*ptr = arr;															\
		mPtr = ptr;															\
	}																		\
																			\
	bool Is##funcType() const												\
	{																		\
		return mType == Type::typeName;										\
	}																		\
																			\
	void operator =(const varType& val)										\
	{																		\
		To##funcType() = val;												\
	}																		\
																			\
	bool Is##funcType##Array() const										\
	{																		\
		return mType == (Type::typeName | Type::Array);						\
	}																		\
																			\
	const Array<varType>& As##funcType##Array() const						\
	{																		\
		return *((const Array<varType>*)mPtr);								\
	}																		\
																			\
	Array<varType>& To##funcType##Array()									\
	{																		\
		if (mType != (Type::typeName | Type::Array))						\
		{																	\
			Release();														\
			mType = Type::typeName | Type::Array;							\
			mPtr = new Array<varType>();									\
		}																	\
		return *(Array<varType>*)mPtr;										\
	}																		\
																			\
	void operator =(const Array<varType>& arr)								\
	{																		\
		To##funcType##Array() = arr;										\
	}																		\
																			\
	bool operator >> (R5::Array<varType>& value) const						\
	{																		\
		if (Is##funcType##Array())											\
		{																	\
			value = As##funcType##Array();									\
			return true;													\
		}																	\
		return false;														\
	}

//============================================================================================================
// Full implementation is used for types that fit into 16 bytes
//============================================================================================================

#define VAR_SMALL_IMPL(varType, funcType, typeName)							\
	explicit Variable(const varType& val) : mType(Type::typeName)			\
	{																		\
		((varType&)mPtr) = val;												\
	}																		\
																			\
	const varType& As##funcType() const										\
	{																		\
		return *((const varType*)&mPtr);									\
	}																		\
																			\
	varType& To##funcType()													\
	{																		\
		if (mType != Type::typeName)										\
		{																	\
			Release();														\
			mType = Type::typeName;											\
		}																	\
		return (varType&)mPtr;												\
	}																		\
																			\
	VAR_COMMON_IMPL(varType, funcType, typeName)

//============================================================================================================
// String implementation
//============================================================================================================

#define VAR_STRING_IMPL(varType, typeName)									\
	explicit Variable(const varType& val) : mType(Type::typeName)			\
	{																		\
		mPtr = new varType(val);											\
	}																		\
																			\
	varType& To##typeName()													\
	{																		\
		if (mType != Type::typeName)										\
		{																	\
			Release();														\
			mType = Type::typeName;											\
			mPtr = new varType();											\
		}																	\
		return *((varType*)mPtr);											\
	}																		\
																			\
	VAR_COMMON_IMPL(varType, typeName, typeName)


//============================================================================================================
// Full implementation is used for types that exceed 16 bytes
//============================================================================================================

//#define VAR_LARGE_IMPL(varType, typeName)
//	VAR_STRING_IMPL(varType, typeName)
//	const varType& As##typeName() const	{ return *((const varType*)mPtr); }