#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================

//============================================================================================================
// Registers a new class type with the scene manager
// TODO: Consider getting rid of 'Scene' parameter
//============================================================================================================

template <typename Type>
void RegisterObject (Scene* scene)
{
	scene->_RegisterObject( Type::ClassID(), &Type::_CreateNew );
}

//============================================================================================================
// Registers a new script type that can be added by the objects
//============================================================================================================

template <typename Type>
void RegisterScript()
{
	Object::_RegisterScript( Type::ClassID(), &Type::_CreateNew );
}

//============================================================================================================
// Templated search function
//============================================================================================================

template <typename Type>
Type* FindObject (Object* root, const String& name, bool recursive = true)
{
	root->Lock();
	Object* obj = root->_FindObject(name, recursive);
	root->Unlock();
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Creates a new child of specified type and name
//============================================================================================================

template <typename Type>
Type* AddObject (Object* root, const String& name)
{
	root->Lock();
	Object* obj = root->_AddObject(Type::ClassID(), name);
	root->Unlock();
	return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
}

//============================================================================================================
// Retrieves an existing script on the object
//============================================================================================================

template <typename Type>
Type* GetScript (Object* owner)
{
	owner->Lock();
	Script* script = owner->_GetScript(Type::ClassID());
	owner->Unlock();
	return ( script != 0 && script->IsOfClass(Type::ClassID()) ) ? (Type*)script : 0;
}

//============================================================================================================
// Adds a new script to the object (only one script of each type can be active on the object)
//============================================================================================================

template <typename Type>
Type* AddScript (Object* owner)
{
	owner->Lock();
	Script* script = owner->_AddScript(Type::ClassID());
	owner->Unlock();
	return ( script != 0 && script->IsOfClass(Type::ClassID()) ) ? (Type*)script : 0;
}

//============================================================================================================
// Removes the specified script from the game object
//============================================================================================================

template <typename Type>
void RemoveScript (Object* owner)
{
	owner->Lock();
	Script* script = owner->_GetScript(Type::ClassID());
	if (script != 0) delete script;
	owner->Unlock();
}