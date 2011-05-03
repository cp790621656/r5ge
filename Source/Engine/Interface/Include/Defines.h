#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Macros for standarized class creation and traversal. They eliminate the need for the 'dynamic_cast'
// functionality and provide a quick way of comparing class types.
// Author: Michael Lyashenko
//============================================================================================================

// Stand-alone class
#ifndef R5_DECLARE_NAMED_CLASS
#define R5_DECLARE_NAMED_CLASS(className) \
	static const String&	ClassName()							{ static String name (className); return name; }
#endif

// Non-createable root class
#ifndef R5_DECLARE_INTERFACE_CLASS
#define R5_DECLARE_INTERFACE_CLASS(className) \
	static const String&	ClassName()							{ static String name (className); return name; }	\
	virtual const String&	GetClassName() const				{ return ClassName(); }								\
	virtual bool			IsOfClass (const String& s) const	{ return (s == ClassName()); }
#endif

// Non-createable inherited class
#ifndef R5_DECLARE_ABSTRACT_CLASS
#define R5_DECLARE_ABSTRACT_CLASS(className, ParentClass) \
	static const String&	ClassName()							{ static String name (className); return name; }	\
	virtual const String&	GetClassName() const				{ return ClassName(); }								\
	virtual bool			IsOfClass (const String& s) const	{ return ((s == ClassName()) || ParentClass::IsOfClass(s)); }
#endif

// Createable root class
#ifndef R5_DECLARE_BASE_CLASS
#define R5_DECLARE_BASE_CLASS(className, MyClass) \
	static MyClass*			_CreateNew()						{ return new MyClass();	}							\
	static const String&	ClassName()							{ static String name (className); return name; }	\
	virtual const String&	GetClassName() const				{ return ClassName(); }								\
	virtual bool			IsOfClass (const String& s) const	{ return (s == ClassName()); }
#endif

// Createable inherited class
#ifndef R5_DECLARE_INHERITED_CLASS
#define R5_DECLARE_INHERITED_CLASS(className, MyClass, ParentClass, BaseClass) \
	static BaseClass*		_CreateNew()						{ return new MyClass();	}							\
	static const String&	ClassName()							{ static String name (className); return name; }	\
	virtual const String&	GetClassName() const				{ return ClassName(); }								\
	virtual bool			IsOfClass (const String& s) const	{ return ((s == ClassName()) || ParentClass::IsOfClass(s)); }
#endif

// Faster version of 'dynamic_cast'
#define R5_CAST(Class, ptr) ( (ptr != 0 && ptr->IsOfClass( Class::ClassName() ) ) ? (Class*)ptr : 0 )