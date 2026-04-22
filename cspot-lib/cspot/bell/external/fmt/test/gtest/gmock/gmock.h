

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_H_

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_ACTIONS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_ACTIONS_H_

#ifndef _WIN32_WCE
# include <errno.h>
#endif

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_

#include <stdio.h>
#include <ostream>
#include <string>
#include <type_traits>

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_

#include <assert.h>
#include <stdlib.h>
#include <cstdint>
#include <iostream>

#include "gtest/gtest.h"

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_PORT_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_PORT_H_

#endif

#if defined(_MSC_VER) && _MSC_VER < 1900
# error "At least Visual C++ 2015 (14.0) is required to compile Google Mock."
#endif

#define GMOCK_FLAG(name) FLAGS_gmock_##name

#if !defined(GMOCK_DECLARE_bool_)

# define GMOCK_DECLARE_bool_(name) extern GTEST_API_ bool GMOCK_FLAG(name)
# define GMOCK_DECLARE_int32_(name) extern GTEST_API_ int32_t GMOCK_FLAG(name)
# define GMOCK_DECLARE_string_(name) \
    extern GTEST_API_ ::std::string GMOCK_FLAG(name)

# define GMOCK_DEFINE_bool_(name, default_val, doc) \
    GTEST_API_ bool GMOCK_FLAG(name) = (default_val)
# define GMOCK_DEFINE_int32_(name, default_val, doc) \
    GTEST_API_ int32_t GMOCK_FLAG(name) = (default_val)
# define GMOCK_DEFINE_string_(name, default_val, doc) \
    GTEST_API_ ::std::string GMOCK_FLAG(name) = (default_val)

#endif

#endif

namespace testing {

template <typename>
class Matcher;

namespace internal {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
# pragma warning(disable:4805)
#endif

GTEST_API_ std::string JoinAsTuple(const Strings& fields);

GTEST_API_ std::string ConvertIdentifierNameToWords(const char* id_name);

template <typename Pointer>
inline const typename Pointer::element_type* GetRawPointer(const Pointer& p) {
  return p.get();
}

template <typename Element>
inline Element* GetRawPointer(Element* p) { return p; }

#if defined(_MSC_VER) && !defined(_NATIVE_WCHAR_T_DEFINED)

#else
# define GMOCK_WCHAR_T_IS_NATIVE_ 1
#endif

enum TypeKind {
  kBool, kInteger, kFloatingPoint, kOther
};

template <typename T> struct KindOf {
  enum { value = kOther };
};

#define GMOCK_DECLARE_KIND_(type, kind) \
  template <> struct KindOf<type> { enum { value = kind }; }

GMOCK_DECLARE_KIND_(bool, kBool);

GMOCK_DECLARE_KIND_(char, kInteger);
GMOCK_DECLARE_KIND_(signed char, kInteger);
GMOCK_DECLARE_KIND_(unsigned char, kInteger);
GMOCK_DECLARE_KIND_(short, kInteger);
GMOCK_DECLARE_KIND_(unsigned short, kInteger);
GMOCK_DECLARE_KIND_(int, kInteger);
GMOCK_DECLARE_KIND_(unsigned int, kInteger);
GMOCK_DECLARE_KIND_(long, kInteger);
GMOCK_DECLARE_KIND_(unsigned long, kInteger);
GMOCK_DECLARE_KIND_(long long, kInteger);
GMOCK_DECLARE_KIND_(unsigned long long, kInteger);

#if GMOCK_WCHAR_T_IS_NATIVE_
GMOCK_DECLARE_KIND_(wchar_t, kInteger);
#endif

GMOCK_DECLARE_KIND_(float, kFloatingPoint);
GMOCK_DECLARE_KIND_(double, kFloatingPoint);
GMOCK_DECLARE_KIND_(long double, kFloatingPoint);

#undef GMOCK_DECLARE_KIND_

#define GMOCK_KIND_OF_(type) \
  static_cast< ::testing::internal::TypeKind>( \
      ::testing::internal::KindOf<type>::value)

template <TypeKind kFromKind, typename From, TypeKind kToKind, typename To>
using LosslessArithmeticConvertibleImpl = std::integral_constant<
    bool,

      (kFromKind == kBool) ? true

    : (kFromKind != kToKind) ? false
    : (kFromKind == kInteger &&

      (((sizeof(From) < sizeof(To)) &&
        !(std::is_signed<From>::value && !std::is_signed<To>::value)) ||

       ((sizeof(From) == sizeof(To)) &&
        (std::is_signed<From>::value == std::is_signed<To>::value)))
       ) ? true

    : (kFromKind == kFloatingPoint && (sizeof(From) <= sizeof(To))) ? true
    : false

    >;

template <typename From, typename To>
using LosslessArithmeticConvertible =
    LosslessArithmeticConvertibleImpl<GMOCK_KIND_OF_(From), From,
                                      GMOCK_KIND_OF_(To), To>;

class FailureReporterInterface {
 public:

  enum FailureType {
    kNonfatal, kFatal
  };

  virtual ~FailureReporterInterface() {}

  virtual void ReportFailure(FailureType type, const char* file, int line,
                             const std::string& message) = 0;
};

GTEST_API_ FailureReporterInterface* GetFailureReporter();

inline void Assert(bool condition, const char* file, int line,
                   const std::string& msg) {
  if (!condition) {
    GetFailureReporter()->ReportFailure(FailureReporterInterface::kFatal,
                                        file, line, msg);
  }
}
inline void Assert(bool condition, const char* file, int line) {
  Assert(condition, file, line, "Assertion failed.");
}

inline void Expect(bool condition, const char* file, int line,
                   const std::string& msg) {
  if (!condition) {
    GetFailureReporter()->ReportFailure(FailureReporterInterface::kNonfatal,
                                        file, line, msg);
  }
}
inline void Expect(bool condition, const char* file, int line) {
  Expect(condition, file, line, "Expectation failed.");
}

enum LogSeverity {
  kInfo = 0,
  kWarning = 1
};

const char kInfoVerbosity[] = "info";

const char kWarningVerbosity[] = "warning";

const char kErrorVerbosity[] = "error";

GTEST_API_ bool LogIsVisible(LogSeverity severity);

GTEST_API_ void Log(LogSeverity severity, const std::string& message,
                    int stack_frames_to_skip);

class WithoutMatchers {
 private:
  WithoutMatchers() {}
  friend GTEST_API_ WithoutMatchers GetWithoutMatchers();
};

GTEST_API_ WithoutMatchers GetWithoutMatchers();

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4717)
#endif

template <typename T>
inline T Invalid() {
  Assert(false, "", -1, "Internal error: attempt to return invalid value");

  return Invalid<T>();
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

template <class RawContainer>
class StlContainerView {
 public:
  typedef RawContainer type;
  typedef const type& const_reference;

  static const_reference ConstReference(const RawContainer& container) {
    static_assert(!std::is_const<RawContainer>::value,
                  "RawContainer type must not be const");
    return container;
  }
  static type Copy(const RawContainer& container) { return container; }
};

template <typename Element, size_t N>
class StlContainerView<Element[N]> {
 public:
  typedef typename std::remove_const<Element>::type RawElement;
  typedef internal::NativeArray<RawElement> type;

  typedef const type const_reference;

  static const_reference ConstReference(const Element (&array)[N]) {
    static_assert(std::is_same<Element, RawElement>::value,
                  "Element type must not be const");
    return type(array, N, RelationToSourceReference());
  }
  static type Copy(const Element (&array)[N]) {
    return type(array, N, RelationToSourceCopy());
  }
};

template <typename ElementPointer, typename Size>
class StlContainerView< ::std::tuple<ElementPointer, Size> > {
 public:
  typedef typename std::remove_const<
      typename std::pointer_traits<ElementPointer>::element_type>::type
      RawElement;
  typedef internal::NativeArray<RawElement> type;
  typedef const type const_reference;

  static const_reference ConstReference(
      const ::std::tuple<ElementPointer, Size>& array) {
    return type(std::get<0>(array), std::get<1>(array),
                RelationToSourceReference());
  }
  static type Copy(const ::std::tuple<ElementPointer, Size>& array) {
    return type(std::get<0>(array), std::get<1>(array), RelationToSourceCopy());
  }
};

template <typename T> class StlContainerView<T&>;

template <typename T>
struct RemoveConstFromKey {
  typedef T type;
};

template <typename K, typename V>
struct RemoveConstFromKey<std::pair<const K, V> > {
  typedef std::pair<K, V> type;
};

GTEST_API_ void IllegalDoDefault(const char* file, int line);

template <typename F, typename Tuple, size_t... Idx>
auto ApplyImpl(F&& f, Tuple&& args, IndexSequence<Idx...>) -> decltype(
    std::forward<F>(f)(std::get<Idx>(std::forward<Tuple>(args))...)) {
  return std::forward<F>(f)(std::get<Idx>(std::forward<Tuple>(args))...);
}

template <typename F, typename Tuple>
auto Apply(F&& f, Tuple&& args) -> decltype(
    ApplyImpl(std::forward<F>(f), std::forward<Tuple>(args),
              MakeIndexSequence<std::tuple_size<
                  typename std::remove_reference<Tuple>::type>::value>())) {
  return ApplyImpl(std::forward<F>(f), std::forward<Tuple>(args),
                   MakeIndexSequence<std::tuple_size<
                       typename std::remove_reference<Tuple>::type>::value>());
}

template <typename T>
struct Function;

template <typename R, typename... Args>
struct Function<R(Args...)> {
  using Result = R;
  static constexpr size_t ArgumentCount = sizeof...(Args);
  template <size_t I>
  using Arg = ElemFromList<I, Args...>;
  using ArgumentTuple = std::tuple<Args...>;
  using ArgumentMatcherTuple = std::tuple<Matcher<Args>...>;
  using MakeResultVoid = void(Args...);
  using MakeResultIgnoredValue = IgnoredValue(Args...);
};

template <typename R, typename... Args>
constexpr size_t Function<R(Args...)>::ArgumentCount;

#ifdef _MSC_VER
# pragma warning(pop)
#endif

}
}

#endif
#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PP_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PP_H_

#define GMOCK_PP_CAT(_1, _2) GMOCK_PP_INTERNAL_CAT(_1, _2)

#define GMOCK_PP_STRINGIZE(...) GMOCK_PP_INTERNAL_STRINGIZE(__VA_ARGS__)

#define GMOCK_PP_EMPTY(...)

#define GMOCK_PP_COMMA(...) ,

#define GMOCK_PP_IDENTITY(_1) _1

