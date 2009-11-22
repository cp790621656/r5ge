#ifndef __FT_SYSTEM_MEMORY_H__
#define __FT_SYSTEM_MEMORY_H__

#include <ft2build.h>

FT_BEGIN_HEADER

 /************************************************************************/
 /************************************************************************/
 /*****                                                              *****/
 /*****    NOTE: THE CONTENT OF THIS FILE IS NOT CURRENTLY USED      *****/
 /*****          IN NORMAL BUILDS.  CONSIDER IT EXPERIMENTAL.        *****/
 /*****                                                              *****/
 /************************************************************************/
 /************************************************************************/


 /*@**********************************************************************
  *
  * @type: FT_Memory
  *
  * @description:
  *   opaque handle to a memory manager handle. Note that since FreeType
  *   2.2, the memory manager structure FT_MemoryRec is hidden to client
  *   applications.
  *
  *   however, you can still define custom allocators easily using the
  *   @ft_memory_new API
  */
  typedef struct FT_MemoryRec_*   FT_Memory;


 /*@**********************************************************************
  *
  * @functype: FT_Memory_AllocFunc
  *
  * @description:
  *   a function used to allocate a block of memory.
  *
  * @input:
  *   size     :: size of blocks in bytes. Always > 0 !!
  *   memData :: memory-manager specific optional argument
  *               (see @ft_memory_new)
  *
  * @return:
  *   address of new block. NULL in case of memory exhaustion
  */
  typedef FT_Pointer  (*FT_Memory_AllocFunc)( FT_ULong   size,
                                              FT_Pointer memData );


 /*@**********************************************************************
  *
  * @functype: FT_Memory_FreeFunc
  *
  * @description:
  *   a function used to release a block of memory created through
  *   @FT_Memory_AllocFunc or @FT_Memory_ReallocFunc
  *
  * @input:
  *   block    :: address of target memory block. cannot be NULL !!
  *   memData :: memory-manager specific optional argument
  *               (see @ft_memory_new)
  */
  typedef void        (*FT_Memory_FreeFunc) ( FT_Pointer  block,
                                              FT_Pointer  memData );


 /*@**********************************************************************
  *
  * @functype: FT_Memory_ReallocFunc
  *
  * @description:
  *   a function used to reallocate a memory block.
  *
  * @input:
  *   block    :: address of target memory block. cannot be NULL !!
  *   new_size :: new requested size in bytes
  *   cur_size :: current block size in bytes
  *   memData :: memory-manager specific optional argument
  *               (see @ft_memory_new)
  */
  typedef FT_Pointer  (*FT_Memory_ReallocFunc)( FT_Pointer   block,
                                                FT_ULong     new_size,
                                                FT_ULong     cur_size,
                                                FT_Pointer   memData );


 /*@**********************************************************************
  *
  * @functype: FT_Memory_CreateFunc
  *
  * @description:
  *   a function used to create a @FT_Memory object to model a
  *   memory manager
  *
  * @input:
  *   size      :: size of memory manager structure in bytes
  *   init_data :: optional initialisation argument
  *
  * @output:
  *   amemData :: memory-manager specific argument to block management
  *                routines.
  *
  * @return:
  *   handle to new memory manager object. NULL in case of failure
  */
  typedef FT_Pointer  (*FT_Memory_CreateFunc)( FT_UInt     size,
                                               FT_Pointer  init_data,
                                               FT_Pointer *amemData );


 /*@**********************************************************************
  *
  * @functype: FT_Memory_DestroyFunc
  *
  * @description:
  *   a function used to destroy a given @FT_Memory manager
  *
  * @input:
  *   memory   :: target memory manager handle
  *   memData :: option manager-specific argument
  */
  typedef void        (*FT_Memory_DestroyFunc)( FT_Memory  memory,
                                                FT_Pointer memData );


 /*@**********************************************************************
  *
  * @struct: FT_Memory_FuncsRec
  *
  * @description:
  *   a function used to hold all methods of a given memory manager
  *   implementation.
  *
  * @fields:
  *   mem_alloc   :: block allocation routine
  *   memFree    :: block release routine
  *   memRealloc :: block re-allocation routine
  *   memCreate  :: manager creation routine
  *   memDestroy :: manager destruction routine
  */
  typedef struct FT_Memory_FuncsRec_
  {
    FT_Memory_AllocFunc     mem_alloc;
    FT_Memory_FreeFunc      memFree;
    FT_Memory_ReallocFunc   memRealloc;
    FT_Memory_CreateFunc    memCreate;
    FT_Memory_DestroyFunc   memDestroy;

  } FT_Memory_FuncsRec, *FT_Memory_Funcs;


 /*@**********************************************************************
  *
  * @type: FT_Memory_Funcs
  *
  * @description:
  *   a pointer to a constant @FT_Memory_FuncsRec structure used to
  *   describe a given memory manager implementation.
  */
  typedef const FT_Memory_FuncsRec*  FT_Memory_Funcs;


 /*@**********************************************************************
  *
  * @function: ft_memory_new
  *
  * @description:
  *   create a new memory manager, given a set of memory methods
  *
  * @input:
  *   memFuncs     :: handle to memory manager implementation descriptor
  *   memInit_data :: optional initialisation argument, passed to
  *                    @FT_Memory_CreateFunc
  *
  * @return:
  *   new memory manager handle. NULL in case of failure
  */
  FT_BASE( FT_Memory )
  ft_memory_new( FT_Memory_Funcs  memFuncs,
                 FT_Pointer       memInit_data );


 /*@**********************************************************************
  *
  * @function: ft_memory_destroy
  *
  * @description:
  *   destroy a given memory manager
  *
  * @input:
  *   memory :: handle to target memory manager
  */
  FT_BASE( void )
  ft_memory_destroy( FT_Memory  memory );

/* */

FT_END_HEADER

#endif /* __FT_SYSTEM_MEMORY_H__ */
