/* attributes.h */

#if defined(__GNUC__)
/* building with GCC (or something compatible */

# pragma once

# if !defined(NORETURN)
#  define NORETURN __attribute__((__noreturn__))
# endif

# if !defined(UNUSED)
#  define UNUSED(v) __attribute__((__unused__)) v
# endif

#else
/* not building with GCC */

# if !defined(NORETURN)
#  define NORETURN
# endif

# if !defined(UNUSED)
#  define UNUSED(v) v
# endif

#endif