#define GMOCK_PP_NARG(...) \
  GMOCK_PP_INTERNAL_16TH(  \
      (__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define GMOCK_PP_HAS_COMMA(...) \
  GMOCK_PP_INTERNAL_16TH(       \
      (__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0))

#define GMOCK_PP_HEAD(...) GMOCK_PP_INTERNAL_HEAD((__VA_ARGS__, unusedArg))

#define GMOCK_PP_TAIL(...) GMOCK_PP_INTERNAL_TAIL((__VA_ARGS__))

#define GMOCK_PP_VARIADIC_CALL(_Macro, ...) \
  GMOCK_PP_IDENTITY(                        \
      GMOCK_PP_CAT(_Macro, GMOCK_PP_NARG(__VA_ARGS__))(__VA_ARGS__))

#define GMOCK_PP_IS_EMPTY(...)                                               \
  GMOCK_PP_INTERNAL_IS_EMPTY(GMOCK_PP_HAS_COMMA(__VA_ARGS__),                \
                             GMOCK_PP_HAS_COMMA(GMOCK_PP_COMMA __VA_ARGS__), \
                             GMOCK_PP_HAS_COMMA(__VA_ARGS__()),              \
                             GMOCK_PP_HAS_COMMA(GMOCK_PP_COMMA __VA_ARGS__()))

#define GMOCK_PP_IF(_Cond, _Then, _Else) \
  GMOCK_PP_CAT(GMOCK_PP_INTERNAL_IF_, _Cond)(_Then, _Else)

#define GMOCK_PP_GENERIC_IF(_Cond, _Then, _Else) \
  GMOCK_PP_REMOVE_PARENS(GMOCK_PP_IF(_Cond, _Then, _Else))

#define GMOCK_PP_NARG0(...) \
  GMOCK_PP_IF(GMOCK_PP_IS_EMPTY(__VA_ARGS__), 0, GMOCK_PP_NARG(__VA_ARGS__))

#define GMOCK_PP_IS_BEGIN_PARENS(...)                              \
  GMOCK_PP_HEAD(GMOCK_PP_CAT(GMOCK_PP_INTERNAL_IBP_IS_VARIADIC_R_, \
                             GMOCK_PP_INTERNAL_IBP_IS_VARIADIC_C __VA_ARGS__))

#define GMOCK_PP_IS_ENCLOSED_PARENS(...)             \
  GMOCK_PP_IF(GMOCK_PP_IS_BEGIN_PARENS(__VA_ARGS__), \
              GMOCK_PP_IS_EMPTY(GMOCK_PP_EMPTY __VA_ARGS__), 0)

#define GMOCK_PP_REMOVE_PARENS(...) GMOCK_PP_INTERNAL_REMOVE_PARENS __VA_ARGS__

#define GMOCK_PP_FOR_EACH(_Macro, _Data, _Tuple)                        \
  GMOCK_PP_CAT(GMOCK_PP_INTERNAL_FOR_EACH_IMPL_, GMOCK_PP_NARG0 _Tuple) \
  (0, _Macro, _Data, _Tuple)

#define GMOCK_PP_REPEAT(_Macro, _Data, _N)           \
  GMOCK_PP_CAT(GMOCK_PP_INTERNAL_FOR_EACH_IMPL_, _N) \
  (0, _Macro, _Data, GMOCK_PP_INTENRAL_EMPTY_TUPLE)

#define GMOCK_PP_INC(_i) GMOCK_PP_CAT(GMOCK_PP_INTERNAL_INC_, _i)

#define GMOCK_PP_COMMA_IF(_i) GMOCK_PP_CAT(GMOCK_PP_INTERNAL_COMMA_IF_, _i)

#define GMOCK_PP_INTENRAL_EMPTY_TUPLE (, , , , , , , , , , , , , , , )
#define GMOCK_PP_INTERNAL_CAT(_1, _2) _1##_2
#define GMOCK_PP_INTERNAL_STRINGIZE(...) #__VA_ARGS__
#define GMOCK_PP_INTERNAL_CAT_5(_1, _2, _3, _4, _5) _1##_2##_3##_4##_5
#define GMOCK_PP_INTERNAL_IS_EMPTY(_1, _2, _3, _4)                             \
  GMOCK_PP_HAS_COMMA(GMOCK_PP_INTERNAL_CAT_5(GMOCK_PP_INTERNAL_IS_EMPTY_CASE_, \
                                             _1, _2, _3, _4))
#define GMOCK_PP_INTERNAL_IS_EMPTY_CASE_0001 ,
#define GMOCK_PP_INTERNAL_IF_1(_Then, _Else) _Then
#define GMOCK_PP_INTERNAL_IF_0(_Then, _Else) _Else

#define GMOCK_PP_INTERNAL_INTERNAL_16TH(_1, _2, _3, _4, _5, _6, _7, _8, _9, \
                                        _10, _11, _12, _13, _14, _15, _16,  \
                                        ...)                                \
  _16
#define GMOCK_PP_INTERNAL_16TH(_Args) \
  GMOCK_PP_IDENTITY(GMOCK_PP_INTERNAL_INTERNAL_16TH _Args)
#define GMOCK_PP_INTERNAL_INTERNAL_HEAD(_1, ...) _1
#define GMOCK_PP_INTERNAL_HEAD(_Args) \
  GMOCK_PP_IDENTITY(GMOCK_PP_INTERNAL_INTERNAL_HEAD _Args)
#define GMOCK_PP_INTERNAL_INTERNAL_TAIL(_1, ...) __VA_ARGS__
#define GMOCK_PP_INTERNAL_TAIL(_Args) \
  GMOCK_PP_IDENTITY(GMOCK_PP_INTERNAL_INTERNAL_TAIL _Args)

#define GMOCK_PP_INTERNAL_IBP_IS_VARIADIC_C(...) 1 _
#define GMOCK_PP_INTERNAL_IBP_IS_VARIADIC_R_1 1,
#define GMOCK_PP_INTERNAL_IBP_IS_VARIADIC_R_GMOCK_PP_INTERNAL_IBP_IS_VARIADIC_C \
  0,
#define GMOCK_PP_INTERNAL_REMOVE_PARENS(...) __VA_ARGS__
#define GMOCK_PP_INTERNAL_INC_0 1
#define GMOCK_PP_INTERNAL_INC_1 2
#define GMOCK_PP_INTERNAL_INC_2 3
#define GMOCK_PP_INTERNAL_INC_3 4
#define GMOCK_PP_INTERNAL_INC_4 5
#define GMOCK_PP_INTERNAL_INC_5 6
#define GMOCK_PP_INTERNAL_INC_6 7
#define GMOCK_PP_INTERNAL_INC_7 8
#define GMOCK_PP_INTERNAL_INC_8 9
#define GMOCK_PP_INTERNAL_INC_9 10
#define GMOCK_PP_INTERNAL_INC_10 11
#define GMOCK_PP_INTERNAL_INC_11 12
#define GMOCK_PP_INTERNAL_INC_12 13
#define GMOCK_PP_INTERNAL_INC_13 14
#define GMOCK_PP_INTERNAL_INC_14 15
#define GMOCK_PP_INTERNAL_INC_15 16
#define GMOCK_PP_INTERNAL_COMMA_IF_0
#define GMOCK_PP_INTERNAL_COMMA_IF_1 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_2 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_3 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_4 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_5 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_6 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_7 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_8 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_9 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_10 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_11 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_12 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_13 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_14 ,
#define GMOCK_PP_INTERNAL_COMMA_IF_15 ,
#define GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, _element) \
  _Macro(_i, _Data, _element)
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_0(_i, _Macro, _Data, _Tuple)
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_1(_i, _Macro, _Data, _Tuple) \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple)
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_2(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_1(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_3(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_2(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_4(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_3(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_5(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_4(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_6(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_5(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_7(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_6(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_8(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_7(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_9(_i, _Macro, _Data, _Tuple)    \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_8(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_10(_i, _Macro, _Data, _Tuple)   \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_9(GMOCK_PP_INC(_i), _Macro, _Data,    \
                                    (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_11(_i, _Macro, _Data, _Tuple)   \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_10(GMOCK_PP_INC(_i), _Macro, _Data,   \
                                     (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_12(_i, _Macro, _Data, _Tuple)   \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_11(GMOCK_PP_INC(_i), _Macro, _Data,   \
                                     (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_13(_i, _Macro, _Data, _Tuple)   \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_12(GMOCK_PP_INC(_i), _Macro, _Data,   \
                                     (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_14(_i, _Macro, _Data, _Tuple)   \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_13(GMOCK_PP_INC(_i), _Macro, _Data,   \
                                     (GMOCK_PP_TAIL _Tuple))
#define GMOCK_PP_INTERNAL_FOR_EACH_IMPL_15(_i, _Macro, _Data, _Tuple)   \
  GMOCK_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, GMOCK_PP_HEAD _Tuple) \
  GMOCK_PP_INTERNAL_FOR_EACH_IMPL_14(GMOCK_PP_INC(_i), _Macro, _Data,   \
                                     (GMOCK_PP_TAIL _Tuple))

#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif

namespace testing {

namespace internal {

template <typename T, bool kDefaultConstructible>
struct BuiltInDefaultValueGetter {
  static T Get() { return T(); }
};
template <typename T>
struct BuiltInDefaultValueGetter<T, false> {
  static T Get() {
    Assert(false, __FILE__, __LINE__,
           "Default action undefined for the function return type.");
    return internal::Invalid<T>();

  }
};

template <typename T>
class BuiltInDefaultValue {
 public:

  static bool Exists() {
    return ::std::is_default_constructible<T>::value;
  }

  static T Get() {
    return BuiltInDefaultValueGetter<
        T, ::std::is_default_constructible<T>::value>::Get();
  }
};

template <typename T>
class BuiltInDefaultValue<const T> {
 public:
  static bool Exists() { return BuiltInDefaultValue<T>::Exists(); }
  static T Get() { return BuiltInDefaultValue<T>::Get(); }
};

template <typename T>
class BuiltInDefaultValue<T*> {
 public:
  static bool Exists() { return true; }
  static T* Get() { return nullptr; }
};

#define GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(type, value) \
  template <> \
  class BuiltInDefaultValue<type> { \
   public: \
    static bool Exists() { return true; } \
    static type Get() { return value; } \
  }

GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(void, );
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(::std::string, "");
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(bool, false);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned char, '\0');
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed char, '\0');
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(char, '\0');

#if GMOCK_WCHAR_T_IS_NATIVE_
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(wchar_t, 0U);
#endif

GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned short, 0U);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed short, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned int, 0U);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed int, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned long, 0UL);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed long, 0L);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned long long, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed long long, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(float, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(double, 0);

#undef GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_

template <typename P, typename Q>
using disjunction = typename ::std::conditional<P::value, P, Q>::type;

}

template <typename T>
class DefaultValue {
 public:

  static void Set(T x) {
    delete producer_;
    producer_ = new FixedValueProducer(x);
  }

  typedef T (*FactoryFunction)();
  static void SetFactory(FactoryFunction factory) {
    delete producer_;
    producer_ = new FactoryValueProducer(factory);
  }

  static void Clear() {
    delete producer_;
    producer_ = nullptr;
  }

  static bool IsSet() { return producer_ != nullptr; }

  static bool Exists() {
    return IsSet() || internal::BuiltInDefaultValue<T>::Exists();
  }

  static T Get() {
    return producer_ == nullptr ? internal::BuiltInDefaultValue<T>::Get()
                                : producer_->Produce();
  }

 private:
  class ValueProducer {
   public:
    virtual ~ValueProducer() {}
    virtual T Produce() = 0;
  };

  class FixedValueProducer : public ValueProducer {
   public:
    explicit FixedValueProducer(T value) : value_(value) {}
    T Produce() override { return value_; }

   private:
    const T value_;
    GTEST_DISALLOW_COPY_AND_ASSIGN_(FixedValueProducer);
  };

  class FactoryValueProducer : public ValueProducer {
   public:
    explicit FactoryValueProducer(FactoryFunction factory)
        : factory_(factory) {}
    T Produce() override { return factory_(); }

   private:
    const FactoryFunction factory_;
    GTEST_DISALLOW_COPY_AND_ASSIGN_(FactoryValueProducer);
  };

  static ValueProducer* producer_;
};

template <typename T>
class DefaultValue<T&> {
 public:

  static void Set(T& x) {
    address_ = &x;
  }

  static void Clear() { address_ = nullptr; }

  static bool IsSet() { return address_ != nullptr; }

  static bool Exists() {
    return IsSet() || internal::BuiltInDefaultValue<T&>::Exists();
  }

  static T& Get() {
    return address_ == nullptr ? internal::BuiltInDefaultValue<T&>::Get()
                               : *address_;
  }

 private:
  static T* address_;
};

template <>
class DefaultValue<void> {
 public:
  static bool Exists() { return true; }
  static void Get() {}
};

template <typename T>
typename DefaultValue<T>::ValueProducer* DefaultValue<T>::producer_ = nullptr;

template <typename T>
T* DefaultValue<T&>::address_ = nullptr;

template <typename F>
class ActionInterface {
 public:
  typedef typename internal::Function<F>::Result Result;
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  ActionInterface() {}
  virtual ~ActionInterface() {}

  virtual Result Perform(const ArgumentTuple& args) = 0;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(ActionInterface);
};

template <typename F>
class Action {

  struct ActionAdapter {

    ::std::shared_ptr<ActionInterface<F>> impl_;

    template <typename... Args>
    typename internal::Function<F>::Result operator()(Args&&... args) {
      return impl_->Perform(
          ::std::forward_as_tuple(::std::forward<Args>(args)...));
    }
  };

  template <typename G>
  using IsCompatibleFunctor = std::is_constructible<std::function<F>, G>;

 public:
  typedef typename internal::Function<F>::Result Result;
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  Action() {}

  template <
      typename G,
      typename = typename std::enable_if<internal::disjunction<
          IsCompatibleFunctor<G>, std::is_constructible<std::function<Result()>,
                                                        G>>::value>::type>
  Action(G&& fun) {
    Init(::std::forward<G>(fun), IsCompatibleFunctor<G>());
  }

  explicit Action(ActionInterface<F>* impl)
      : fun_(ActionAdapter{::std::shared_ptr<ActionInterface<F>>(impl)}) {}

  template <typename Func>
  explicit Action(const Action<Func>& action) : fun_(action.fun_) {}

  bool IsDoDefault() const { return fun_ == nullptr; }

  Result Perform(ArgumentTuple args) const {
    if (IsDoDefault()) {
      internal::IllegalDoDefault(__FILE__, __LINE__);
    }
    return internal::Apply(fun_, ::std::move(args));
  }

 private:
  template <typename G>
  friend class Action;

  template <typename G>
  void Init(G&& g, ::std::true_type) {
    fun_ = ::std::forward<G>(g);
  }

  template <typename G>
  void Init(G&& g, ::std::false_type) {
    fun_ = IgnoreArgs<typename ::std::decay<G>::type>{::std::forward<G>(g)};
  }

  template <typename FunctionImpl>
  struct IgnoreArgs {
    template <typename... Args>
    Result operator()(const Args&...) const {
      return function_impl();
    }

    FunctionImpl function_impl;
  };

  ::std::function<F> fun_;
};

template <typename Impl>
class PolymorphicAction {
 public:
  explicit PolymorphicAction(const Impl& impl) : impl_(impl) {}

  template <typename F>
  operator Action<F>() const {
    return Action<F>(new MonomorphicImpl<F>(impl_));
  }

 private:
  template <typename F>
  class MonomorphicImpl : public ActionInterface<F> {
   public:
    typedef typename internal::Function<F>::Result Result;
    typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

    explicit MonomorphicImpl(const Impl& impl) : impl_(impl) {}

    Result Perform(const ArgumentTuple& args) override {
      return impl_.template Perform<Result>(args);
    }

   private:
    Impl impl_;
  };

  Impl impl_;
};

template <typename F>
Action<F> MakeAction(ActionInterface<F>* impl) {
  return Action<F>(impl);
}

template <typename Impl>
inline PolymorphicAction<Impl> MakePolymorphicAction(const Impl& impl) {
  return PolymorphicAction<Impl>(impl);
}

namespace internal {

template <typename T>
struct ByMoveWrapper {
  explicit ByMoveWrapper(T value) : payload(std::move(value)) {}
  T payload;
};

template <typename R>
class ReturnAction {
 public:

  explicit ReturnAction(R value) : value_(new R(std::move(value))) {}

  template <typename F>
  operator Action<F>() const {

    typedef typename Function<F>::Result Result;
    GTEST_COMPILE_ASSERT_(
        !std::is_reference<Result>::value,
        use_ReturnRef_instead_of_Return_to_return_a_reference);
    static_assert(!std::is_void<Result>::value,
                  "Can't use Return() on an action expected to return `void`.");
    return Action<F>(new Impl<R, F>(value_));
  }

 private:

  template <typename R_, typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(const std::shared_ptr<R>& value)
        : value_before_cast_(*value),
          value_(ImplicitCast_<Result>(value_before_cast_)) {}

    Result Perform(const ArgumentTuple&) override { return value_; }

   private:
    GTEST_COMPILE_ASSERT_(!std::is_reference<Result>::value,
                          Result_cannot_be_a_reference_type);

    R value_before_cast_;
    Result value_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(Impl);
  };

  template <typename R_, typename F>
  class Impl<ByMoveWrapper<R_>, F> : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(const std::shared_ptr<R>& wrapper)
        : performed_(false), wrapper_(wrapper) {}

    Result Perform(const ArgumentTuple&) override {
      GTEST_CHECK_(!performed_)
          << "A ByMove() action should only be performed once.";
      performed_ = true;
      return std::move(wrapper_->payload);
    }

   private:
    bool performed_;
    const std::shared_ptr<R> wrapper_;
  };

  const std::shared_ptr<R> value_;
};

class ReturnNullAction {
 public:

  template <typename Result, typename ArgumentTuple>
  static Result Perform(const ArgumentTuple&) {
    return nullptr;
  }
};

class ReturnVoidAction {
 public:

  template <typename Result, typename ArgumentTuple>
  static void Perform(const ArgumentTuple&) {
    static_assert(std::is_void<Result>::value, "Result should be void.");
  }
};

template <typename T>
class ReturnRefAction {
 public:

  explicit ReturnRefAction(T& ref) : ref_(ref) {}

  template <typename F>
  operator Action<F>() const {
    typedef typename Function<F>::Result Result;

    GTEST_COMPILE_ASSERT_(std::is_reference<Result>::value,
                          use_Return_instead_of_ReturnRef_to_return_a_value);
    return Action<F>(new Impl<F>(ref_));
  }

 private:

  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(T& ref) : ref_(ref) {}

    Result Perform(const ArgumentTuple&) override { return ref_; }

   private:
    T& ref_;
  };

  T& ref_;
};

template <typename T>
class ReturnRefOfCopyAction {
 public:

  explicit ReturnRefOfCopyAction(const T& value) : value_(value) {}

  template <typename F>
  operator Action<F>() const {
    typedef typename Function<F>::Result Result;

    GTEST_COMPILE_ASSERT_(
        std::is_reference<Result>::value,
        use_Return_instead_of_ReturnRefOfCopy_to_return_a_value);
    return Action<F>(new Impl<F>(value_));
  }

 private:

  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(const T& value) : value_(value) {}

    Result Perform(const ArgumentTuple&) override { return value_; }

   private:
    T value_;
  };

  const T value_;
};

template <typename T>
class ReturnRoundRobinAction {
 public:
  explicit ReturnRoundRobinAction(std::vector<T> values) {
    GTEST_CHECK_(!values.empty())
        << "ReturnRoundRobin requires at least one element.";
    state_->values = std::move(values);
  }

  template <typename... Args>
  T operator()(Args&&...) const {
     return state_->Next();
  }

 private:
  struct State {
    T Next() {
      T ret_val = values[i++];
      if (i == values.size()) i = 0;
      return ret_val;
    }

    std::vector<T> values;
    size_t i = 0;
  };
  std::shared_ptr<State> state_ = std::make_shared<State>();
};

class DoDefaultAction {
 public:

  template <typename F>
  operator Action<F>() const { return Action<F>(); }
};

template <typename T1, typename T2>
class AssignAction {
 public:
  AssignAction(T1* ptr, T2 value) : ptr_(ptr), value_(value) {}

  template <typename Result, typename ArgumentTuple>
  void Perform(const ArgumentTuple&  ) const {
    *ptr_ = value_;
  }

 private:
  T1* const ptr_;
  const T2 value_;
};

#if !GTEST_OS_WINDOWS_MOBILE

template <typename T>
class SetErrnoAndReturnAction {
 public:
  SetErrnoAndReturnAction(int errno_value, T result)
      : errno_(errno_value),
        result_(result) {}
  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple&  ) const {
    errno = errno_;
    return result_;
  }

 private:
  const int errno_;
  const T result_;
};

#endif

template <size_t N, typename A, typename = void>
struct SetArgumentPointeeAction {
  A value;

  template <typename... Args>
  void operator()(const Args&... args) const {
    *::std::get<N>(std::tie(args...)) = value;
  }
};

template <class Class, typename MethodPtr>
struct InvokeMethodAction {
  Class* const obj_ptr;
  const MethodPtr method_ptr;

  template <typename... Args>
  auto operator()(Args&&... args) const
      -> decltype((obj_ptr->*method_ptr)(std::forward<Args>(args)...)) {
    return (obj_ptr->*method_ptr)(std::forward<Args>(args)...);
  }
};

template <typename FunctionImpl>
struct InvokeWithoutArgsAction {
  FunctionImpl function_impl;

  template <typename... Args>
  auto operator()(const Args&...) -> decltype(function_impl()) {
    return function_impl();
  }
};

template <class Class, typename MethodPtr>
struct InvokeMethodWithoutArgsAction {
  Class* const obj_ptr;
  const MethodPtr method_ptr;

  using ReturnType =
      decltype((std::declval<Class*>()->*std::declval<MethodPtr>())());

  template <typename... Args>
  ReturnType operator()(const Args&...) const {
    return (obj_ptr->*method_ptr)();
  }
};

template <typename A>
class IgnoreResultAction {
 public:
  explicit IgnoreResultAction(const A& action) : action_(action) {}

  template <typename F>
  operator Action<F>() const {

    typedef typename internal::Function<F>::Result Result;

    static_assert(std::is_void<Result>::value, "Result type should be void.");

    return Action<F>(new Impl<F>(action_));
  }

 private:
  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename internal::Function<F>::Result Result;
    typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(const A& action) : action_(action) {}

    void Perform(const ArgumentTuple& args) override {

      action_.Perform(args);
    }

   private:

    typedef typename internal::Function<F>::MakeResultIgnoredValue
        OriginalFunction;

    const Action<OriginalFunction> action_;
  };

  const A action_;
};

template <typename InnerAction, size_t... I>
struct WithArgsAction {
  InnerAction action;

  template <typename R, typename... Args>
  operator Action<R(Args...)>() const {
    using TupleType = std::tuple<Args...>;
    Action<R(typename std::tuple_element<I, TupleType>::type...)>
        converted(action);

    return [converted](Args... args) -> R {
      return converted.Perform(std::forward_as_tuple(
        std::get<I>(std::forward_as_tuple(std::forward<Args>(args)...))...));
    };
  }
};

template <typename... Actions>
struct DoAllAction {
 private:
  template <typename T>
  using NonFinalType =
      typename std::conditional<std::is_scalar<T>::value, T, const T&>::type;

  template <typename ActionT, size_t... I>
  std::vector<ActionT> Convert(IndexSequence<I...>) const {
    return {ActionT(std::get<I>(actions))...};
  }

 public:
  std::tuple<Actions...> actions;

  template <typename R, typename... Args>
  operator Action<R(Args...)>() const {
    struct Op {
      std::vector<Action<void(NonFinalType<Args>...)>> converted;
      Action<R(Args...)> last;
      R operator()(Args... args) const {
        auto tuple_args = std::forward_as_tuple(std::forward<Args>(args)...);
        for (auto& a : converted) {
          a.Perform(tuple_args);
        }
        return last.Perform(std::move(tuple_args));
      }
    };
    return Op{Convert<Action<void(NonFinalType<Args>...)>>(
                  MakeIndexSequence<sizeof...(Actions) - 1>()),
              std::get<sizeof...(Actions) - 1>(actions)};
  }
};

template <typename T, typename... Params>
struct ReturnNewAction {
  T* operator()() const {
    return internal::Apply(
        [](const Params&... unpacked_params) {
          return new T(unpacked_params...);
        },
        params);
  }
  std::tuple<Params...> params;
};

template <size_t k>
struct ReturnArgAction {
  template <typename... Args>
  auto operator()(const Args&... args) const ->
      typename std::tuple_element<k, std::tuple<Args...>>::type {
    return std::get<k>(std::tie(args...));
  }
};

template <size_t k, typename Ptr>
struct SaveArgAction {
  Ptr pointer;

  template <typename... Args>
  void operator()(const Args&... args) const {
    *pointer = std::get<k>(std::tie(args...));
  }
};

template <size_t k, typename Ptr>
struct SaveArgPointeeAction {
  Ptr pointer;

  template <typename... Args>
  void operator()(const Args&... args) const {
    *pointer = *std::get<k>(std::tie(args...));
  }
};

template <size_t k, typename T>
struct SetArgRefereeAction {
  T value;

  template <typename... Args>
  void operator()(Args&&... args) const {
    using argk_type =
        typename ::std::tuple_element<k, std::tuple<Args...>>::type;
    static_assert(std::is_lvalue_reference<argk_type>::value,
                  "Argument must be a reference type.");
    std::get<k>(std::tie(args...)) = value;
  }
};

template <size_t k, typename I1, typename I2>
struct SetArrayArgumentAction {
  I1 first;
  I2 last;

  template <typename... Args>
  void operator()(const Args&... args) const {
    auto value = std::get<k>(std::tie(args...));
    for (auto it = first; it != last; ++it, (void)++value) {
      *value = *it;
    }
  }
};

template <size_t k>
struct DeleteArgAction {
  template <typename... Args>
  void operator()(const Args&... args) const {
    delete std::get<k>(std::tie(args...));
  }
};

template <typename Ptr>
struct ReturnPointeeAction {
  Ptr pointer;
  template <typename... Args>
  auto operator()(const Args&...) const -> decltype(*pointer) {
    return *pointer;
  }
};

#if GTEST_HAS_EXCEPTIONS
template <typename T>
struct ThrowAction {
  T exception;

  template <typename R, typename... Args>
  operator Action<R(Args...)>() const {
    T copy = exception;
    return [copy](Args...) -> R { throw copy; };
  }
};
#endif

}

typedef internal::IgnoredValue Unused;

template <typename... Action>
internal::DoAllAction<typename std::decay<Action>::type...> DoAll(
    Action&&... action) {
  return {std::forward_as_tuple(std::forward<Action>(action)...)};
}

template <size_t k, typename InnerAction>
internal::WithArgsAction<typename std::decay<InnerAction>::type, k>
WithArg(InnerAction&& action) {
  return {std::forward<InnerAction>(action)};
}

template <size_t k, size_t... ks, typename InnerAction>
internal::WithArgsAction<typename std::decay<InnerAction>::type, k, ks...>
WithArgs(InnerAction&& action) {
  return {std::forward<InnerAction>(action)};
}

template <typename InnerAction>
internal::WithArgsAction<typename std::decay<InnerAction>::type>
WithoutArgs(InnerAction&& action) {
  return {std::forward<InnerAction>(action)};
}

template <typename R>
internal::ReturnAction<R> Return(R value) {
  return internal::ReturnAction<R>(std::move(value));
}

inline PolymorphicAction<internal::ReturnNullAction> ReturnNull() {
  return MakePolymorphicAction(internal::ReturnNullAction());
}

inline PolymorphicAction<internal::ReturnVoidAction> Return() {
  return MakePolymorphicAction(internal::ReturnVoidAction());
}

template <typename R>
inline internal::ReturnRefAction<R> ReturnRef(R& x) {
  return internal::ReturnRefAction<R>(x);
}

template <typename R, R* = nullptr>
internal::ReturnRefAction<R> ReturnRef(R&&) = delete;

template <typename R>
inline internal::ReturnRefOfCopyAction<R> ReturnRefOfCopy(const R& x) {
  return internal::ReturnRefOfCopyAction<R>(x);
}

template <typename R>
internal::ByMoveWrapper<R> ByMove(R x) {
  return internal::ByMoveWrapper<R>(std::move(x));
}

template <typename T>
internal::ReturnRoundRobinAction<T> ReturnRoundRobin(std::vector<T> vals) {
  return internal::ReturnRoundRobinAction<T>(std::move(vals));
}

template <typename T>
internal::ReturnRoundRobinAction<T> ReturnRoundRobin(
    std::initializer_list<T> vals) {
  return internal::ReturnRoundRobinAction<T>(std::vector<T>(vals));
}

inline internal::DoDefaultAction DoDefault() {
  return internal::DoDefaultAction();
}

template <size_t N, typename T>
internal::SetArgumentPointeeAction<N, T> SetArgPointee(T value) {
  return {std::move(value)};
}

template <size_t N, typename T>
internal::SetArgumentPointeeAction<N, T> SetArgumentPointee(T value) {
  return {std::move(value)};
}

template <typename T1, typename T2>
PolymorphicAction<internal::AssignAction<T1, T2> > Assign(T1* ptr, T2 val) {
  return MakePolymorphicAction(internal::AssignAction<T1, T2>(ptr, val));
}

#if !GTEST_OS_WINDOWS_MOBILE

template <typename T>
PolymorphicAction<internal::SetErrnoAndReturnAction<T> >
SetErrnoAndReturn(int errval, T result) {
  return MakePolymorphicAction(
      internal::SetErrnoAndReturnAction<T>(errval, result));
}

#endif

template <typename FunctionImpl>
typename std::decay<FunctionImpl>::type Invoke(FunctionImpl&& function_impl) {
  return std::forward<FunctionImpl>(function_impl);
}

template <class Class, typename MethodPtr>
internal::InvokeMethodAction<Class, MethodPtr> Invoke(Class* obj_ptr,
                                                      MethodPtr method_ptr) {
  return {obj_ptr, method_ptr};
}

template <typename FunctionImpl>
internal::InvokeWithoutArgsAction<typename std::decay<FunctionImpl>::type>
InvokeWithoutArgs(FunctionImpl function_impl) {
  return {std::move(function_impl)};
}

template <class Class, typename MethodPtr>
internal::InvokeMethodWithoutArgsAction<Class, MethodPtr> InvokeWithoutArgs(
    Class* obj_ptr, MethodPtr method_ptr) {
  return {obj_ptr, method_ptr};
}

template <typename A>
inline internal::IgnoreResultAction<A> IgnoreResult(const A& an_action) {
  return internal::IgnoreResultAction<A>(an_action);
}

template <typename T>
inline ::std::reference_wrapper<T> ByRef(T& l_value) {
  return ::std::reference_wrapper<T>(l_value);
}

template <typename T, typename... Params>
internal::ReturnNewAction<T, typename std::decay<Params>::type...> ReturnNew(
    Params&&... params) {
  return {std::forward_as_tuple(std::forward<Params>(params)...)};
}

template <size_t k>
internal::ReturnArgAction<k> ReturnArg() {
  return {};
}

template <size_t k, typename Ptr>
internal::SaveArgAction<k, Ptr> SaveArg(Ptr pointer) {
  return {pointer};
}

template <size_t k, typename Ptr>
internal::SaveArgPointeeAction<k, Ptr> SaveArgPointee(Ptr pointer) {
  return {pointer};
}

template <size_t k, typename T>
internal::SetArgRefereeAction<k, typename std::decay<T>::type> SetArgReferee(
    T&& value) {
  return {std::forward<T>(value)};
}

template <size_t k, typename I1, typename I2>
internal::SetArrayArgumentAction<k, I1, I2> SetArrayArgument(I1 first,
                                                             I2 last) {
  return {first, last};
}

template <size_t k>
internal::DeleteArgAction<k> DeleteArg() {
  return {};
}

template <typename Ptr>
internal::ReturnPointeeAction<Ptr> ReturnPointee(Ptr pointer) {
  return {pointer};
}

#if GTEST_HAS_EXCEPTIONS
template <typename T>
internal::ThrowAction<typename std::decay<T>::type> Throw(T&& exception) {
  return {std::forward<T>(exception)};
}
#endif

namespace internal {

struct ExcessiveArg {};

template <typename F, typename Impl> struct ActionImpl;

template <typename Impl>
struct ImplBase {
  struct Holder {

    explicit operator const Impl&() const { return *ptr; }
    std::shared_ptr<Impl> ptr;
  };
  using type = typename std::conditional<std::is_constructible<Impl>::value,
                                         Impl, Holder>::type;
};

template <typename R, typename... Args, typename Impl>
struct ActionImpl<R(Args...), Impl> : ImplBase<Impl>::type {
  using Base = typename ImplBase<Impl>::type;
  using function_type = R(Args...);
  using args_type = std::tuple<Args...>;

  ActionImpl() = default;
  explicit ActionImpl(std::shared_ptr<Impl> impl) : Base{std::move(impl)} { }

  R operator()(Args&&... arg) const {
    static constexpr size_t kMaxArgs =
        sizeof...(Args) <= 10 ? sizeof...(Args) : 10;
    return Apply(MakeIndexSequence<kMaxArgs>{},
                 MakeIndexSequence<10 - kMaxArgs>{},
                 args_type{std::forward<Args>(arg)...});
  }

  template <std::size_t... arg_id, std::size_t... excess_id>
  R Apply(IndexSequence<arg_id...>, IndexSequence<excess_id...>,
          const args_type& args) const {

    static constexpr ExcessiveArg kExcessArg{};
    return static_cast<const Impl&>(*this).template gmock_PerformImpl<
         function_type,  R,
         args_type,
         typename std::tuple_element<arg_id, args_type>::type...>(
         args, std::get<arg_id>(args)...,
        ((void)excess_id, kExcessArg)...);
  }
};

template <typename F, typename Impl>
::testing::Action<F> MakeAction() {
  return ::testing::Action<F>(ActionImpl<F, Impl>());
}

template <typename F, typename Impl>
::testing::Action<F> MakeAction(std::shared_ptr<Impl> impl) {
  return ::testing::Action<F>(ActionImpl<F, Impl>(std::move(impl)));
}

#define GMOCK_INTERNAL_ARG_UNUSED(i, data, el) \
  , const arg##i##_type& arg##i GTEST_ATTRIBUTE_UNUSED_
#define GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_           \
  const args_type& args GTEST_ATTRIBUTE_UNUSED_ GMOCK_PP_REPEAT( \
      GMOCK_INTERNAL_ARG_UNUSED, , 10)

#define GMOCK_INTERNAL_ARG(i, data, el) , const arg##i##_type& arg##i
#define GMOCK_ACTION_ARG_TYPES_AND_NAMES_ \
  const args_type& args GMOCK_PP_REPEAT(GMOCK_INTERNAL_ARG, , 10)

#define GMOCK_INTERNAL_TEMPLATE_ARG(i, data, el) , typename arg##i##_type
#define GMOCK_ACTION_TEMPLATE_ARGS_NAMES_ \
  GMOCK_PP_TAIL(GMOCK_PP_REPEAT(GMOCK_INTERNAL_TEMPLATE_ARG, , 10))

#define GMOCK_INTERNAL_TYPENAME_PARAM(i, data, param) , typename param##_type
#define GMOCK_ACTION_TYPENAME_PARAMS_(params) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_TYPENAME_PARAM, , params))

#define GMOCK_INTERNAL_TYPE_PARAM(i, data, param) , param##_type
#define GMOCK_ACTION_TYPE_PARAMS_(params) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_TYPE_PARAM, , params))

#define GMOCK_INTERNAL_TYPE_GVALUE_PARAM(i, data, param) \
  , param##_type gmock_p##i
#define GMOCK_ACTION_TYPE_GVALUE_PARAMS_(params) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_TYPE_GVALUE_PARAM, , params))

#define GMOCK_INTERNAL_GVALUE_PARAM(i, data, param) \
  , std::forward<param##_type>(gmock_p##i)
#define GMOCK_ACTION_GVALUE_PARAMS_(params) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_GVALUE_PARAM, , params))

#define GMOCK_INTERNAL_INIT_PARAM(i, data, param) \
  , param(::std::forward<param##_type>(gmock_p##i))
#define GMOCK_ACTION_INIT_PARAMS_(params) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_INIT_PARAM, , params))

#define GMOCK_INTERNAL_FIELD_PARAM(i, data, param) param##_type param;
#define GMOCK_ACTION_FIELD_PARAMS_(params) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_FIELD_PARAM, , params)

#define GMOCK_INTERNAL_ACTION(name, full_name, params)                        \
  template <GMOCK_ACTION_TYPENAME_PARAMS_(params)>                            \
  class full_name {                                                           \
   public:                                                                    \
    explicit full_name(GMOCK_ACTION_TYPE_GVALUE_PARAMS_(params))              \
        : impl_(std::make_shared<gmock_Impl>(                                 \
                GMOCK_ACTION_GVALUE_PARAMS_(params))) { }                     \
    full_name(const full_name&) = default;                                    \
    full_name(full_name&&) noexcept = default;                                \
    template <typename F>                                                     \
    operator ::testing::Action<F>() const {                                   \
      return ::testing::internal::MakeAction<F>(impl_);                       \
    }                                                                         \
   private:                                                                   \
    class gmock_Impl {                                                        \
     public:                                                                  \
      explicit gmock_Impl(GMOCK_ACTION_TYPE_GVALUE_PARAMS_(params))           \
          : GMOCK_ACTION_INIT_PARAMS_(params) {}                              \
      template <typename function_type, typename return_type,                 \
                typename args_type, GMOCK_ACTION_TEMPLATE_ARGS_NAMES_>        \
      return_type gmock_PerformImpl(GMOCK_ACTION_ARG_TYPES_AND_NAMES_) const; \
      GMOCK_ACTION_FIELD_PARAMS_(params)                                      \
    };                                                                        \
    std::shared_ptr<const gmock_Impl> impl_;                                  \
  };                                                                          \
  template <GMOCK_ACTION_TYPENAME_PARAMS_(params)>                            \
  inline full_name<GMOCK_ACTION_TYPE_PARAMS_(params)> name(                   \
      GMOCK_ACTION_TYPE_GVALUE_PARAMS_(params)) {                             \
    return full_name<GMOCK_ACTION_TYPE_PARAMS_(params)>(                      \
        GMOCK_ACTION_GVALUE_PARAMS_(params));                                 \
  }                                                                           \
  template <GMOCK_ACTION_TYPENAME_PARAMS_(params)>                            \
  template <typename function_type, typename return_type, typename args_type, \
            GMOCK_ACTION_TEMPLATE_ARGS_NAMES_>                                \
  return_type full_name<GMOCK_ACTION_TYPE_PARAMS_(params)>::gmock_Impl::      \
  gmock_PerformImpl(GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

}

#define ACTION(name)                                                          \
  class name##Action {                                                        \
   public:                                                                    \
   explicit name##Action() noexcept {}                                        \
   name##Action(const name##Action&) noexcept {}                              \
    template <typename F>                                                     \
    operator ::testing::Action<F>() const {                                   \
      return ::testing::internal::MakeAction<F, gmock_Impl>();                \
    }                                                                         \
   private:                                                                   \
    class gmock_Impl {                                                        \
     public:                                                                  \
      template <typename function_type, typename return_type,                 \
                typename args_type, GMOCK_ACTION_TEMPLATE_ARGS_NAMES_>        \
      return_type gmock_PerformImpl(GMOCK_ACTION_ARG_TYPES_AND_NAMES_) const; \
    };                                                                        \
  };                                                                          \
  inline name##Action name() GTEST_MUST_USE_RESULT_;                          \
  inline name##Action name() { return name##Action(); }                       \
  template <typename function_type, typename return_type, typename args_type, \
            GMOCK_ACTION_TEMPLATE_ARGS_NAMES_>                                \
  return_type name##Action::gmock_Impl::gmock_PerformImpl(                    \
      GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP, (__VA_ARGS__))

#define ACTION_P2(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP2, (__VA_ARGS__))

#define ACTION_P3(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP3, (__VA_ARGS__))

#define ACTION_P4(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP4, (__VA_ARGS__))

#define ACTION_P5(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP5, (__VA_ARGS__))

#define ACTION_P6(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP6, (__VA_ARGS__))

#define ACTION_P7(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP7, (__VA_ARGS__))

#define ACTION_P8(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP8, (__VA_ARGS__))

#define ACTION_P9(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP9, (__VA_ARGS__))

#define ACTION_P10(name, ...) \
  GMOCK_INTERNAL_ACTION(name, name##ActionP10, (__VA_ARGS__))

}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_

#include <limits.h>
#include <memory>
#include <ostream>

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
 )

namespace testing {

class CardinalityInterface {
 public:
  virtual ~CardinalityInterface() {}

  virtual int ConservativeLowerBound() const { return 0; }
  virtual int ConservativeUpperBound() const { return INT_MAX; }

  virtual bool IsSatisfiedByCallCount(int call_count) const = 0;

  virtual bool IsSaturatedByCallCount(int call_count) const = 0;

  virtual void DescribeTo(::std::ostream* os) const = 0;
};

class GTEST_API_ Cardinality {
 public:

  Cardinality() {}

  explicit Cardinality(const CardinalityInterface* impl) : impl_(impl) {}

  int ConservativeLowerBound() const { return impl_->ConservativeLowerBound(); }
  int ConservativeUpperBound() const { return impl_->ConservativeUpperBound(); }

  bool IsSatisfiedByCallCount(int call_count) const {
    return impl_->IsSatisfiedByCallCount(call_count);
  }

  bool IsSaturatedByCallCount(int call_count) const {
    return impl_->IsSaturatedByCallCount(call_count);
  }

  bool IsOverSaturatedByCallCount(int call_count) const {
    return impl_->IsSaturatedByCallCount(call_count) &&
        !impl_->IsSatisfiedByCallCount(call_count);
  }

  void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }

  static void DescribeActualCallCountTo(int actual_call_count,
                                        ::std::ostream* os);

 private:
  std::shared_ptr<const CardinalityInterface> impl_;
};

GTEST_API_ Cardinality AtLeast(int n);

GTEST_API_ Cardinality AtMost(int n);

GTEST_API_ Cardinality AnyNumber();

GTEST_API_ Cardinality Between(int min, int max);

GTEST_API_ Cardinality Exactly(int n);

inline Cardinality MakeCardinality(const CardinalityInterface* c) {
  return Cardinality(c);
}

}

GTEST_DISABLE_MSC_WARNINGS_POP_()

#endif

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_FUNCTION_MOCKER_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_FUNCTION_MOCKER_H_

#include <type_traits>
#include <utility>

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_SPEC_BUILDERS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_SPEC_BUILDERS_H_

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(_MSC_VER) && _MSC_VER >= 1915
#define GMOCK_MAYBE_5046_ 5046
#else
#define GMOCK_MAYBE_5046_
#endif

GTEST_DISABLE_MSC_WARNINGS_PUSH_(
    4251 GMOCK_MAYBE_5046_
     )

namespace testing {

class StringMatchResultListener : public MatchResultListener {
 public:
  StringMatchResultListener() : MatchResultListener(&ss_) {}

  std::string str() const { return ss_.str(); }

  void Clear() { ss_.str(""); }

 private:
  ::std::stringstream ss_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(StringMatchResultListener);
};

namespace internal {

template <typename T, typename M>
class MatcherCastImpl {
 public:
  static Matcher<T> Cast(const M& polymorphic_matcher_or_value) {

    return CastImpl(polymorphic_matcher_or_value,
                    std::is_convertible<M, Matcher<T>>{},
                    std::is_convertible<M, T>{});
  }

 private:
  template <bool Ignore>
  static Matcher<T> CastImpl(const M& polymorphic_matcher_or_value,
                             std::true_type  ,
                             std::integral_constant<bool, Ignore>) {

    return polymorphic_matcher_or_value;
  }

  static Matcher<T> CastImpl(const M& value,
                             std::false_type  ,
                             std::true_type  ) {
    return Matcher<T>(ImplicitCast_<T>(value));
  }

  static Matcher<T> CastImpl(const M& value,
                             std::false_type  ,
                             std::false_type  );
};

template <typename T, typename U>
class MatcherCastImpl<T, Matcher<U> > {
 public:
  static Matcher<T> Cast(const Matcher<U>& source_matcher) {
    return Matcher<T>(new Impl(source_matcher));
  }

 private:
  class Impl : public MatcherInterface<T> {
   public:
    explicit Impl(const Matcher<U>& source_matcher)
        : source_matcher_(source_matcher) {}

    bool MatchAndExplain(T x, MatchResultListener* listener) const override {
      using FromType = typename std::remove_cv<typename std::remove_pointer<
          typename std::remove_reference<T>::type>::type>::type;
      using ToType = typename std::remove_cv<typename std::remove_pointer<
          typename std::remove_reference<U>::type>::type>::type;

      static_assert(

          (std::is_pointer<typename std::remove_reference<T>::type>::value !=
           std::is_pointer<typename std::remove_reference<U>::type>::value) ||
              std::is_same<FromType, ToType>::value ||
              !std::is_base_of<FromType, ToType>::value,
          "Can't implicitly convert from <base> to <derived>");

      using CastType =
          typename std::conditional<std::is_convertible<T&, const U&>::value,
                                    T&, U>::type;

      return source_matcher_.MatchAndExplain(static_cast<CastType>(x),
                                             listener);
    }

    void DescribeTo(::std::ostream* os) const override {
      source_matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      source_matcher_.DescribeNegationTo(os);
    }

   private:
    const Matcher<U> source_matcher_;
  };
};

template <typename T>
class MatcherCastImpl<T, Matcher<T> > {
 public:
  static Matcher<T> Cast(const Matcher<T>& matcher) { return matcher; }
};

template <typename Derived>
class MatcherBaseImpl {
 public:
  MatcherBaseImpl() = default;

  template <typename T>
  operator ::testing::Matcher<T>() const {
    return ::testing::Matcher<T>(new
                                 typename Derived::template gmock_Impl<T>());
  }
};

template <template <typename...> class Derived, typename... Ts>
class MatcherBaseImpl<Derived<Ts...>> {
 public:

  template <typename E = std::enable_if<sizeof...(Ts) == 1>,
            typename E::type* = nullptr>
  explicit MatcherBaseImpl(Ts... params)
      : params_(std::forward<Ts>(params)...) {}
  template <typename E = std::enable_if<sizeof...(Ts) != 1>,
            typename = typename E::type>
  MatcherBaseImpl(Ts... params)
      : params_(std::forward<Ts>(params)...) {}

  template <typename F>
  operator ::testing::Matcher<F>() const {
    return Apply<F>(MakeIndexSequence<sizeof...(Ts)>{});
  }

 private:
  template <typename F, std::size_t... tuple_ids>
  ::testing::Matcher<F> Apply(IndexSequence<tuple_ids...>) const {
    return ::testing::Matcher<F>(
        new typename Derived<Ts...>::template gmock_Impl<F>(
            std::get<tuple_ids>(params_)...));
  }

  const std::tuple<Ts...> params_;
};

}

template <typename T, typename M>
inline Matcher<T> MatcherCast(const M& matcher) {
  return internal::MatcherCastImpl<T, M>::Cast(matcher);
}

template <typename T, typename M>
inline Matcher<T> SafeMatcherCast(const M& polymorphic_matcher_or_value) {
  return MatcherCast<T>(polymorphic_matcher_or_value);
}

template <typename T, typename U>
inline Matcher<T> SafeMatcherCast(const Matcher<U>& matcher) {

  static_assert(std::is_convertible<const T&, const U&>::value,
                "T must be implicitly convertible to U");

  GTEST_COMPILE_ASSERT_(
      std::is_reference<T>::value || !std::is_reference<U>::value,
      cannot_convert_non_reference_arg_to_reference);

  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(T) RawT;
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(U) RawU;
  constexpr bool kTIsOther = GMOCK_KIND_OF_(RawT) == internal::kOther;
  constexpr bool kUIsOther = GMOCK_KIND_OF_(RawU) == internal::kOther;
  GTEST_COMPILE_ASSERT_(
      kTIsOther || kUIsOther ||
      (internal::LosslessArithmeticConvertible<RawT, RawU>::value),
      conversion_of_arithmetic_types_must_be_lossless);
  return MatcherCast<T>(matcher);
}

template <typename T>
Matcher<T> A();

namespace internal {

inline void PrintIfNotEmpty(const std::string& explanation,
                            ::std::ostream* os) {
  if (explanation != "" && os != nullptr) {
    *os << ", " << explanation;
  }
}

inline bool IsReadableTypeName(const std::string& type_name) {

  return (type_name.length() <= 20 ||
          type_name.find_first_of("<(") == std::string::npos);
}

template <typename Value, typename T>
bool MatchPrintAndExplain(Value& value, const Matcher<T>& matcher,
                          MatchResultListener* listener) {
  if (!listener->IsInterested()) {

    return matcher.Matches(value);
  }

  StringMatchResultListener inner_listener;
  const bool match = matcher.MatchAndExplain(value, &inner_listener);

  UniversalPrint(value, listener->stream());
#if GTEST_HAS_RTTI
  const std::string& type_name = GetTypeName<Value>();
  if (IsReadableTypeName(type_name))
    *listener->stream() << " (of type " << type_name << ")";
#endif
  PrintIfNotEmpty(inner_listener.str(), listener->stream());

  return match;
}

template <size_t N>
class TuplePrefix {
 public:

  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& matcher_tuple,
                      const ValueTuple& value_tuple) {
    return TuplePrefix<N - 1>::Matches(matcher_tuple, value_tuple) &&
           std::get<N - 1>(matcher_tuple).Matches(std::get<N - 1>(value_tuple));
  }

  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple& matchers,
                                     const ValueTuple& values,
                                     ::std::ostream* os) {

    TuplePrefix<N - 1>::ExplainMatchFailuresTo(matchers, values, os);

    typename std::tuple_element<N - 1, MatcherTuple>::type matcher =
        std::get<N - 1>(matchers);
    typedef typename std::tuple_element<N - 1, ValueTuple>::type Value;
    const Value& value = std::get<N - 1>(values);
    StringMatchResultListener listener;
    if (!matcher.MatchAndExplain(value, &listener)) {
      *os << "  Expected arg #" << N - 1 << ": ";
      std::get<N - 1>(matchers).DescribeTo(os);
      *os << "\n           Actual: ";

      internal::UniversalPrint(value, os);
      PrintIfNotEmpty(listener.str(), os);
      *os << "\n";
    }
  }
};

template <>
class TuplePrefix<0> {
 public:
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple&  ,
                      const ValueTuple&  ) {
    return true;
  }

  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple&  ,
                                     const ValueTuple&  ,
                                     ::std::ostream*  ) {}
};

template <typename MatcherTuple, typename ValueTuple>
bool TupleMatches(const MatcherTuple& matcher_tuple,
                  const ValueTuple& value_tuple) {

  GTEST_COMPILE_ASSERT_(std::tuple_size<MatcherTuple>::value ==
                            std::tuple_size<ValueTuple>::value,
                        matcher_and_value_have_different_numbers_of_fields);
  return TuplePrefix<std::tuple_size<ValueTuple>::value>::Matches(matcher_tuple,
                                                                  value_tuple);
}

template <typename MatcherTuple, typename ValueTuple>
void ExplainMatchFailureTupleTo(const MatcherTuple& matchers,
                                const ValueTuple& values,
                                ::std::ostream* os) {
  TuplePrefix<std::tuple_size<MatcherTuple>::value>::ExplainMatchFailuresTo(
      matchers, values, os);
}

template <typename Tuple, typename Func, typename OutIter>
class TransformTupleValuesHelper {
 private:
  typedef ::std::tuple_size<Tuple> TupleSize;

 public:

  static OutIter Run(Func f, const Tuple& t, OutIter out) {
    return IterateOverTuple<Tuple, TupleSize::value>()(f, t, out);
  }

 private:
  template <typename Tup, size_t kRemainingSize>
  struct IterateOverTuple {
    OutIter operator() (Func f, const Tup& t, OutIter out) const {
      *out++ = f(::std::get<TupleSize::value - kRemainingSize>(t));
      return IterateOverTuple<Tup, kRemainingSize - 1>()(f, t, out);
    }
  };
  template <typename Tup>
  struct IterateOverTuple<Tup, 0> {
    OutIter operator() (Func  , const Tup&  , OutIter out) const {
      return out;
    }
  };
};

template <typename Tuple, typename Func, typename OutIter>
OutIter TransformTupleValues(Func f, const Tuple& t, OutIter out) {
  return TransformTupleValuesHelper<Tuple, Func, OutIter>::Run(f, t, out);
}

class AnythingMatcher {
 public:
  using is_gtest_matcher = void;

  template <typename T>
  bool MatchAndExplain(const T&  , std::ostream*  ) const {
    return true;
  }
  void DescribeTo(std::ostream* os) const { *os << "is anything"; }
  void DescribeNegationTo(::std::ostream* os) const {

    *os << "never matches";
  }
};

class IsNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener*  ) const {
    return p == nullptr;
  }

  void DescribeTo(::std::ostream* os) const { *os << "is NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "isn't NULL";
  }
};

class NotNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener*  ) const {
    return p != nullptr;
  }

  void DescribeTo(::std::ostream* os) const { *os << "isn't NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is NULL";
  }
};

template <typename T>
class RefMatcher;

template <typename T>
class RefMatcher<T&> {

 public:

  explicit RefMatcher(T& x) : object_(x) {}

  template <typename Super>
  operator Matcher<Super&>() const {

    return MakeMatcher(new Impl<Super>(object_));
  }

 private:
  template <typename Super>
  class Impl : public MatcherInterface<Super&> {
   public:
    explicit Impl(Super& x) : object_(x) {}

    bool MatchAndExplain(Super& x,
                         MatchResultListener* listener) const override {
      *listener << "which is located @" << static_cast<const void*>(&x);
      return &x == &object_;
    }

    void DescribeTo(::std::ostream* os) const override {
      *os << "references the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "does not reference the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

   private:
    const Super& object_;
  };

  T& object_;
};

inline bool CaseInsensitiveCStringEquals(const char* lhs, const char* rhs) {
  return String::CaseInsensitiveCStringEquals(lhs, rhs);
}

inline bool CaseInsensitiveCStringEquals(const wchar_t* lhs,
                                         const wchar_t* rhs) {
  return String::CaseInsensitiveWideCStringEquals(lhs, rhs);
}

template <typename StringType>
bool CaseInsensitiveStringEquals(const StringType& s1,
                                 const StringType& s2) {

  if (!CaseInsensitiveCStringEquals(s1.c_str(), s2.c_str())) {
    return false;
  }

  const typename StringType::value_type nul = 0;
  const size_t i1 = s1.find(nul), i2 = s2.find(nul);

  if (i1 == StringType::npos || i2 == StringType::npos) {
    return i1 == i2;
  }

  return CaseInsensitiveStringEquals(s1.substr(i1 + 1), s2.substr(i2 + 1));
}

template <typename StringType>
class StrEqualityMatcher {
 public:
  StrEqualityMatcher(StringType str, bool expect_eq, bool case_sensitive)
      : string_(std::move(str)),
        expect_eq_(expect_eq),
        case_sensitive_(case_sensitive) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {

    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif

  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    if (s == nullptr) {
      return !expect_eq_;
    }
    return MatchAndExplain(StringType(s), listener);
  }

  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener*  ) const {
    const StringType s2(s);
    const bool eq = case_sensitive_ ? s2 == string_ :
        CaseInsensitiveStringEquals(s2, string_);
    return expect_eq_ == eq;
  }

  void DescribeTo(::std::ostream* os) const {
    DescribeToHelper(expect_eq_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    DescribeToHelper(!expect_eq_, os);
  }

 private:
  void DescribeToHelper(bool expect_eq, ::std::ostream* os) const {
    *os << (expect_eq ? "is " : "isn't ");
    *os << "equal to ";
    if (!case_sensitive_) {
      *os << "(ignoring case) ";
    }
    UniversalPrint(string_, os);
  }

  const StringType string_;
  const bool expect_eq_;
  const bool case_sensitive_;
};

template <typename StringType>
class HasSubstrMatcher {
 public:
  explicit HasSubstrMatcher(const StringType& substring)
      : substring_(substring) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {

    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif

  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != nullptr && MatchAndExplain(StringType(s), listener);
  }

  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener*  ) const {
    return StringType(s).find(substring_) != StringType::npos;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "has substring ";
    UniversalPrint(substring_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "has no substring ";
    UniversalPrint(substring_, os);
  }

 private:
  const StringType substring_;
};

template <typename StringType>
class StartsWithMatcher {
 public:
  explicit StartsWithMatcher(const StringType& prefix) : prefix_(prefix) {
  }

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {

    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif

  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != nullptr && MatchAndExplain(StringType(s), listener);
  }

  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener*  ) const {
    const StringType& s2(s);
    return s2.length() >= prefix_.length() &&
        s2.substr(0, prefix_.length()) == prefix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "starts with ";
    UniversalPrint(prefix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't start with ";
    UniversalPrint(prefix_, os);
  }

 private:
  const StringType prefix_;
};

template <typename StringType>
class EndsWithMatcher {
 public:
  explicit EndsWithMatcher(const StringType& suffix) : suffix_(suffix) {}

#if GTEST_INTERNAL_HAS_STRING_VIEW
  bool MatchAndExplain(const internal::StringView& s,
                       MatchResultListener* listener) const {

    const StringType& str = std::string(s);
    return MatchAndExplain(str, listener);
  }
#endif

  template <typename CharType>
  bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
    return s != nullptr && MatchAndExplain(StringType(s), listener);
  }

  template <typename MatcheeStringType>
  bool MatchAndExplain(const MatcheeStringType& s,
                       MatchResultListener*  ) const {
    const StringType& s2(s);
    return s2.length() >= suffix_.length() &&
        s2.substr(s2.length() - suffix_.length()) == suffix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "ends with ";
    UniversalPrint(suffix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't end with ";
    UniversalPrint(suffix_, os);
  }

 private:
  const StringType suffix_;
};

template <typename D, typename Op>
class PairMatchBase {
 public:
  template <typename T1, typename T2>
  operator Matcher<::std::tuple<T1, T2>>() const {
    return Matcher<::std::tuple<T1, T2>>(new Impl<const ::std::tuple<T1, T2>&>);
  }
  template <typename T1, typename T2>
  operator Matcher<const ::std::tuple<T1, T2>&>() const {
    return MakeMatcher(new Impl<const ::std::tuple<T1, T2>&>);
  }

 private:
  static ::std::ostream& GetDesc(::std::ostream& os) {
    return os << D::Desc();
  }

  template <typename Tuple>
  class Impl : public MatcherInterface<Tuple> {
   public:
    bool MatchAndExplain(Tuple args,
                         MatchResultListener*  ) const override {
      return Op()(::std::get<0>(args), ::std::get<1>(args));
    }
    void DescribeTo(::std::ostream* os) const override {
      *os << "are " << GetDesc;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "aren't " << GetDesc;
    }
  };
};

class Eq2Matcher : public PairMatchBase<Eq2Matcher, AnyEq> {
 public:
  static const char* Desc() { return "an equal pair"; }
};
class Ne2Matcher : public PairMatchBase<Ne2Matcher, AnyNe> {
 public:
  static const char* Desc() { return "an unequal pair"; }
};
class Lt2Matcher : public PairMatchBase<Lt2Matcher, AnyLt> {
 public:
  static const char* Desc() { return "a pair where the first < the second"; }
};
class Gt2Matcher : public PairMatchBase<Gt2Matcher, AnyGt> {
 public:
  static const char* Desc() { return "a pair where the first > the second"; }
};
class Le2Matcher : public PairMatchBase<Le2Matcher, AnyLe> {
 public:
  static const char* Desc() { return "a pair where the first <= the second"; }
};
class Ge2Matcher : public PairMatchBase<Ge2Matcher, AnyGe> {
 public:
  static const char* Desc() { return "a pair where the first >= the second"; }
};

template <typename T>
class NotMatcherImpl : public MatcherInterface<const T&> {
 public:
  explicit NotMatcherImpl(const Matcher<T>& matcher)
      : matcher_(matcher) {}

  bool MatchAndExplain(const T& x,
                       MatchResultListener* listener) const override {
    return !matcher_.MatchAndExplain(x, listener);
  }

  void DescribeTo(::std::ostream* os) const override {
    matcher_.DescribeNegationTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    matcher_.DescribeTo(os);
  }

 private:
  const Matcher<T> matcher_;
};

template <typename InnerMatcher>
class NotMatcher {
 public:
  explicit NotMatcher(InnerMatcher matcher) : matcher_(matcher) {}

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new NotMatcherImpl<T>(SafeMatcherCast<T>(matcher_)));
  }

 private:
  InnerMatcher matcher_;
};

template <typename T>
class AllOfMatcherImpl : public MatcherInterface<const T&> {
 public:
  explicit AllOfMatcherImpl(std::vector<Matcher<T> > matchers)
      : matchers_(std::move(matchers)) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") and (";
      matchers_[i].DescribeTo(os);
    }
    *os << ")";
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") or (";
      matchers_[i].DescribeNegationTo(os);
    }
    *os << ")";
  }

  bool MatchAndExplain(const T& x,
                       MatchResultListener* listener) const override {

    std::string all_match_result;

    for (size_t i = 0; i < matchers_.size(); ++i) {
      StringMatchResultListener slistener;
      if (matchers_[i].MatchAndExplain(x, &slistener)) {
        if (all_match_result.empty()) {
          all_match_result = slistener.str();
        } else {
          std::string result = slistener.str();
          if (!result.empty()) {
            all_match_result += ", and ";
            all_match_result += result;
          }
        }
      } else {
        *listener << slistener.str();
        return false;
      }
    }

    *listener << all_match_result;
    return true;
  }

 private:
  const std::vector<Matcher<T> > matchers_;
};

