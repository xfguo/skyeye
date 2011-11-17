#ifndef __PORTABLE_DEF__
#define __PORTABLE_DEF__
#ifdef __WIN32__
/* on *nix platform, the suffix of shared library is so. */
const char* Default_libsuffix = ".dll";
/* we will not load the prefix with the following string */
const char* Reserved_libprefix = "libcommon-0";
#else
/* on *nix platform, the suffix of shared library is so. */
const char* Default_libsuffix = ".so";
/* we will not load the prefix with the following string */
const char* Reserved_libprefix = "libcommon";
#endif
#endif
