#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

//============================================================================================================
// Finds a child by name
//============================================================================================================

template <typename Type>
Type* FindWidget (Area* root, const String& name, bool recursive = true)
{
	Area* obj = root->_FindChild(name, recursive);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================

template <typename Type>
Type* FindWidget (Root* root, const String& name, bool recursive = true)
{
	Area* obj = root->_FindChild(name, recursive);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Finds a child by position
//============================================================================================================

template <typename Type>
Type* FindWidget (Area* root, const Vector2i& pos)
{
	Area* obj = root->_FindChild(pos);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================

template <typename Type>
Type* FindWidget (Root* root, const Vector2i& pos)
{
	Area* obj = root->_FindChild(pos);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Adds a new child
//============================================================================================================

template <typename Type>
Type* AddWidget (Area* root, const String& name)
{
	Area* obj = root->_AddChild(Type::ClassID(), name);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================

template <typename Type>
Type* AddWidget (Root* root, const String& name)
{
	Area* obj = root->_AddChild(Type::ClassID(), name);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Registers a new class type with the UI manager
//============================================================================================================

template <typename Type>
void RegisterWidget (Root* root)
{
	root->_RegisterWidget( Type::ClassID(), &Type::_CreateNew );
}