template <template <typename T> class CombiningMatcher, typename... Args>
class VariadicMatcher {
 public:
  VariadicMatcher(const Args&... matchers)
      : matchers_(matchers...) {
    static_assert(sizeof...(Args) > 0, "Must have at least one matcher.");
  }

  VariadicMatcher(const VariadicMatcher&) = default;
  VariadicMatcher& operator=(const VariadicMatcher&) = delete;

  template <typename T>
  operator Matcher<T>() const {
    std::vector<Matcher<T> > values;
    CreateVariadicMatcher<T>(&values, std::integral_constant<size_t, 0>());
    return Matcher<T>(new CombiningMatcher<T>(std::move(values)));
  }

 private:
  template <typename T, size_t I>
  void CreateVariadicMatcher(std::vector<Matcher<T> >* values,
                             std::integral_constant<size_t, I>) const {
    values->push_back(SafeMatcherCast<T>(std::get<I>(matchers_)));
    CreateVariadicMatcher<T>(values, std::integral_constant<size_t, I + 1>());
  }

  template <typename T>
  void CreateVariadicMatcher(
      std::vector<Matcher<T> >*,
      std::integral_constant<size_t, sizeof...(Args)>) const {}

  std::tuple<Args...> matchers_;
};

template <typename... Args>
using AllOfMatcher = VariadicMatcher<AllOfMatcherImpl, Args...>;

template <typename T>
class AnyOfMatcherImpl : public MatcherInterface<const T&> {
 public:
  explicit AnyOfMatcherImpl(std::vector<Matcher<T> > matchers)
      : matchers_(std::move(matchers)) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") or (";
      matchers_[i].DescribeTo(os);
    }
    *os << ")";
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "(";
    for (size_t i = 0; i < matchers_.size(); ++i) {
      if (i != 0) *os << ") and (";
      matchers_[i].DescribeNegationTo(os);
    }
    *os << ")";
  }

  bool MatchAndExplain(const T& x,
                       MatchResultListener* listener) const override {
    std::string no_match_result;

    for (size_t i = 0; i < matchers_.size(); ++i) {
      StringMatchResultListener slistener;
      if (matchers_[i].MatchAndExplain(x, &slistener)) {
        *listener << slistener.str();
        return true;
      } else {
        if (no_match_result.empty()) {
          no_match_result = slistener.str();
        } else {
          std::string result = slistener.str();
          if (!result.empty()) {
            no_match_result += ", and ";
            no_match_result += result;
          }
        }
      }
    }

    *listener << no_match_result;
    return false;
  }

 private:
  const std::vector<Matcher<T> > matchers_;
};

template <typename... Args>
using AnyOfMatcher = VariadicMatcher<AnyOfMatcherImpl, Args...>;

template <template <class> class MatcherImpl, typename T>
class SomeOfArrayMatcher {
 public:

  template <typename Iter>
  SomeOfArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}

  template <typename U>
  operator Matcher<U>() const {
    using RawU = typename std::decay<U>::type;
    std::vector<Matcher<RawU>> matchers;
    for (const auto& matcher : matchers_) {
      matchers.push_back(MatcherCast<RawU>(matcher));
    }
    return Matcher<U>(new MatcherImpl<RawU>(std::move(matchers)));
  }

 private:
  const ::std::vector<T> matchers_;
};

template <typename T>
using AllOfArrayMatcher = SomeOfArrayMatcher<AllOfMatcherImpl, T>;

template <typename T>
using AnyOfArrayMatcher = SomeOfArrayMatcher<AnyOfMatcherImpl, T>;

template <typename Predicate>
class TrulyMatcher {
 public:
  explicit TrulyMatcher(Predicate pred) : predicate_(pred) {}

  template <typename T>
  bool MatchAndExplain(T& x,
                       MatchResultListener* listener) const {

    if (predicate_(x))
      return true;
    *listener << "didn't satisfy the given predicate";
    return false;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "satisfies the given predicate";
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't satisfy the given predicate";
  }

 private:
  Predicate predicate_;
};

template <typename M>
class MatcherAsPredicate {
 public:
  explicit MatcherAsPredicate(M matcher) : matcher_(matcher) {}

  template <typename T>
  bool operator()(const T& x) const {

    return MatcherCast<const T&>(matcher_).Matches(x);
  }

 private:
  M matcher_;
};

template <typename M>
class PredicateFormatterFromMatcher {
 public:
  explicit PredicateFormatterFromMatcher(M m) : matcher_(std::move(m)) {}

  template <typename T>
  AssertionResult operator()(const char* value_text, const T& x) const {

    const Matcher<const T&> matcher = SafeMatcherCast<const T&>(matcher_);

    if (matcher.Matches(x)) {
      return AssertionSuccess();
    }

    ::std::stringstream ss;
    ss << "Value of: " << value_text << "\n"
       << "Expected: ";
    matcher.DescribeTo(&ss);

    StringMatchResultListener listener;
    if (MatchPrintAndExplain(x, matcher, &listener)) {
      ss << "\n  The matcher failed on the initial attempt; but passed when "
            "rerun to generate the explanation.";
    }
    ss << "\n  Actual: " << listener.str();
    return AssertionFailure() << ss.str();
  }

 private:
  const M matcher_;
};

template <typename M>
inline PredicateFormatterFromMatcher<M>
MakePredicateFormatterFromMatcher(M matcher) {
  return PredicateFormatterFromMatcher<M>(std::move(matcher));
}

class IsNanMatcher {
 public:
  template <typename FloatType>
  bool MatchAndExplain(const FloatType& f,
                       MatchResultListener*  ) const {
    return (::std::isnan)(f);
  }

  void DescribeTo(::std::ostream* os) const { *os << "is NaN"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "isn't NaN";
  }
};

template <typename FloatType>
class FloatingEqMatcher {
 public:

  FloatingEqMatcher(FloatType expected, bool nan_eq_nan) :
    expected_(expected), nan_eq_nan_(nan_eq_nan), max_abs_error_(-1) {
  }

  FloatingEqMatcher(FloatType expected, bool nan_eq_nan,
                    FloatType max_abs_error)
      : expected_(expected),
        nan_eq_nan_(nan_eq_nan),
        max_abs_error_(max_abs_error) {
    GTEST_CHECK_(max_abs_error >= 0)
        << ", where max_abs_error is" << max_abs_error;
  }

  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    Impl(FloatType expected, bool nan_eq_nan, FloatType max_abs_error)
        : expected_(expected),
          nan_eq_nan_(nan_eq_nan),
          max_abs_error_(max_abs_error) {}

    bool MatchAndExplain(T value,
                         MatchResultListener* listener) const override {
      const FloatingPoint<FloatType> actual(value), expected(expected_);

      if (actual.is_nan() || expected.is_nan()) {
        if (actual.is_nan() && expected.is_nan()) {
          return nan_eq_nan_;
        }

        return false;
      }
      if (HasMaxAbsError()) {

        if (value == expected_) {
          return true;
        }

        const FloatType diff = value - expected_;
        if (::std::fabs(diff) <= max_abs_error_) {
          return true;
        }

        if (listener->IsInterested()) {
          *listener << "which is " << diff << " from " << expected_;
        }
        return false;
      } else {
        return actual.AlmostEquals(expected);
      }
    }

    void DescribeTo(::std::ostream* os) const override {

      const ::std::streamsize old_precision = os->precision(
          ::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(expected_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "is NaN";
        } else {
          *os << "never matches";
        }
      } else {
        *os << "is approximately " << expected_;
        if (HasMaxAbsError()) {
          *os << " (absolute error <= " << max_abs_error_ << ")";
        }
      }
      os->precision(old_precision);
    }

    void DescribeNegationTo(::std::ostream* os) const override {

      const ::std::streamsize old_precision = os->precision(
          ::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(expected_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "isn't NaN";
        } else {
          *os << "is anything";
        }
      } else {
        *os << "isn't approximately " << expected_;
        if (HasMaxAbsError()) {
          *os << " (absolute error > " << max_abs_error_ << ")";
        }
      }

      os->precision(old_precision);
    }

   private:
    bool HasMaxAbsError() const {
      return max_abs_error_ >= 0;
    }

    const FloatType expected_;
    const bool nan_eq_nan_;

    const FloatType max_abs_error_;
  };

  operator Matcher<FloatType>() const {
    return MakeMatcher(
        new Impl<FloatType>(expected_, nan_eq_nan_, max_abs_error_));
  }

  operator Matcher<const FloatType&>() const {
    return MakeMatcher(
        new Impl<const FloatType&>(expected_, nan_eq_nan_, max_abs_error_));
  }

  operator Matcher<FloatType&>() const {
    return MakeMatcher(
        new Impl<FloatType&>(expected_, nan_eq_nan_, max_abs_error_));
  }

 private:
  const FloatType expected_;
  const bool nan_eq_nan_;

  const FloatType max_abs_error_;
};

template <typename FloatType>
class FloatingEq2Matcher {
 public:
  FloatingEq2Matcher() { Init(-1, false); }

  explicit FloatingEq2Matcher(bool nan_eq_nan) { Init(-1, nan_eq_nan); }

  explicit FloatingEq2Matcher(FloatType max_abs_error) {
    Init(max_abs_error, false);
  }

  FloatingEq2Matcher(FloatType max_abs_error, bool nan_eq_nan) {
    Init(max_abs_error, nan_eq_nan);
  }

  template <typename T1, typename T2>
  operator Matcher<::std::tuple<T1, T2>>() const {
    return MakeMatcher(
        new Impl<::std::tuple<T1, T2>>(max_abs_error_, nan_eq_nan_));
  }
  template <typename T1, typename T2>
  operator Matcher<const ::std::tuple<T1, T2>&>() const {
    return MakeMatcher(
        new Impl<const ::std::tuple<T1, T2>&>(max_abs_error_, nan_eq_nan_));
  }

 private:
  static ::std::ostream& GetDesc(::std::ostream& os) {
    return os << "an almost-equal pair";
  }

  template <typename Tuple>
  class Impl : public MatcherInterface<Tuple> {
   public:
    Impl(FloatType max_abs_error, bool nan_eq_nan) :
        max_abs_error_(max_abs_error),
        nan_eq_nan_(nan_eq_nan) {}

    bool MatchAndExplain(Tuple args,
                         MatchResultListener* listener) const override {
      if (max_abs_error_ == -1) {
        FloatingEqMatcher<FloatType> fm(::std::get<0>(args), nan_eq_nan_);
        return static_cast<Matcher<FloatType>>(fm).MatchAndExplain(
            ::std::get<1>(args), listener);
      } else {
        FloatingEqMatcher<FloatType> fm(::std::get<0>(args), nan_eq_nan_,
                                        max_abs_error_);
        return static_cast<Matcher<FloatType>>(fm).MatchAndExplain(
            ::std::get<1>(args), listener);
      }
    }
    void DescribeTo(::std::ostream* os) const override {
      *os << "are " << GetDesc;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "aren't " << GetDesc;
    }

   private:
    FloatType max_abs_error_;
    const bool nan_eq_nan_;
  };

  void Init(FloatType max_abs_error_val, bool nan_eq_nan_val) {
    max_abs_error_ = max_abs_error_val;
    nan_eq_nan_ = nan_eq_nan_val;
  }
  FloatType max_abs_error_;
  bool nan_eq_nan_;
};

template <typename InnerMatcher>
class PointeeMatcher {
 public:
  explicit PointeeMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}

  template <typename Pointer>
  operator Matcher<Pointer>() const {
    return Matcher<Pointer>(new Impl<const Pointer&>(matcher_));
  }

 private:

  template <typename Pointer>
  class Impl : public MatcherInterface<Pointer> {
   public:
    using Pointee =
        typename std::pointer_traits<GTEST_REMOVE_REFERENCE_AND_CONST_(
            Pointer)>::element_type;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<const Pointee&>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "points to a value that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "does not point to a value that ";
      matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(Pointer pointer,
                         MatchResultListener* listener) const override {
      if (GetRawPointer(pointer) == nullptr) return false;

      *listener << "which points to ";
      return MatchPrintAndExplain(*pointer, matcher_, listener);
    }

   private:
    const Matcher<const Pointee&> matcher_;
  };

  const InnerMatcher matcher_;
};

template <typename InnerMatcher>
class PointerMatcher {
 public:
  explicit PointerMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}

  template <typename PointerType>
  operator Matcher<PointerType>() const {
    return Matcher<PointerType>(new Impl<const PointerType&>(matcher_));
  }

 private:

  template <typename PointerType>
  class Impl : public MatcherInterface<PointerType> {
   public:
    using Pointer =
        const typename std::pointer_traits<GTEST_REMOVE_REFERENCE_AND_CONST_(
            PointerType)>::element_type*;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<Pointer>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "is a pointer that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "is not a pointer that ";
      matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(PointerType pointer,
                         MatchResultListener* listener) const override {
      *listener << "which is a pointer that ";
      Pointer p = GetRawPointer(pointer);
      return MatchPrintAndExplain(p, matcher_, listener);
    }

   private:
    Matcher<Pointer> matcher_;
  };

  const InnerMatcher matcher_;
};

#if GTEST_HAS_RTTI

template <typename To>
class WhenDynamicCastToMatcherBase {
 public:
  explicit WhenDynamicCastToMatcherBase(const Matcher<To>& matcher)
      : matcher_(matcher) {}

  void DescribeTo(::std::ostream* os) const {
    GetCastTypeDescription(os);
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    GetCastTypeDescription(os);
    matcher_.DescribeNegationTo(os);
  }

 protected:
  const Matcher<To> matcher_;

  static std::string GetToName() {
    return GetTypeName<To>();
  }

 private:
  static void GetCastTypeDescription(::std::ostream* os) {
    *os << "when dynamic_cast to " << GetToName() << ", ";
  }
};

template <typename To>
class WhenDynamicCastToMatcher : public WhenDynamicCastToMatcherBase<To> {
 public:
  explicit WhenDynamicCastToMatcher(const Matcher<To>& matcher)
      : WhenDynamicCastToMatcherBase<To>(matcher) {}

  template <typename From>
  bool MatchAndExplain(From from, MatchResultListener* listener) const {
    To to = dynamic_cast<To>(from);
    return MatchPrintAndExplain(to, this->matcher_, listener);
  }
};

template <typename To>
class WhenDynamicCastToMatcher<To&> : public WhenDynamicCastToMatcherBase<To&> {
 public:
  explicit WhenDynamicCastToMatcher(const Matcher<To&>& matcher)
      : WhenDynamicCastToMatcherBase<To&>(matcher) {}

  template <typename From>
  bool MatchAndExplain(From& from, MatchResultListener* listener) const {

    To* to = dynamic_cast<To*>(&from);
    if (to == nullptr) {
      *listener << "which cannot be dynamic_cast to " << this->GetToName();
      return false;
    }
    return MatchPrintAndExplain(*to, this->matcher_, listener);
  }
};
#endif

template <typename Class, typename FieldType>
class FieldMatcher {
 public:
  FieldMatcher(FieldType Class::*field,
               const Matcher<const FieldType&>& matcher)
      : field_(field), matcher_(matcher), whose_field_("whose given field ") {}

  FieldMatcher(const std::string& field_name, FieldType Class::*field,
               const Matcher<const FieldType&>& matcher)
      : field_(field),
        matcher_(matcher),
        whose_field_("whose field `" + field_name + "` ") {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object " << whose_field_;
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object " << whose_field_;
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T& value, MatchResultListener* listener) const {

    return MatchAndExplainImpl(
        typename std::is_pointer<typename std::remove_const<T>::type>::type(),
        value, listener);
  }

 private:
  bool MatchAndExplainImpl(std::false_type  ,
                           const Class& obj,
                           MatchResultListener* listener) const {
    *listener << whose_field_ << "is ";
    return MatchPrintAndExplain(obj.*field_, matcher_, listener);
  }

  bool MatchAndExplainImpl(std::true_type  , const Class* p,
                           MatchResultListener* listener) const {
    if (p == nullptr) return false;

    *listener << "which points to an object ";

    return MatchAndExplainImpl(std::false_type(), *p, listener);
  }

  const FieldType Class::*field_;
  const Matcher<const FieldType&> matcher_;

  const std::string whose_field_;
};

template <typename Class, typename PropertyType, typename Property>
class PropertyMatcher {
 public:
  typedef const PropertyType& RefToConstProperty;

  PropertyMatcher(Property property, const Matcher<RefToConstProperty>& matcher)
      : property_(property),
        matcher_(matcher),
        whose_property_("whose given property ") {}

  PropertyMatcher(const std::string& property_name, Property property,
                  const Matcher<RefToConstProperty>& matcher)
      : property_(property),
        matcher_(matcher),
        whose_property_("whose property `" + property_name + "` ") {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object " << whose_property_;
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object " << whose_property_;
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T&value, MatchResultListener* listener) const {
    return MatchAndExplainImpl(
        typename std::is_pointer<typename std::remove_const<T>::type>::type(),
        value, listener);
  }

 private:
  bool MatchAndExplainImpl(std::false_type  ,
                           const Class& obj,
                           MatchResultListener* listener) const {
    *listener << whose_property_ << "is ";

    RefToConstProperty result = (obj.*property_)();
    return MatchPrintAndExplain(result, matcher_, listener);
  }

  bool MatchAndExplainImpl(std::true_type  , const Class* p,
                           MatchResultListener* listener) const {
    if (p == nullptr) return false;

    *listener << "which points to an object ";

    return MatchAndExplainImpl(std::false_type(), *p, listener);
  }

  Property property_;
  const Matcher<RefToConstProperty> matcher_;

  const std::string whose_property_;
};

template <typename Functor>
struct CallableTraits {
  typedef Functor StorageType;

  static void CheckIsValid(Functor  ) {}

  template <typename T>
  static auto Invoke(Functor f, const T& arg) -> decltype(f(arg)) {
    return f(arg);
  }
};

template <typename ArgType, typename ResType>
struct CallableTraits<ResType(*)(ArgType)> {
  typedef ResType ResultType;
  typedef ResType(*StorageType)(ArgType);

  static void CheckIsValid(ResType(*f)(ArgType)) {
    GTEST_CHECK_(f != nullptr)
        << "NULL function pointer is passed into ResultOf().";
  }
  template <typename T>
  static ResType Invoke(ResType(*f)(ArgType), T arg) {
    return (*f)(arg);
  }
};

template <typename Callable, typename InnerMatcher>
class ResultOfMatcher {
 public:
  ResultOfMatcher(Callable callable, InnerMatcher matcher)
      : callable_(std::move(callable)), matcher_(std::move(matcher)) {
    CallableTraits<Callable>::CheckIsValid(callable_);
  }

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new Impl<const T&>(callable_, matcher_));
  }

 private:
  typedef typename CallableTraits<Callable>::StorageType CallableStorageType;

  template <typename T>
  class Impl : public MatcherInterface<T> {
    using ResultType = decltype(CallableTraits<Callable>::template Invoke<T>(
        std::declval<CallableStorageType>(), std::declval<T>()));

   public:
    template <typename M>
    Impl(const CallableStorageType& callable, const M& matcher)
        : callable_(callable), matcher_(MatcherCast<ResultType>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "is mapped by the given callable to a value that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "is mapped by the given callable to a value that ";
      matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(T obj, MatchResultListener* listener) const override {
      *listener << "which is mapped by the given callable to ";

      ResultType result =
          CallableTraits<Callable>::template Invoke<T>(callable_, obj);
      return MatchPrintAndExplain(result, matcher_, listener);
    }

   private:

    mutable CallableStorageType callable_;
    const Matcher<ResultType> matcher_;
  };

  const CallableStorageType callable_;
  const InnerMatcher matcher_;
};

