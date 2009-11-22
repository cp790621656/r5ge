#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Macros for standarized class creation and traversal. They eliminate the need for the 'dynamic_cast'
// functionality and provide a quick way of comparing class types.
//============================================================================================================

// Stand-alone class
#ifndef R5_DECLARE_SOLO_CLASS
#define R5_DECLARE_SOLO_CLASS(ClassName)				\
	static const char*	ClassID()						{ return ClassName;			}
#endif

// Non-creatable root class
#ifndef R5_DECLARE_INTERFACE_CLASS
#define R5_DECLARE_INTERFACE_CLASS(ClassName)			\
	static const char*	ClassID()						{ return ClassName;			}	\
	virtual const char* GetClassID() const				{ return ClassID();			}	\
	virtual bool		IsOfClass(const char* id) const	{ return (ClassID() == id); }
#endif

// Creatable root class
#ifndef R5_DECLARE_BASE_CLASS
#define R5_DECLARE_BASE_CLASS(ClassName, MyClass)		\
	static MyClass*		_CreateNew()					{ return new MyClass();		}	\
	static const char*	ClassID()						{ return ClassName;			}	\
	virtual const char* GetClassID() const				{ return ClassID();			}	\
	virtual bool		IsOfClass(const char* id) const	{ return (ClassID() == id); }
#endif

// Non-creatable inherited class
#ifndef R5_DECLARE_ABSTRACT_CLASS
#define R5_DECLARE_ABSTRACT_CLASS(ClassName, ParentClass)								\
	static const char*	ClassID()						{ return ClassName;			}	\
	virtual const char* GetClassID() const				{ return ClassID();			}	\
	virtual bool		IsOfClass(const char* id) const	{ return ((ClassID() == id) || ParentClass::IsOfClass(id)); }
#endif

// Creatable inherited class
#ifndef R5_DECLARE_INHERITED_CLASS
#define R5_DECLARE_INHERITED_CLASS(ClassName, MyClass, ParentClass, BaseClass)			\
	static BaseClass*	_CreateNew()					{ return new MyClass();		}	\
	static const char*	ClassID()						{ return ClassName;			}	\
	virtual const char* GetClassID() const				{ return ClassID();			}	\
	virtual bool		IsOfClass(const char* id) const	{ return ((ClassID() == id) || ParentClass::IsOfClass(id)); }
#endif

// Faster version of 'dynamic_cast'
#define R5_CAST(Class, ptr) ( (ptr != 0 && ptr->IsOfClass( Class::ClassID() ) ) ? (Class*)ptr : 0 )