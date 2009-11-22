#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

//============================================================================================================
// Templated search function
//============================================================================================================

template <typename Type>
Type* FindObject (Object* root, const String& name, bool recursive = true)
{
	Object* obj = root->_FindObject(name, recursive);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Creates a new child of specified type and name
//============================================================================================================

template <typename Type>
Type* AddObject (Object* root, const String& name)
{
	Object* obj = root->_AddObject(Type::ClassID(), name);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Registers a new class type with the scene manager
//============================================================================================================

template <typename Type>
void RegisterObject (Scene* scene)
{
	scene->_RegisterObject( Type::ClassID(), &Type::_CreateNew );
}