template <typename SizeMatcher>
class SizeIsMatcher {
 public:
  explicit SizeIsMatcher(const SizeMatcher& size_matcher)
       : size_matcher_(size_matcher) {
  }

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(new Impl<const Container&>(size_matcher_));
  }

  template <typename Container>
  class Impl : public MatcherInterface<Container> {
   public:
    using SizeType = decltype(std::declval<Container>().size());
    explicit Impl(const SizeMatcher& size_matcher)
        : size_matcher_(MatcherCast<SizeType>(size_matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "size ";
      size_matcher_.DescribeTo(os);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "size ";
      size_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(Container container,
                         MatchResultListener* listener) const override {
      SizeType size = container.size();
      StringMatchResultListener size_listener;
      const bool result = size_matcher_.MatchAndExplain(size, &size_listener);
      *listener
          << "whose size " << size << (result ? " matches" : " doesn't match");
      PrintIfNotEmpty(size_listener.str(), listener->stream());
      return result;
    }

   private:
    const Matcher<SizeType> size_matcher_;
  };

 private:
  const SizeMatcher size_matcher_;
};

template <typename DistanceMatcher>
class BeginEndDistanceIsMatcher {
 public:
  explicit BeginEndDistanceIsMatcher(const DistanceMatcher& distance_matcher)
      : distance_matcher_(distance_matcher) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(new Impl<const Container&>(distance_matcher_));
  }

  template <typename Container>
  class Impl : public MatcherInterface<Container> {
   public:
    typedef internal::StlContainerView<
        GTEST_REMOVE_REFERENCE_AND_CONST_(Container)> ContainerView;
    typedef typename std::iterator_traits<
        typename ContainerView::type::const_iterator>::difference_type
        DistanceType;
    explicit Impl(const DistanceMatcher& distance_matcher)
        : distance_matcher_(MatcherCast<DistanceType>(distance_matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "distance between begin() and end() ";
      distance_matcher_.DescribeTo(os);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "distance between begin() and end() ";
      distance_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(Container container,
                         MatchResultListener* listener) const override {
      using std::begin;
      using std::end;
      DistanceType distance = std::distance(begin(container), end(container));
      StringMatchResultListener distance_listener;
      const bool result =
          distance_matcher_.MatchAndExplain(distance, &distance_listener);
      *listener << "whose distance between begin() and end() " << distance
                << (result ? " matches" : " doesn't match");
      PrintIfNotEmpty(distance_listener.str(), listener->stream());
      return result;
    }

   private:
    const Matcher<DistanceType> distance_matcher_;
  };

 private:
  const DistanceMatcher distance_matcher_;
};

template <typename Container>
class ContainerEqMatcher {
 public:
  typedef internal::StlContainerView<Container> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;

  static_assert(!std::is_const<Container>::value,
                "Container type must not be const");
  static_assert(!std::is_reference<Container>::value,
                "Container type must not be a reference");

  explicit ContainerEqMatcher(const Container& expected)
      : expected_(View::Copy(expected)) {}

  void DescribeTo(::std::ostream* os) const {
    *os << "equals ";
    UniversalPrint(expected_, os);
  }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not equal ";
    UniversalPrint(expected_, os);
  }

  template <typename LhsContainer>
  bool MatchAndExplain(const LhsContainer& lhs,
                       MatchResultListener* listener) const {
    typedef internal::StlContainerView<
        typename std::remove_const<LhsContainer>::type>
        LhsView;
    typedef typename LhsView::type LhsStlContainer;
    StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
    if (lhs_stl_container == expected_)
      return true;

    ::std::ostream* const os = listener->stream();
    if (os != nullptr) {

      bool printed_header = false;
      for (typename LhsStlContainer::const_iterator it =
               lhs_stl_container.begin();
           it != lhs_stl_container.end(); ++it) {
        if (internal::ArrayAwareFind(expected_.begin(), expected_.end(), *it) ==
            expected_.end()) {
          if (printed_header) {
            *os << ", ";
          } else {
            *os << "which has these unexpected elements: ";
            printed_header = true;
          }
          UniversalPrint(*it, os);
        }
      }

      bool printed_header2 = false;
      for (typename StlContainer::const_iterator it = expected_.begin();
           it != expected_.end(); ++it) {
        if (internal::ArrayAwareFind(
                lhs_stl_container.begin(), lhs_stl_container.end(), *it) ==
            lhs_stl_container.end()) {
          if (printed_header2) {
            *os << ", ";
          } else {
            *os << (printed_header ? ",\nand" : "which")
                << " doesn't have these expected elements: ";
            printed_header2 = true;
          }
          UniversalPrint(*it, os);
        }
      }
    }

    return false;
  }

 private:
  const StlContainer expected_;
};

struct LessComparator {
  template <typename T, typename U>
  bool operator()(const T& lhs, const U& rhs) const { return lhs < rhs; }
};

template <typename Comparator, typename ContainerMatcher>
class WhenSortedByMatcher {
 public:
  WhenSortedByMatcher(const Comparator& comparator,
                      const ContainerMatcher& matcher)
      : comparator_(comparator), matcher_(matcher) {}

  template <typename LhsContainer>
  operator Matcher<LhsContainer>() const {
    return MakeMatcher(new Impl<LhsContainer>(comparator_, matcher_));
  }

  template <typename LhsContainer>
  class Impl : public MatcherInterface<LhsContainer> {
   public:
    typedef internal::StlContainerView<
         GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)> LhsView;
    typedef typename LhsView::type LhsStlContainer;
    typedef typename LhsView::const_reference LhsStlContainerReference;

    typedef typename RemoveConstFromKey<
        typename LhsStlContainer::value_type>::type LhsValue;

    Impl(const Comparator& comparator, const ContainerMatcher& matcher)
        : comparator_(comparator), matcher_(matcher) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "(when sorted) ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "(when sorted) ";
      matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(LhsContainer lhs,
                         MatchResultListener* listener) const override {
      LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
      ::std::vector<LhsValue> sorted_container(lhs_stl_container.begin(),
                                               lhs_stl_container.end());
      ::std::sort(
           sorted_container.begin(), sorted_container.end(), comparator_);

      if (!listener->IsInterested()) {

        return matcher_.Matches(sorted_container);
      }

      *listener << "which is ";
      UniversalPrint(sorted_container, listener->stream());
      *listener << " when sorted";

      StringMatchResultListener inner_listener;
      const bool match = matcher_.MatchAndExplain(sorted_container,
                                                  &inner_listener);
      PrintIfNotEmpty(inner_listener.str(), listener->stream());
      return match;
    }

   private:
    const Comparator comparator_;
    const Matcher<const ::std::vector<LhsValue>&> matcher_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(Impl);
  };

 private:
  const Comparator comparator_;
  const ContainerMatcher matcher_;
};

template <typename TupleMatcher, typename RhsContainer>
class PointwiseMatcher {
  GTEST_COMPILE_ASSERT_(
      !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(RhsContainer)>::value,
      use_UnorderedPointwise_with_hash_tables);

 public:
  typedef internal::StlContainerView<RhsContainer> RhsView;
  typedef typename RhsView::type RhsStlContainer;
  typedef typename RhsStlContainer::value_type RhsValue;

  static_assert(!std::is_const<RhsContainer>::value,
                "RhsContainer type must not be const");
  static_assert(!std::is_reference<RhsContainer>::value,
                "RhsContainer type must not be a reference");

  PointwiseMatcher(const TupleMatcher& tuple_matcher, const RhsContainer& rhs)
      : tuple_matcher_(tuple_matcher), rhs_(RhsView::Copy(rhs)) {}

  template <typename LhsContainer>
  operator Matcher<LhsContainer>() const {
    GTEST_COMPILE_ASSERT_(
        !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)>::value,
        use_UnorderedPointwise_with_hash_tables);

    return Matcher<LhsContainer>(
        new Impl<const LhsContainer&>(tuple_matcher_, rhs_));
  }

  template <typename LhsContainer>
  class Impl : public MatcherInterface<LhsContainer> {
   public:
    typedef internal::StlContainerView<
         GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)> LhsView;
    typedef typename LhsView::type LhsStlContainer;
    typedef typename LhsView::const_reference LhsStlContainerReference;
    typedef typename LhsStlContainer::value_type LhsValue;

    typedef ::std::tuple<const LhsValue&, const RhsValue&> InnerMatcherArg;

    Impl(const TupleMatcher& tuple_matcher, const RhsStlContainer& rhs)

        : mono_tuple_matcher_(SafeMatcherCast<InnerMatcherArg>(tuple_matcher)),
          rhs_(rhs) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "contains " << rhs_.size()
          << " values, where each value and its corresponding value in ";
      UniversalPrinter<RhsStlContainer>::Print(rhs_, os);
      *os << " ";
      mono_tuple_matcher_.DescribeTo(os);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "doesn't contain exactly " << rhs_.size()
          << " values, or contains a value x at some index i"
          << " where x and the i-th value of ";
      UniversalPrint(rhs_, os);
      *os << " ";
      mono_tuple_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(LhsContainer lhs,
                         MatchResultListener* listener) const override {
      LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
      const size_t actual_size = lhs_stl_container.size();
      if (actual_size != rhs_.size()) {
        *listener << "which contains " << actual_size << " values";
        return false;
      }

      typename LhsStlContainer::const_iterator left = lhs_stl_container.begin();
      typename RhsStlContainer::const_iterator right = rhs_.begin();
      for (size_t i = 0; i != actual_size; ++i, ++left, ++right) {
        if (listener->IsInterested()) {
          StringMatchResultListener inner_listener;

          if (!mono_tuple_matcher_.MatchAndExplain(
                  InnerMatcherArg(ImplicitCast_<const LhsValue&>(*left),
                                  ImplicitCast_<const RhsValue&>(*right)),
                  &inner_listener)) {
            *listener << "where the value pair (";
            UniversalPrint(*left, listener->stream());
            *listener << ", ";
            UniversalPrint(*right, listener->stream());
            *listener << ") at index #" << i << " don't match";
            PrintIfNotEmpty(inner_listener.str(), listener->stream());
            return false;
          }
        } else {
          if (!mono_tuple_matcher_.Matches(
                  InnerMatcherArg(ImplicitCast_<const LhsValue&>(*left),
                                  ImplicitCast_<const RhsValue&>(*right))))
            return false;
        }
      }

      return true;
    }

   private:
    const Matcher<InnerMatcherArg> mono_tuple_matcher_;
    const RhsStlContainer rhs_;
  };

 private:
  const TupleMatcher tuple_matcher_;
  const RhsStlContainer rhs_;
};

template <typename Container>
class QuantifierMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InnerMatcher>
  explicit QuantifierMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
           testing::SafeMatcherCast<const Element&>(inner_matcher)) {}

  bool MatchAndExplainImpl(bool all_elements_should_match,
                           Container container,
                           MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    size_t i = 0;
    for (typename StlContainer::const_iterator it = stl_container.begin();
         it != stl_container.end(); ++it, ++i) {
      StringMatchResultListener inner_listener;
      const bool matches = inner_matcher_.MatchAndExplain(*it, &inner_listener);

      if (matches != all_elements_should_match) {
        *listener << "whose element #" << i
                  << (matches ? " matches" : " doesn't match");
        PrintIfNotEmpty(inner_listener.str(), listener->stream());
        return !all_elements_should_match;
      }
    }
    return all_elements_should_match;
  }

 protected:
  const Matcher<const Element&> inner_matcher_;
};

template <typename Container>
class ContainsMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit ContainsMatcherImpl(InnerMatcher inner_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "contains at least one element that ";
    this->inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "doesn't contain any element that ";
    this->inner_matcher_.DescribeTo(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    return this->MatchAndExplainImpl(false, container, listener);
  }
};

template <typename Container>
class EachMatcherImpl : public QuantifierMatcherImpl<Container> {
 public:
  template <typename InnerMatcher>
  explicit EachMatcherImpl(InnerMatcher inner_matcher)
      : QuantifierMatcherImpl<Container>(inner_matcher) {}

  void DescribeTo(::std::ostream* os) const override {
    *os << "only contains elements that ";
    this->inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "contains some element that ";
    this->inner_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    return this->MatchAndExplainImpl(true, container, listener);
  }
};

template <typename M>
class ContainsMatcher {
 public:
  explicit ContainsMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(
        new ContainsMatcherImpl<const Container&>(inner_matcher_));
  }

 private:
  const M inner_matcher_;
};

template <typename M>
class EachMatcher {
 public:
  explicit EachMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(
        new EachMatcherImpl<const Container&>(inner_matcher_));
  }

 private:
  const M inner_matcher_;
};

struct Rank1 {};
struct Rank0 : Rank1 {};

namespace pair_getters {
using std::get;
template <typename T>
auto First(T& x, Rank1) -> decltype(get<0>(x)) {
  return get<0>(x);
}
template <typename T>
auto First(T& x, Rank0) -> decltype((x.first)) {
  return x.first;
}

template <typename T>
auto Second(T& x, Rank1) -> decltype(get<1>(x)) {
  return get<1>(x);
}
template <typename T>
auto Second(T& x, Rank0) -> decltype((x.second)) {
  return x.second;
}
}

template <typename PairType>
class KeyMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
  typedef typename RawPairType::first_type KeyType;

  template <typename InnerMatcher>
  explicit KeyMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
          testing::SafeMatcherCast<const KeyType&>(inner_matcher)) {
  }

  bool MatchAndExplain(PairType key_value,
                       MatchResultListener* listener) const override {
    StringMatchResultListener inner_listener;
    const bool match = inner_matcher_.MatchAndExplain(
        pair_getters::First(key_value, Rank0()), &inner_listener);
    const std::string explanation = inner_listener.str();
    if (explanation != "") {
      *listener << "whose first field is a value " << explanation;
    }
    return match;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "has a key that ";
    inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "doesn't have a key that ";
    inner_matcher_.DescribeTo(os);
  }

 private:
  const Matcher<const KeyType&> inner_matcher_;
};

template <typename M>
class KeyMatcher {
 public:
  explicit KeyMatcher(M m) : matcher_for_key_(m) {}

  template <typename PairType>
  operator Matcher<PairType>() const {
    return Matcher<PairType>(
        new KeyMatcherImpl<const PairType&>(matcher_for_key_));
  }

 private:
  const M matcher_for_key_;
};

template <typename InnerMatcher>
class AddressMatcher {
 public:
  explicit AddressMatcher(InnerMatcher m) : matcher_(m) {}

  template <typename Type>
  operator Matcher<Type>() const {
    return Matcher<Type>(new Impl<const Type&>(matcher_));
  }

 private:

  template <typename Type>
  class Impl : public MatcherInterface<Type> {
   public:
    using Address = const GTEST_REMOVE_REFERENCE_AND_CONST_(Type) *;
    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<Address>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "has address that ";
      matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "does not have address that ";
      matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(Type object,
                         MatchResultListener* listener) const override {
      *listener << "which has address ";
      Address address = std::addressof(object);
      return MatchPrintAndExplain(address, matcher_, listener);
    }

   private:
    const Matcher<Address> matcher_;
  };
  const InnerMatcher matcher_;
};

template <typename PairType>
class PairMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
  typedef typename RawPairType::first_type FirstType;
  typedef typename RawPairType::second_type SecondType;

  template <typename FirstMatcher, typename SecondMatcher>
  PairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(
            testing::SafeMatcherCast<const FirstType&>(first_matcher)),
        second_matcher_(
            testing::SafeMatcherCast<const SecondType&>(second_matcher)) {
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "has a first field that ";
    first_matcher_.DescribeTo(os);
    *os << ", and has a second field that ";
    second_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "has a first field that ";
    first_matcher_.DescribeNegationTo(os);
    *os << ", or has a second field that ";
    second_matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(PairType a_pair,
                       MatchResultListener* listener) const override {
    if (!listener->IsInterested()) {

      return first_matcher_.Matches(pair_getters::First(a_pair, Rank0())) &&
             second_matcher_.Matches(pair_getters::Second(a_pair, Rank0()));
    }
    StringMatchResultListener first_inner_listener;
    if (!first_matcher_.MatchAndExplain(pair_getters::First(a_pair, Rank0()),
                                        &first_inner_listener)) {
      *listener << "whose first field does not match";
      PrintIfNotEmpty(first_inner_listener.str(), listener->stream());
      return false;
    }
    StringMatchResultListener second_inner_listener;
    if (!second_matcher_.MatchAndExplain(pair_getters::Second(a_pair, Rank0()),
                                         &second_inner_listener)) {
      *listener << "whose second field does not match";
      PrintIfNotEmpty(second_inner_listener.str(), listener->stream());
      return false;
    }
    ExplainSuccess(first_inner_listener.str(), second_inner_listener.str(),
                   listener);
    return true;
  }

 private:
  void ExplainSuccess(const std::string& first_explanation,
                      const std::string& second_explanation,
                      MatchResultListener* listener) const {
    *listener << "whose both fields match";
    if (first_explanation != "") {
      *listener << ", where the first field is a value " << first_explanation;
    }
    if (second_explanation != "") {
      *listener << ", ";
      if (first_explanation != "") {
        *listener << "and ";
      } else {
        *listener << "where ";
      }
      *listener << "the second field is a value " << second_explanation;
    }
  }

  const Matcher<const FirstType&> first_matcher_;
  const Matcher<const SecondType&> second_matcher_;
};

template <typename FirstMatcher, typename SecondMatcher>
class PairMatcher {
 public:
  PairMatcher(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  template <typename PairType>
  operator Matcher<PairType> () const {
    return Matcher<PairType>(
        new PairMatcherImpl<const PairType&>(first_matcher_, second_matcher_));
  }

 private:
  const FirstMatcher first_matcher_;
  const SecondMatcher second_matcher_;
};

template <typename T, size_t... I>
auto UnpackStructImpl(const T& t, IndexSequence<I...>, int)
    -> decltype(std::tie(get<I>(t)...)) {
  static_assert(std::tuple_size<T>::value == sizeof...(I),
                "Number of arguments doesn't match the number of fields.");
  return std::tie(get<I>(t)...);
}

#if defined(__cpp_structured_bindings) && __cpp_structured_bindings >= 201606
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<1>, char) {
  const auto& [a] = t;
  return std::tie(a);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<2>, char) {
  const auto& [a, b] = t;
  return std::tie(a, b);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<3>, char) {
  const auto& [a, b, c] = t;
  return std::tie(a, b, c);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<4>, char) {
  const auto& [a, b, c, d] = t;
  return std::tie(a, b, c, d);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<5>, char) {
  const auto& [a, b, c, d, e] = t;
  return std::tie(a, b, c, d, e);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<6>, char) {
  const auto& [a, b, c, d, e, f] = t;
  return std::tie(a, b, c, d, e, f);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<7>, char) {
  const auto& [a, b, c, d, e, f, g] = t;
  return std::tie(a, b, c, d, e, f, g);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<8>, char) {
  const auto& [a, b, c, d, e, f, g, h] = t;
  return std::tie(a, b, c, d, e, f, g, h);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<9>, char) {
  const auto& [a, b, c, d, e, f, g, h, i] = t;
  return std::tie(a, b, c, d, e, f, g, h, i);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<10>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<11>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<12>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<13>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<14>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<15>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
}
template <typename T>
auto UnpackStructImpl(const T& t, MakeIndexSequence<16>, char) {
  const auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p] = t;
  return std::tie(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
}
#endif

template <size_t I, typename T>
auto UnpackStruct(const T& t)
    -> decltype((UnpackStructImpl)(t, MakeIndexSequence<I>{}, 0)) {
  return (UnpackStructImpl)(t, MakeIndexSequence<I>{}, 0);
}

template <typename T, size_t N>
void VariadicExpand(const T (&)[N]) {}

template <typename Struct, typename StructSize>
class FieldsAreMatcherImpl;

template <typename Struct, size_t... I>
class FieldsAreMatcherImpl<Struct, IndexSequence<I...>>
    : public MatcherInterface<Struct> {
  using UnpackedType =
      decltype(UnpackStruct<sizeof...(I)>(std::declval<const Struct&>()));
  using MatchersType = std::tuple<
      Matcher<const typename std::tuple_element<I, UnpackedType>::type&>...>;

 public:
  template <typename Inner>
  explicit FieldsAreMatcherImpl(const Inner& matchers)
      : matchers_(testing::SafeMatcherCast<
                  const typename std::tuple_element<I, UnpackedType>::type&>(
            std::get<I>(matchers))...) {}

  void DescribeTo(::std::ostream* os) const override {
    const char* separator = "";
    VariadicExpand(
        {(*os << separator << "has field #" << I << " that ",
          std::get<I>(matchers_).DescribeTo(os), separator = ", and ")...});
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    const char* separator = "";
    VariadicExpand({(*os << separator << "has field #" << I << " that ",
                     std::get<I>(matchers_).DescribeNegationTo(os),
                     separator = ", or ")...});
  }

  bool MatchAndExplain(Struct t, MatchResultListener* listener) const override {
    return MatchInternal((UnpackStruct<sizeof...(I)>)(t), listener);
  }

 private:
  bool MatchInternal(UnpackedType tuple, MatchResultListener* listener) const {
    if (!listener->IsInterested()) {

      bool good = true;
      VariadicExpand({good = good && std::get<I>(matchers_).Matches(
                                         std::get<I>(tuple))...});
      return good;
    }

    size_t failed_pos = ~size_t{};

    std::vector<StringMatchResultListener> inner_listener(sizeof...(I));

    VariadicExpand(
        {failed_pos == ~size_t{} && !std::get<I>(matchers_).MatchAndExplain(
                                        std::get<I>(tuple), &inner_listener[I])
             ? failed_pos = I
             : 0 ...});
    if (failed_pos != ~size_t{}) {
      *listener << "whose field #" << failed_pos << " does not match";
      PrintIfNotEmpty(inner_listener[failed_pos].str(), listener->stream());
      return false;
    }

    *listener << "whose all elements match";
    const char* separator = ", where";
    for (size_t index = 0; index < sizeof...(I); ++index) {
      const std::string str = inner_listener[index].str();
      if (!str.empty()) {
        *listener << separator << " field #" << index << " is a value " << str;
        separator = ", and";
      }
    }

    return true;
  }

  MatchersType matchers_;
};

template <typename... Inner>
class FieldsAreMatcher {
 public:
  explicit FieldsAreMatcher(Inner... inner) : matchers_(std::move(inner)...) {}

  template <typename Struct>
  operator Matcher<Struct>() const {
    return Matcher<Struct>(
        new FieldsAreMatcherImpl<const Struct&, IndexSequenceFor<Inner...>>(
            matchers_));
  }

 private:
  std::tuple<Inner...> matchers_;
};

template <typename Container>
class ElementsAreMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InputIter>
  ElementsAreMatcherImpl(InputIter first, InputIter last) {
    while (first != last) {
      matchers_.push_back(MatcherCast<const Element&>(*first++));
    }
  }

  void DescribeTo(::std::ostream* os) const override {
    if (count() == 0) {
      *os << "is empty";
    } else if (count() == 1) {
      *os << "has 1 element that ";
      matchers_[0].DescribeTo(os);
    } else {
      *os << "has " << Elements(count()) << " where\n";
      for (size_t i = 0; i != count(); ++i) {
        *os << "element #" << i << " ";
        matchers_[i].DescribeTo(os);
        if (i + 1 < count()) {
          *os << ",\n";
        }
      }
    }
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    if (count() == 0) {
      *os << "isn't empty";
      return;
    }

    *os << "doesn't have " << Elements(count()) << ", or\n";
    for (size_t i = 0; i != count(); ++i) {
      *os << "element #" << i << " ";
      matchers_[i].DescribeNegationTo(os);
      if (i + 1 < count()) {
        *os << ", or\n";
      }
    }
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {

    const bool listener_interested = listener->IsInterested();

    ::std::vector<std::string> explanations(count());
    StlContainerReference stl_container = View::ConstReference(container);
    typename StlContainer::const_iterator it = stl_container.begin();
    size_t exam_pos = 0;
    bool mismatch_found = false;

    for (; it != stl_container.end() && exam_pos != count(); ++it, ++exam_pos) {
      bool match;
      if (listener_interested) {
        StringMatchResultListener s;
        match = matchers_[exam_pos].MatchAndExplain(*it, &s);
        explanations[exam_pos] = s.str();
      } else {
        match = matchers_[exam_pos].Matches(*it);
      }

      if (!match) {
        mismatch_found = true;
        break;
      }
    }

    size_t actual_count = exam_pos;
    for (; it != stl_container.end(); ++it) {
      ++actual_count;
    }

    if (actual_count != count()) {

      if (listener_interested && (actual_count != 0)) {
        *listener << "which has " << Elements(actual_count);
      }
      return false;
    }

    if (mismatch_found) {

      if (listener_interested) {
        *listener << "whose element #" << exam_pos << " doesn't match";
        PrintIfNotEmpty(explanations[exam_pos], listener->stream());
      }
      return false;
    }

    if (listener_interested) {
      bool reason_printed = false;
      for (size_t i = 0; i != count(); ++i) {
        const std::string& s = explanations[i];
        if (!s.empty()) {
          if (reason_printed) {
            *listener << ",\nand ";
          }
          *listener << "whose element #" << i << " matches, " << s;
          reason_printed = true;
        }
      }
    }
    return true;
  }

 private:
  static Message Elements(size_t count) {
    return Message() << count << (count == 1 ? " element" : " elements");
  }

  size_t count() const { return matchers_.size(); }

  ::std::vector<Matcher<const Element&> > matchers_;
};

class GTEST_API_ MatchMatrix {
 public:
  MatchMatrix(size_t num_elements, size_t num_matchers)
      : num_elements_(num_elements),
        num_matchers_(num_matchers),
        matched_(num_elements_* num_matchers_, 0) {
  }

  size_t LhsSize() const { return num_elements_; }
  size_t RhsSize() const { return num_matchers_; }
  bool HasEdge(size_t ilhs, size_t irhs) const {
    return matched_[SpaceIndex(ilhs, irhs)] == 1;
  }
  void SetEdge(size_t ilhs, size_t irhs, bool b) {
    matched_[SpaceIndex(ilhs, irhs)] = b ? 1 : 0;
  }

  bool NextGraph();

  void Randomize();

  std::string DebugString() const;

 private:
  size_t SpaceIndex(size_t ilhs, size_t irhs) const {
    return ilhs * num_matchers_ + irhs;
  }

  size_t num_elements_;
  size_t num_matchers_;

  ::std::vector<char> matched_;
};

typedef ::std::pair<size_t, size_t> ElementMatcherPair;
typedef ::std::vector<ElementMatcherPair> ElementMatcherPairs;

GTEST_API_ ElementMatcherPairs
FindMaxBipartiteMatching(const MatchMatrix& g);

struct UnorderedMatcherRequire {
  enum Flags {
    Superset = 1 << 0,
    Subset = 1 << 1,
    ExactMatch = Superset | Subset,
  };
};

class GTEST_API_ UnorderedElementsAreMatcherImplBase {
 protected:
  explicit UnorderedElementsAreMatcherImplBase(
      UnorderedMatcherRequire::Flags matcher_flags)
      : match_flags_(matcher_flags) {}

  typedef ::std::vector<const MatcherDescriberInterface*> MatcherDescriberVec;

  void DescribeToImpl(::std::ostream* os) const;

  void DescribeNegationToImpl(::std::ostream* os) const;

  bool VerifyMatchMatrix(const ::std::vector<std::string>& element_printouts,
                         const MatchMatrix& matrix,
                         MatchResultListener* listener) const;

  bool FindPairing(const MatchMatrix& matrix,
                   MatchResultListener* listener) const;

  MatcherDescriberVec& matcher_describers() {
    return matcher_describers_;
  }

  static Message Elements(size_t n) {
    return Message() << n << " element" << (n == 1 ? "" : "s");
  }

  UnorderedMatcherRequire::Flags match_flags() const { return match_flags_; }

 private:
  UnorderedMatcherRequire::Flags match_flags_;
  MatcherDescriberVec matcher_describers_;
};

template <typename Container>
class UnorderedElementsAreMatcherImpl
    : public MatcherInterface<Container>,
      public UnorderedElementsAreMatcherImplBase {
 public:
  typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::const_iterator StlContainerConstIterator;
  typedef typename StlContainer::value_type Element;

  template <typename InputIter>
  UnorderedElementsAreMatcherImpl(UnorderedMatcherRequire::Flags matcher_flags,
                                  InputIter first, InputIter last)
      : UnorderedElementsAreMatcherImplBase(matcher_flags) {
    for (; first != last; ++first) {
      matchers_.push_back(MatcherCast<const Element&>(*first));
    }
    for (const auto& m : matchers_) {
      matcher_describers().push_back(m.GetDescriber());
    }
  }

  void DescribeTo(::std::ostream* os) const override {
    return UnorderedElementsAreMatcherImplBase::DescribeToImpl(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    return UnorderedElementsAreMatcherImplBase::DescribeNegationToImpl(os);
  }

  bool MatchAndExplain(Container container,
                       MatchResultListener* listener) const override {
    StlContainerReference stl_container = View::ConstReference(container);
    ::std::vector<std::string> element_printouts;
    MatchMatrix matrix =
        AnalyzeElements(stl_container.begin(), stl_container.end(),
                        &element_printouts, listener);

    if (matrix.LhsSize() == 0 && matrix.RhsSize() == 0) {
      return true;
    }

    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      if (matrix.LhsSize() != matrix.RhsSize()) {

        if (matrix.LhsSize() != 0 && listener->IsInterested()) {
          *listener << "which has " << Elements(matrix.LhsSize());
        }
        return false;
      }
    }

    return VerifyMatchMatrix(element_printouts, matrix, listener) &&
           FindPairing(matrix, listener);
  }

 private:
  template <typename ElementIter>
  MatchMatrix AnalyzeElements(ElementIter elem_first, ElementIter elem_last,
                              ::std::vector<std::string>* element_printouts,
                              MatchResultListener* listener) const {
    element_printouts->clear();
    ::std::vector<char> did_match;
    size_t num_elements = 0;
    DummyMatchResultListener dummy;
    for (; elem_first != elem_last; ++num_elements, ++elem_first) {
      if (listener->IsInterested()) {
        element_printouts->push_back(PrintToString(*elem_first));
      }
      for (size_t irhs = 0; irhs != matchers_.size(); ++irhs) {
        did_match.push_back(
            matchers_[irhs].MatchAndExplain(*elem_first, &dummy));
      }
    }

    MatchMatrix matrix(num_elements, matchers_.size());
    ::std::vector<char>::const_iterator did_match_iter = did_match.begin();
    for (size_t ilhs = 0; ilhs != num_elements; ++ilhs) {
      for (size_t irhs = 0; irhs != matchers_.size(); ++irhs) {
        matrix.SetEdge(ilhs, irhs, *did_match_iter++ != 0);
      }
    }
    return matrix;
  }

  ::std::vector<Matcher<const Element&> > matchers_;
};

template <typename Target>
struct CastAndAppendTransform {
  template <typename Arg>
  Matcher<Target> operator()(const Arg& a) const {
    return MatcherCast<Target>(a);
  }
};

template <typename MatcherTuple>
class UnorderedElementsAreMatcher {
 public:
  explicit UnorderedElementsAreMatcher(const MatcherTuple& args)
      : matchers_(args) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type View;
    typedef typename View::value_type Element;
    typedef ::std::vector<Matcher<const Element&> > MatcherVec;
    MatcherVec matchers;
    matchers.reserve(::std::tuple_size<MatcherTuple>::value);
    TransformTupleValues(CastAndAppendTransform<const Element&>(), matchers_,
                         ::std::back_inserter(matchers));
    return Matcher<Container>(
        new UnorderedElementsAreMatcherImpl<const Container&>(
            UnorderedMatcherRequire::ExactMatch, matchers.begin(),
            matchers.end()));
  }

 private:
  const MatcherTuple matchers_;
};

template <typename MatcherTuple>
class ElementsAreMatcher {
 public:
  explicit ElementsAreMatcher(const MatcherTuple& args) : matchers_(args) {}

  template <typename Container>
  operator Matcher<Container>() const {
    GTEST_COMPILE_ASSERT_(
        !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(Container)>::value ||
            ::std::tuple_size<MatcherTuple>::value < 2,
        use_UnorderedElementsAre_with_hash_tables);

    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type View;
    typedef typename View::value_type Element;
    typedef ::std::vector<Matcher<const Element&> > MatcherVec;
    MatcherVec matchers;
    matchers.reserve(::std::tuple_size<MatcherTuple>::value);
    TransformTupleValues(CastAndAppendTransform<const Element&>(), matchers_,
                         ::std::back_inserter(matchers));
    return Matcher<Container>(new ElementsAreMatcherImpl<const Container&>(
        matchers.begin(), matchers.end()));
  }

