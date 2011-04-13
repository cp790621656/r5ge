#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#undef CHAR_IS_UNSIGNED

/* Define this if your system has an ANSI-conforming <stddef.h> file.
 */
#define HAVE_STDDEF_H

/* Define this if your system has an ANSI-conforming <stdlib.h> file.
 */
#define HAVE_STDLIB_H

/* Define this if your system does not have an ANSI/SysV <string.h>,
 * but does have a BSD-style <strings.h>.
 */
#undef NEED_BSD_STRINGS

/* Define this if your system does not provide typedef size_t in any of the
 * ANSI-standard places (stddef.h, stdlib.h, or stdio.h), but places it in
 * <sys/types.h> instead.
 */
#undef NEED_SYS_TYPES_H

#undef NEED_FAR_POINTERS
#undef NEED_SHORT_EXTERNAL_NAMES
#undef INCOMPLETE_TYPES_BROKEN

/* Define "boolean" as unsigned char, not int, on Windows systems.
 */
#ifdef _WIN32
#ifndef __RPCNDR_H__		/* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
#define HAVE_BOOLEAN		/* prevent jmorecfg.h from redefining it */
#endif


/*
 * The following options affect code selection within the JPEG library,
 * but they don't need to be visible to applications using the library.
 * To minimize application namespace pollution, the symbols won't be
 * defined unless JPEG_INTERNALS has been defined.
 */

#ifdef JPEG_INTERNALS

/* Define this if your compiler implements ">>" on signed values as a logical
 * (unsigned) shift; leave it undefined if ">>" is a signed (arithmetic) shift,
 * which is the normal and rational definition.
 */
#undef RIGHT_SHIFT_IS_UNSIGNED


#endif /* JPEG_INTERNALS */
