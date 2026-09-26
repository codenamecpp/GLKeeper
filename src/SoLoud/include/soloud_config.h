#pragma once

#if !defined(WINAPI)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#  endif
#include <windows.h>
#  undef WIN32_LEAN_AND_MEAN
#endif