 private:
  const MatcherTuple matchers_;
};

template <typename T>
class UnorderedElementsAreArrayMatcher {
 public:
  template <typename Iter>
  UnorderedElementsAreArrayMatcher(UnorderedMatcherRequire::Flags match_flags,
                                   Iter first, Iter last)
      : match_flags_(match_flags), matchers_(first, last) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return Matcher<Container>(
        new UnorderedElementsAreMatcherImpl<const Container&>(
            match_flags_, matchers_.begin(), matchers_.end()));
  }

 private:
  UnorderedMatcherRequire::Flags match_flags_;
  ::std::vector<T> matchers_;
};

template <typename T>
class ElementsAreArrayMatcher {
 public:
  template <typename Iter>
  ElementsAreArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}

  template <typename Container>
  operator Matcher<Container>() const {
    GTEST_COMPILE_ASSERT_(
        !IsHashTable<GTEST_REMOVE_REFERENCE_AND_CONST_(Container)>::value,
        use_UnorderedElementsAreArray_with_hash_tables);

    return Matcher<Container>(new ElementsAreMatcherImpl<const Container&>(
        matchers_.begin(), matchers_.end()));
  }

 private:
  const ::std::vector<T> matchers_;
};

template <typename Tuple2Matcher, typename Second>
class BoundSecondMatcher {
 public:
  BoundSecondMatcher(const Tuple2Matcher& tm, const Second& second)
      : tuple2_matcher_(tm), second_value_(second) {}

  BoundSecondMatcher(const BoundSecondMatcher& other) = default;

  template <typename T>
  operator Matcher<T>() const {
    return MakeMatcher(new Impl<T>(tuple2_matcher_, second_value_));
  }

  void operator=(const BoundSecondMatcher&  ) {
    GTEST_LOG_(FATAL) << "BoundSecondMatcher should never be assigned.";
  }

 private:
  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    typedef ::std::tuple<T, Second> ArgTuple;

    Impl(const Tuple2Matcher& tm, const Second& second)
        : mono_tuple2_matcher_(SafeMatcherCast<const ArgTuple&>(tm)),
          second_value_(second) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "and ";
      UniversalPrint(second_value_, os);
      *os << " ";
      mono_tuple2_matcher_.DescribeTo(os);
    }

    bool MatchAndExplain(T x, MatchResultListener* listener) const override {
      return mono_tuple2_matcher_.MatchAndExplain(ArgTuple(x, second_value_),
                                                  listener);
    }

   private:
    const Matcher<const ArgTuple&> mono_tuple2_matcher_;
    const Second second_value_;
  };

  const Tuple2Matcher tuple2_matcher_;
  const Second second_value_;
};

template <typename Tuple2Matcher, typename Second>
BoundSecondMatcher<Tuple2Matcher, Second> MatcherBindSecond(
    const Tuple2Matcher& tm, const Second& second) {
  return BoundSecondMatcher<Tuple2Matcher, Second>(tm, second);
}

GTEST_API_ std::string FormatMatcherDescription(bool negation,
                                                const char* matcher_name,
                                                const Strings& param_values);

template <typename ValueMatcher>
class OptionalMatcher {
 public:
  explicit OptionalMatcher(const ValueMatcher& value_matcher)
      : value_matcher_(value_matcher) {}

  template <typename Optional>
  operator Matcher<Optional>() const {
    return Matcher<Optional>(new Impl<const Optional&>(value_matcher_));
  }

  template <typename Optional>
  class Impl : public MatcherInterface<Optional> {
   public:
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Optional) OptionalView;
    typedef typename OptionalView::value_type ValueType;
    explicit Impl(const ValueMatcher& value_matcher)
        : value_matcher_(MatcherCast<ValueType>(value_matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "value ";
      value_matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "value ";
      value_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(Optional optional,
                         MatchResultListener* listener) const override {
      if (!optional) {
        *listener << "which is not engaged";
        return false;
      }
      const ValueType& value = *optional;
      StringMatchResultListener value_listener;
      const bool match = value_matcher_.MatchAndExplain(value, &value_listener);
      *listener << "whose value " << PrintToString(value)
                << (match ? " matches" : " doesn't match");
      PrintIfNotEmpty(value_listener.str(), listener->stream());
      return match;
    }

   private:
    const Matcher<ValueType> value_matcher_;
  };

 private:
  const ValueMatcher value_matcher_;
};

namespace variant_matcher {

template <typename T>
void holds_alternative() {}
template <typename T>
void get() {}

template <typename T>
class VariantMatcher {
 public:
  explicit VariantMatcher(::testing::Matcher<const T&> matcher)
      : matcher_(std::move(matcher)) {}

  template <typename Variant>
  bool MatchAndExplain(const Variant& value,
                       ::testing::MatchResultListener* listener) const {
    using std::get;
    if (!listener->IsInterested()) {
      return holds_alternative<T>(value) && matcher_.Matches(get<T>(value));
    }

    if (!holds_alternative<T>(value)) {
      *listener << "whose value is not of type '" << GetTypeName() << "'";
      return false;
    }

    const T& elem = get<T>(value);
    StringMatchResultListener elem_listener;
    const bool match = matcher_.MatchAndExplain(elem, &elem_listener);
    *listener << "whose value " << PrintToString(elem)
              << (match ? " matches" : " doesn't match");
    PrintIfNotEmpty(elem_listener.str(), listener->stream());
    return match;
  }

  void DescribeTo(std::ostream* os) const {
    *os << "is a variant<> with value of type '" << GetTypeName()
        << "' and the value ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "is a variant<> with value of type other than '" << GetTypeName()
        << "' or the value ";
    matcher_.DescribeNegationTo(os);
  }

 private:
  static std::string GetTypeName() {
#if GTEST_HAS_RTTI
    GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(
        return internal::GetTypeName<T>());
#endif
    return "the element type";
  }

  const ::testing::Matcher<const T&> matcher_;
};

}

namespace any_cast_matcher {

template <typename T>
void any_cast() {}

template <typename T>
class AnyCastMatcher {
 public:
  explicit AnyCastMatcher(const ::testing::Matcher<const T&>& matcher)
      : matcher_(matcher) {}

  template <typename AnyType>
  bool MatchAndExplain(const AnyType& value,
                       ::testing::MatchResultListener* listener) const {
    if (!listener->IsInterested()) {
      const T* ptr = any_cast<T>(&value);
      return ptr != nullptr && matcher_.Matches(*ptr);
    }

    const T* elem = any_cast<T>(&value);
    if (elem == nullptr) {
      *listener << "whose value is not of type '" << GetTypeName() << "'";
      return false;
    }

    StringMatchResultListener elem_listener;
    const bool match = matcher_.MatchAndExplain(*elem, &elem_listener);
    *listener << "whose value " << PrintToString(*elem)
              << (match ? " matches" : " doesn't match");
    PrintIfNotEmpty(elem_listener.str(), listener->stream());
    return match;
  }

  void DescribeTo(std::ostream* os) const {
    *os << "is an 'any' type with value of type '" << GetTypeName()
        << "' and the value ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "is an 'any' type with value of type other than '" << GetTypeName()
        << "' or the value ";
    matcher_.DescribeNegationTo(os);
  }

 private:
  static std::string GetTypeName() {
#if GTEST_HAS_RTTI
    GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(
        return internal::GetTypeName<T>());
#endif
    return "the element type";
  }

  const ::testing::Matcher<const T&> matcher_;
};

}

template <class ArgsTuple, size_t... k>
class ArgsMatcherImpl : public MatcherInterface<ArgsTuple> {
 public:
  using RawArgsTuple = typename std::decay<ArgsTuple>::type;
  using SelectedArgs =
      std::tuple<typename std::tuple_element<k, RawArgsTuple>::type...>;
  using MonomorphicInnerMatcher = Matcher<const SelectedArgs&>;

  template <typename InnerMatcher>
  explicit ArgsMatcherImpl(const InnerMatcher& inner_matcher)
      : inner_matcher_(SafeMatcherCast<const SelectedArgs&>(inner_matcher)) {}

  bool MatchAndExplain(ArgsTuple args,
                       MatchResultListener* listener) const override {

    (void)args;
    const SelectedArgs& selected_args =
        std::forward_as_tuple(std::get<k>(args)...);
    if (!listener->IsInterested()) return inner_matcher_.Matches(selected_args);

    PrintIndices(listener->stream());
    *listener << "are " << PrintToString(selected_args);

    StringMatchResultListener inner_listener;
    const bool match =
        inner_matcher_.MatchAndExplain(selected_args, &inner_listener);
    PrintIfNotEmpty(inner_listener.str(), listener->stream());
    return match;
  }

  void DescribeTo(::std::ostream* os) const override {
    *os << "are a tuple ";
    PrintIndices(os);
    inner_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const override {
    *os << "are a tuple ";
    PrintIndices(os);
    inner_matcher_.DescribeNegationTo(os);
  }

 private:

  static void PrintIndices(::std::ostream* os) {
    *os << "whose fields (";
    const char* sep = "";

    (void)sep;
    const char* dummy[] = {"", (*os << sep << "#" << k, sep = ", ")...};
    (void)dummy;
    *os << ") ";
  }

  MonomorphicInnerMatcher inner_matcher_;
};

template <class InnerMatcher, size_t... k>
class ArgsMatcher {
 public:
  explicit ArgsMatcher(InnerMatcher inner_matcher)
      : inner_matcher_(std::move(inner_matcher)) {}

  template <typename ArgsTuple>
  operator Matcher<ArgsTuple>() const {
    return MakeMatcher(new ArgsMatcherImpl<ArgsTuple, k...>(inner_matcher_));
  }

 private:
  InnerMatcher inner_matcher_;
};

}

template <typename Iter>
inline internal::ElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
ElementsAreArray(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::ElementsAreArrayMatcher<T>(first, last);
}

template <typename T>
inline internal::ElementsAreArrayMatcher<T> ElementsAreArray(
    const T* pointer, size_t count) {
  return ElementsAreArray(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::ElementsAreArrayMatcher<T> ElementsAreArray(
    const T (&array)[N]) {
  return ElementsAreArray(array, N);
}

template <typename Container>
inline internal::ElementsAreArrayMatcher<typename Container::value_type>
ElementsAreArray(const Container& container) {
  return ElementsAreArray(container.begin(), container.end());
}

template <typename T>
inline internal::ElementsAreArrayMatcher<T>
ElementsAreArray(::std::initializer_list<T> xs) {
  return ElementsAreArray(xs.begin(), xs.end());
}

template <typename Iter>
inline internal::UnorderedElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
UnorderedElementsAreArray(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::UnorderedElementsAreArrayMatcher<T>(
      internal::UnorderedMatcherRequire::ExactMatch, first, last);
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T>
UnorderedElementsAreArray(const T* pointer, size_t count) {
  return UnorderedElementsAreArray(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::UnorderedElementsAreArrayMatcher<T>
UnorderedElementsAreArray(const T (&array)[N]) {
  return UnorderedElementsAreArray(array, N);
}

template <typename Container>
inline internal::UnorderedElementsAreArrayMatcher<
    typename Container::value_type>
UnorderedElementsAreArray(const Container& container) {
  return UnorderedElementsAreArray(container.begin(), container.end());
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T>
UnorderedElementsAreArray(::std::initializer_list<T> xs) {
  return UnorderedElementsAreArray(xs.begin(), xs.end());
}

const internal::AnythingMatcher _ = {};

template <typename T>
inline Matcher<T> A() {
  return _;
}

template <typename T>
inline Matcher<T> An() {
  return _;
}

template <typename T, typename M>
Matcher<T> internal::MatcherCastImpl<T, M>::CastImpl(
    const M& value, std::false_type  ,
    std::false_type  ) {
  return Eq(value);
}

inline PolymorphicMatcher<internal::IsNullMatcher > IsNull() {
  return MakePolymorphicMatcher(internal::IsNullMatcher());
}

inline PolymorphicMatcher<internal::NotNullMatcher > NotNull() {
  return MakePolymorphicMatcher(internal::NotNullMatcher());
}

template <typename T>
inline internal::RefMatcher<T&> Ref(T& x) {
  return internal::RefMatcher<T&>(x);
}

inline PolymorphicMatcher<internal::IsNanMatcher> IsNan() {
  return MakePolymorphicMatcher(internal::IsNanMatcher());
}

inline internal::FloatingEqMatcher<double> DoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, false);
}

inline internal::FloatingEqMatcher<double> NanSensitiveDoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, true);
}

inline internal::FloatingEqMatcher<double> DoubleNear(
    double rhs, double max_abs_error) {
  return internal::FloatingEqMatcher<double>(rhs, false, max_abs_error);
}

inline internal::FloatingEqMatcher<double> NanSensitiveDoubleNear(
    double rhs, double max_abs_error) {
  return internal::FloatingEqMatcher<double>(rhs, true, max_abs_error);
}

inline internal::FloatingEqMatcher<float> FloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, false);
}

inline internal::FloatingEqMatcher<float> NanSensitiveFloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, true);
}

inline internal::FloatingEqMatcher<float> FloatNear(
    float rhs, float max_abs_error) {
  return internal::FloatingEqMatcher<float>(rhs, false, max_abs_error);
}

inline internal::FloatingEqMatcher<float> NanSensitiveFloatNear(
    float rhs, float max_abs_error) {
  return internal::FloatingEqMatcher<float>(rhs, true, max_abs_error);
}

template <typename InnerMatcher>
inline internal::PointeeMatcher<InnerMatcher> Pointee(
    const InnerMatcher& inner_matcher) {
  return internal::PointeeMatcher<InnerMatcher>(inner_matcher);
}

#if GTEST_HAS_RTTI

template <typename To>
inline PolymorphicMatcher<internal::WhenDynamicCastToMatcher<To> >
WhenDynamicCastTo(const Matcher<To>& inner_matcher) {
  return MakePolymorphicMatcher(
      internal::WhenDynamicCastToMatcher<To>(inner_matcher));
}
#endif

template <typename Class, typename FieldType, typename FieldMatcher>
inline PolymorphicMatcher<
  internal::FieldMatcher<Class, FieldType> > Field(
    FieldType Class::*field, const FieldMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::FieldMatcher<Class, FieldType>(
          field, MatcherCast<const FieldType&>(matcher)));

}

template <typename Class, typename FieldType, typename FieldMatcher>
inline PolymorphicMatcher<internal::FieldMatcher<Class, FieldType> > Field(
    const std::string& field_name, FieldType Class::*field,
    const FieldMatcher& matcher) {
  return MakePolymorphicMatcher(internal::FieldMatcher<Class, FieldType>(
      field_name, field, MatcherCast<const FieldType&>(matcher)));
}

template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const> >
Property(PropertyType (Class::*property)() const,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const>(
          property, MatcherCast<const PropertyType&>(matcher)));

}

template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const> >
Property(const std::string& property_name,
         PropertyType (Class::*property)() const,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const>(
          property_name, property, MatcherCast<const PropertyType&>(matcher)));
}

template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const &> >
Property(PropertyType (Class::*property)() const &,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const&>(
          property, MatcherCast<const PropertyType&>(matcher)));
}

template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<internal::PropertyMatcher<
    Class, PropertyType, PropertyType (Class::*)() const &> >
Property(const std::string& property_name,
         PropertyType (Class::*property)() const &,
         const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType,
                                PropertyType (Class::*)() const&>(
          property_name, property, MatcherCast<const PropertyType&>(matcher)));
}

template <typename Callable, typename InnerMatcher>
internal::ResultOfMatcher<Callable, InnerMatcher> ResultOf(
    Callable callable, InnerMatcher matcher) {
  return internal::ResultOfMatcher<Callable, InnerMatcher>(
      std::move(callable), std::move(matcher));
}

template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string> > StrEq(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::string>(std::string(str), true, true));
}

template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string> > StrNe(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::string>(std::string(str), false, true));
}

template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string> > StrCaseEq(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::string>(std::string(str), true, false));
}

template <typename T = std::string>
PolymorphicMatcher<internal::StrEqualityMatcher<std::string> > StrCaseNe(
    const internal::StringLike<T>& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<std::string>(
      std::string(str), false, false));
}

template <typename T = std::string>
PolymorphicMatcher<internal::HasSubstrMatcher<std::string> > HasSubstr(
    const internal::StringLike<T>& substring) {
  return MakePolymorphicMatcher(
      internal::HasSubstrMatcher<std::string>(std::string(substring)));
}

template <typename T = std::string>
PolymorphicMatcher<internal::StartsWithMatcher<std::string> > StartsWith(
    const internal::StringLike<T>& prefix) {
  return MakePolymorphicMatcher(
      internal::StartsWithMatcher<std::string>(std::string(prefix)));
}

template <typename T = std::string>
PolymorphicMatcher<internal::EndsWithMatcher<std::string> > EndsWith(
    const internal::StringLike<T>& suffix) {
  return MakePolymorphicMatcher(
      internal::EndsWithMatcher<std::string>(std::string(suffix)));
}

#if GTEST_HAS_STD_WSTRING

inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring> > StrEq(
    const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, true, true));
}

inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring> > StrNe(
    const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, false, true));
}

inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring> >
StrCaseEq(const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, true, false));
}

inline PolymorphicMatcher<internal::StrEqualityMatcher<std::wstring> >
StrCaseNe(const std::wstring& str) {
  return MakePolymorphicMatcher(
      internal::StrEqualityMatcher<std::wstring>(str, false, false));
}

inline PolymorphicMatcher<internal::HasSubstrMatcher<std::wstring> > HasSubstr(
    const std::wstring& substring) {
  return MakePolymorphicMatcher(
      internal::HasSubstrMatcher<std::wstring>(substring));
}

inline PolymorphicMatcher<internal::StartsWithMatcher<std::wstring> >
StartsWith(const std::wstring& prefix) {
  return MakePolymorphicMatcher(
      internal::StartsWithMatcher<std::wstring>(prefix));
}

inline PolymorphicMatcher<internal::EndsWithMatcher<std::wstring> > EndsWith(
    const std::wstring& suffix) {
  return MakePolymorphicMatcher(
      internal::EndsWithMatcher<std::wstring>(suffix));
}

#endif

inline internal::Eq2Matcher Eq() { return internal::Eq2Matcher(); }

inline internal::Ge2Matcher Ge() { return internal::Ge2Matcher(); }

inline internal::Gt2Matcher Gt() { return internal::Gt2Matcher(); }

inline internal::Le2Matcher Le() { return internal::Le2Matcher(); }

inline internal::Lt2Matcher Lt() { return internal::Lt2Matcher(); }

inline internal::Ne2Matcher Ne() { return internal::Ne2Matcher(); }

inline internal::FloatingEq2Matcher<float> FloatEq() {
  return internal::FloatingEq2Matcher<float>();
}

inline internal::FloatingEq2Matcher<double> DoubleEq() {
  return internal::FloatingEq2Matcher<double>();
}

inline internal::FloatingEq2Matcher<float> NanSensitiveFloatEq() {
  return internal::FloatingEq2Matcher<float>(true);
}

inline internal::FloatingEq2Matcher<double> NanSensitiveDoubleEq() {
  return internal::FloatingEq2Matcher<double>(true);
}

inline internal::FloatingEq2Matcher<float> FloatNear(float max_abs_error) {
  return internal::FloatingEq2Matcher<float>(max_abs_error);
}

inline internal::FloatingEq2Matcher<double> DoubleNear(double max_abs_error) {
  return internal::FloatingEq2Matcher<double>(max_abs_error);
}

inline internal::FloatingEq2Matcher<float> NanSensitiveFloatNear(
    float max_abs_error) {
  return internal::FloatingEq2Matcher<float>(max_abs_error, true);
}

inline internal::FloatingEq2Matcher<double> NanSensitiveDoubleNear(
    double max_abs_error) {
  return internal::FloatingEq2Matcher<double>(max_abs_error, true);
}

template <typename InnerMatcher>
inline internal::NotMatcher<InnerMatcher> Not(InnerMatcher m) {
  return internal::NotMatcher<InnerMatcher>(m);
}

template <typename Predicate>
inline PolymorphicMatcher<internal::TrulyMatcher<Predicate> >
Truly(Predicate pred) {
  return MakePolymorphicMatcher(internal::TrulyMatcher<Predicate>(pred));
}

template <typename SizeMatcher>
inline internal::SizeIsMatcher<SizeMatcher>
SizeIs(const SizeMatcher& size_matcher) {
  return internal::SizeIsMatcher<SizeMatcher>(size_matcher);
}

template <typename DistanceMatcher>
inline internal::BeginEndDistanceIsMatcher<DistanceMatcher>
BeginEndDistanceIs(const DistanceMatcher& distance_matcher) {
  return internal::BeginEndDistanceIsMatcher<DistanceMatcher>(distance_matcher);
}

template <typename Container>
inline PolymorphicMatcher<internal::ContainerEqMatcher<
    typename std::remove_const<Container>::type>>
ContainerEq(const Container& rhs) {
  return MakePolymorphicMatcher(internal::ContainerEqMatcher<Container>(rhs));
}

template <typename Comparator, typename ContainerMatcher>
inline internal::WhenSortedByMatcher<Comparator, ContainerMatcher>
WhenSortedBy(const Comparator& comparator,
             const ContainerMatcher& container_matcher) {
  return internal::WhenSortedByMatcher<Comparator, ContainerMatcher>(
      comparator, container_matcher);
}

template <typename ContainerMatcher>
inline internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher>
WhenSorted(const ContainerMatcher& container_matcher) {
  return
      internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher>(
          internal::LessComparator(), container_matcher);
}

template <typename TupleMatcher, typename Container>
inline internal::PointwiseMatcher<TupleMatcher,
                                  typename std::remove_const<Container>::type>
Pointwise(const TupleMatcher& tuple_matcher, const Container& rhs) {
  return internal::PointwiseMatcher<TupleMatcher, Container>(tuple_matcher,
                                                             rhs);
}

template <typename TupleMatcher, typename T>
inline internal::PointwiseMatcher<TupleMatcher, std::vector<T> > Pointwise(
    const TupleMatcher& tuple_matcher, std::initializer_list<T> rhs) {
  return Pointwise(tuple_matcher, std::vector<T>(rhs));
}

template <typename Tuple2Matcher, typename RhsContainer>
inline internal::UnorderedElementsAreArrayMatcher<
    typename internal::BoundSecondMatcher<
        Tuple2Matcher,
        typename internal::StlContainerView<
            typename std::remove_const<RhsContainer>::type>::type::value_type>>
UnorderedPointwise(const Tuple2Matcher& tuple2_matcher,
                   const RhsContainer& rhs_container) {

  typedef typename internal::StlContainerView<RhsContainer> RhsView;
  typedef typename RhsView::type RhsStlContainer;
  typedef typename RhsStlContainer::value_type Second;
  const RhsStlContainer& rhs_stl_container =
      RhsView::ConstReference(rhs_container);

  ::std::vector<internal::BoundSecondMatcher<Tuple2Matcher, Second> > matchers;
  for (typename RhsStlContainer::const_iterator it = rhs_stl_container.begin();
       it != rhs_stl_container.end(); ++it) {
    matchers.push_back(
        internal::MatcherBindSecond(tuple2_matcher, *it));
  }

  return UnorderedElementsAreArray(matchers);
}

template <typename Tuple2Matcher, typename T>
inline internal::UnorderedElementsAreArrayMatcher<
    typename internal::BoundSecondMatcher<Tuple2Matcher, T> >
UnorderedPointwise(const Tuple2Matcher& tuple2_matcher,
                   std::initializer_list<T> rhs) {
  return UnorderedPointwise(tuple2_matcher, std::vector<T>(rhs));
}

template <typename M>
inline internal::ContainsMatcher<M> Contains(M matcher) {
  return internal::ContainsMatcher<M>(matcher);
}

template <typename Iter>
inline internal::UnorderedElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
IsSupersetOf(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::UnorderedElementsAreArrayMatcher<T>(
      internal::UnorderedMatcherRequire::Superset, first, last);
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSupersetOf(
    const T* pointer, size_t count) {
  return IsSupersetOf(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSupersetOf(
    const T (&array)[N]) {
  return IsSupersetOf(array, N);
}

template <typename Container>
inline internal::UnorderedElementsAreArrayMatcher<
    typename Container::value_type>
IsSupersetOf(const Container& container) {
  return IsSupersetOf(container.begin(), container.end());
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSupersetOf(
    ::std::initializer_list<T> xs) {
  return IsSupersetOf(xs.begin(), xs.end());
}

template <typename Iter>
inline internal::UnorderedElementsAreArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
IsSubsetOf(Iter first, Iter last) {
  typedef typename ::std::iterator_traits<Iter>::value_type T;
  return internal::UnorderedElementsAreArrayMatcher<T>(
      internal::UnorderedMatcherRequire::Subset, first, last);
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSubsetOf(
    const T* pointer, size_t count) {
  return IsSubsetOf(pointer, pointer + count);
}

template <typename T, size_t N>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSubsetOf(
    const T (&array)[N]) {
  return IsSubsetOf(array, N);
}

template <typename Container>
inline internal::UnorderedElementsAreArrayMatcher<
    typename Container::value_type>
IsSubsetOf(const Container& container) {
  return IsSubsetOf(container.begin(), container.end());
}

template <typename T>
inline internal::UnorderedElementsAreArrayMatcher<T> IsSubsetOf(
    ::std::initializer_list<T> xs) {
  return IsSubsetOf(xs.begin(), xs.end());
}

template <typename M>
inline internal::EachMatcher<M> Each(M matcher) {
  return internal::EachMatcher<M>(matcher);
}

template <typename M>
inline internal::KeyMatcher<M> Key(M inner_matcher) {
  return internal::KeyMatcher<M>(inner_matcher);
}

template <typename FirstMatcher, typename SecondMatcher>
inline internal::PairMatcher<FirstMatcher, SecondMatcher>
Pair(FirstMatcher first_matcher, SecondMatcher second_matcher) {
  return internal::PairMatcher<FirstMatcher, SecondMatcher>(
      first_matcher, second_matcher);
}

namespace no_adl {

template <typename... M>
internal::FieldsAreMatcher<typename std::decay<M>::type...> FieldsAre(
    M&&... matchers) {
  return internal::FieldsAreMatcher<typename std::decay<M>::type...>(
      std::forward<M>(matchers)...);
}

template <typename InnerMatcher>
inline internal::PointerMatcher<InnerMatcher> Pointer(
    const InnerMatcher& inner_matcher) {
  return internal::PointerMatcher<InnerMatcher>(inner_matcher);
}

template <typename InnerMatcher>
inline internal::AddressMatcher<InnerMatcher> Address(
    const InnerMatcher& inner_matcher) {
  return internal::AddressMatcher<InnerMatcher>(inner_matcher);
}
}

template <typename M>
inline internal::MatcherAsPredicate<M> Matches(M matcher) {
  return internal::MatcherAsPredicate<M>(matcher);
}

template <typename T, typename M>
inline bool Value(const T& value, M matcher) {
  return testing::Matches(matcher)(value);
}

template <typename T, typename M>
inline bool ExplainMatchResult(
    M matcher, const T& value, MatchResultListener* listener) {
  return SafeMatcherCast<const T&>(matcher).MatchAndExplain(value, listener);
}

template <typename T, typename M>
std::string DescribeMatcher(const M& matcher, bool negation = false) {
  ::std::stringstream ss;
  Matcher<T> monomorphic_matcher = SafeMatcherCast<T>(matcher);
  if (negation) {
    monomorphic_matcher.DescribeNegationTo(&ss);
  } else {
    monomorphic_matcher.DescribeTo(&ss);
  }
  return ss.str();
}

template <typename... Args>
internal::ElementsAreMatcher<
    std::tuple<typename std::decay<const Args&>::type...>>
ElementsAre(const Args&... matchers) {
  return internal::ElementsAreMatcher<
      std::tuple<typename std::decay<const Args&>::type...>>(
      std::make_tuple(matchers...));
}

template <typename... Args>
internal::UnorderedElementsAreMatcher<
    std::tuple<typename std::decay<const Args&>::type...>>
UnorderedElementsAre(const Args&... matchers) {
  return internal::UnorderedElementsAreMatcher<
      std::tuple<typename std::decay<const Args&>::type...>>(
      std::make_tuple(matchers...));
}

template <typename... Args>
internal::AllOfMatcher<typename std::decay<const Args&>::type...> AllOf(
    const Args&... matchers) {
  return internal::AllOfMatcher<typename std::decay<const Args&>::type...>(
      matchers...);
}

template <typename... Args>
internal::AnyOfMatcher<typename std::decay<const Args&>::type...> AnyOf(
    const Args&... matchers) {
  return internal::AnyOfMatcher<typename std::decay<const Args&>::type...>(
      matchers...);
}

template <typename Iter>
inline internal::AnyOfArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
AnyOfArray(Iter first, Iter last) {
  return internal::AnyOfArrayMatcher<
      typename ::std::iterator_traits<Iter>::value_type>(first, last);
}

template <typename Iter>
inline internal::AllOfArrayMatcher<
    typename ::std::iterator_traits<Iter>::value_type>
AllOfArray(Iter first, Iter last) {
  return internal::AllOfArrayMatcher<
      typename ::std::iterator_traits<Iter>::value_type>(first, last);
}

template <typename T>
inline internal::AnyOfArrayMatcher<T> AnyOfArray(const T* ptr, size_t count) {
  return AnyOfArray(ptr, ptr + count);
}

template <typename T>
inline internal::AllOfArrayMatcher<T> AllOfArray(const T* ptr, size_t count) {
  return AllOfArray(ptr, ptr + count);
}

template <typename T, size_t N>
inline internal::AnyOfArrayMatcher<T> AnyOfArray(const T (&array)[N]) {
  return AnyOfArray(array, N);
}

template <typename T, size_t N>
inline internal::AllOfArrayMatcher<T> AllOfArray(const T (&array)[N]) {
  return AllOfArray(array, N);
}

template <typename Container>
inline internal::AnyOfArrayMatcher<typename Container::value_type> AnyOfArray(
    const Container& container) {
  return AnyOfArray(container.begin(), container.end());
}

template <typename Container>
inline internal::AllOfArrayMatcher<typename Container::value_type> AllOfArray(
    const Container& container) {
  return AllOfArray(container.begin(), container.end());
}

template <typename T>
inline internal::AnyOfArrayMatcher<T> AnyOfArray(
    ::std::initializer_list<T> xs) {
  return AnyOfArray(xs.begin(), xs.end());
}

template <typename T>
inline internal::AllOfArrayMatcher<T> AllOfArray(
    ::std::initializer_list<T> xs) {
  return AllOfArray(xs.begin(), xs.end());
}

template <size_t... k, typename InnerMatcher>
internal::ArgsMatcher<typename std::decay<InnerMatcher>::type, k...> Args(
    InnerMatcher&& matcher) {
  return internal::ArgsMatcher<typename std::decay<InnerMatcher>::type, k...>(
      std::forward<InnerMatcher>(matcher));
}

template <typename InnerMatcher>
inline InnerMatcher AllArgs(const InnerMatcher& matcher) { return matcher; }

template <typename ValueMatcher>
inline internal::OptionalMatcher<ValueMatcher> Optional(
    const ValueMatcher& value_matcher) {
  return internal::OptionalMatcher<ValueMatcher>(value_matcher);
}

template <typename T>
PolymorphicMatcher<internal::any_cast_matcher::AnyCastMatcher<T> > AnyWith(
    const Matcher<const T&>& matcher) {
  return MakePolymorphicMatcher(
      internal::any_cast_matcher::AnyCastMatcher<T>(matcher));
}

template <typename T>
PolymorphicMatcher<internal::variant_matcher::VariantMatcher<T> > VariantWith(
    const Matcher<const T&>& matcher) {
  return MakePolymorphicMatcher(
      internal::variant_matcher::VariantMatcher<T>(matcher));
}

#if GTEST_HAS_EXCEPTIONS

namespace internal {

class WithWhatMatcherImpl {
 public:
  WithWhatMatcherImpl(Matcher<std::string> matcher)
      : matcher_(std::move(matcher)) {}

  void DescribeTo(std::ostream* os) const {
    *os << "contains .what() that ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "contains .what() that does not ";
    matcher_.DescribeTo(os);
  }

  template <typename Err>
  bool MatchAndExplain(const Err& err, MatchResultListener* listener) const {
    *listener << "which contains .what() that ";
    return matcher_.MatchAndExplain(err.what(), listener);
  }

 private:
  const Matcher<std::string> matcher_;
};

inline PolymorphicMatcher<WithWhatMatcherImpl> WithWhat(
    Matcher<std::string> m) {
  return MakePolymorphicMatcher(WithWhatMatcherImpl(std::move(m)));
}

template <typename Err>
class ExceptionMatcherImpl {
  class NeverThrown {
   public:
    const char* what() const noexcept {
      return "this exception should never be thrown";
    }
  };

  using DefaultExceptionType = typename std::conditional<
      std::is_same<typename std::remove_cv<
                       typename std::remove_reference<Err>::type>::type,
                   std::exception>::value,
      const NeverThrown&, const std::exception&>::type;

 public:
  ExceptionMatcherImpl(Matcher<const Err&> matcher)
      : matcher_(std::move(matcher)) {}

  void DescribeTo(std::ostream* os) const {
    *os << "throws an exception which is a " << GetTypeName<Err>();
    *os << " which ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "throws an exception which is not a " << GetTypeName<Err>();
    *os << " which ";
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(T&& x, MatchResultListener* listener) const {
    try {
      (void)(std::forward<T>(x)());
    } catch (const Err& err) {
      *listener << "throws an exception which is a " << GetTypeName<Err>();
      *listener << " ";
      return matcher_.MatchAndExplain(err, listener);
    } catch (DefaultExceptionType err) {
#if GTEST_HAS_RTTI
      *listener << "throws an exception of type " << GetTypeName(typeid(err));
      *listener << " ";
#else
      *listener << "throws an std::exception-derived type ";
#endif
      *listener << "with description \"" << err.what() << "\"";
      return false;
    } catch (...) {
      *listener << "throws an exception of an unknown type";
      return false;
    }

    *listener << "does not throw any exception";
    return false;
  }

 private:
  const Matcher<const Err&> matcher_;
};

}

template <typename Err>
PolymorphicMatcher<internal::ExceptionMatcherImpl<Err>> Throws() {
  return MakePolymorphicMatcher(
      internal::ExceptionMatcherImpl<Err>(A<const Err&>()));
}

template <typename Err, typename ExceptionMatcher>
PolymorphicMatcher<internal::ExceptionMatcherImpl<Err>> Throws(
    const ExceptionMatcher& exception_matcher) {

  return MakePolymorphicMatcher(internal::ExceptionMatcherImpl<Err>(
      SafeMatcherCast<const Err&>(exception_matcher)));
}

template <typename Err, typename MessageMatcher>
PolymorphicMatcher<internal::ExceptionMatcherImpl<Err>> ThrowsMessage(
    MessageMatcher&& message_matcher) {
  static_assert(std::is_base_of<std::exception, Err>::value,
                "expected an std::exception-derived type");
  return Throws<Err>(internal::WithWhat(
      MatcherCast<std::string>(std::forward<MessageMatcher>(message_matcher))));
}

#endif

#define ASSERT_THAT(value, matcher) ASSERT_PRED_FORMAT1(\
    ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)
#define EXPECT_THAT(value, matcher) EXPECT_PRED_FORMAT1(\
    ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)

#define MATCHER(name, description)                                             \
  class name##Matcher                                                          \
      : public ::testing::internal::MatcherBaseImpl<name##Matcher> {           \
   public:                                                                     \
    template <typename arg_type>                                               \
    class gmock_Impl : public ::testing::MatcherInterface<const arg_type&> {   \
     public:                                                                   \
      gmock_Impl() {}                                                          \
      bool MatchAndExplain(                                                    \
          const arg_type& arg,                                                 \
          ::testing::MatchResultListener* result_listener) const override;     \
      void DescribeTo(::std::ostream* gmock_os) const override {               \
        *gmock_os << FormatDescription(false);                                 \
      }                                                                        \
      void DescribeNegationTo(::std::ostream* gmock_os) const override {       \
        *gmock_os << FormatDescription(true);                                  \
      }                                                                        \
                                                                               \
     private:                                                                  \
      ::std::string FormatDescription(bool negation) const {                   \
        ::std::string gmock_description = (description);                       \
        if (!gmock_description.empty()) {                                      \
          return gmock_description;                                            \
        }                                                                      \
        return ::testing::internal::FormatMatcherDescription(negation, #name,  \
                                                             {});              \
      }                                                                        \
    };                                                                         \
  };                                                                           \
  GTEST_ATTRIBUTE_UNUSED_ inline name##Matcher name() { return {}; }           \
  template <typename arg_type>                                                 \
  bool name##Matcher::gmock_Impl<arg_type>::MatchAndExplain(                   \
      const arg_type& arg,                                                     \
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_) \
      const

#define MATCHER_P(name, p0, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP, description, (p0))
#define MATCHER_P2(name, p0, p1, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP2, description, (p0, p1))
#define MATCHER_P3(name, p0, p1, p2, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP3, description, (p0, p1, p2))
#define MATCHER_P4(name, p0, p1, p2, p3, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP4, description, (p0, p1, p2, p3))
#define MATCHER_P5(name, p0, p1, p2, p3, p4, description)    \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP5, description, \
                         (p0, p1, p2, p3, p4))
#define MATCHER_P6(name, p0, p1, p2, p3, p4, p5, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP6, description,  \
                         (p0, p1, p2, p3, p4, p5))
