/*
 * This file registers the FreeType modules compiled into the library.
 *
 * If you use GNU make, this file IS NOT USED!  Instead, it is created in
 * the objects directory (normally `<topdir>/objs/`) based on information
 * from `<topdir>/modules.cfg`.
 *
 * Please read `docs/INSTALL.ANY` and `docs/CUSTOMIZE` how to compile
 * FreeType without GNU make.
 *
 */

#include <nuttx/config.h>

#ifdef CONFIG_LIB_FREETYPE_HINTING_MODULES_AUTOFIT
FT_USE_MODULE( FT_Module_Class, autofit_module_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_TRUETYPE
FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_TYPE1
FT_USE_MODULE( FT_Driver_ClassRec, t1_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_CFF
FT_USE_MODULE( FT_Driver_ClassRec, cff_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_CID
FT_USE_MODULE( FT_Driver_ClassRec, t1cid_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_PFR
FT_USE_MODULE( FT_Driver_ClassRec, pfr_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_TYPE42
FT_USE_MODULE( FT_Driver_ClassRec, t42_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_WINFONTS
FT_USE_MODULE( FT_Driver_ClassRec, winfnt_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_PCF
FT_USE_MODULE( FT_Driver_ClassRec, pcf_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_BDF
FT_USE_MODULE( FT_Driver_ClassRec, bdf_driver_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_AUX_MODULES_PSAUX
FT_USE_MODULE( FT_Module_Class, psaux_module_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_AUX_MODULES_PSNAMES
FT_USE_MODULE( FT_Module_Class, psnames_module_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_HINTING_MODULES_PSHINTER
FT_USE_MODULE( FT_Module_Class, pshinter_module_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_FONT_MODULES_SFNT
FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_RASTER_MODULES_SMOOTH
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_RASTER_MODULES_RASTER
FT_USE_MODULE( FT_Renderer_Class, ft_raster1_renderer_class )
#endif

#ifdef CONFIG_LIB_FREETYPE_RASTER_MODULES_SDF
FT_USE_MODULE( FT_Renderer_Class, ft_sdf_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_bitmap_sdf_renderer_class )
#endif

/* EOF */
