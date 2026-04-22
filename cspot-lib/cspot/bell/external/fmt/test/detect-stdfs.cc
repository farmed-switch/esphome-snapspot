

#include <exception>

#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE == 8
#  error libfound "stdc++fs"
#elif !defined(__apple_build_version__) && defined(_LIBCPP_VERSION) && \
    _LIBCPP_VERSION >= 7000 && _LIBCPP_VERSION < 9000
#  error libfound "c++fs"
#else

#  error libfound ""
#endif