#define MATCHER_P7(name, p0, p1, p2, p3, p4, p5, p6, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP7, description,      \
                         (p0, p1, p2, p3, p4, p5, p6))
#define MATCHER_P8(name, p0, p1, p2, p3, p4, p5, p6, p7, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP8, description,          \
                         (p0, p1, p2, p3, p4, p5, p6, p7))
#define MATCHER_P9(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP9, description,              \
                         (p0, p1, p2, p3, p4, p5, p6, p7, p8))
#define MATCHER_P10(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, description) \
  GMOCK_INTERNAL_MATCHER(name, name##MatcherP10, description,                  \
                         (p0, p1, p2, p3, p4, p5, p6, p7, p8, p9))

#define GMOCK_INTERNAL_MATCHER(name, full_name, description, args)             \
  template <GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args)>                      \
  class full_name : public ::testing::internal::MatcherBaseImpl<               \
                        full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>> { \
   public:                                                                     \
    using full_name::MatcherBaseImpl::MatcherBaseImpl;                         \
    template <typename arg_type>                                               \
    class gmock_Impl : public ::testing::MatcherInterface<const arg_type&> {   \
     public:                                                                   \
      explicit gmock_Impl(GMOCK_INTERNAL_MATCHER_FUNCTION_ARGS(args))          \
          : GMOCK_INTERNAL_MATCHER_FORWARD_ARGS(args) {}                       \
      bool MatchAndExplain(                                                    \
          const arg_type& arg,                                                 \
          ::testing::MatchResultListener* result_listener) const override;     \
      void DescribeTo(::std::ostream* gmock_os) const override {               \
        *gmock_os << FormatDescription(false);                                 \
      }                                                                        \
      void DescribeNegationTo(::std::ostream* gmock_os) const override {       \
        *gmock_os << FormatDescription(true);                                  \
      }                                                                        \
      GMOCK_INTERNAL_MATCHER_MEMBERS(args)                                     \
                                                                               \
     private:                                                                  \
      ::std::string FormatDescription(bool negation) const {                   \
        ::std::string gmock_description = (description);                       \
        if (!gmock_description.empty()) {                                      \
          return gmock_description;                                            \
        }                                                                      \
        return ::testing::internal::FormatMatcherDescription(                  \
            negation, #name,                                                   \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(      \
                ::std::tuple<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>(        \
                    GMOCK_INTERNAL_MATCHER_MEMBERS_USAGE(args))));             \
      }                                                                        \
    };                                                                         \
  };                                                                           \
  template <GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args)>                      \
  inline full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)> name(             \
      GMOCK_INTERNAL_MATCHER_FUNCTION_ARGS(args)) {                            \
    return full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>(                \
        GMOCK_INTERNAL_MATCHER_ARGS_USAGE(args));                              \
  }                                                                            \
  template <GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args)>                      \
  template <typename arg_type>                                                 \
  bool full_name<GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args)>::gmock_Impl<        \
      arg_type>::MatchAndExplain(const arg_type& arg,                          \
                                 ::testing::MatchResultListener*               \
                                     result_listener GTEST_ATTRIBUTE_UNUSED_)  \
      const

#define GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAMS(args) \
  GMOCK_PP_TAIL(                                     \
      GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAM, , args))
#define GMOCK_INTERNAL_MATCHER_TEMPLATE_PARAM(i_unused, data_unused, arg) \
  , typename arg##_type

#define GMOCK_INTERNAL_MATCHER_TYPE_PARAMS(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_TYPE_PARAM, , args))
#define GMOCK_INTERNAL_MATCHER_TYPE_PARAM(i_unused, data_unused, arg) \
  , arg##_type

#define GMOCK_INTERNAL_MATCHER_FUNCTION_ARGS(args) \
  GMOCK_PP_TAIL(dummy_first GMOCK_PP_FOR_EACH(     \
      GMOCK_INTERNAL_MATCHER_FUNCTION_ARG, , args))
#define GMOCK_INTERNAL_MATCHER_FUNCTION_ARG(i, data_unused, arg) \
  , arg##_type gmock_p##i

#define GMOCK_INTERNAL_MATCHER_FORWARD_ARGS(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_FORWARD_ARG, , args))
#define GMOCK_INTERNAL_MATCHER_FORWARD_ARG(i, data_unused, arg) \
  , arg(::std::forward<arg##_type>(gmock_p##i))

#define GMOCK_INTERNAL_MATCHER_MEMBERS(args) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_MEMBER, , args)
#define GMOCK_INTERNAL_MATCHER_MEMBER(i_unused, data_unused, arg) \
  const arg##_type arg;

#define GMOCK_INTERNAL_MATCHER_MEMBERS_USAGE(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_MEMBER_USAGE, , args))
#define GMOCK_INTERNAL_MATCHER_MEMBER_USAGE(i_unused, data_unused, arg) , arg

#define GMOCK_INTERNAL_MATCHER_ARGS_USAGE(args) \
  GMOCK_PP_TAIL(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_MATCHER_ARG_USAGE, , args))
#define GMOCK_INTERNAL_MATCHER_ARG_USAGE(i, data_unused, arg_unused) \
  , gmock_p##i

using namespace no_adl;

}

GTEST_DISABLE_MSC_WARNINGS_POP_()

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_MATCHERS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_MATCHERS_H_
#endif

#endif

#if GTEST_HAS_EXCEPTIONS
# include <stdexcept>
#endif

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
 )

namespace testing {

class Expectation;

class ExpectationSet;

namespace internal {

template <typename F> class FunctionMocker;

class ExpectationBase;

template <typename F> class TypedExpectation;

class ExpectationTester;

template <typename MockClass>
class NiceMockImpl;
template <typename MockClass>
class StrictMockImpl;
template <typename MockClass>
class NaggyMockImpl;

GTEST_API_ GTEST_DECLARE_STATIC_MUTEX_(g_gmock_mutex);

class UntypedActionResultHolderBase;

class GTEST_API_ UntypedFunctionMockerBase {
 public:
  UntypedFunctionMockerBase();
  virtual ~UntypedFunctionMockerBase();

  bool VerifyAndClearExpectationsLocked()
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex);

  virtual void ClearDefaultActionsLocked()
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) = 0;

  virtual UntypedActionResultHolderBase* UntypedPerformDefaultAction(
      void* untyped_args, const std::string& call_description) const = 0;

  virtual UntypedActionResultHolderBase* UntypedPerformAction(
      const void* untyped_action, void* untyped_args) const = 0;

  virtual void UntypedDescribeUninterestingCall(
      const void* untyped_args,
      ::std::ostream* os) const
          GTEST_LOCK_EXCLUDED_(g_gmock_mutex) = 0;

  virtual const ExpectationBase* UntypedFindMatchingExpectation(
      const void* untyped_args,
      const void** untyped_action, bool* is_excessive,
      ::std::ostream* what, ::std::ostream* why)
          GTEST_LOCK_EXCLUDED_(g_gmock_mutex) = 0;

  virtual void UntypedPrintArgs(const void* untyped_args,
                                ::std::ostream* os) const = 0;

  void RegisterOwner(const void* mock_obj)
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex);

  void SetOwnerAndName(const void* mock_obj, const char* name)
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex);

  const void* MockObject() const
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex);

  const char* Name() const
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex);

  UntypedActionResultHolderBase* UntypedInvokeWith(void* untyped_args)
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex);

 protected:
  typedef std::vector<const void*> UntypedOnCallSpecs;

  using UntypedExpectations = std::vector<std::shared_ptr<ExpectationBase>>;

  Expectation GetHandleOf(ExpectationBase* exp);

  const void* mock_obj_;

  const char* name_;

  UntypedOnCallSpecs untyped_on_call_specs_;

  UntypedExpectations untyped_expectations_;
};

class UntypedOnCallSpecBase {
 public:

  UntypedOnCallSpecBase(const char* a_file, int a_line)
      : file_(a_file), line_(a_line), last_clause_(kNone) {}

  const char* file() const { return file_; }
  int line() const { return line_; }

 protected:

  enum Clause {

    kNone,
    kWith,
    kWillByDefault
  };

  void AssertSpecProperty(bool property,
                          const std::string& failure_message) const {
    Assert(property, file_, line_, failure_message);
  }

  void ExpectSpecProperty(bool property,
                          const std::string& failure_message) const {
    Expect(property, file_, line_, failure_message);
  }

  const char* file_;
  int line_;

  Clause last_clause_;
};

template <typename F>
class OnCallSpec : public UntypedOnCallSpecBase {
 public:
  typedef typename Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename Function<F>::ArgumentMatcherTuple ArgumentMatcherTuple;

  OnCallSpec(const char* a_file, int a_line,
             const ArgumentMatcherTuple& matchers)
      : UntypedOnCallSpecBase(a_file, a_line),
        matchers_(matchers),

        extra_matcher_(A<const ArgumentTuple&>()) {}

  OnCallSpec& With(const Matcher<const ArgumentTuple&>& m) {

    ExpectSpecProperty(last_clause_ < kWith,
                       ".With() cannot appear "
                       "more than once in an ON_CALL().");
    last_clause_ = kWith;

    extra_matcher_ = m;
    return *this;
  }

  OnCallSpec& WillByDefault(const Action<F>& action) {
    ExpectSpecProperty(last_clause_ < kWillByDefault,
                       ".WillByDefault() must appear "
                       "exactly once in an ON_CALL().");
    last_clause_ = kWillByDefault;

    ExpectSpecProperty(!action.IsDoDefault(),
                       "DoDefault() cannot be used in ON_CALL().");
    action_ = action;
    return *this;
  }

  bool Matches(const ArgumentTuple& args) const {
    return TupleMatches(matchers_, args) && extra_matcher_.Matches(args);
  }

  const Action<F>& GetAction() const {
    AssertSpecProperty(last_clause_ == kWillByDefault,
                       ".WillByDefault() must appear exactly "
                       "once in an ON_CALL().");
    return action_;
  }

 private:

  ArgumentMatcherTuple matchers_;
  Matcher<const ArgumentTuple&> extra_matcher_;
  Action<F> action_;
};

enum CallReaction {
  kAllow,
  kWarn,
  kFail,
};

}

class GTEST_API_ Mock {
 public:

  static void AllowLeak(const void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static bool VerifyAndClearExpectations(void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static bool VerifyAndClear(void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static bool IsNaggy(void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static bool IsNice(void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static bool IsStrict(void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

 private:
  friend class internal::UntypedFunctionMockerBase;

  template <typename F>
  friend class internal::FunctionMocker;

  template <typename MockClass>
  friend class internal::NiceMockImpl;
  template <typename MockClass>
  friend class internal::NaggyMockImpl;
  template <typename MockClass>
  friend class internal::StrictMockImpl;

  static void AllowUninterestingCalls(const void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static void WarnUninterestingCalls(const void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static void FailUninterestingCalls(const void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static void UnregisterCallReaction(const void* mock_obj)
      GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static internal::CallReaction GetReactionOnUninterestingCalls(
      const void* mock_obj)
          GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static bool VerifyAndClearExpectationsLocked(void* mock_obj)
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex);

  static void ClearDefaultActionsLocked(void* mock_obj)
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex);

  static void Register(
      const void* mock_obj,
      internal::UntypedFunctionMockerBase* mocker)
          GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static void RegisterUseByOnCallOrExpectCall(
      const void* mock_obj, const char* file, int line)
          GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex);

  static void UnregisterLocked(internal::UntypedFunctionMockerBase* mocker)
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex);
};

class GTEST_API_ Expectation {
 public:

  Expectation();
  Expectation(Expectation&&) = default;
  Expectation(const Expectation&) = default;
  Expectation& operator=(Expectation&&) = default;
  Expectation& operator=(const Expectation&) = default;
  ~Expectation();

  Expectation(internal::ExpectationBase& exp);

  bool operator==(const Expectation& rhs) const {
    return expectation_base_ == rhs.expectation_base_;
  }

  bool operator!=(const Expectation& rhs) const { return !(*this == rhs); }

 private:
  friend class ExpectationSet;
  friend class Sequence;
  friend class ::testing::internal::ExpectationBase;
  friend class ::testing::internal::UntypedFunctionMockerBase;

  template <typename F>
  friend class ::testing::internal::FunctionMocker;

  template <typename F>
  friend class ::testing::internal::TypedExpectation;

  class Less {
   public:
    bool operator()(const Expectation& lhs, const Expectation& rhs) const {
      return lhs.expectation_base_.get() < rhs.expectation_base_.get();
    }
  };

  typedef ::std::set<Expectation, Less> Set;

  Expectation(
      const std::shared_ptr<internal::ExpectationBase>& expectation_base);

  const std::shared_ptr<internal::ExpectationBase>& expectation_base() const {
    return expectation_base_;
  }

  std::shared_ptr<internal::ExpectationBase> expectation_base_;
};

class ExpectationSet {
 public:

  typedef Expectation::Set::const_iterator const_iterator;

  typedef Expectation::Set::value_type value_type;

  ExpectationSet() {}

  ExpectationSet(internal::ExpectationBase& exp) {
    *this += Expectation(exp);
  }

  ExpectationSet(const Expectation& e) {
    *this += e;
  }

  bool operator==(const ExpectationSet& rhs) const {
    return expectations_ == rhs.expectations_;
  }

  bool operator!=(const ExpectationSet& rhs) const { return !(*this == rhs); }

  ExpectationSet& operator+=(const Expectation& e) {
    expectations_.insert(e);
    return *this;
  }

  int size() const { return static_cast<int>(expectations_.size()); }

  const_iterator begin() const { return expectations_.begin(); }
  const_iterator end() const { return expectations_.end(); }

 private:
  Expectation::Set expectations_;
};

class GTEST_API_ Sequence {
 public:

  Sequence() : last_expectation_(new Expectation) {}

  void AddExpectation(const Expectation& expectation) const;

 private:

  std::shared_ptr<Expectation> last_expectation_;
};

class GTEST_API_ InSequence {
 public:
  InSequence();
  ~InSequence();
 private:
  bool sequence_created_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(InSequence);
} GTEST_ATTRIBUTE_UNUSED_;

namespace internal {

GTEST_API_ extern ThreadLocal<Sequence*> g_gmock_implicit_sequence;

class GTEST_API_ ExpectationBase {
 public:

  ExpectationBase(const char* file, int line, const std::string& source_text);

  virtual ~ExpectationBase();

  const char* file() const { return file_; }
  int line() const { return line_; }
  const char* source_text() const { return source_text_.c_str(); }

  const Cardinality& cardinality() const { return cardinality_; }

  void DescribeLocationTo(::std::ostream* os) const {
    *os << FormatFileLocation(file(), line()) << " ";
  }

  void DescribeCallCountTo(::std::ostream* os) const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex);

  virtual void MaybeDescribeExtraMatcherTo(::std::ostream* os) = 0;

 protected:
  friend class ::testing::Expectation;
  friend class UntypedFunctionMockerBase;

  enum Clause {

    kNone,
    kWith,
    kTimes,
    kInSequence,
    kAfter,
    kWillOnce,
    kWillRepeatedly,
    kRetiresOnSaturation
  };

  typedef std::vector<const void*> UntypedActions;

  virtual Expectation GetHandle() = 0;

  void AssertSpecProperty(bool property,
                          const std::string& failure_message) const {
    Assert(property, file_, line_, failure_message);
  }

  void ExpectSpecProperty(bool property,
                          const std::string& failure_message) const {
    Expect(property, file_, line_, failure_message);
  }

  void SpecifyCardinality(const Cardinality& cardinality);

  bool cardinality_specified() const { return cardinality_specified_; }

  void set_cardinality(const Cardinality& a_cardinality) {
    cardinality_ = a_cardinality;
  }

  void RetireAllPreRequisites()
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex);

  bool is_retired() const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    return retired_;
  }

  void Retire()
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    retired_ = true;
  }

  bool IsSatisfied() const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    return cardinality().IsSatisfiedByCallCount(call_count_);
  }

  bool IsSaturated() const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    return cardinality().IsSaturatedByCallCount(call_count_);
  }

  bool IsOverSaturated() const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    return cardinality().IsOverSaturatedByCallCount(call_count_);
  }

  bool AllPrerequisitesAreSatisfied() const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex);

  void FindUnsatisfiedPrerequisites(ExpectationSet* result) const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex);

  int call_count() const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    return call_count_;
  }

  void IncrementCallCount()
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    call_count_++;
  }

  void CheckActionCountIfNotDone() const
      GTEST_LOCK_EXCLUDED_(mutex_);

  friend class ::testing::Sequence;
  friend class ::testing::internal::ExpectationTester;

  template <typename Function>
  friend class TypedExpectation;

  void UntypedTimes(const Cardinality& a_cardinality);

  const char* file_;
  int line_;
  const std::string source_text_;

  bool cardinality_specified_;
  Cardinality cardinality_;

  ExpectationSet immediate_prerequisites_;

  int call_count_;
  bool retired_;
  UntypedActions untyped_actions_;
  bool extra_matcher_specified_;
  bool repeated_action_specified_;
  bool retires_on_saturation_;
  Clause last_clause_;
  mutable bool action_count_checked_;
  mutable Mutex mutex_;
};

template <typename F>
class TypedExpectation : public ExpectationBase {
 public:
  typedef typename Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename Function<F>::ArgumentMatcherTuple ArgumentMatcherTuple;
  typedef typename Function<F>::Result Result;

  TypedExpectation(FunctionMocker<F>* owner, const char* a_file, int a_line,
                   const std::string& a_source_text,
                   const ArgumentMatcherTuple& m)
      : ExpectationBase(a_file, a_line, a_source_text),
        owner_(owner),
        matchers_(m),

        extra_matcher_(A<const ArgumentTuple&>()),
        repeated_action_(DoDefault()) {}

  ~TypedExpectation() override {

    CheckActionCountIfNotDone();
    for (UntypedActions::const_iterator it = untyped_actions_.begin();
         it != untyped_actions_.end(); ++it) {
      delete static_cast<const Action<F>*>(*it);
    }
  }

  TypedExpectation& With(const Matcher<const ArgumentTuple&>& m) {
    if (last_clause_ == kWith) {
      ExpectSpecProperty(false,
                         ".With() cannot appear "
                         "more than once in an EXPECT_CALL().");
    } else {
      ExpectSpecProperty(last_clause_ < kWith,
                         ".With() must be the first "
                         "clause in an EXPECT_CALL().");
    }
    last_clause_ = kWith;

    extra_matcher_ = m;
    extra_matcher_specified_ = true;
    return *this;
  }

  TypedExpectation& Times(const Cardinality& a_cardinality) {
    ExpectationBase::UntypedTimes(a_cardinality);
    return *this;
  }

  TypedExpectation& Times(int n) {
    return Times(Exactly(n));
  }

  TypedExpectation& InSequence(const Sequence& s) {
    ExpectSpecProperty(last_clause_ <= kInSequence,
                       ".InSequence() cannot appear after .After(),"
                       " .WillOnce(), .WillRepeatedly(), or "
                       ".RetiresOnSaturation().");
    last_clause_ = kInSequence;

    s.AddExpectation(GetHandle());
    return *this;
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2) {
    return InSequence(s1).InSequence(s2);
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2,
                               const Sequence& s3) {
    return InSequence(s1, s2).InSequence(s3);
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2,
                               const Sequence& s3, const Sequence& s4) {
    return InSequence(s1, s2, s3).InSequence(s4);
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2,
                               const Sequence& s3, const Sequence& s4,
                               const Sequence& s5) {
    return InSequence(s1, s2, s3, s4).InSequence(s5);
  }

  TypedExpectation& After(const ExpectationSet& s) {
    ExpectSpecProperty(last_clause_ <= kAfter,
                       ".After() cannot appear after .WillOnce(),"
                       " .WillRepeatedly(), or "
                       ".RetiresOnSaturation().");
    last_clause_ = kAfter;

    for (ExpectationSet::const_iterator it = s.begin(); it != s.end(); ++it) {
      immediate_prerequisites_ += *it;
    }
    return *this;
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2) {
    return After(s1).After(s2);
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2,
                          const ExpectationSet& s3) {
    return After(s1, s2).After(s3);
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2,
                          const ExpectationSet& s3, const ExpectationSet& s4) {
    return After(s1, s2, s3).After(s4);
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2,
                          const ExpectationSet& s3, const ExpectationSet& s4,
                          const ExpectationSet& s5) {
    return After(s1, s2, s3, s4).After(s5);
  }

  TypedExpectation& WillOnce(const Action<F>& action) {
    ExpectSpecProperty(last_clause_ <= kWillOnce,
                       ".WillOnce() cannot appear after "
                       ".WillRepeatedly() or .RetiresOnSaturation().");
    last_clause_ = kWillOnce;

    untyped_actions_.push_back(new Action<F>(action));
    if (!cardinality_specified()) {
      set_cardinality(Exactly(static_cast<int>(untyped_actions_.size())));
    }
    return *this;
  }

  TypedExpectation& WillRepeatedly(const Action<F>& action) {
    if (last_clause_ == kWillRepeatedly) {
      ExpectSpecProperty(false,
                         ".WillRepeatedly() cannot appear "
                         "more than once in an EXPECT_CALL().");
    } else {
      ExpectSpecProperty(last_clause_ < kWillRepeatedly,
                         ".WillRepeatedly() cannot appear "
                         "after .RetiresOnSaturation().");
    }
    last_clause_ = kWillRepeatedly;
    repeated_action_specified_ = true;

    repeated_action_ = action;
    if (!cardinality_specified()) {
      set_cardinality(AtLeast(static_cast<int>(untyped_actions_.size())));
    }

    CheckActionCountIfNotDone();
    return *this;
  }

  TypedExpectation& RetiresOnSaturation() {
    ExpectSpecProperty(last_clause_ < kRetiresOnSaturation,
                       ".RetiresOnSaturation() cannot appear "
                       "more than once.");
    last_clause_ = kRetiresOnSaturation;
    retires_on_saturation_ = true;

    CheckActionCountIfNotDone();
    return *this;
  }

  const ArgumentMatcherTuple& matchers() const {
    return matchers_;
  }

  const Matcher<const ArgumentTuple&>& extra_matcher() const {
    return extra_matcher_;
  }

  const Action<F>& repeated_action() const { return repeated_action_; }

  void MaybeDescribeExtraMatcherTo(::std::ostream* os) override {
    if (extra_matcher_specified_) {
      *os << "    Expected args: ";
      extra_matcher_.DescribeTo(os);
      *os << "\n";
    }
  }

 private:
  template <typename Function>
  friend class FunctionMocker;

  Expectation GetHandle() override { return owner_->GetHandleOf(this); }

  bool Matches(const ArgumentTuple& args) const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    return TupleMatches(matchers_, args) && extra_matcher_.Matches(args);
  }

  bool ShouldHandleArguments(const ArgumentTuple& args) const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();

    CheckActionCountIfNotDone();
    return !is_retired() && AllPrerequisitesAreSatisfied() && Matches(args);
  }

  void ExplainMatchResultTo(
      const ArgumentTuple& args,
      ::std::ostream* os) const
          GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();

    if (is_retired()) {
      *os << "         Expected: the expectation is active\n"
          << "           Actual: it is retired\n";
    } else if (!Matches(args)) {
      if (!TupleMatches(matchers_, args)) {
        ExplainMatchFailureTupleTo(matchers_, args, os);
      }
      StringMatchResultListener listener;
      if (!extra_matcher_.MatchAndExplain(args, &listener)) {
        *os << "    Expected args: ";
        extra_matcher_.DescribeTo(os);
        *os << "\n           Actual: don't match";

        internal::PrintIfNotEmpty(listener.str(), os);
        *os << "\n";
      }
    } else if (!AllPrerequisitesAreSatisfied()) {
      *os << "         Expected: all pre-requisites are satisfied\n"
          << "           Actual: the following immediate pre-requisites "
          << "are not satisfied:\n";
      ExpectationSet unsatisfied_prereqs;
      FindUnsatisfiedPrerequisites(&unsatisfied_prereqs);
      int i = 0;
      for (ExpectationSet::const_iterator it = unsatisfied_prereqs.begin();
           it != unsatisfied_prereqs.end(); ++it) {
        it->expectation_base()->DescribeLocationTo(os);
        *os << "pre-requisite #" << i++ << "\n";
      }
      *os << "                   (end of pre-requisites)\n";
    } else {

      *os << "The call matches the expectation.\n";
    }
  }

  const Action<F>& GetCurrentAction(const FunctionMocker<F>* mocker,
                                    const ArgumentTuple& args) const
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    const int count = call_count();
    Assert(count >= 1, __FILE__, __LINE__,
           "call_count() is <= 0 when GetCurrentAction() is "
           "called - this should never happen.");

    const int action_count = static_cast<int>(untyped_actions_.size());
    if (action_count > 0 && !repeated_action_specified_ &&
        count > action_count) {

      ::std::stringstream ss;
      DescribeLocationTo(&ss);
      ss << "Actions ran out in " << source_text() << "...\n"
         << "Called " << count << " times, but only "
         << action_count << " WillOnce()"
         << (action_count == 1 ? " is" : "s are") << " specified - ";
      mocker->DescribeDefaultActionTo(args, &ss);
      Log(kWarning, ss.str(), 1);
    }

    return count <= action_count
               ? *static_cast<const Action<F>*>(
                     untyped_actions_[static_cast<size_t>(count - 1)])
               : repeated_action();
  }

  const Action<F>* GetActionForArguments(const FunctionMocker<F>* mocker,
                                         const ArgumentTuple& args,
                                         ::std::ostream* what,
                                         ::std::ostream* why)
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    if (IsSaturated()) {

      IncrementCallCount();
      *what << "Mock function called more times than expected - ";
      mocker->DescribeDefaultActionTo(args, what);
      DescribeCallCountTo(why);

      return nullptr;
    }

    IncrementCallCount();
    RetireAllPreRequisites();

    if (retires_on_saturation_ && IsSaturated()) {
      Retire();
    }

    *what << "Mock function call matches " << source_text() <<"...\n";
    return &(GetCurrentAction(mocker, args));
  }

  FunctionMocker<F>* const owner_;
  ArgumentMatcherTuple matchers_;
  Matcher<const ArgumentTuple&> extra_matcher_;
  Action<F> repeated_action_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TypedExpectation);
};

