/***************************************************************************/
/*                                                                         */
/*  svmm.h                                                                 */
/*                                                                         */
/*    The FreeType Multiple Masters and GX var services (specification).   */
/*                                                                         */
/*  Copyright 2003, 2004 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __SVMM_H__
#define __SVMM_H__

#include FT_INTERNAL_SERVICE_H


FT_BEGIN_HEADER


  /*
   *  A service used to manage multiple-masters data in a given face.
   *
   *  See the related APIs in `ftmm.h' (FT_MULTIPLE_MASTERS_H).
   *
   */

#define FT_SERVICE_ID_MULTI_MASTERS  "multi-masters"


  typedef FT_Error
  (*FT_Get_MM_Func)( FT_Face           face,
                     FT_Multi_Master*  master );

  typedef FT_Error
  (*FT_Get_MM_Var_Func)( FT_Face      face,
                         FT_MM_Var*  *master );

  typedef FT_Error
  (*FT_Set_MM_Design_Func)( FT_Face   face,
                            FT_UInt   numCoords,
                            FT_Long*  coords );

  typedef FT_Error
  (*FT_Set_Var_Design_Func)( FT_Face    face,
                             FT_UInt    numCoords,
                             FT_Fixed*  coords );

  typedef FT_Error
  (*FT_Set_MM_Blend_Func)( FT_Face   face,
                           FT_UInt   numCoords,
                           FT_Long*  coords );


  FT_DEFINE_SERVICE( MultiMasters )
  {
    FT_Get_MM_Func          get_mm;
    FT_Set_MM_Design_Func   set_mmDesign;
    FT_Set_MM_Blend_Func    set_mmBlend;
    FT_Get_MM_Var_Func      get_mmVar;
    FT_Set_Var_Design_Func  set_var_design;
  };

  /* */


FT_END_HEADER

#endif /* __SVMM_H__ */


/* END */
