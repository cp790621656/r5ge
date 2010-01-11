#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================

//============================================================================================================
// Finds a child by name
//============================================================================================================

template <typename Type>
Type* FindWidget (UIArea* root, const String& name, bool recursive = true)
{
	UIArea* obj = root->_FindChild(name, recursive);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================

template <typename Type>
Type* FindWidget (UIRoot* root, const String& name, bool recursive = true)
{
	UIArea* obj = root->_FindChild(name, recursive);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Finds a child by position
//============================================================================================================

template <typename Type>
Type* FindWidget (UIArea* root, const Vector2i& pos)
{
	UIArea* obj = root->_FindChild(pos);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================

template <typename Type>
Type* FindWidget (UIRoot* root, const Vector2i& pos)
{
	UIArea* obj = root->_FindChild(pos);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Adds a new child
//============================================================================================================

template <typename Type>
Type* AddWidget (UIArea* root, const String& name)
{
	UIArea* obj = root->_AddChild(Type::ClassID(), name);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================

template <typename Type>
Type* AddWidget (UIRoot* root, const String& name)
{
	UIArea* obj = root->_AddChild(Type::ClassID(), name);
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Registers a new class type with the UI manager
//============================================================================================================

template <typename Type>
void RegisterWidget (UIRoot* root)
{
	root->_RegisterWidget( Type::ClassID(), &Type::_CreateNew );
}