GTEST_API_ void LogWithLocation(testing::internal::LogSeverity severity,
                                const char* file, int line,
                                const std::string& message);

template <typename F>
class MockSpec {
 public:
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename internal::Function<F>::ArgumentMatcherTuple
      ArgumentMatcherTuple;

  MockSpec(internal::FunctionMocker<F>* function_mocker,
           const ArgumentMatcherTuple& matchers)
      : function_mocker_(function_mocker), matchers_(matchers) {}

  internal::OnCallSpec<F>& InternalDefaultActionSetAt(
      const char* file, int line, const char* obj, const char* call) {
    LogWithLocation(internal::kInfo, file, line,
                    std::string("ON_CALL(") + obj + ", " + call + ") invoked");
    return function_mocker_->AddNewOnCallSpec(file, line, matchers_);
  }

  internal::TypedExpectation<F>& InternalExpectedAt(
      const char* file, int line, const char* obj, const char* call) {
    const std::string source_text(std::string("EXPECT_CALL(") + obj + ", " +
                                  call + ")");
    LogWithLocation(internal::kInfo, file, line, source_text + " invoked");
    return function_mocker_->AddNewExpectation(
        file, line, source_text, matchers_);
  }

  MockSpec<F>& operator()(const internal::WithoutMatchers&, void* const) {
    return *this;
  }

 private:
  template <typename Function>
  friend class internal::FunctionMocker;

  internal::FunctionMocker<F>* const function_mocker_;

  ArgumentMatcherTuple matchers_;
};

template <typename T>
class ReferenceOrValueWrapper {
 public:

  explicit ReferenceOrValueWrapper(T value)
      : value_(std::move(value)) {
  }

  T Unwrap() { return std::move(value_); }

  const T& Peek() const {
    return value_;
  }

 private:
  T value_;
};

template <typename T>
class ReferenceOrValueWrapper<T&> {
 public:

  typedef T& reference;
  explicit ReferenceOrValueWrapper(reference ref)
      : value_ptr_(&ref) {}
  T& Unwrap() { return *value_ptr_; }
  const T& Peek() const { return *value_ptr_; }

 private:
  T* value_ptr_;
};

class UntypedActionResultHolderBase {
 public:
  virtual ~UntypedActionResultHolderBase() {}

  virtual void PrintAsActionResult(::std::ostream* os) const = 0;
};

template <typename T>
class ActionResultHolder : public UntypedActionResultHolderBase {
 public:

  T Unwrap() {
    return result_.Unwrap();
  }

  void PrintAsActionResult(::std::ostream* os) const override {
    *os << "\n          Returns: ";

    UniversalPrinter<T>::Print(result_.Peek(), os);
  }

  template <typename F>
  static ActionResultHolder* PerformDefaultAction(
      const FunctionMocker<F>* func_mocker,
      typename Function<F>::ArgumentTuple&& args,
      const std::string& call_description) {
    return new ActionResultHolder(Wrapper(func_mocker->PerformDefaultAction(
        std::move(args), call_description)));
  }

  template <typename F>
  static ActionResultHolder* PerformAction(
      const Action<F>& action, typename Function<F>::ArgumentTuple&& args) {
    return new ActionResultHolder(
        Wrapper(action.Perform(std::move(args))));
  }

 private:
  typedef ReferenceOrValueWrapper<T> Wrapper;

  explicit ActionResultHolder(Wrapper result)
      : result_(std::move(result)) {
  }

  Wrapper result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ActionResultHolder);
};

template <>
class ActionResultHolder<void> : public UntypedActionResultHolderBase {
 public:
  void Unwrap() { }

  void PrintAsActionResult(::std::ostream*  ) const override {}

  template <typename F>
  static ActionResultHolder* PerformDefaultAction(
      const FunctionMocker<F>* func_mocker,
      typename Function<F>::ArgumentTuple&& args,
      const std::string& call_description) {
    func_mocker->PerformDefaultAction(std::move(args), call_description);
    return new ActionResultHolder;
  }

  template <typename F>
  static ActionResultHolder* PerformAction(
      const Action<F>& action, typename Function<F>::ArgumentTuple&& args) {
    action.Perform(std::move(args));
    return new ActionResultHolder;
  }

 private:
  ActionResultHolder() {}
  GTEST_DISALLOW_COPY_AND_ASSIGN_(ActionResultHolder);
};

template <typename F>
class FunctionMocker;

template <typename R, typename... Args>
class FunctionMocker<R(Args...)> final : public UntypedFunctionMockerBase {
  using F = R(Args...);

 public:
  using Result = R;
  using ArgumentTuple = std::tuple<Args...>;
  using ArgumentMatcherTuple = std::tuple<Matcher<Args>...>;

  FunctionMocker() {}

  FunctionMocker(const FunctionMocker&) = delete;
  FunctionMocker& operator=(const FunctionMocker&) = delete;

  ~FunctionMocker() override GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
    MutexLock l(&g_gmock_mutex);
    VerifyAndClearExpectationsLocked();
    Mock::UnregisterLocked(this);
    ClearDefaultActionsLocked();
  }

  const OnCallSpec<F>* FindOnCallSpec(
      const ArgumentTuple& args) const {
    for (UntypedOnCallSpecs::const_reverse_iterator it
             = untyped_on_call_specs_.rbegin();
         it != untyped_on_call_specs_.rend(); ++it) {
      const OnCallSpec<F>* spec = static_cast<const OnCallSpec<F>*>(*it);
      if (spec->Matches(args))
        return spec;
    }

    return nullptr;
  }

  Result PerformDefaultAction(ArgumentTuple&& args,
                              const std::string& call_description) const {
    const OnCallSpec<F>* const spec =
        this->FindOnCallSpec(args);
    if (spec != nullptr) {
      return spec->GetAction().Perform(std::move(args));
    }
    const std::string message =
        call_description +
        "\n    The mock function has no default action "
        "set, and its return type has no default value set.";
#if GTEST_HAS_EXCEPTIONS
    if (!DefaultValue<Result>::Exists()) {
      throw std::runtime_error(message);
    }
#else
    Assert(DefaultValue<Result>::Exists(), "", -1, message);
#endif
    return DefaultValue<Result>::Get();
  }

  UntypedActionResultHolderBase* UntypedPerformDefaultAction(
      void* untyped_args,
      const std::string& call_description) const override {
    ArgumentTuple* args = static_cast<ArgumentTuple*>(untyped_args);
    return ResultHolder::PerformDefaultAction(this, std::move(*args),
                                              call_description);
  }

  UntypedActionResultHolderBase* UntypedPerformAction(
      const void* untyped_action, void* untyped_args) const override {

    const Action<F> action = *static_cast<const Action<F>*>(untyped_action);
    ArgumentTuple* args = static_cast<ArgumentTuple*>(untyped_args);
    return ResultHolder::PerformAction(action, std::move(*args));
  }

  void ClearDefaultActionsLocked() override
      GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();

    UntypedOnCallSpecs specs_to_delete;
    untyped_on_call_specs_.swap(specs_to_delete);

    g_gmock_mutex.Unlock();
    for (UntypedOnCallSpecs::const_iterator it =
             specs_to_delete.begin();
         it != specs_to_delete.end(); ++it) {
      delete static_cast<const OnCallSpec<F>*>(*it);
    }

    g_gmock_mutex.Lock();
  }

  Result Invoke(Args... args) GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
    ArgumentTuple tuple(std::forward<Args>(args)...);
    std::unique_ptr<ResultHolder> holder(DownCast_<ResultHolder*>(
        this->UntypedInvokeWith(static_cast<void*>(&tuple))));
    return holder->Unwrap();
  }

  MockSpec<F> With(Matcher<Args>... m) {
    return MockSpec<F>(this, ::std::make_tuple(std::move(m)...));
  }

 protected:
  template <typename Function>
  friend class MockSpec;

  typedef ActionResultHolder<Result> ResultHolder;

  OnCallSpec<F>& AddNewOnCallSpec(
      const char* file, int line,
      const ArgumentMatcherTuple& m)
          GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
    Mock::RegisterUseByOnCallOrExpectCall(MockObject(), file, line);
    OnCallSpec<F>* const on_call_spec = new OnCallSpec<F>(file, line, m);
    untyped_on_call_specs_.push_back(on_call_spec);
    return *on_call_spec;
  }

  TypedExpectation<F>& AddNewExpectation(const char* file, int line,
                                         const std::string& source_text,
                                         const ArgumentMatcherTuple& m)
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
    Mock::RegisterUseByOnCallOrExpectCall(MockObject(), file, line);
    TypedExpectation<F>* const expectation =
        new TypedExpectation<F>(this, file, line, source_text, m);
    const std::shared_ptr<ExpectationBase> untyped_expectation(expectation);

    untyped_expectations_.push_back(untyped_expectation);

    Sequence* const implicit_sequence = g_gmock_implicit_sequence.get();
    if (implicit_sequence != nullptr) {
      implicit_sequence->AddExpectation(Expectation(untyped_expectation));
    }

    return *expectation;
  }

 private:
  template <typename Func> friend class TypedExpectation;

  void DescribeDefaultActionTo(const ArgumentTuple& args,
                               ::std::ostream* os) const {
    const OnCallSpec<F>* const spec = FindOnCallSpec(args);

    if (spec == nullptr) {
      *os << (std::is_void<Result>::value ? "returning directly.\n"
                                          : "returning default value.\n");
    } else {
      *os << "taking default action specified at:\n"
          << FormatFileLocation(spec->file(), spec->line()) << "\n";
    }
  }

  void UntypedDescribeUninterestingCall(const void* untyped_args,
                                        ::std::ostream* os) const override
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
    const ArgumentTuple& args =
        *static_cast<const ArgumentTuple*>(untyped_args);
    *os << "Uninteresting mock function call - ";
    DescribeDefaultActionTo(args, os);
    *os << "    Function call: " << Name();
    UniversalPrint(args, os);
  }

  const ExpectationBase* UntypedFindMatchingExpectation(
      const void* untyped_args, const void** untyped_action, bool* is_excessive,
      ::std::ostream* what, ::std::ostream* why) override
      GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
    const ArgumentTuple& args =
        *static_cast<const ArgumentTuple*>(untyped_args);
    MutexLock l(&g_gmock_mutex);
    TypedExpectation<F>* exp = this->FindMatchingExpectationLocked(args);
    if (exp == nullptr) {
      this->FormatUnexpectedCallMessageLocked(args, what, why);
      return nullptr;
    }

    *is_excessive = exp->IsSaturated();
    const Action<F>* action = exp->GetActionForArguments(this, args, what, why);
    if (action != nullptr && action->IsDoDefault())
      action = nullptr;
    *untyped_action = action;
    return exp;
  }

  void UntypedPrintArgs(const void* untyped_args,
                        ::std::ostream* os) const override {
    const ArgumentTuple& args =
        *static_cast<const ArgumentTuple*>(untyped_args);
    UniversalPrint(args, os);
  }

  TypedExpectation<F>* FindMatchingExpectationLocked(
      const ArgumentTuple& args) const
          GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();

    for (typename UntypedExpectations::const_reverse_iterator it =
             untyped_expectations_.rbegin();
         it != untyped_expectations_.rend(); ++it) {
      TypedExpectation<F>* const exp =
          static_cast<TypedExpectation<F>*>(it->get());
      if (exp->ShouldHandleArguments(args)) {
        return exp;
      }
    }
    return nullptr;
  }

  void FormatUnexpectedCallMessageLocked(
      const ArgumentTuple& args,
      ::std::ostream* os,
      ::std::ostream* why) const
          GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    *os << "\nUnexpected mock function call - ";
    DescribeDefaultActionTo(args, os);
    PrintTriedExpectationsLocked(args, why);
  }

  void PrintTriedExpectationsLocked(
      const ArgumentTuple& args,
      ::std::ostream* why) const
          GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
    g_gmock_mutex.AssertHeld();
    const size_t count = untyped_expectations_.size();
    *why << "Google Mock tried the following " << count << " "
         << (count == 1 ? "expectation, but it didn't match" :
             "expectations, but none matched")
         << ":\n";
    for (size_t i = 0; i < count; i++) {
      TypedExpectation<F>* const expectation =
          static_cast<TypedExpectation<F>*>(untyped_expectations_[i].get());
      *why << "\n";
      expectation->DescribeLocationTo(why);
      if (count > 1) {
        *why << "tried expectation #" << i << ": ";
      }
      *why << expectation->source_text() << "...\n";
      expectation->ExplainMatchResultTo(args, why);
      expectation->DescribeCallCountTo(why);
    }
  }
};

void ReportUninterestingCall(CallReaction reaction, const std::string& msg);

}

namespace internal {

template <typename F>
class MockFunction;

template <typename R, typename... Args>
class MockFunction<R(Args...)> {
 public:
  MockFunction(const MockFunction&) = delete;
  MockFunction& operator=(const MockFunction&) = delete;

  std::function<R(Args...)> AsStdFunction() {
    return [this](Args... args) -> R {
      return this->Call(std::forward<Args>(args)...);
    };
  }

  R Call(Args... args) {
    mock_.SetOwnerAndName(this, "Call");
    return mock_.Invoke(std::forward<Args>(args)...);
  }

  MockSpec<R(Args...)> gmock_Call(Matcher<Args>... m) {
    mock_.RegisterOwner(this);
    return mock_.With(std::move(m)...);
  }

  MockSpec<R(Args...)> gmock_Call(const WithoutMatchers&, R (*)(Args...)) {
    return this->gmock_Call(::testing::A<Args>()...);
  }

 protected:
  MockFunction() = default;
  ~MockFunction() = default;

 private:
  FunctionMocker<R(Args...)> mock_;
};

template <typename F>
struct SignatureOf;

template <typename R, typename... Args>
struct SignatureOf<R(Args...)> {
  using type = R(Args...);
};

template <typename F>
struct SignatureOf<std::function<F>> : SignatureOf<F> {};

template <typename F>
using SignatureOfT = typename SignatureOf<F>::type;

}

template <typename F>
class MockFunction : public internal::MockFunction<internal::SignatureOfT<F>> {
  using Base = internal::MockFunction<internal::SignatureOfT<F>>;

 public:
  using Base::Base;
};

using internal::MockSpec;

template <typename T>
inline const T& Const(const T& x) { return x; }

inline Expectation::Expectation(internal::ExpectationBase& exp)
    : expectation_base_(exp.GetHandle().expectation_base()) {}

}

GTEST_DISABLE_MSC_WARNINGS_POP_()

#define GMOCK_ON_CALL_IMPL_(mock_expr, Setter, call)                    \
  ((mock_expr).gmock_##call)(::testing::internal::GetWithoutMatchers(), \
                             nullptr)                                   \
      .Setter(__FILE__, __LINE__, #mock_expr, #call)

#define ON_CALL(obj, call) \
  GMOCK_ON_CALL_IMPL_(obj, InternalDefaultActionSetAt, call)

#define EXPECT_CALL(obj, call) \
  GMOCK_ON_CALL_IMPL_(obj, InternalExpectedAt, call)

#endif

namespace testing {
namespace internal {
template <typename T>
using identity_t = T;

template <typename Pattern>
struct ThisRefAdjuster {
  template <typename T>
  using AdjustT = typename std::conditional<
      std::is_const<typename std::remove_reference<Pattern>::type>::value,
      typename std::conditional<std::is_lvalue_reference<Pattern>::value,
                                const T&, const T&&>::type,
      typename std::conditional<std::is_lvalue_reference<Pattern>::value, T&,
                                T&&>::type>::type;

  template <typename MockType>
  static AdjustT<MockType> Adjust(const MockType& mock) {
    return static_cast<AdjustT<MockType>>(const_cast<MockType&>(mock));
  }
};

}

using internal::FunctionMocker;
}

#define MOCK_METHOD(...) \
  GMOCK_PP_VARIADIC_CALL(GMOCK_INTERNAL_MOCK_METHOD_ARG_, __VA_ARGS__)

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_1(...) \
  GMOCK_INTERNAL_WRONG_ARITY(__VA_ARGS__)

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_2(...) \
  GMOCK_INTERNAL_WRONG_ARITY(__VA_ARGS__)

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_3(_Ret, _MethodName, _Args) \
  GMOCK_INTERNAL_MOCK_METHOD_ARG_4(_Ret, _MethodName, _Args, ())

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_4(_Ret, _MethodName, _Args, _Spec)     \
  GMOCK_INTERNAL_ASSERT_PARENTHESIS(_Args);                                   \
  GMOCK_INTERNAL_ASSERT_PARENTHESIS(_Spec);                                   \
  GMOCK_INTERNAL_ASSERT_VALID_SIGNATURE(                                      \
      GMOCK_PP_NARG0 _Args, GMOCK_INTERNAL_SIGNATURE(_Ret, _Args));           \
  GMOCK_INTERNAL_ASSERT_VALID_SPEC(_Spec)                                     \
  GMOCK_INTERNAL_MOCK_METHOD_IMPL(                                            \
      GMOCK_PP_NARG0 _Args, _MethodName, GMOCK_INTERNAL_HAS_CONST(_Spec),     \
      GMOCK_INTERNAL_HAS_OVERRIDE(_Spec), GMOCK_INTERNAL_HAS_FINAL(_Spec),    \
      GMOCK_INTERNAL_GET_NOEXCEPT_SPEC(_Spec),                                \
      GMOCK_INTERNAL_GET_CALLTYPE(_Spec), GMOCK_INTERNAL_GET_REF_SPEC(_Spec), \
      (GMOCK_INTERNAL_SIGNATURE(_Ret, _Args)))

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_5(...) \
  GMOCK_INTERNAL_WRONG_ARITY(__VA_ARGS__)

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_6(...) \
  GMOCK_INTERNAL_WRONG_ARITY(__VA_ARGS__)

#define GMOCK_INTERNAL_MOCK_METHOD_ARG_7(...) \
  GMOCK_INTERNAL_WRONG_ARITY(__VA_ARGS__)

#define GMOCK_INTERNAL_WRONG_ARITY(...)                                      \
  static_assert(                                                             \
      false,                                                                 \
      "MOCK_METHOD must be called with 3 or 4 arguments. _Ret, "             \
      "_MethodName, _Args and optionally _Spec. _Args and _Spec must be "    \
      "enclosed in parentheses. If _Ret is a type with unprotected commas, " \
      "it must also be enclosed in parentheses.")

#define GMOCK_INTERNAL_ASSERT_PARENTHESIS(_Tuple) \
  static_assert(                                  \
      GMOCK_PP_IS_ENCLOSED_PARENS(_Tuple),        \
      GMOCK_PP_STRINGIZE(_Tuple) " should be enclosed in parentheses.")

#define GMOCK_INTERNAL_ASSERT_VALID_SIGNATURE(_N, ...)                 \
  static_assert(                                                       \
      std::is_function<__VA_ARGS__>::value,                            \
      "Signature must be a function type, maybe return type contains " \
      "unprotected comma.");                                           \
  static_assert(                                                       \
      ::testing::tuple_size<typename ::testing::internal::Function<    \
              __VA_ARGS__>::ArgumentTuple>::value == _N,               \
      "This method does not take " GMOCK_PP_STRINGIZE(                 \
          _N) " arguments. Parenthesize all types with unprotected commas.")

#define GMOCK_INTERNAL_ASSERT_VALID_SPEC(_Spec) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_ASSERT_VALID_SPEC_ELEMENT, ~, _Spec)

#define GMOCK_INTERNAL_MOCK_METHOD_IMPL(_N, _MethodName, _Constness,           \
                                        _Override, _Final, _NoexceptSpec,      \
                                        _CallType, _RefSpec, _Signature)       \
  typename ::testing::internal::Function<GMOCK_PP_REMOVE_PARENS(               \
      _Signature)>::Result                                                     \
  GMOCK_INTERNAL_EXPAND(_CallType)                                             \
      _MethodName(GMOCK_PP_REPEAT(GMOCK_INTERNAL_PARAMETER, _Signature, _N))   \
          GMOCK_PP_IF(_Constness, const, ) _RefSpec _NoexceptSpec              \
          GMOCK_PP_IF(_Override, override, ) GMOCK_PP_IF(_Final, final, ) {    \
    GMOCK_MOCKER_(_N, _Constness, _MethodName)                                 \
        .SetOwnerAndName(this, #_MethodName);                                  \
    return GMOCK_MOCKER_(_N, _Constness, _MethodName)                          \
        .Invoke(GMOCK_PP_REPEAT(GMOCK_INTERNAL_FORWARD_ARG, _Signature, _N));  \
  }                                                                            \
  ::testing::MockSpec<GMOCK_PP_REMOVE_PARENS(_Signature)> gmock_##_MethodName( \
      GMOCK_PP_REPEAT(GMOCK_INTERNAL_MATCHER_PARAMETER, _Signature, _N))       \
      GMOCK_PP_IF(_Constness, const, ) _RefSpec {                              \
    GMOCK_MOCKER_(_N, _Constness, _MethodName).RegisterOwner(this);            \
    return GMOCK_MOCKER_(_N, _Constness, _MethodName)                          \
        .With(GMOCK_PP_REPEAT(GMOCK_INTERNAL_MATCHER_ARGUMENT, , _N));         \
  }                                                                            \
  ::testing::MockSpec<GMOCK_PP_REMOVE_PARENS(_Signature)> gmock_##_MethodName( \
      const ::testing::internal::WithoutMatchers&,                             \
      GMOCK_PP_IF(_Constness, const, )::testing::internal::Function<           \
          GMOCK_PP_REMOVE_PARENS(_Signature)>*) const _RefSpec _NoexceptSpec { \
    return ::testing::internal::ThisRefAdjuster<GMOCK_PP_IF(                   \
        _Constness, const, ) int _RefSpec>::Adjust(*this)                      \
        .gmock_##_MethodName(GMOCK_PP_REPEAT(                                  \
            GMOCK_INTERNAL_A_MATCHER_ARGUMENT, _Signature, _N));               \
  }                                                                            \
  mutable ::testing::FunctionMocker<GMOCK_PP_REMOVE_PARENS(_Signature)>        \
      GMOCK_MOCKER_(_N, _Constness, _MethodName)

#define GMOCK_INTERNAL_EXPAND(...) __VA_ARGS__

#define GMOCK_INTERNAL_HAS_CONST(_Tuple) \
  GMOCK_PP_HAS_COMMA(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_DETECT_CONST, ~, _Tuple))

#define GMOCK_INTERNAL_HAS_OVERRIDE(_Tuple) \
  GMOCK_PP_HAS_COMMA(                       \
      GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_DETECT_OVERRIDE, ~, _Tuple))

#define GMOCK_INTERNAL_HAS_FINAL(_Tuple) \
  GMOCK_PP_HAS_COMMA(GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_DETECT_FINAL, ~, _Tuple))

#define GMOCK_INTERNAL_GET_NOEXCEPT_SPEC(_Tuple) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_NOEXCEPT_SPEC_IF_NOEXCEPT, ~, _Tuple)

#define GMOCK_INTERNAL_NOEXCEPT_SPEC_IF_NOEXCEPT(_i, _, _elem)          \
  GMOCK_PP_IF(                                                          \
      GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_NOEXCEPT(_i, _, _elem)), \
      _elem, )

#define GMOCK_INTERNAL_GET_REF_SPEC(_Tuple) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_REF_SPEC_IF_REF, ~, _Tuple)

#define GMOCK_INTERNAL_REF_SPEC_IF_REF(_i, _, _elem)                       \
  GMOCK_PP_IF(GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_REF(_i, _, _elem)), \
              GMOCK_PP_CAT(GMOCK_INTERNAL_UNPACK_, _elem), )

#define GMOCK_INTERNAL_GET_CALLTYPE(_Tuple) \
  GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_GET_CALLTYPE_IMPL, ~, _Tuple)

#define GMOCK_INTERNAL_ASSERT_VALID_SPEC_ELEMENT(_i, _, _elem)            \
  static_assert(                                                          \
      (GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_CONST(_i, _, _elem)) +    \
       GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_OVERRIDE(_i, _, _elem)) + \
       GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_FINAL(_i, _, _elem)) +    \
       GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_NOEXCEPT(_i, _, _elem)) + \
       GMOCK_PP_HAS_COMMA(GMOCK_INTERNAL_DETECT_REF(_i, _, _elem)) +      \
       GMOCK_INTERNAL_IS_CALLTYPE(_elem)) == 1,                           \
      GMOCK_PP_STRINGIZE(                                                 \
          _elem) " cannot be recognized as a valid specification modifier.");

#define GMOCK_INTERNAL_DETECT_CONST(_i, _, _elem) \
  GMOCK_PP_CAT(GMOCK_INTERNAL_DETECT_CONST_I_, _elem)

#define GMOCK_INTERNAL_DETECT_CONST_I_const ,

#define GMOCK_INTERNAL_DETECT_OVERRIDE(_i, _, _elem) \
  GMOCK_PP_CAT(GMOCK_INTERNAL_DETECT_OVERRIDE_I_, _elem)

#define GMOCK_INTERNAL_DETECT_OVERRIDE_I_override ,

#define GMOCK_INTERNAL_DETECT_FINAL(_i, _, _elem) \
  GMOCK_PP_CAT(GMOCK_INTERNAL_DETECT_FINAL_I_, _elem)

#define GMOCK_INTERNAL_DETECT_FINAL_I_final ,

#define GMOCK_INTERNAL_DETECT_NOEXCEPT(_i, _, _elem) \
  GMOCK_PP_CAT(GMOCK_INTERNAL_DETECT_NOEXCEPT_I_, _elem)

#define GMOCK_INTERNAL_DETECT_NOEXCEPT_I_noexcept ,

#define GMOCK_INTERNAL_DETECT_REF(_i, _, _elem) \
  GMOCK_PP_CAT(GMOCK_INTERNAL_DETECT_REF_I_, _elem)

#define GMOCK_INTERNAL_DETECT_REF_I_ref ,

#define GMOCK_INTERNAL_UNPACK_ref(x) x

#define GMOCK_INTERNAL_GET_CALLTYPE_IMPL(_i, _, _elem)           \
  GMOCK_PP_IF(GMOCK_INTERNAL_IS_CALLTYPE(_elem),                 \
              GMOCK_INTERNAL_GET_VALUE_CALLTYPE, GMOCK_PP_EMPTY) \
  (_elem)

#define GMOCK_INTERNAL_IS_CALLTYPE(_arg) \
  GMOCK_INTERNAL_IS_CALLTYPE_I(          \
      GMOCK_PP_CAT(GMOCK_INTERNAL_IS_CALLTYPE_HELPER_, _arg))
#define GMOCK_INTERNAL_IS_CALLTYPE_I(_arg) GMOCK_PP_IS_ENCLOSED_PARENS(_arg)

#define GMOCK_INTERNAL_GET_VALUE_CALLTYPE(_arg) \
  GMOCK_INTERNAL_GET_VALUE_CALLTYPE_I(          \
      GMOCK_PP_CAT(GMOCK_INTERNAL_IS_CALLTYPE_HELPER_, _arg))
#define GMOCK_INTERNAL_GET_VALUE_CALLTYPE_I(_arg) \
  GMOCK_PP_IDENTITY _arg

#define GMOCK_INTERNAL_IS_CALLTYPE_HELPER_Calltype

#define GMOCK_INTERNAL_SIGNATURE(_Ret, _Args)                                 \
  ::testing::internal::identity_t<GMOCK_PP_IF(GMOCK_PP_IS_BEGIN_PARENS(_Ret), \
                                              GMOCK_PP_REMOVE_PARENS,         \
                                              GMOCK_PP_IDENTITY)(_Ret)>(      \
      GMOCK_PP_FOR_EACH(GMOCK_INTERNAL_GET_TYPE, _, _Args))

#define GMOCK_INTERNAL_GET_TYPE(_i, _, _elem)                          \
  GMOCK_PP_COMMA_IF(_i)                                                \
  GMOCK_PP_IF(GMOCK_PP_IS_BEGIN_PARENS(_elem), GMOCK_PP_REMOVE_PARENS, \
              GMOCK_PP_IDENTITY)                                       \
  (_elem)

#define GMOCK_INTERNAL_PARAMETER(_i, _Signature, _)            \
  GMOCK_PP_COMMA_IF(_i)                                        \
  GMOCK_INTERNAL_ARG_O(_i, GMOCK_PP_REMOVE_PARENS(_Signature)) \
  gmock_a##_i

#define GMOCK_INTERNAL_FORWARD_ARG(_i, _Signature, _) \
  GMOCK_PP_COMMA_IF(_i)                               \
  ::std::forward<GMOCK_INTERNAL_ARG_O(                \
      _i, GMOCK_PP_REMOVE_PARENS(_Signature))>(gmock_a##_i)

#define GMOCK_INTERNAL_MATCHER_PARAMETER(_i, _Signature, _)        \
  GMOCK_PP_COMMA_IF(_i)                                            \
  GMOCK_INTERNAL_MATCHER_O(_i, GMOCK_PP_REMOVE_PARENS(_Signature)) \
  gmock_a##_i

#define GMOCK_INTERNAL_MATCHER_ARGUMENT(_i, _1, _2) \
  GMOCK_PP_COMMA_IF(_i)                             \
  gmock_a##_i

#define GMOCK_INTERNAL_A_MATCHER_ARGUMENT(_i, _Signature, _) \
  GMOCK_PP_COMMA_IF(_i)                                      \
  ::testing::A<GMOCK_INTERNAL_ARG_O(_i, GMOCK_PP_REMOVE_PARENS(_Signature))>()

#define GMOCK_INTERNAL_ARG_O(_i, ...) \
  typename ::testing::internal::Function<__VA_ARGS__>::template Arg<_i>::type

#define GMOCK_INTERNAL_MATCHER_O(_i, ...)                          \
  const ::testing::Matcher<typename ::testing::internal::Function< \
      __VA_ARGS__>::template Arg<_i>::type>&

#define MOCK_METHOD0(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 0, __VA_ARGS__)
#define MOCK_METHOD1(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 1, __VA_ARGS__)
#define MOCK_METHOD2(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 2, __VA_ARGS__)
#define MOCK_METHOD3(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 3, __VA_ARGS__)
#define MOCK_METHOD4(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 4, __VA_ARGS__)
#define MOCK_METHOD5(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 5, __VA_ARGS__)
#define MOCK_METHOD6(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 6, __VA_ARGS__)
#define MOCK_METHOD7(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 7, __VA_ARGS__)
#define MOCK_METHOD8(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 8, __VA_ARGS__)
#define MOCK_METHOD9(m, ...) GMOCK_INTERNAL_MOCK_METHODN(, , m, 9, __VA_ARGS__)
#define MOCK_METHOD10(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, , m, 10, __VA_ARGS__)

#define MOCK_CONST_METHOD0(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 0, __VA_ARGS__)
#define MOCK_CONST_METHOD1(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 1, __VA_ARGS__)
#define MOCK_CONST_METHOD2(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 2, __VA_ARGS__)
#define MOCK_CONST_METHOD3(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 3, __VA_ARGS__)
#define MOCK_CONST_METHOD4(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 4, __VA_ARGS__)
#define MOCK_CONST_METHOD5(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 5, __VA_ARGS__)
#define MOCK_CONST_METHOD6(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 6, __VA_ARGS__)
#define MOCK_CONST_METHOD7(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 7, __VA_ARGS__)
#define MOCK_CONST_METHOD8(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 8, __VA_ARGS__)
#define MOCK_CONST_METHOD9(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 9, __VA_ARGS__)
#define MOCK_CONST_METHOD10(m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, , m, 10, __VA_ARGS__)

#define MOCK_METHOD0_T(m, ...) MOCK_METHOD0(m, __VA_ARGS__)
#define MOCK_METHOD1_T(m, ...) MOCK_METHOD1(m, __VA_ARGS__)
#define MOCK_METHOD2_T(m, ...) MOCK_METHOD2(m, __VA_ARGS__)
#define MOCK_METHOD3_T(m, ...) MOCK_METHOD3(m, __VA_ARGS__)
#define MOCK_METHOD4_T(m, ...) MOCK_METHOD4(m, __VA_ARGS__)
#define MOCK_METHOD5_T(m, ...) MOCK_METHOD5(m, __VA_ARGS__)
#define MOCK_METHOD6_T(m, ...) MOCK_METHOD6(m, __VA_ARGS__)
#define MOCK_METHOD7_T(m, ...) MOCK_METHOD7(m, __VA_ARGS__)
#define MOCK_METHOD8_T(m, ...) MOCK_METHOD8(m, __VA_ARGS__)
#define MOCK_METHOD9_T(m, ...) MOCK_METHOD9(m, __VA_ARGS__)
#define MOCK_METHOD10_T(m, ...) MOCK_METHOD10(m, __VA_ARGS__)

#define MOCK_CONST_METHOD0_T(m, ...) MOCK_CONST_METHOD0(m, __VA_ARGS__)
#define MOCK_CONST_METHOD1_T(m, ...) MOCK_CONST_METHOD1(m, __VA_ARGS__)
#define MOCK_CONST_METHOD2_T(m, ...) MOCK_CONST_METHOD2(m, __VA_ARGS__)
#define MOCK_CONST_METHOD3_T(m, ...) MOCK_CONST_METHOD3(m, __VA_ARGS__)
#define MOCK_CONST_METHOD4_T(m, ...) MOCK_CONST_METHOD4(m, __VA_ARGS__)
#define MOCK_CONST_METHOD5_T(m, ...) MOCK_CONST_METHOD5(m, __VA_ARGS__)
#define MOCK_CONST_METHOD6_T(m, ...) MOCK_CONST_METHOD6(m, __VA_ARGS__)
#define MOCK_CONST_METHOD7_T(m, ...) MOCK_CONST_METHOD7(m, __VA_ARGS__)
#define MOCK_CONST_METHOD8_T(m, ...) MOCK_CONST_METHOD8(m, __VA_ARGS__)
#define MOCK_CONST_METHOD9_T(m, ...) MOCK_CONST_METHOD9(m, __VA_ARGS__)
#define MOCK_CONST_METHOD10_T(m, ...) MOCK_CONST_METHOD10(m, __VA_ARGS__)

#define MOCK_METHOD0_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 0, __VA_ARGS__)
#define MOCK_METHOD1_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 1, __VA_ARGS__)
#define MOCK_METHOD2_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 2, __VA_ARGS__)
#define MOCK_METHOD3_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 3, __VA_ARGS__)
#define MOCK_METHOD4_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 4, __VA_ARGS__)
#define MOCK_METHOD5_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 5, __VA_ARGS__)
#define MOCK_METHOD6_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 6, __VA_ARGS__)
#define MOCK_METHOD7_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 7, __VA_ARGS__)
#define MOCK_METHOD8_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 8, __VA_ARGS__)
#define MOCK_METHOD9_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 9, __VA_ARGS__)
#define MOCK_METHOD10_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(, ct, m, 10, __VA_ARGS__)

#define MOCK_CONST_METHOD0_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 0, __VA_ARGS__)
#define MOCK_CONST_METHOD1_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 1, __VA_ARGS__)
#define MOCK_CONST_METHOD2_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 2, __VA_ARGS__)
#define MOCK_CONST_METHOD3_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 3, __VA_ARGS__)
#define MOCK_CONST_METHOD4_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 4, __VA_ARGS__)
#define MOCK_CONST_METHOD5_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 5, __VA_ARGS__)
#define MOCK_CONST_METHOD6_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 6, __VA_ARGS__)
#define MOCK_CONST_METHOD7_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 7, __VA_ARGS__)
#define MOCK_CONST_METHOD8_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 8, __VA_ARGS__)
#define MOCK_CONST_METHOD9_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 9, __VA_ARGS__)
#define MOCK_CONST_METHOD10_WITH_CALLTYPE(ct, m, ...) \
  GMOCK_INTERNAL_MOCK_METHODN(const, ct, m, 10, __VA_ARGS__)

#define MOCK_METHOD0_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD0_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD1_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD1_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD2_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD2_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD3_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD3_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD4_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD4_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD5_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD5_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD6_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD6_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD7_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD7_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD8_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD8_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD9_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD9_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_METHOD10_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_METHOD10_WITH_CALLTYPE(ct, m, __VA_ARGS__)

#define MOCK_CONST_METHOD0_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD0_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD1_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD1_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD2_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD2_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD3_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD3_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD4_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD4_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD5_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD5_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD6_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD6_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD7_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD7_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD8_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD8_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD9_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD9_WITH_CALLTYPE(ct, m, __VA_ARGS__)
#define MOCK_CONST_METHOD10_T_WITH_CALLTYPE(ct, m, ...) \
  MOCK_CONST_METHOD10_WITH_CALLTYPE(ct, m, __VA_ARGS__)

#define GMOCK_INTERNAL_MOCK_METHODN(constness, ct, Method, args_num, ...) \
  GMOCK_INTERNAL_ASSERT_VALID_SIGNATURE(                                  \
      args_num, ::testing::internal::identity_t<__VA_ARGS__>);            \
  GMOCK_INTERNAL_MOCK_METHOD_IMPL(                                        \
      args_num, Method, GMOCK_PP_NARG0(constness), 0, 0, , ct, ,          \
      (::testing::internal::identity_t<__VA_ARGS__>))

#define GMOCK_MOCKER_(arity, constness, Method) \
  GTEST_CONCAT_TOKEN_(gmock##constness##arity##_##Method##_, __LINE__)

#endif

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_ACTIONS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_ACTIONS_H_

#include <memory>
#include <utility>

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_GENERATED_ACTIONS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_INTERNAL_CUSTOM_GMOCK_GENERATED_ACTIONS_H_

#endif

#define GMOCK_INTERNAL_DECL_HAS_1_TEMPLATE_PARAMS(kind0, name0) kind0 name0
#define GMOCK_INTERNAL_DECL_HAS_2_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1) kind0 name0, kind1 name1
#define GMOCK_INTERNAL_DECL_HAS_3_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2) kind0 name0, kind1 name1, kind2 name2
#define GMOCK_INTERNAL_DECL_HAS_4_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3) kind0 name0, kind1 name1, kind2 name2, \
    kind3 name3
#define GMOCK_INTERNAL_DECL_HAS_5_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4) kind0 name0, kind1 name1, \
    kind2 name2, kind3 name3, kind4 name4
#define GMOCK_INTERNAL_DECL_HAS_6_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5) kind0 name0, \
    kind1 name1, kind2 name2, kind3 name3, kind4 name4, kind5 name5
#define GMOCK_INTERNAL_DECL_HAS_7_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6) kind0 name0, kind1 name1, kind2 name2, kind3 name3, kind4 name4, \
    kind5 name5, kind6 name6
#define GMOCK_INTERNAL_DECL_HAS_8_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7) kind0 name0, kind1 name1, kind2 name2, kind3 name3, \
    kind4 name4, kind5 name5, kind6 name6, kind7 name7
#define GMOCK_INTERNAL_DECL_HAS_9_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7, kind8, name8) kind0 name0, kind1 name1, kind2 name2, \
    kind3 name3, kind4 name4, kind5 name5, kind6 name6, kind7 name7, \
    kind8 name8
#define GMOCK_INTERNAL_DECL_HAS_10_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1, kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6, kind7, name7, kind8, name8, kind9, name9) kind0 name0, \
    kind1 name1, kind2 name2, kind3 name3, kind4 name4, kind5 name5, \
    kind6 name6, kind7 name7, kind8 name8, kind9 name9

#define GMOCK_INTERNAL_LIST_HAS_1_TEMPLATE_PARAMS(kind0, name0) name0
#define GMOCK_INTERNAL_LIST_HAS_2_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1) name0, name1
#define GMOCK_INTERNAL_LIST_HAS_3_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2) name0, name1, name2
#define GMOCK_INTERNAL_LIST_HAS_4_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3) name0, name1, name2, name3
#define GMOCK_INTERNAL_LIST_HAS_5_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4) name0, name1, name2, name3, \
    name4
#define GMOCK_INTERNAL_LIST_HAS_6_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5) name0, name1, \
    name2, name3, name4, name5
#define GMOCK_INTERNAL_LIST_HAS_7_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6) name0, name1, name2, name3, name4, name5, name6
#define GMOCK_INTERNAL_LIST_HAS_8_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7) name0, name1, name2, name3, name4, name5, name6, name7
#define GMOCK_INTERNAL_LIST_HAS_9_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7, kind8, name8) name0, name1, name2, name3, name4, name5, \
    name6, name7, name8
#define GMOCK_INTERNAL_LIST_HAS_10_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1, kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6, kind7, name7, kind8, name8, kind9, name9) name0, name1, name2, \
    name3, name4, name5, name6, name7, name8, name9

#define GMOCK_INTERNAL_DECL_TYPE_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_DECL_TYPE_AND_1_VALUE_PARAMS(p0) , typename p0##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_2_VALUE_PARAMS(p0, p1) , \
    typename p0##_type, typename p1##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_3_VALUE_PARAMS(p0, p1, p2) , \
    typename p0##_type, typename p1##_type, typename p2##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_4_VALUE_PARAMS(p0, p1, p2, p3) , \
    typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) , \
    typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) , \
    typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) , typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type, \
    typename p6##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7) , typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type, \
    typename p6##_type, typename p7##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8) , typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type, \
    typename p6##_type, typename p7##_type, typename p8##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8, p9) , typename p0##_type, typename p1##_type, \
    typename p2##_type, typename p3##_type, typename p4##_type, \
    typename p5##_type, typename p6##_type, typename p7##_type, \
    typename p8##_type, typename p9##_type

#define GMOCK_INTERNAL_INIT_AND_0_VALUE_PARAMS()\
    ()
#define GMOCK_INTERNAL_INIT_AND_1_VALUE_PARAMS(p0)\
    (p0##_type gmock_p0) : p0(::std::move(gmock_p0))
#define GMOCK_INTERNAL_INIT_AND_2_VALUE_PARAMS(p0, p1)\
    (p0##_type gmock_p0, p1##_type gmock_p1) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1))
#define GMOCK_INTERNAL_INIT_AND_3_VALUE_PARAMS(p0, p1, p2)\
    (p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2))
#define GMOCK_INTERNAL_INIT_AND_4_VALUE_PARAMS(p0, p1, p2, p3)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3))
#define GMOCK_INTERNAL_INIT_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3)), p4(::std::move(gmock_p4))
#define GMOCK_INTERNAL_INIT_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3)), p4(::std::move(gmock_p4)), \
        p5(::std::move(gmock_p5))
#define GMOCK_INTERNAL_INIT_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3)), p4(::std::move(gmock_p4)), \
        p5(::std::move(gmock_p5)), p6(::std::move(gmock_p6))
#define GMOCK_INTERNAL_INIT_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6, p7##_type gmock_p7) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3)), p4(::std::move(gmock_p4)), \
        p5(::std::move(gmock_p5)), p6(::std::move(gmock_p6)), \
        p7(::std::move(gmock_p7))
#define GMOCK_INTERNAL_INIT_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6, p7##_type gmock_p7, \
        p8##_type gmock_p8) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3)), p4(::std::move(gmock_p4)), \
        p5(::std::move(gmock_p5)), p6(::std::move(gmock_p6)), \
        p7(::std::move(gmock_p7)), p8(::std::move(gmock_p8))
#define GMOCK_INTERNAL_INIT_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6, p7##_type gmock_p7, p8##_type gmock_p8, \
        p9##_type gmock_p9) : p0(::std::move(gmock_p0)), \
        p1(::std::move(gmock_p1)), p2(::std::move(gmock_p2)), \
        p3(::std::move(gmock_p3)), p4(::std::move(gmock_p4)), \
        p5(::std::move(gmock_p5)), p6(::std::move(gmock_p6)), \
        p7(::std::move(gmock_p7)), p8(::std::move(gmock_p8)), \
        p9(::std::move(gmock_p9))

#define GMOCK_INTERNAL_DEFN_COPY_AND_0_VALUE_PARAMS() \
    {}
#define GMOCK_INTERNAL_DEFN_COPY_AND_1_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_2_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_3_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_4_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_5_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_6_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_7_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_8_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_9_VALUE_PARAMS(...) = default;
#define GMOCK_INTERNAL_DEFN_COPY_AND_10_VALUE_PARAMS(...) = default;

#define GMOCK_INTERNAL_DEFN_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_DEFN_AND_1_VALUE_PARAMS(p0) p0##_type p0;
#define GMOCK_INTERNAL_DEFN_AND_2_VALUE_PARAMS(p0, p1) p0##_type p0; \
    p1##_type p1;
#define GMOCK_INTERNAL_DEFN_AND_3_VALUE_PARAMS(p0, p1, p2) p0##_type p0; \
    p1##_type p1; p2##_type p2;
#define GMOCK_INTERNAL_DEFN_AND_4_VALUE_PARAMS(p0, p1, p2, p3) p0##_type p0; \
    p1##_type p1; p2##_type p2; p3##_type p3;
#define GMOCK_INTERNAL_DEFN_AND_5_VALUE_PARAMS(p0, p1, p2, p3, \
    p4) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4;
#define GMOCK_INTERNAL_DEFN_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, \
    p5) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4; \
    p5##_type p5;
#define GMOCK_INTERNAL_DEFN_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4; \
    p5##_type p5; p6##_type p6;
#define GMOCK_INTERNAL_DEFN_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4; \
    p5##_type p5; p6##_type p6; p7##_type p7;
#define GMOCK_INTERNAL_DEFN_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; \
    p4##_type p4; p5##_type p5; p6##_type p6; p7##_type p7; p8##_type p8;
#define GMOCK_INTERNAL_DEFN_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; \
    p4##_type p4; p5##_type p5; p6##_type p6; p7##_type p7; p8##_type p8; \
    p9##_type p9;

#define GMOCK_INTERNAL_LIST_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_LIST_AND_1_VALUE_PARAMS(p0) p0
#define GMOCK_INTERNAL_LIST_AND_2_VALUE_PARAMS(p0, p1) p0, p1
#define GMOCK_INTERNAL_LIST_AND_3_VALUE_PARAMS(p0, p1, p2) p0, p1, p2
#define GMOCK_INTERNAL_LIST_AND_4_VALUE_PARAMS(p0, p1, p2, p3) p0, p1, p2, p3
#define GMOCK_INTERNAL_LIST_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) p0, p1, \
    p2, p3, p4
#define GMOCK_INTERNAL_LIST_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) p0, \
    p1, p2, p3, p4, p5
#define GMOCK_INTERNAL_LIST_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) p0, p1, p2, p3, p4, p5, p6
#define GMOCK_INTERNAL_LIST_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) p0, p1, p2, p3, p4, p5, p6, p7
#define GMOCK_INTERNAL_LIST_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) p0, p1, p2, p3, p4, p5, p6, p7, p8
#define GMOCK_INTERNAL_LIST_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) p0, p1, p2, p3, p4, p5, p6, p7, p8, p9

#define GMOCK_INTERNAL_LIST_TYPE_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_LIST_TYPE_AND_1_VALUE_PARAMS(p0) , p0##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_2_VALUE_PARAMS(p0, p1) , p0##_type, \
    p1##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_3_VALUE_PARAMS(p0, p1, p2) , p0##_type, \
    p1##_type, p2##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_4_VALUE_PARAMS(p0, p1, p2, p3) , \
    p0##_type, p1##_type, p2##_type, p3##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) , \
    p0##_type, p1##_type, p2##_type, p3##_type, p4##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) , \
    p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, p5##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, p5##_type, \
    p6##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
    p5##_type, p6##_type, p7##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
    p5##_type, p6##_type, p7##_type, p8##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8, p9) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
    p5##_type, p6##_type, p7##_type, p8##_type, p9##_type

#define GMOCK_INTERNAL_DECL_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_DECL_AND_1_VALUE_PARAMS(p0) p0##_type p0
#define GMOCK_INTERNAL_DECL_AND_2_VALUE_PARAMS(p0, p1) p0##_type p0, \
    p1##_type p1
#define GMOCK_INTERNAL_DECL_AND_3_VALUE_PARAMS(p0, p1, p2) p0##_type p0, \
    p1##_type p1, p2##_type p2
#define GMOCK_INTERNAL_DECL_AND_4_VALUE_PARAMS(p0, p1, p2, p3) p0##_type p0, \
    p1##_type p1, p2##_type p2, p3##_type p3
#define GMOCK_INTERNAL_DECL_AND_5_VALUE_PARAMS(p0, p1, p2, p3, \
    p4) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4
#define GMOCK_INTERNAL_DECL_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, \
    p5) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, \
    p5##_type p5
#define GMOCK_INTERNAL_DECL_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, \
    p5##_type p5, p6##_type p6
#define GMOCK_INTERNAL_DECL_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, \
    p5##_type p5, p6##_type p6, p7##_type p7
#define GMOCK_INTERNAL_DECL_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
    p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, p8##_type p8
#define GMOCK_INTERNAL_DECL_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
    p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, p8##_type p8, \
    p9##_type p9

#define GMOCK_INTERNAL_COUNT_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_COUNT_AND_1_VALUE_PARAMS(p0) P
#define GMOCK_INTERNAL_COUNT_AND_2_VALUE_PARAMS(p0, p1) P2
#define GMOCK_INTERNAL_COUNT_AND_3_VALUE_PARAMS(p0, p1, p2) P3
#define GMOCK_INTERNAL_COUNT_AND_4_VALUE_PARAMS(p0, p1, p2, p3) P4
#define GMOCK_INTERNAL_COUNT_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) P5
#define GMOCK_INTERNAL_COUNT_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) P6
#define GMOCK_INTERNAL_COUNT_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6) P7
#define GMOCK_INTERNAL_COUNT_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) P8
#define GMOCK_INTERNAL_COUNT_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) P9
#define GMOCK_INTERNAL_COUNT_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) P10

#define GMOCK_ACTION_CLASS_(name, value_params)\
    GTEST_CONCAT_TOKEN_(name##Action, GMOCK_INTERNAL_COUNT_##value_params)

#define ACTION_TEMPLATE(name, template_params, value_params)                   \
  template <GMOCK_INTERNAL_DECL_##template_params                              \
            GMOCK_INTERNAL_DECL_TYPE_##value_params>                           \
  class GMOCK_ACTION_CLASS_(name, value_params) {                              \
   public:                                                                     \
    explicit GMOCK_ACTION_CLASS_(name, value_params)(                          \
        GMOCK_INTERNAL_DECL_##value_params)                                    \
        GMOCK_PP_IF(GMOCK_PP_IS_EMPTY(GMOCK_INTERNAL_COUNT_##value_params),    \
                    = default; ,                                               \
                    : impl_(std::make_shared<gmock_Impl>(                      \
                                GMOCK_INTERNAL_LIST_##value_params)) { })      \
    GMOCK_ACTION_CLASS_(name, value_params)(                                   \
        const GMOCK_ACTION_CLASS_(name, value_params)&) noexcept               \
        GMOCK_INTERNAL_DEFN_COPY_##value_params                                \
    GMOCK_ACTION_CLASS_(name, value_params)(                                   \
        GMOCK_ACTION_CLASS_(name, value_params)&&) noexcept                    \
        GMOCK_INTERNAL_DEFN_COPY_##value_params                                \
    template <typename F>                                                      \
    operator ::testing::Action<F>() const {                                    \
      return GMOCK_PP_IF(                                                      \
          GMOCK_PP_IS_EMPTY(GMOCK_INTERNAL_COUNT_##value_params),              \
                      (::testing::internal::MakeAction<F, gmock_Impl>()),      \
                      (::testing::internal::MakeAction<F>(impl_)));            \
    }                                                                          \
   private:                                                                    \
    class gmock_Impl {                                                         \
     public:                                                                   \
      explicit gmock_Impl GMOCK_INTERNAL_INIT_##value_params {}                \
      template <typename function_type, typename return_type,                  \
                typename args_type, GMOCK_ACTION_TEMPLATE_ARGS_NAMES_>         \
      return_type gmock_PerformImpl(GMOCK_ACTION_ARG_TYPES_AND_NAMES_) const;  \
      GMOCK_INTERNAL_DEFN_##value_params                                       \
    };                                                                         \
    GMOCK_PP_IF(GMOCK_PP_IS_EMPTY(GMOCK_INTERNAL_COUNT_##value_params),        \
                , std::shared_ptr<const gmock_Impl> impl_;)                    \
  };                                                                           \
  template <GMOCK_INTERNAL_DECL_##template_params                              \
            GMOCK_INTERNAL_DECL_TYPE_##value_params>                           \
  GMOCK_ACTION_CLASS_(name, value_params)<                                     \
      GMOCK_INTERNAL_LIST_##template_params                                    \
      GMOCK_INTERNAL_LIST_TYPE_##value_params> name(                           \
          GMOCK_INTERNAL_DECL_##value_params) GTEST_MUST_USE_RESULT_;          \
  template <GMOCK_INTERNAL_DECL_##template_params                              \
            GMOCK_INTERNAL_DECL_TYPE_##value_params>                           \
  inline GMOCK_ACTION_CLASS_(name, value_params)<                              \
      GMOCK_INTERNAL_LIST_##template_params                                    \
      GMOCK_INTERNAL_LIST_TYPE_##value_params> name(                           \
          GMOCK_INTERNAL_DECL_##value_params) {                                \
    return GMOCK_ACTION_CLASS_(name, value_params)<                            \
        GMOCK_INTERNAL_LIST_##template_params                                  \
        GMOCK_INTERNAL_LIST_TYPE_##value_params>(                              \
            GMOCK_INTERNAL_LIST_##value_params);                               \
  }                                                                            \
  template <GMOCK_INTERNAL_DECL_##template_params                              \
            GMOCK_INTERNAL_DECL_TYPE_##value_params>                           \
  template <typename function_type, typename return_type, typename args_type,  \
            GMOCK_ACTION_TEMPLATE_ARGS_NAMES_>                                 \
  return_type GMOCK_ACTION_CLASS_(name, value_params)<                         \
      GMOCK_INTERNAL_LIST_##template_params                                    \
      GMOCK_INTERNAL_LIST_TYPE_##value_params>::gmock_Impl::gmock_PerformImpl( \
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

namespace testing {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif

namespace internal {

template <typename F, typename... Args>
auto InvokeArgument(F f, Args... args) -> decltype(f(args...)) {
  return f(args...);
}

template <std::size_t index, typename... Params>
struct InvokeArgumentAction {
  template <typename... Args>
  auto operator()(Args&&... args) const -> decltype(internal::InvokeArgument(
      std::get<index>(std::forward_as_tuple(std::forward<Args>(args)...)),
      std::declval<const Params&>()...)) {
    internal::FlatTuple<Args&&...> args_tuple(FlatTupleConstructTag{},
                                              std::forward<Args>(args)...);
    return params.Apply([&](const Params&... unpacked_params) {
      auto&& callable = args_tuple.template Get<index>();
      return internal::InvokeArgument(
          std::forward<decltype(callable)>(callable), unpacked_params...);
    });
  }

  internal::FlatTuple<Params...> params;
};

}

template <std::size_t index, typename... Params>
internal::InvokeArgumentAction<index, typename std::decay<Params>::type...>
InvokeArgument(Params&&... params) {
  return {internal::FlatTuple<typename std::decay<Params>::type...>(
      internal::FlatTupleConstructTag{}, std::forward<Params>(params)...)};
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

}

#endif

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_MATCHERS_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_MORE_MATCHERS_H_

namespace testing {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#if (_MSC_VER == 1900)

# pragma warning(disable:4800)
  #endif
#endif

MATCHER(IsEmpty, negation ? "isn't empty" : "is empty") {
  if (arg.empty()) {
    return true;
  }
  *result_listener << "whose size is " << arg.size();
  return false;
}

MATCHER(IsTrue, negation ? "is false" : "is true") {
  return static_cast<bool>(arg);
}

MATCHER(IsFalse, negation ? "is true" : "is false") {
  return !static_cast<bool>(arg);
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

}

#endif

#ifndef GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_NICE_STRICT_H_
#define GOOGLEMOCK_INCLUDE_GMOCK_GMOCK_NICE_STRICT_H_

#include <type_traits>

namespace testing {
template <class MockClass>
class NiceMock;
template <class MockClass>
class NaggyMock;
template <class MockClass>
class StrictMock;

namespace internal {
template <typename T>
std::true_type StrictnessModifierProbe(const NiceMock<T>&);
template <typename T>
std::true_type StrictnessModifierProbe(const NaggyMock<T>&);
template <typename T>
std::true_type StrictnessModifierProbe(const StrictMock<T>&);
std::false_type StrictnessModifierProbe(...);

template <typename T>
constexpr bool HasStrictnessModifier() {
  return decltype(StrictnessModifierProbe(std::declval<const T&>()))::value;
}

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MINGW && \
    (defined(_MSC_VER) || defined(__clang__))

#define GTEST_INTERNAL_EMPTY_BASE_CLASS __declspec(empty_bases)
#else
#define GTEST_INTERNAL_EMPTY_BASE_CLASS
#endif

template <typename Base>
class NiceMockImpl {
 public:
  NiceMockImpl() { ::testing::Mock::AllowUninterestingCalls(this); }

  ~NiceMockImpl() { ::testing::Mock::UnregisterCallReaction(this); }
};

template <typename Base>
class NaggyMockImpl {
 public:
  NaggyMockImpl() { ::testing::Mock::WarnUninterestingCalls(this); }

  ~NaggyMockImpl() { ::testing::Mock::UnregisterCallReaction(this); }
};

template <typename Base>
class StrictMockImpl {
 public:
  StrictMockImpl() { ::testing::Mock::FailUninterestingCalls(this); }

  ~StrictMockImpl() { ::testing::Mock::UnregisterCallReaction(this); }
};

}

template <class MockClass>
class GTEST_INTERNAL_EMPTY_BASE_CLASS NiceMock
    : private internal::NiceMockImpl<MockClass>,
      public MockClass {
 public:
  static_assert(!internal::HasStrictnessModifier<MockClass>(),
                "Can't apply NiceMock to a class hierarchy that already has a "
                "strictness modifier. See "
                "https://google.github.io/googletest/"
                "gmock_cook_book.html#NiceStrictNaggy");
  NiceMock() : MockClass() {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename A>
  explicit NiceMock(A&& arg) : MockClass(std::forward<A>(arg)) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename TArg1, typename TArg2, typename... An>
  NiceMock(TArg1&& arg1, TArg2&& arg2, An&&... args)
      : MockClass(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2),
                  std::forward<An>(args)...) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(NiceMock);
};

template <class MockClass>
class GTEST_INTERNAL_EMPTY_BASE_CLASS NaggyMock
    : private internal::NaggyMockImpl<MockClass>,
      public MockClass {
  static_assert(!internal::HasStrictnessModifier<MockClass>(),
                "Can't apply NaggyMock to a class hierarchy that already has a "
                "strictness modifier. See "
                "https://google.github.io/googletest/"
                "gmock_cook_book.html#NiceStrictNaggy");

 public:
  NaggyMock() : MockClass() {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename A>
  explicit NaggyMock(A&& arg) : MockClass(std::forward<A>(arg)) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename TArg1, typename TArg2, typename... An>
  NaggyMock(TArg1&& arg1, TArg2&& arg2, An&&... args)
      : MockClass(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2),
                  std::forward<An>(args)...) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(NaggyMock);
};

template <class MockClass>
class GTEST_INTERNAL_EMPTY_BASE_CLASS StrictMock
    : private internal::StrictMockImpl<MockClass>,
      public MockClass {
 public:
  static_assert(
      !internal::HasStrictnessModifier<MockClass>(),
      "Can't apply StrictMock to a class hierarchy that already has a "
      "strictness modifier. See "
      "https://google.github.io/googletest/"
      "gmock_cook_book.html#NiceStrictNaggy");
  StrictMock() : MockClass() {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename A>
  explicit StrictMock(A&& arg) : MockClass(std::forward<A>(arg)) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

  template <typename TArg1, typename TArg2, typename... An>
  StrictMock(TArg1&& arg1, TArg2&& arg2, An&&... args)
      : MockClass(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2),
                  std::forward<An>(args)...) {
    static_assert(sizeof(*this) == sizeof(MockClass),
                  "The impl subclass shouldn't introduce any padding");
  }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(StrictMock);
};

#undef GTEST_INTERNAL_EMPTY_BASE_CLASS

}

#endif

namespace testing {

GMOCK_DECLARE_bool_(catch_leaked_mocks);
GMOCK_DECLARE_string_(verbose);
GMOCK_DECLARE_int32_(default_mock_behavior);

GTEST_API_ void InitGoogleMock(int* argc, char** argv);

GTEST_API_ void InitGoogleMock(int* argc, wchar_t** argv);

GTEST_API_ void InitGoogleMock();

}

#endif
