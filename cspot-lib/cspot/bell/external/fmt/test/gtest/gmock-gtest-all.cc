

#include "gtest/gtest.h"

#ifndef GOOGLETEST_INCLUDE_GTEST_GTEST_SPI_H_
#define GOOGLETEST_INCLUDE_GTEST_GTEST_SPI_H_

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
 )

namespace testing {

class GTEST_API_ ScopedFakeTestPartResultReporter
    : public TestPartResultReporterInterface {
 public:

  enum InterceptMode {
    INTERCEPT_ONLY_CURRENT_THREAD,
    INTERCEPT_ALL_THREADS
  };

  explicit ScopedFakeTestPartResultReporter(TestPartResultArray* result);

  ScopedFakeTestPartResultReporter(InterceptMode intercept_mode,
                                   TestPartResultArray* result);

  ~ScopedFakeTestPartResultReporter() override;

  void ReportTestPartResult(const TestPartResult& result) override;

 private:
  void Init();

  const InterceptMode intercept_mode_;
  TestPartResultReporterInterface* old_reporter_;
  TestPartResultArray* const result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ScopedFakeTestPartResultReporter);
};

namespace internal {

class GTEST_API_ SingleFailureChecker {
 public:

  SingleFailureChecker(const TestPartResultArray* results,
                       TestPartResult::Type type, const std::string& substr);
  ~SingleFailureChecker();
 private:
  const TestPartResultArray* const results_;
  const TestPartResult::Type type_;
  const std::string substr_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(SingleFailureChecker);
};

}

}

GTEST_DISABLE_MSC_WARNINGS_POP_()

#define EXPECT_FATAL_FAILURE(statement, substr) \
  do { \
    class GTestExpectFatalFailureHelper {\
     public:\
      static void Execute() { statement; }\
    };\
    ::testing::TestPartResultArray gtest_failures;\
    ::testing::internal::SingleFailureChecker gtest_checker(\
        &gtest_failures, ::testing::TestPartResult::kFatalFailure, (substr));\
    {\
      ::testing::ScopedFakeTestPartResultReporter gtest_reporter(\
          ::testing::ScopedFakeTestPartResultReporter:: \
          INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);\
      GTestExpectFatalFailureHelper::Execute();\
    }\
  } while (::testing::internal::AlwaysFalse())

#define EXPECT_FATAL_FAILURE_ON_ALL_THREADS(statement, substr) \
  do { \
    class GTestExpectFatalFailureHelper {\
     public:\
      static void Execute() { statement; }\
    };\
    ::testing::TestPartResultArray gtest_failures;\
    ::testing::internal::SingleFailureChecker gtest_checker(\
        &gtest_failures, ::testing::TestPartResult::kFatalFailure, (substr));\
    {\
      ::testing::ScopedFakeTestPartResultReporter gtest_reporter(\
          ::testing::ScopedFakeTestPartResultReporter:: \
          INTERCEPT_ALL_THREADS, &gtest_failures);\
      GTestExpectFatalFailureHelper::Execute();\
    }\
  } while (::testing::internal::AlwaysFalse())

#define EXPECT_NONFATAL_FAILURE(statement, substr) \
  do {\
    ::testing::TestPartResultArray gtest_failures;\
    ::testing::internal::SingleFailureChecker gtest_checker(\
        &gtest_failures, ::testing::TestPartResult::kNonFatalFailure, \
        (substr));\
    {\
      ::testing::ScopedFakeTestPartResultReporter gtest_reporter(\
          ::testing::ScopedFakeTestPartResultReporter:: \
          INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);\
      if (::testing::internal::AlwaysTrue()) { statement; }\
    }\
  } while (::testing::internal::AlwaysFalse())

#define EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS(statement, substr) \
  do {\
    ::testing::TestPartResultArray gtest_failures;\
    ::testing::internal::SingleFailureChecker gtest_checker(\
        &gtest_failures, ::testing::TestPartResult::kNonFatalFailure, \
        (substr));\
    {\
      ::testing::ScopedFakeTestPartResultReporter gtest_reporter(\
          ::testing::ScopedFakeTestPartResultReporter::INTERCEPT_ALL_THREADS, \
          &gtest_failures);\
      if (::testing::internal::AlwaysTrue()) { statement; }\
    }\
  } while (::testing::internal::AlwaysFalse())

#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
#include <vector>

#if GTEST_OS_LINUX

# include <fcntl.h>
# include <limits.h>
# include <sched.h>

# include <strings.h>
# include <sys/mman.h>
# include <sys/time.h>
# include <unistd.h>
# include <string>

#elif GTEST_OS_ZOS
# include <sys/time.h>

# include <strings.h>

#elif GTEST_OS_WINDOWS_MOBILE

# include <windows.h>
# undef min

#elif GTEST_OS_WINDOWS

# include <windows.h>
# undef min

#ifdef _MSC_VER
# include <crtdbg.h>
#endif

# include <io.h>
# include <sys/timeb.h>
# include <sys/types.h>
# include <sys/stat.h>

# if GTEST_OS_WINDOWS_MINGW
#  include <sys/time.h>
# endif

#else

# include <sys/time.h>
# include <unistd.h>

#endif

#if GTEST_HAS_EXCEPTIONS
# include <stdexcept>
#endif

#if GTEST_CAN_STREAM_RESULTS_
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/socket.h>
# include <sys/types.h>
#endif

#ifndef GOOGLETEST_SRC_GTEST_INTERNAL_INL_H_
#define GOOGLETEST_SRC_GTEST_INTERNAL_INL_H_

#ifndef _WIN32_WCE
# include <errno.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#if GTEST_CAN_STREAM_RESULTS_
# include <arpa/inet.h>
# include <netdb.h>
#endif

#if GTEST_OS_WINDOWS
# include <windows.h>
#endif

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
 )

namespace testing {

GTEST_DECLARE_bool_(death_test_use_fork);

namespace internal {

GTEST_API_ extern const TypeId kTestTypeIdInGoogleTest;

const char kAlsoRunDisabledTestsFlag[] = "also_run_disabled_tests";
const char kBreakOnFailureFlag[] = "break_on_failure";
const char kCatchExceptionsFlag[] = "catch_exceptions";
const char kColorFlag[] = "color";
const char kFailFast[] = "fail_fast";
const char kFilterFlag[] = "filter";
const char kListTestsFlag[] = "list_tests";
const char kOutputFlag[] = "output";
const char kBriefFlag[] = "brief";
const char kPrintTimeFlag[] = "print_time";
const char kPrintUTF8Flag[] = "print_utf8";
const char kRandomSeedFlag[] = "random_seed";
const char kRepeatFlag[] = "repeat";
const char kShuffleFlag[] = "shuffle";
const char kStackTraceDepthFlag[] = "stack_trace_depth";
const char kStreamResultToFlag[] = "stream_result_to";
const char kThrowOnFailureFlag[] = "throw_on_failure";
const char kFlagfileFlag[] = "flagfile";

const int kMaxRandomSeed = 99999;

GTEST_API_ extern bool g_help_flag;

GTEST_API_ TimeInMillis GetTimeInMillis();

GTEST_API_ bool ShouldUseColor(bool stdout_is_tty);

GTEST_API_ std::string FormatTimeInMillisAsSeconds(TimeInMillis ms);

GTEST_API_ std::string FormatEpochTimeInMillisAsIso8601(TimeInMillis ms);

GTEST_API_ bool ParseInt32Flag(
    const char* str, const char* flag, int32_t* value);

inline int GetRandomSeedFromFlag(int32_t random_seed_flag) {
  const unsigned int raw_seed = (random_seed_flag == 0) ?
      static_cast<unsigned int>(GetTimeInMillis()) :
      static_cast<unsigned int>(random_seed_flag);

  const int normalized_seed =
      static_cast<int>((raw_seed - 1U) %
                       static_cast<unsigned int>(kMaxRandomSeed)) + 1;
  return normalized_seed;
}

inline int GetNextRandomSeed(int seed) {
  GTEST_CHECK_(1 <= seed && seed <= kMaxRandomSeed)
      << "Invalid random seed " << seed << " - must be in [1, "
      << kMaxRandomSeed << "].";
  const int next_seed = seed + 1;
  return (next_seed > kMaxRandomSeed) ? 1 : next_seed;
}

class GTestFlagSaver {
 public:

  GTestFlagSaver() {
    also_run_disabled_tests_ = GTEST_FLAG(also_run_disabled_tests);
    break_on_failure_ = GTEST_FLAG(break_on_failure);
    catch_exceptions_ = GTEST_FLAG(catch_exceptions);
    color_ = GTEST_FLAG(color);
    death_test_style_ = GTEST_FLAG(death_test_style);
    death_test_use_fork_ = GTEST_FLAG(death_test_use_fork);
    fail_fast_ = GTEST_FLAG(fail_fast);
    filter_ = GTEST_FLAG(filter);
    internal_run_death_test_ = GTEST_FLAG(internal_run_death_test);
    list_tests_ = GTEST_FLAG(list_tests);
    output_ = GTEST_FLAG(output);
    brief_ = GTEST_FLAG(brief);
    print_time_ = GTEST_FLAG(print_time);
    print_utf8_ = GTEST_FLAG(print_utf8);
    random_seed_ = GTEST_FLAG(random_seed);
    repeat_ = GTEST_FLAG(repeat);
    shuffle_ = GTEST_FLAG(shuffle);
    stack_trace_depth_ = GTEST_FLAG(stack_trace_depth);
    stream_result_to_ = GTEST_FLAG(stream_result_to);
    throw_on_failure_ = GTEST_FLAG(throw_on_failure);
  }

  ~GTestFlagSaver() {
    GTEST_FLAG(also_run_disabled_tests) = also_run_disabled_tests_;
    GTEST_FLAG(break_on_failure) = break_on_failure_;
    GTEST_FLAG(catch_exceptions) = catch_exceptions_;
    GTEST_FLAG(color) = color_;
    GTEST_FLAG(death_test_style) = death_test_style_;
    GTEST_FLAG(death_test_use_fork) = death_test_use_fork_;
    GTEST_FLAG(filter) = filter_;
    GTEST_FLAG(fail_fast) = fail_fast_;
    GTEST_FLAG(internal_run_death_test) = internal_run_death_test_;
    GTEST_FLAG(list_tests) = list_tests_;
    GTEST_FLAG(output) = output_;
    GTEST_FLAG(brief) = brief_;
    GTEST_FLAG(print_time) = print_time_;
    GTEST_FLAG(print_utf8) = print_utf8_;
    GTEST_FLAG(random_seed) = random_seed_;
    GTEST_FLAG(repeat) = repeat_;
    GTEST_FLAG(shuffle) = shuffle_;
    GTEST_FLAG(stack_trace_depth) = stack_trace_depth_;
    GTEST_FLAG(stream_result_to) = stream_result_to_;
    GTEST_FLAG(throw_on_failure) = throw_on_failure_;
  }

 private:

  bool also_run_disabled_tests_;
  bool break_on_failure_;
  bool catch_exceptions_;
  std::string color_;
  std::string death_test_style_;
  bool death_test_use_fork_;
  bool fail_fast_;
  std::string filter_;
  std::string internal_run_death_test_;
  bool list_tests_;
  std::string output_;
  bool brief_;
  bool print_time_;
  bool print_utf8_;
  int32_t random_seed_;
  int32_t repeat_;
  bool shuffle_;
  int32_t stack_trace_depth_;
  std::string stream_result_to_;
  bool throw_on_failure_;
} GTEST_ATTRIBUTE_UNUSED_;

GTEST_API_ std::string CodePointToUtf8(uint32_t code_point);

GTEST_API_ std::string WideStringToUtf8(const wchar_t* str, int num_chars);

void WriteToShardStatusFileIfNeeded();

GTEST_API_ bool ShouldShard(const char* total_shards_str,
                            const char* shard_index_str,
                            bool in_subprocess_for_death_test);

GTEST_API_ int32_t Int32FromEnvOrDie(const char* env_var, int32_t default_val);

GTEST_API_ bool ShouldRunTestOnShard(
    int total_shards, int shard_index, int test_id);

template <class Container, typename Predicate>
inline int CountIf(const Container& c, Predicate predicate) {

  int count = 0;
  for (typename Container::const_iterator it = c.begin(); it != c.end(); ++it) {
    if (predicate(*it))
      ++count;
  }
  return count;
}

template <class Container, typename Functor>
void ForEach(const Container& c, Functor functor) {
  std::for_each(c.begin(), c.end(), functor);
}

template <typename E>
inline E GetElementOr(const std::vector<E>& v, int i, E default_value) {
  return (i < 0 || i >= static_cast<int>(v.size())) ? default_value
                                                    : v[static_cast<size_t>(i)];
}

template <typename E>
void ShuffleRange(internal::Random* random, int begin, int end,
                  std::vector<E>* v) {
  const int size = static_cast<int>(v->size());
  GTEST_CHECK_(0 <= begin && begin <= size)
      << "Invalid shuffle range start " << begin << ": must be in range [0, "
      << size << "].";
  GTEST_CHECK_(begin <= end && end <= size)
      << "Invalid shuffle range finish " << end << ": must be in range ["
      << begin << ", " << size << "].";

  for (int range_width = end - begin; range_width >= 2; range_width--) {
    const int last_in_range = begin + range_width - 1;
    const int selected =
        begin +
        static_cast<int>(random->Generate(static_cast<uint32_t>(range_width)));
    std::swap((*v)[static_cast<size_t>(selected)],
              (*v)[static_cast<size_t>(last_in_range)]);
  }
}

template <typename E>
inline void Shuffle(internal::Random* random, std::vector<E>* v) {
  ShuffleRange(random, 0, static_cast<int>(v->size()), v);
}

template <typename T>
static void Delete(T* x) {
  delete x;
}

class TestPropertyKeyIs {
 public:

  explicit TestPropertyKeyIs(const std::string& key) : key_(key) {}

  bool operator()(const TestProperty& test_property) const {
    return test_property.key() == key_;
  }

 private:
  std::string key_;
};

class GTEST_API_ UnitTestOptions {
 public:

  static std::string GetOutputFormat();

  static std::string GetAbsolutePathToOutputFile();

  static bool FilterMatchesTest(const std::string& test_suite_name,
                                const std::string& test_name);

#if GTEST_OS_WINDOWS

  static int GTestShouldProcessSEH(DWORD exception_code);
#endif

  static bool MatchesFilter(const std::string& name, const char* filter);
};

GTEST_API_ FilePath GetCurrentExecutableName();

class OsStackTraceGetterInterface {
 public:
  OsStackTraceGetterInterface() {}
  virtual ~OsStackTraceGetterInterface() {}

  virtual std::string CurrentStackTrace(int max_depth, int skip_count) = 0;

  virtual void UponLeavingGTest() = 0;

  static const char* const kElidedFramesMarker;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(OsStackTraceGetterInterface);
};

class OsStackTraceGetter : public OsStackTraceGetterInterface {
 public:
  OsStackTraceGetter() {}

  std::string CurrentStackTrace(int max_depth, int skip_count) override;
  void UponLeavingGTest() override;

 private:
#if GTEST_HAS_ABSL
  Mutex mutex_;

  void* caller_frame_ = nullptr;
#endif

  GTEST_DISALLOW_COPY_AND_ASSIGN_(OsStackTraceGetter);
};

struct TraceInfo {
  const char* file;
  int line;
  std::string message;
};

class DefaultGlobalTestPartResultReporter
  : public TestPartResultReporterInterface {
 public:
  explicit DefaultGlobalTestPartResultReporter(UnitTestImpl* unit_test);

  void ReportTestPartResult(const TestPartResult& result) override;

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultGlobalTestPartResultReporter);
};

class DefaultPerThreadTestPartResultReporter
    : public TestPartResultReporterInterface {
 public:
  explicit DefaultPerThreadTestPartResultReporter(UnitTestImpl* unit_test);

  void ReportTestPartResult(const TestPartResult& result) override;

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultPerThreadTestPartResultReporter);
};

class GTEST_API_ UnitTestImpl {
 public:
  explicit UnitTestImpl(UnitTest* parent);
  virtual ~UnitTestImpl();

  TestPartResultReporterInterface* GetGlobalTestPartResultReporter();

  void SetGlobalTestPartResultReporter(
      TestPartResultReporterInterface* reporter);

  TestPartResultReporterInterface* GetTestPartResultReporterForCurrentThread();

  void SetTestPartResultReporterForCurrentThread(
      TestPartResultReporterInterface* reporter);

  int successful_test_suite_count() const;

  int failed_test_suite_count() const;

  int total_test_suite_count() const;

  int test_suite_to_run_count() const;

  int successful_test_count() const;

  int skipped_test_count() const;

  int failed_test_count() const;

  int reportable_disabled_test_count() const;

  int disabled_test_count() const;

  int reportable_test_count() const;

  int total_test_count() const;

  int test_to_run_count() const;

  TimeInMillis start_timestamp() const { return start_timestamp_; }

  TimeInMillis elapsed_time() const { return elapsed_time_; }

  bool Passed() const { return !Failed(); }

  bool Failed() const {
    return failed_test_suite_count() > 0 || ad_hoc_test_result()->Failed();
  }

  const TestSuite* GetTestSuite(int i) const {
    const int index = GetElementOr(test_suite_indices_, i, -1);
    return index < 0 ? nullptr : test_suites_[static_cast<size_t>(i)];
  }

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  const TestCase* GetTestCase(int i) const { return GetTestSuite(i); }
#endif

  TestSuite* GetMutableSuiteCase(int i) {
    const int index = GetElementOr(test_suite_indices_, i, -1);
    return index < 0 ? nullptr : test_suites_[static_cast<size_t>(index)];
  }

  TestEventListeners* listeners() { return &listeners_; }

  TestResult* current_test_result();

  const TestResult* ad_hoc_test_result() const { return &ad_hoc_test_result_; }

  void set_os_stack_trace_getter(OsStackTraceGetterInterface* getter);

  OsStackTraceGetterInterface* os_stack_trace_getter();

  std::string CurrentOsStackTraceExceptTop(int skip_count) GTEST_NO_INLINE_;

  TestSuite* GetTestSuite(const char* test_suite_name, const char* type_param,
                          internal::SetUpTestSuiteFunc set_up_tc,
                          internal::TearDownTestSuiteFunc tear_down_tc);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  TestCase* GetTestCase(const char* test_case_name, const char* type_param,
                        internal::SetUpTestSuiteFunc set_up_tc,
                        internal::TearDownTestSuiteFunc tear_down_tc) {
    return GetTestSuite(test_case_name, type_param, set_up_tc, tear_down_tc);
  }
#endif

  void AddTestInfo(internal::SetUpTestSuiteFunc set_up_tc,
                   internal::TearDownTestSuiteFunc tear_down_tc,
                   TestInfo* test_info) {
#if GTEST_HAS_DEATH_TEST

    if (original_working_dir_.IsEmpty()) {
      original_working_dir_.Set(FilePath::GetCurrentDir());
      GTEST_CHECK_(!original_working_dir_.IsEmpty())
          << "Failed to get the current working directory.";
    }
#endif

    GetTestSuite(test_info->test_suite_name(), test_info->type_param(),
                 set_up_tc, tear_down_tc)
        ->AddTestInfo(test_info);
  }

  internal::ParameterizedTestSuiteRegistry& parameterized_test_registry() {
    return parameterized_test_registry_;
  }

  std::set<std::string>* ignored_parameterized_test_suites() {
    return &ignored_parameterized_test_suites_;
  }

  internal::TypeParameterizedTestSuiteRegistry&
  type_parameterized_test_registry() {
    return type_parameterized_test_registry_;
  }

  void set_current_test_suite(TestSuite* a_current_test_suite) {
    current_test_suite_ = a_current_test_suite;
  }

  void set_current_test_info(TestInfo* a_current_test_info) {
    current_test_info_ = a_current_test_info;
  }

  void RegisterParameterizedTests();

  bool RunAllTests();

  void ClearNonAdHocTestResult() {
    ForEach(test_suites_, TestSuite::ClearTestSuiteResult);
  }

  void ClearAdHocTestResult() {
    ad_hoc_test_result_.Clear();
  }

  void RecordProperty(const TestProperty& test_property);

  enum ReactionToSharding {
    HONOR_SHARDING_PROTOCOL,
    IGNORE_SHARDING_PROTOCOL
  };

  int FilterTests(ReactionToSharding shard_tests);

  void ListTestsMatchingFilter();

  const TestSuite* current_test_suite() const { return current_test_suite_; }
  TestInfo* current_test_info() { return current_test_info_; }
  const TestInfo* current_test_info() const { return current_test_info_; }

  std::vector<Environment*>& environments() { return environments_; }

  std::vector<TraceInfo>& gtest_trace_stack() {
    return *(gtest_trace_stack_.pointer());
  }
  const std::vector<TraceInfo>& gtest_trace_stack() const {
    return gtest_trace_stack_.get();
  }

#if GTEST_HAS_DEATH_TEST
  void InitDeathTestSubprocessControlInfo() {
    internal_run_death_test_flag_.reset(ParseInternalRunDeathTestFlag());
  }

  const InternalRunDeathTestFlag* internal_run_death_test_flag() const {
    return internal_run_death_test_flag_.get();
  }

  internal::DeathTestFactory* death_test_factory() {
    return death_test_factory_.get();
  }

  void SuppressTestEventsIfInSubprocess();

  friend class ReplaceDeathTestFactory;
#endif

  void ConfigureXmlOutput();

#if GTEST_CAN_STREAM_RESULTS_

  void ConfigureStreamingOutput();
#endif

  void PostFlagParsingInit();

  int random_seed() const { return random_seed_; }

  internal::Random* random() { return &random_; }

  void ShuffleTests();

  void UnshuffleTests();

  bool catch_exceptions() const { return catch_exceptions_; }

 private:
  friend class ::testing::UnitTest;

  void set_catch_exceptions(bool value) { catch_exceptions_ = value; }

  UnitTest* const parent_;

  internal::FilePath original_working_dir_;

  DefaultGlobalTestPartResultReporter default_global_test_part_result_reporter_;
  DefaultPerThreadTestPartResultReporter
      default_per_thread_test_part_result_reporter_;

  TestPartResultReporterInterface* global_test_part_result_repoter_;

  internal::Mutex global_test_part_result_reporter_mutex_;

  internal::ThreadLocal<TestPartResultReporterInterface*>
      per_thread_test_part_result_reporter_;

  std::vector<Environment*> environments_;

  std::vector<TestSuite*> test_suites_;

  std::vector<int> test_suite_indices_;

  internal::ParameterizedTestSuiteRegistry parameterized_test_registry_;
  internal::TypeParameterizedTestSuiteRegistry
      type_parameterized_test_registry_;

  std::set<std::string> ignored_parameterized_test_suites_;

  bool parameterized_tests_registered_;

  int last_death_test_suite_;

  TestSuite* current_test_suite_;

  TestInfo* current_test_info_;

  TestResult ad_hoc_test_result_;

  TestEventListeners listeners_;

  OsStackTraceGetterInterface* os_stack_trace_getter_;

  bool post_flag_parse_init_performed_;

  int random_seed_;

  internal::Random random_;

  TimeInMillis start_timestamp_;

  TimeInMillis elapsed_time_;

#if GTEST_HAS_DEATH_TEST

  std::unique_ptr<InternalRunDeathTestFlag> internal_run_death_test_flag_;
  std::unique_ptr<internal::DeathTestFactory> death_test_factory_;
#endif

  internal::ThreadLocal<std::vector<TraceInfo> > gtest_trace_stack_;

  bool catch_exceptions_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTestImpl);
};

inline UnitTestImpl* GetUnitTestImpl() {
  return UnitTest::GetInstance()->impl();
}

#if GTEST_USES_SIMPLE_RE

GTEST_API_ bool IsInSet(char ch, const char* str);
GTEST_API_ bool IsAsciiDigit(char ch);
GTEST_API_ bool IsAsciiPunct(char ch);
GTEST_API_ bool IsRepeat(char ch);
GTEST_API_ bool IsAsciiWhiteSpace(char ch);
GTEST_API_ bool IsAsciiWordChar(char ch);
GTEST_API_ bool IsValidEscape(char ch);
GTEST_API_ bool AtomMatchesChar(bool escaped, char pattern, char ch);
GTEST_API_ bool ValidateRegex(const char* regex);
GTEST_API_ bool MatchRegexAtHead(const char* regex, const char* str);
GTEST_API_ bool MatchRepetitionAndRegexAtHead(
    bool escaped, char ch, char repeat, const char* regex, const char* str);
GTEST_API_ bool MatchRegexAnywhere(const char* regex, const char* str);

#endif

GTEST_API_ void ParseGoogleTestFlagsOnly(int* argc, char** argv);
GTEST_API_ void ParseGoogleTestFlagsOnly(int* argc, wchar_t** argv);

#if GTEST_HAS_DEATH_TEST

GTEST_API_ std::string GetLastErrnoDescription();

template <typename Integer>
bool ParseNaturalNumber(const ::std::string& str, Integer* number) {

  if (str.empty() || !IsDigit(str[0])) {
    return false;
  }
  errno = 0;

  char* end;

  using BiggestConvertible = unsigned long long;

  const BiggestConvertible parsed = strtoull(str.c_str(), &end, 10);
  const bool parse_success = *end == '\0' && errno == 0;

  GTEST_CHECK_(sizeof(Integer) <= sizeof(parsed));

  const Integer result = static_cast<Integer>(parsed);
  if (parse_success && static_cast<BiggestConvertible>(result) == parsed) {
    *number = result;
    return true;
  }
  return false;
}
#endif

class TestResultAccessor {
 public:
  static void RecordProperty(TestResult* test_result,
                             const std::string& xml_element,
                             const TestProperty& property) {
    test_result->RecordProperty(xml_element, property);
  }

  static void ClearTestPartResults(TestResult* test_result) {
    test_result->ClearTestPartResults();
  }

  static const std::vector<testing::TestPartResult>& test_part_results(
      const TestResult& test_result) {
    return test_result.test_part_results();
  }
};

#if GTEST_CAN_STREAM_RESULTS_

class StreamingListener : public EmptyTestEventListener {
 public:

  class AbstractSocketWriter {
   public:
    virtual ~AbstractSocketWriter() {}

    virtual void Send(const std::string& message) = 0;

    virtual void CloseConnection() {}

    void SendLn(const std::string& message) { Send(message + "\n"); }
  };

  class SocketWriter : public AbstractSocketWriter {
   public:
    SocketWriter(const std::string& host, const std::string& port)
        : sockfd_(-1), host_name_(host), port_num_(port) {
      MakeConnection();
    }

    ~SocketWriter() override {
      if (sockfd_ != -1)
        CloseConnection();
    }

    void Send(const std::string& message) override {
      GTEST_CHECK_(sockfd_ != -1)
          << "Send() can be called only when there is a connection.";

      const auto len = static_cast<size_t>(message.length());
      if (write(sockfd_, message.c_str(), len) != static_cast<ssize_t>(len)) {
        GTEST_LOG_(WARNING)
            << "stream_result_to: failed to stream to "
            << host_name_ << ":" << port_num_;
      }
    }

   private:

    void MakeConnection();

    void CloseConnection() override {
      GTEST_CHECK_(sockfd_ != -1)
          << "CloseConnection() can be called only when there is a connection.";

      close(sockfd_);
      sockfd_ = -1;
    }

    int sockfd_;
    const std::string host_name_;
    const std::string port_num_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(SocketWriter);
  };

  static std::string UrlEncode(const char* str);

  StreamingListener(const std::string& host, const std::string& port)
      : socket_writer_(new SocketWriter(host, port)) {
    Start();
  }

  explicit StreamingListener(AbstractSocketWriter* socket_writer)
      : socket_writer_(socket_writer) { Start(); }

  void OnTestProgramStart(const UnitTest&  ) override {
    SendLn("event=TestProgramStart");
  }

  void OnTestProgramEnd(const UnitTest& unit_test) override {

    SendLn("event=TestProgramEnd&passed=" + FormatBool(unit_test.Passed()));

    socket_writer_->CloseConnection();
  }

  void OnTestIterationStart(const UnitTest&  ,
                            int iteration) override {
    SendLn("event=TestIterationStart&iteration=" +
           StreamableToString(iteration));
  }

  void OnTestIterationEnd(const UnitTest& unit_test,
                          int  ) override {
    SendLn("event=TestIterationEnd&passed=" +
           FormatBool(unit_test.Passed()) + "&elapsed_time=" +
           StreamableToString(unit_test.elapsed_time()) + "ms");
  }

  void OnTestCaseStart(const TestCase& test_case) override {
    SendLn(std::string("event=TestCaseStart&name=") + test_case.name());
  }

  void OnTestCaseEnd(const TestCase& test_case) override {
    SendLn("event=TestCaseEnd&passed=" + FormatBool(test_case.Passed()) +
           "&elapsed_time=" + StreamableToString(test_case.elapsed_time()) +
           "ms");
  }

  void OnTestStart(const TestInfo& test_info) override {
    SendLn(std::string("event=TestStart&name=") + test_info.name());
  }

  void OnTestEnd(const TestInfo& test_info) override {
    SendLn("event=TestEnd&passed=" +
           FormatBool((test_info.result())->Passed()) +
           "&elapsed_time=" +
           StreamableToString((test_info.result())->elapsed_time()) + "ms");
  }

  void OnTestPartResult(const TestPartResult& test_part_result) override {
    const char* file_name = test_part_result.file_name();
    if (file_name == nullptr) file_name = "";
    SendLn("event=TestPartResult&file=" + UrlEncode(file_name) +
           "&line=" + StreamableToString(test_part_result.line_number()) +
           "&message=" + UrlEncode(test_part_result.message()));
  }

 private:

  void SendLn(const std::string& message) { socket_writer_->SendLn(message); }

  void Start() { SendLn("gtest_streaming_protocol_version=1.0"); }

  std::string FormatBool(bool value) { return value ? "1" : "0"; }

  const std::unique_ptr<AbstractSocketWriter> socket_writer_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(StreamingListener);
};

#endif

}
}

GTEST_DISABLE_MSC_WARNINGS_POP_()

#endif

#if GTEST_OS_WINDOWS
# define vsnprintf _vsnprintf
#endif

#if GTEST_OS_MAC
#ifndef GTEST_OS_IOS
#include <crt_externs.h>
#endif
#endif

#if GTEST_HAS_ABSL
#include "absl/debugging/failure_signal_handler.h"
#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"
#include "absl/strings/str_cat.h"
#endif

namespace testing {

using internal::CountIf;
using internal::ForEach;
using internal::GetElementOr;
using internal::Shuffle;

static const char kDisableTestFilter[] = "DISABLED_*:*/DISABLED_*";

static const char kDeathTestSuiteFilter[] = "*DeathTest:*DeathTest/*";

static const char kUniversalFilter[] = "*";

static const char kDefaultOutputFormat[] = "xml";

static const char kDefaultOutputFile[] = "test_detail";

static const char kTestShardIndex[] = "GTEST_SHARD_INDEX";

static const char kTestTotalShards[] = "GTEST_TOTAL_SHARDS";

static const char kTestShardStatusFile[] = "GTEST_SHARD_STATUS_FILE";

namespace internal {

const char kStackTraceMarker[] = "\nStack trace:\n";

bool g_help_flag = false;

static FILE* OpenFileForWriting(const std::string& output_file) {
  FILE* fileout = nullptr;
  FilePath output_file_path(output_file);
  FilePath output_dir(output_file_path.RemoveFileName());

  if (output_dir.CreateDirectoriesRecursively()) {
    fileout = posix::FOpen(output_file.c_str(), "w");
  }
  if (fileout == nullptr) {
    GTEST_LOG_(FATAL) << "Unable to open file \"" << output_file << "\"";
  }
  return fileout;
}

}

static const char* GetDefaultFilter() {
  const char* const testbridge_test_only =
      internal::posix::GetEnv("TESTBRIDGE_TEST_ONLY");
  if (testbridge_test_only != nullptr) {
    return testbridge_test_only;
  }
  return kUniversalFilter;
}

static bool GetDefaultFailFast() {
  const char* const testbridge_test_runner_fail_fast =
      internal::posix::GetEnv("TESTBRIDGE_TEST_RUNNER_FAIL_FAST");
  if (testbridge_test_runner_fail_fast != nullptr) {
    return strcmp(testbridge_test_runner_fail_fast, "1") == 0;
  }
  return false;
}

GTEST_DEFINE_bool_(
    fail_fast, internal::BoolFromGTestEnv("fail_fast", GetDefaultFailFast()),
    "True if and only if a test failure should stop further test execution.");

GTEST_DEFINE_bool_(
    also_run_disabled_tests,
    internal::BoolFromGTestEnv("also_run_disabled_tests", false),
    "Run disabled tests too, in addition to the tests normally being run.");

GTEST_DEFINE_bool_(
    break_on_failure, internal::BoolFromGTestEnv("break_on_failure", false),
    "True if and only if a failed assertion should be a debugger "
    "break-point.");

GTEST_DEFINE_bool_(catch_exceptions,
                   internal::BoolFromGTestEnv("catch_exceptions", true),
                   "True if and only if " GTEST_NAME_
                   " should catch exceptions and treat them as test failures.");

GTEST_DEFINE_string_(
    color,
    internal::StringFromGTestEnv("color", "auto"),
    "Whether to use colors in the output.  Valid values: yes, no, "
    "and auto.  'auto' means to use colors if the output is "
    "being sent to a terminal and the TERM environment variable "
    "is set to a terminal type that supports colors.");

GTEST_DEFINE_string_(
    filter,
    internal::StringFromGTestEnv("filter", GetDefaultFilter()),
    "A colon-separated list of glob (not regex) patterns "
    "for filtering the tests to run, optionally followed by a "
    "'-' and a : separated list of negative patterns (tests to "
    "exclude).  A test is run if it matches one of the positive "
    "patterns and does not match any of the negative patterns.");

GTEST_DEFINE_bool_(
    install_failure_signal_handler,
    internal::BoolFromGTestEnv("install_failure_signal_handler", false),
    "If true and supported on the current platform, " GTEST_NAME_ " should "
    "install a signal handler that dumps debugging information when fatal "
    "signals are raised.");

GTEST_DEFINE_bool_(list_tests, false,
                   "List all tests without running them.");

GTEST_DEFINE_string_(
    output,
    internal::StringFromGTestEnv("output",
      internal::OutputFlagAlsoCheckEnvVar().c_str()),
    "A format (defaults to \"xml\" but can be specified to be \"json\"), "
    "optionally followed by a colon and an output file name or directory. "
    "A directory is indicated by a trailing pathname separator. "
    "Examples: \"xml:filename.xml\", \"xml::directoryname/\". "
    "If a directory is specified, output files will be created "
    "within that directory, with file-names based on the test "
    "executable's name and, if necessary, made unique by adding "
    "digits.");

GTEST_DEFINE_bool_(
    brief, internal::BoolFromGTestEnv("brief", false),
    "True if only test failures should be displayed in text output.");

GTEST_DEFINE_bool_(print_time, internal::BoolFromGTestEnv("print_time", true),
                   "True if and only if " GTEST_NAME_
                   " should display elapsed time in text output.");

GTEST_DEFINE_bool_(print_utf8, internal::BoolFromGTestEnv("print_utf8", true),
                   "True if and only if " GTEST_NAME_
                   " prints UTF8 characters as text.");

GTEST_DEFINE_int32_(
    random_seed,
    internal::Int32FromGTestEnv("random_seed", 0),
    "Random number seed to use when shuffling test orders.  Must be in range "
    "[1, 99999], or 0 to use a seed based on the current time.");

GTEST_DEFINE_int32_(
    repeat,
    internal::Int32FromGTestEnv("repeat", 1),
    "How many times to repeat each test.  Specify a negative number "
    "for repeating forever.  Useful for shaking out flaky tests.");

GTEST_DEFINE_bool_(show_internal_stack_frames, false,
                   "True if and only if " GTEST_NAME_
                   " should include internal stack frames when "
                   "printing test failure stack traces.");

GTEST_DEFINE_bool_(shuffle, internal::BoolFromGTestEnv("shuffle", false),
                   "True if and only if " GTEST_NAME_
                   " should randomize tests' order on every run.");

GTEST_DEFINE_int32_(
    stack_trace_depth,
    internal::Int32FromGTestEnv("stack_trace_depth", kMaxStackTraceDepth),
    "The maximum number of stack frames to print when an "
    "assertion fails.  The valid range is 0 through 100, inclusive.");

GTEST_DEFINE_string_(
    stream_result_to,
    internal::StringFromGTestEnv("stream_result_to", ""),
    "This flag specifies the host name and the port number on which to stream "
    "test results. Example: \"localhost:555\". The flag is effective only on "
    "Linux.");

GTEST_DEFINE_bool_(
    throw_on_failure,
    internal::BoolFromGTestEnv("throw_on_failure", false),
    "When this flag is specified, a failed assertion will throw an exception "
    "if exceptions are enabled or exit the program with a non-zero code "
    "otherwise. For use with an external test framework.");

#if GTEST_USE_OWN_FLAGFILE_FLAG_
GTEST_DEFINE_string_(
    flagfile,
    internal::StringFromGTestEnv("flagfile", ""),
    "This flag specifies the flagfile to read command-line flags from.");
#endif

namespace internal {

uint32_t Random::Generate(uint32_t range) {

  state_ = static_cast<uint32_t>(1103515245ULL*state_ + 12345U) % kMaxRange;

  GTEST_CHECK_(range > 0)
      << "Cannot generate a number in the range [0, 0).";
  GTEST_CHECK_(range <= kMaxRange)
      << "Generation of a number in [0, " << range << ") was requested, "
      << "but this can only generate numbers in [0, " << kMaxRange << ").";

  return state_ % range;
}

static bool GTestIsInitialized() { return GetArgvs().size() > 0; }

static int SumOverTestSuiteList(const std::vector<TestSuite*>& case_list,
                                int (TestSuite::*method)() const) {
  int sum = 0;
  for (size_t i = 0; i < case_list.size(); i++) {
    sum += (case_list[i]->*method)();
  }
  return sum;
}

static bool TestSuitePassed(const TestSuite* test_suite) {
  return test_suite->should_run() && test_suite->Passed();
}

static bool TestSuiteFailed(const TestSuite* test_suite) {
  return test_suite->should_run() && test_suite->Failed();
}

static bool ShouldRunTestSuite(const TestSuite* test_suite) {
  return test_suite->should_run();
}

AssertHelper::AssertHelper(TestPartResult::Type type,
                           const char* file,
                           int line,
                           const char* message)
    : data_(new AssertHelperData(type, file, line, message)) {
}

AssertHelper::~AssertHelper() {
  delete data_;
}

void AssertHelper::operator=(const Message& message) const {
  UnitTest::GetInstance()->
    AddTestPartResult(data_->type, data_->file, data_->line,
                      AppendUserMessage(data_->message, message),
                      UnitTest::GetInstance()->impl()
                      ->CurrentOsStackTraceExceptTop(1)

                      );
}

namespace {

constexpr bool kErrorOnUninstantiatedParameterizedTest = true;
constexpr bool kErrorOnUninstantiatedTypeParameterizedTest = true;

class FailureTest : public Test {
 public:
  explicit FailureTest(const CodeLocation& loc, std::string error_message,
                       bool as_error)
      : loc_(loc),
        error_message_(std::move(error_message)),
        as_error_(as_error) {}

  void TestBody() override {
    if (as_error_) {
      AssertHelper(TestPartResult::kNonFatalFailure, loc_.file.c_str(),
                   loc_.line, "") = Message() << error_message_;
    } else {
      std::cout << error_message_ << std::endl;
    }
  }

 private:
  const CodeLocation loc_;
  const std::string error_message_;
  const bool as_error_;
};

}

std::set<std::string>* GetIgnoredParameterizedTestSuites() {
  return UnitTest::GetInstance()->impl()->ignored_parameterized_test_suites();
}

MarkAsIgnored::MarkAsIgnored(const char* test_suite) {
  GetIgnoredParameterizedTestSuites()->insert(test_suite);
}

void InsertSyntheticTestCase(const std::string& name, CodeLocation location,
                             bool has_test_p) {
  const auto& ignored = *GetIgnoredParameterizedTestSuites();
  if (ignored.find(name) != ignored.end()) return;

  const char kMissingInstantiation[] =
      " is defined via TEST_P, but never instantiated. None of the test cases "
      "will run. Either no INSTANTIATE_TEST_SUITE_P is provided or the only "
      "ones provided expand to nothing."
      "\n\n"
      "Ideally, TEST_P definitions should only ever be included as part of "
      "binaries that intend to use them. (As opposed to, for example, being "
      "placed in a library that may be linked in to get other utilities.)";

  const char kMissingTestCase[] =
      " is instantiated via INSTANTIATE_TEST_SUITE_P, but no tests are "
      "defined via TEST_P . No test cases will run."
      "\n\n"
      "Ideally, INSTANTIATE_TEST_SUITE_P should only ever be invoked from "
      "code that always depend on code that provides TEST_P. Failing to do "
      "so is often an indication of dead code, e.g. the last TEST_P was "
      "removed but the rest got left behind.";

  std::string message =
      "Parameterized test suite " + name +
      (has_test_p ? kMissingInstantiation : kMissingTestCase) +
      "\n\n"
      "To suppress this error for this test suite, insert the following line "
      "(in a non-header) in the namespace it is defined in:"
      "\n\n"
      "GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(" + name + ");";

  std::string full_name = "UninstantiatedParameterizedTestSuite<" + name + ">";
  RegisterTest(
      "GoogleTestVerification", full_name.c_str(),
      nullptr,
      nullptr,
      location.file.c_str(), location.line, [message, location] {
        return new FailureTest(location, message,
                               kErrorOnUninstantiatedParameterizedTest);
      });
}

void RegisterTypeParameterizedTestSuite(const char* test_suite_name,
                                        CodeLocation code_location) {
  GetUnitTestImpl()->type_parameterized_test_registry().RegisterTestSuite(
      test_suite_name, code_location);
}

void RegisterTypeParameterizedTestSuiteInstantiation(const char* case_name) {
  GetUnitTestImpl()
      ->type_parameterized_test_registry()
      .RegisterInstantiation(case_name);
}

void TypeParameterizedTestSuiteRegistry::RegisterTestSuite(
    const char* test_suite_name, CodeLocation code_location) {
  suites_.emplace(std::string(test_suite_name),
                 TypeParameterizedTestSuiteInfo(code_location));
}

void TypeParameterizedTestSuiteRegistry::RegisterInstantiation(
        const char* test_suite_name) {
  auto it = suites_.find(std::string(test_suite_name));
  if (it != suites_.end()) {
    it->second.instantiated = true;
  } else {
    GTEST_LOG_(ERROR) << "Unknown type parameterized test suit '"
                      << test_suite_name << "'";
  }
}

void TypeParameterizedTestSuiteRegistry::CheckForInstantiations() {
  const auto& ignored = *GetIgnoredParameterizedTestSuites();
  for (const auto& testcase : suites_) {
    if (testcase.second.instantiated) continue;
    if (ignored.find(testcase.first) != ignored.end()) continue;

    std::string message =
        "Type parameterized test suite " + testcase.first +
        " is defined via REGISTER_TYPED_TEST_SUITE_P, but never instantiated "
        "via INSTANTIATE_TYPED_TEST_SUITE_P. None of the test cases will run."
        "\n\n"
        "Ideally, TYPED_TEST_P definitions should only ever be included as "
        "part of binaries that intend to use them. (As opposed to, for "
        "example, being placed in a library that may be linked in to get other "
        "utilities.)"
        "\n\n"
        "To suppress this error for this test suite, insert the following line "
        "(in a non-header) in the namespace it is defined in:"
        "\n\n"
        "GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(" +
        testcase.first + ");";

    std::string full_name =
        "UninstantiatedTypeParameterizedTestSuite<" + testcase.first + ">";
    RegisterTest(
        "GoogleTestVerification", full_name.c_str(),
        nullptr,
        nullptr,
        testcase.second.code_location.file.c_str(),
        testcase.second.code_location.line, [message, testcase] {
          return new FailureTest(testcase.second.code_location, message,
                                 kErrorOnUninstantiatedTypeParameterizedTest);
        });
  }
}

static ::std::vector<std::string> g_argvs;

::std::vector<std::string> GetArgvs() {
#if defined(GTEST_CUSTOM_GET_ARGVS_)

  const auto& custom = GTEST_CUSTOM_GET_ARGVS_();
  return ::std::vector<std::string>(custom.begin(), custom.end());
#else
  return g_argvs;
#endif
}

FilePath GetCurrentExecutableName() {
  FilePath result;

#if GTEST_OS_WINDOWS || GTEST_OS_OS2
  result.Set(FilePath(GetArgvs()[0]).RemoveExtension("exe"));
#else
  result.Set(FilePath(GetArgvs()[0]));
#endif

  return result.RemoveDirectoryName();
}

std::string UnitTestOptions::GetOutputFormat() {
  const char* const gtest_output_flag = GTEST_FLAG(output).c_str();
  const char* const colon = strchr(gtest_output_flag, ':');
  return (colon == nullptr)
             ? std::string(gtest_output_flag)
             : std::string(gtest_output_flag,
                           static_cast<size_t>(colon - gtest_output_flag));
}

std::string UnitTestOptions::GetAbsolutePathToOutputFile() {
  const char* const gtest_output_flag = GTEST_FLAG(output).c_str();

  std::string format = GetOutputFormat();
  if (format.empty())
    format = std::string(kDefaultOutputFormat);

  const char* const colon = strchr(gtest_output_flag, ':');
  if (colon == nullptr)
    return internal::FilePath::MakeFileName(
        internal::FilePath(
            UnitTest::GetInstance()->original_working_dir()),
        internal::FilePath(kDefaultOutputFile), 0,
        format.c_str()).string();

  internal::FilePath output_name(colon + 1);
  if (!output_name.IsAbsolutePath())
    output_name = internal::FilePath::ConcatPaths(
        internal::FilePath(UnitTest::GetInstance()->original_working_dir()),
        internal::FilePath(colon + 1));

  if (!output_name.IsDirectory())
    return output_name.string();

  internal::FilePath result(internal::FilePath::GenerateUniqueFileName(
      output_name, internal::GetCurrentExecutableName(),
      GetOutputFormat().c_str()));
  return result.string();
}

static bool PatternMatchesString(const std::string& name_str,
                                 const char* pattern, const char* pattern_end) {
  const char* name = name_str.c_str();
  const char* const name_begin = name;
  const char* const name_end = name + name_str.size();

  const char* pattern_next = pattern;
  const char* name_next = name;

  while (pattern < pattern_end || name < name_end) {
    if (pattern < pattern_end) {
      switch (*pattern) {
        default:
          if (name < name_end && *name == *pattern) {
            ++pattern;
            ++name;
            continue;
          }
          break;
        case '?':
          if (name < name_end) {
            ++pattern;
            ++name;
            continue;
          }
          break;
        case '*':

          pattern_next = pattern;
          name_next = name + 1;
          ++pattern;
          continue;
      }
    }

    if (name_begin < name_next && name_next <= name_end) {
      pattern = pattern_next;
      name = name_next;
      continue;
    }
    return false;
  }
  return true;
}

bool UnitTestOptions::MatchesFilter(const std::string& name_str,
                                    const char* filter) {

  const char* pattern = filter;
  while (true) {

    const char* const next_sep = strchr(pattern, ':');
    const char* const pattern_end =
        next_sep != nullptr ? next_sep : pattern + strlen(pattern);

    if (PatternMatchesString(name_str, pattern, pattern_end)) {
      break;
    }

    if (next_sep == nullptr) {
      return false;
    }
    pattern = next_sep + 1;
  }
  return true;
}

bool UnitTestOptions::FilterMatchesTest(const std::string& test_suite_name,
                                        const std::string& test_name) {
  const std::string& full_name = test_suite_name + "." + test_name.c_str();

  const char* const p = GTEST_FLAG(filter).c_str();
  const char* const dash = strchr(p, '-');
  std::string positive;
  std::string negative;
  if (dash == nullptr) {
    positive = GTEST_FLAG(filter).c_str();
    negative = "";
  } else {
    positive = std::string(p, dash);
    negative = std::string(dash + 1);
    if (positive.empty()) {

      positive = kUniversalFilter;
    }
  }

  return (MatchesFilter(full_name, positive.c_str()) &&
          !MatchesFilter(full_name, negative.c_str()));
}

#if GTEST_HAS_SEH

int UnitTestOptions::GTestShouldProcessSEH(DWORD exception_code) {

  const DWORD kCxxExceptionCode = 0xe06d7363;

  bool should_handle = true;

  if (!GTEST_FLAG(catch_exceptions))
    should_handle = false;
  else if (exception_code == EXCEPTION_BREAKPOINT)
    should_handle = false;
  else if (exception_code == kCxxExceptionCode)
    should_handle = false;

  return should_handle ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}
#endif

}

ScopedFakeTestPartResultReporter::ScopedFakeTestPartResultReporter(
    TestPartResultArray* result)
    : intercept_mode_(INTERCEPT_ONLY_CURRENT_THREAD),
      result_(result) {
  Init();
}

ScopedFakeTestPartResultReporter::ScopedFakeTestPartResultReporter(
    InterceptMode intercept_mode, TestPartResultArray* result)
    : intercept_mode_(intercept_mode),
      result_(result) {
  Init();
}

void ScopedFakeTestPartResultReporter::Init() {
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  if (intercept_mode_ == INTERCEPT_ALL_THREADS) {
    old_reporter_ = impl->GetGlobalTestPartResultReporter();
    impl->SetGlobalTestPartResultReporter(this);
  } else {
    old_reporter_ = impl->GetTestPartResultReporterForCurrentThread();
    impl->SetTestPartResultReporterForCurrentThread(this);
  }
}

ScopedFakeTestPartResultReporter::~ScopedFakeTestPartResultReporter() {
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  if (intercept_mode_ == INTERCEPT_ALL_THREADS) {
    impl->SetGlobalTestPartResultReporter(old_reporter_);
  } else {
    impl->SetTestPartResultReporterForCurrentThread(old_reporter_);
  }
}

void ScopedFakeTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  result_->Append(result);
}

namespace internal {

TypeId GetTestTypeId() {
  return GetTypeId<Test>();
}

extern const TypeId kTestTypeIdInGoogleTest = GetTestTypeId();

static AssertionResult HasOneFailure(const char*  ,
                                     const char*  ,
                                     const char*  ,
                                     const TestPartResultArray& results,
                                     TestPartResult::Type type,
                                     const std::string& substr) {
  const std::string expected(type == TestPartResult::kFatalFailure ?
                        "1 fatal failure" :
                        "1 non-fatal failure");
  Message msg;
  if (results.size() != 1) {
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << results.size() << " failures";
    for (int i = 0; i < results.size(); i++) {
      msg << "\n" << results.GetTestPartResult(i);
    }
    return AssertionFailure() << msg;
  }

  const TestPartResult& r = results.GetTestPartResult(0);
  if (r.type() != type) {
    return AssertionFailure() << "Expected: " << expected << "\n"
                              << "  Actual:\n"
                              << r;
  }

  if (strstr(r.message(), substr.c_str()) == nullptr) {
    return AssertionFailure() << "Expected: " << expected << " containing \""
                              << substr << "\"\n"
                              << "  Actual:\n"
                              << r;
  }

  return AssertionSuccess();
}

SingleFailureChecker::SingleFailureChecker(const TestPartResultArray* results,
                                           TestPartResult::Type type,
                                           const std::string& substr)
    : results_(results), type_(type), substr_(substr) {}

SingleFailureChecker::~SingleFailureChecker() {
  EXPECT_PRED_FORMAT3(HasOneFailure, *results_, type_, substr_);
}

DefaultGlobalTestPartResultReporter::DefaultGlobalTestPartResultReporter(
    UnitTestImpl* unit_test) : unit_test_(unit_test) {}

void DefaultGlobalTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  unit_test_->current_test_result()->AddTestPartResult(result);
  unit_test_->listeners()->repeater()->OnTestPartResult(result);
}

DefaultPerThreadTestPartResultReporter::DefaultPerThreadTestPartResultReporter(
    UnitTestImpl* unit_test) : unit_test_(unit_test) {}

void DefaultPerThreadTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  unit_test_->GetGlobalTestPartResultReporter()->ReportTestPartResult(result);
}

TestPartResultReporterInterface*
UnitTestImpl::GetGlobalTestPartResultReporter() {
  internal::MutexLock lock(&global_test_part_result_reporter_mutex_);
  return global_test_part_result_repoter_;
}

void UnitTestImpl::SetGlobalTestPartResultReporter(
    TestPartResultReporterInterface* reporter) {
  internal::MutexLock lock(&global_test_part_result_reporter_mutex_);
  global_test_part_result_repoter_ = reporter;
}

TestPartResultReporterInterface*
UnitTestImpl::GetTestPartResultReporterForCurrentThread() {
  return per_thread_test_part_result_reporter_.get();
}

void UnitTestImpl::SetTestPartResultReporterForCurrentThread(
    TestPartResultReporterInterface* reporter) {
  per_thread_test_part_result_reporter_.set(reporter);
}

int UnitTestImpl::successful_test_suite_count() const {
  return CountIf(test_suites_, TestSuitePassed);
}

int UnitTestImpl::failed_test_suite_count() const {
  return CountIf(test_suites_, TestSuiteFailed);
}

int UnitTestImpl::total_test_suite_count() const {
  return static_cast<int>(test_suites_.size());
}

int UnitTestImpl::test_suite_to_run_count() const {
  return CountIf(test_suites_, ShouldRunTestSuite);
}

int UnitTestImpl::successful_test_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::successful_test_count);
}

int UnitTestImpl::skipped_test_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::skipped_test_count);
}

int UnitTestImpl::failed_test_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::failed_test_count);
}

int UnitTestImpl::reportable_disabled_test_count() const {
  return SumOverTestSuiteList(test_suites_,
                              &TestSuite::reportable_disabled_test_count);
}

int UnitTestImpl::disabled_test_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::disabled_test_count);
}

int UnitTestImpl::reportable_test_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::reportable_test_count);
}

int UnitTestImpl::total_test_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::total_test_count);
}

int UnitTestImpl::test_to_run_count() const {
  return SumOverTestSuiteList(test_suites_, &TestSuite::test_to_run_count);
}

std::string UnitTestImpl::CurrentOsStackTraceExceptTop(int skip_count) {
  return os_stack_trace_getter()->CurrentStackTrace(
      static_cast<int>(GTEST_FLAG(stack_trace_depth)),
      skip_count + 1

      );
}

class Timer {
 public:
  Timer() : start_(std::chrono::steady_clock::now()) {}

  TimeInMillis Elapsed() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_)
        .count();
  }

 private:
  std::chrono::steady_clock::time_point start_;
};

TimeInMillis GetTimeInMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now() -
             std::chrono::system_clock::from_time_t(0))
      .count();
}

#if GTEST_OS_WINDOWS_MOBILE

LPCWSTR String::AnsiToUtf16(const char* ansi) {
  if (!ansi) return nullptr;
  const int length = strlen(ansi);
  const int unicode_length =
      MultiByteToWideChar(CP_ACP, 0, ansi, length, nullptr, 0);
  WCHAR* unicode = new WCHAR[unicode_length + 1];
  MultiByteToWideChar(CP_ACP, 0, ansi, length,
                      unicode, unicode_length);
  unicode[unicode_length] = 0;
  return unicode;
}

const char* String::Utf16ToAnsi(LPCWSTR utf16_str)  {
  if (!utf16_str) return nullptr;
  const int ansi_length = WideCharToMultiByte(CP_ACP, 0, utf16_str, -1, nullptr,
                                              0, nullptr, nullptr);
  char* ansi = new char[ansi_length + 1];
  WideCharToMultiByte(CP_ACP, 0, utf16_str, -1, ansi, ansi_length, nullptr,
                      nullptr);
  ansi[ansi_length] = 0;
  return ansi;
}

#endif

bool String::CStringEquals(const char * lhs, const char * rhs) {
  if (lhs == nullptr) return rhs == nullptr;

  if (rhs == nullptr) return false;

  return strcmp(lhs, rhs) == 0;
}

#if GTEST_HAS_STD_WSTRING

static void StreamWideCharsToMessage(const wchar_t* wstr, size_t length,
                                     Message* msg) {
  for (size_t i = 0; i != length; ) {
    if (wstr[i] != L'\0') {
      *msg << WideStringToUtf8(wstr + i, static_cast<int>(length - i));
      while (i != length && wstr[i] != L'\0')
        i++;
    } else {
      *msg << '\0';
      i++;
    }
  }
}

#endif

void SplitString(const ::std::string& str, char delimiter,
                 ::std::vector< ::std::string>* dest) {
  ::std::vector< ::std::string> parsed;
  ::std::string::size_type pos = 0;
  while (::testing::internal::AlwaysTrue()) {
    const ::std::string::size_type colon = str.find(delimiter, pos);
    if (colon == ::std::string::npos) {
      parsed.push_back(str.substr(pos));
      break;
    } else {
      parsed.push_back(str.substr(pos, colon - pos));
      pos = colon + 1;
    }
  }
  dest->swap(parsed);
}

}

Message::Message() : ss_(new ::std::stringstream) {

  *ss_ << std::setprecision(std::numeric_limits<double>::digits10 + 2);
}

Message& Message::operator <<(const wchar_t* wide_c_str) {
  return *this << internal::String::ShowWideCString(wide_c_str);
}
Message& Message::operator <<(wchar_t* wide_c_str) {
  return *this << internal::String::ShowWideCString(wide_c_str);
}

#if GTEST_HAS_STD_WSTRING

Message& Message::operator <<(const ::std::wstring& wstr) {
  internal::StreamWideCharsToMessage(wstr.c_str(), wstr.length(), this);
  return *this;
}
#endif

std::string Message::GetString() const {
  return internal::StringStreamToString(ss_.get());
}

AssertionResult::AssertionResult(const AssertionResult& other)
    : success_(other.success_),
      message_(other.message_.get() != nullptr
                   ? new ::std::string(*other.message_)
                   : static_cast< ::std::string*>(nullptr)) {}

void AssertionResult::swap(AssertionResult& other) {
  using std::swap;
  swap(success_, other.success_);
  swap(message_, other.message_);
}

AssertionResult AssertionResult::operator!() const {
  AssertionResult negation(!success_);
  if (message_.get() != nullptr) negation << *message_;
  return negation;
}

AssertionResult AssertionSuccess() {
  return AssertionResult(true);
}

AssertionResult AssertionFailure() {
  return AssertionResult(false);
}

AssertionResult AssertionFailure(const Message& message) {
  return AssertionFailure() << message;
}

namespace internal {

namespace edit_distance {
std::vector<EditType> CalculateOptimalEdits(const std::vector<size_t>& left,
                                            const std::vector<size_t>& right) {
  std::vector<std::vector<double> > costs(
      left.size() + 1, std::vector<double>(right.size() + 1));
  std::vector<std::vector<EditType> > best_move(
      left.size() + 1, std::vector<EditType>(right.size() + 1));

  for (size_t l_i = 0; l_i < costs.size(); ++l_i) {
    costs[l_i][0] = static_cast<double>(l_i);
    best_move[l_i][0] = kRemove;
  }

  for (size_t r_i = 1; r_i < costs[0].size(); ++r_i) {
    costs[0][r_i] = static_cast<double>(r_i);
    best_move[0][r_i] = kAdd;
  }

  for (size_t l_i = 0; l_i < left.size(); ++l_i) {
    for (size_t r_i = 0; r_i < right.size(); ++r_i) {
      if (left[l_i] == right[r_i]) {

        costs[l_i + 1][r_i + 1] = costs[l_i][r_i];
        best_move[l_i + 1][r_i + 1] = kMatch;
        continue;
      }

      const double add = costs[l_i + 1][r_i];
      const double remove = costs[l_i][r_i + 1];
      const double replace = costs[l_i][r_i];
      if (add < remove && add < replace) {
        costs[l_i + 1][r_i + 1] = add + 1;
        best_move[l_i + 1][r_i + 1] = kAdd;
      } else if (remove < add && remove < replace) {
        costs[l_i + 1][r_i + 1] = remove + 1;
        best_move[l_i + 1][r_i + 1] = kRemove;
      } else {

        costs[l_i + 1][r_i + 1] = replace + 1.00001;
        best_move[l_i + 1][r_i + 1] = kReplace;
      }
    }
  }

  std::vector<EditType> best_path;
  for (size_t l_i = left.size(), r_i = right.size(); l_i > 0 || r_i > 0;) {
    EditType move = best_move[l_i][r_i];
    best_path.push_back(move);
    l_i -= move != kAdd;
    r_i -= move != kRemove;
  }
  std::reverse(best_path.begin(), best_path.end());
  return best_path;
}

namespace {

class InternalStrings {
 public:
  size_t GetId(const std::string& str) {
    IdMap::iterator it = ids_.find(str);
    if (it != ids_.end()) return it->second;
    size_t id = ids_.size();
    return ids_[str] = id;
  }

 private:
  typedef std::map<std::string, size_t> IdMap;
  IdMap ids_;
};

}

std::vector<EditType> CalculateOptimalEdits(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right) {
  std::vector<size_t> left_ids, right_ids;
  {
    InternalStrings intern_table;
    for (size_t i = 0; i < left.size(); ++i) {
      left_ids.push_back(intern_table.GetId(left[i]));
    }
    for (size_t i = 0; i < right.size(); ++i) {
      right_ids.push_back(intern_table.GetId(right[i]));
    }
  }
  return CalculateOptimalEdits(left_ids, right_ids);
}

namespace {

class Hunk {
 public:
  Hunk(size_t left_start, size_t right_start)
      : left_start_(left_start),
        right_start_(right_start),
        adds_(),
        removes_(),
        common_() {}

  void PushLine(char edit, const char* line) {
    switch (edit) {
      case ' ':
        ++common_;
        FlushEdits();
        hunk_.push_back(std::make_pair(' ', line));
        break;
      case '-':
        ++removes_;
        hunk_removes_.push_back(std::make_pair('-', line));
        break;
      case '+':
        ++adds_;
        hunk_adds_.push_back(std::make_pair('+', line));
        break;
    }
  }

  void PrintTo(std::ostream* os) {
    PrintHeader(os);
    FlushEdits();
    for (std::list<std::pair<char, const char*> >::const_iterator it =
             hunk_.begin();
         it != hunk_.end(); ++it) {
      *os << it->first << it->second << "\n";
    }
  }

  bool has_edits() const { return adds_ || removes_; }

 private:
  void FlushEdits() {
    hunk_.splice(hunk_.end(), hunk_removes_);
    hunk_.splice(hunk_.end(), hunk_adds_);
  }

  void PrintHeader(std::ostream* ss) const {
    *ss << "@@ ";
    if (removes_) {
      *ss << "-" << left_start_ << "," << (removes_ + common_);
    }
    if (removes_ && adds_) {
      *ss << " ";
    }
    if (adds_) {
      *ss << "+" << right_start_ << "," << (adds_ + common_);
    }
    *ss << " @@\n";
  }

  size_t left_start_, right_start_;
  size_t adds_, removes_, common_;
  std::list<std::pair<char, const char*> > hunk_, hunk_adds_, hunk_removes_;
};

}

std::string CreateUnifiedDiff(const std::vector<std::string>& left,
                              const std::vector<std::string>& right,
                              size_t context) {
  const std::vector<EditType> edits = CalculateOptimalEdits(left, right);

  size_t l_i = 0, r_i = 0, edit_i = 0;
  std::stringstream ss;
  while (edit_i < edits.size()) {

    while (edit_i < edits.size() && edits[edit_i] == kMatch) {
      ++l_i;
      ++r_i;
      ++edit_i;
    }

    const size_t prefix_context = std::min(l_i, context);
    Hunk hunk(l_i - prefix_context + 1, r_i - prefix_context + 1);
    for (size_t i = prefix_context; i > 0; --i) {
      hunk.PushLine(' ', left[l_i - i].c_str());
    }

    size_t n_suffix = 0;
    for (; edit_i < edits.size(); ++edit_i) {
      if (n_suffix >= context) {

        auto it = edits.begin() + static_cast<int>(edit_i);
        while (it != edits.end() && *it == kMatch) ++it;
        if (it == edits.end() ||
            static_cast<size_t>(it - edits.begin()) - edit_i >= context) {

          break;
        }
      }

      EditType edit = edits[edit_i];

      n_suffix = edit == kMatch ? n_suffix + 1 : 0;

      if (edit == kMatch || edit == kRemove || edit == kReplace) {
        hunk.PushLine(edit == kMatch ? ' ' : '-', left[l_i].c_str());
      }
      if (edit == kAdd || edit == kReplace) {
        hunk.PushLine('+', right[r_i].c_str());
      }

      l_i += edit != kAdd;
      r_i += edit != kRemove;
    }

    if (!hunk.has_edits()) {

      break;
    }

    hunk.PrintTo(&ss);
  }
  return ss.str();
}

}

namespace {

std::vector<std::string> SplitEscapedString(const std::string& str) {
  std::vector<std::string> lines;
  size_t start = 0, end = str.size();
  if (end > 2 && str[0] == '"' && str[end - 1] == '"') {
    ++start;
    --end;
  }
  bool escaped = false;
  for (size_t i = start; i + 1 < end; ++i) {
    if (escaped) {
      escaped = false;
      if (str[i] == 'n') {
        lines.push_back(str.substr(start, i - start - 1));
        start = i + 1;
      }
    } else {
      escaped = str[i] == '\\';
    }
  }
  lines.push_back(str.substr(start, end - start));
  return lines;
}

}

AssertionResult EqFailure(const char* lhs_expression,
                          const char* rhs_expression,
                          const std::string& lhs_value,
                          const std::string& rhs_value,
                          bool ignoring_case) {
  Message msg;
  msg << "Expected equality of these values:";
  msg << "\n  " << lhs_expression;
  if (lhs_value != lhs_expression) {
    msg << "\n    Which is: " << lhs_value;
  }
  msg << "\n  " << rhs_expression;
  if (rhs_value != rhs_expression) {
    msg << "\n    Which is: " << rhs_value;
  }

  if (ignoring_case) {
    msg << "\nIgnoring case";
  }

  if (!lhs_value.empty() && !rhs_value.empty()) {
    const std::vector<std::string> lhs_lines =
        SplitEscapedString(lhs_value);
    const std::vector<std::string> rhs_lines =
        SplitEscapedString(rhs_value);
    if (lhs_lines.size() > 1 || rhs_lines.size() > 1) {
      msg << "\nWith diff:\n"
          << edit_distance::CreateUnifiedDiff(lhs_lines, rhs_lines);
    }
  }

  return AssertionFailure() << msg;
}

std::string GetBoolAssertionFailureMessage(
    const AssertionResult& assertion_result,
    const char* expression_text,
    const char* actual_predicate_value,
    const char* expected_predicate_value) {
  const char* actual_message = assertion_result.message();
  Message msg;
  msg << "Value of: " << expression_text
      << "\n  Actual: " << actual_predicate_value;
  if (actual_message[0] != '\0')
    msg << " (" << actual_message << ")";
  msg << "\nExpected: " << expected_predicate_value;
  return msg.GetString();
}

AssertionResult DoubleNearPredFormat(const char* expr1,
                                     const char* expr2,
                                     const char* abs_error_expr,
                                     double val1,
                                     double val2,
                                     double abs_error) {
  const double diff = fabs(val1 - val2);
  if (diff <= abs_error) return AssertionSuccess();

  const double min_abs = std::min(fabs(val1), fabs(val2));

  const double epsilon =
      nextafter(min_abs, std::numeric_limits<double>::infinity()) - min_abs;

  if (!(std::isnan)(val1) && !(std::isnan)(val2) && abs_error > 0 &&
      abs_error < epsilon) {
    return AssertionFailure()
           << "The difference between " << expr1 << " and " << expr2 << " is "
           << diff << ", where\n"
           << expr1 << " evaluates to " << val1 << ",\n"
           << expr2 << " evaluates to " << val2 << ".\nThe abs_error parameter "
           << abs_error_expr << " evaluates to " << abs_error
           << " which is smaller than the minimum distance between doubles for "
              "numbers of this magnitude which is "
           << epsilon
           << ", thus making this EXPECT_NEAR check equivalent to "
              "EXPECT_EQUAL. Consider using EXPECT_DOUBLE_EQ instead.";
  }
  return AssertionFailure()
      << "The difference between " << expr1 << " and " << expr2
      << " is " << diff << ", which exceeds " << abs_error_expr << ", where\n"
      << expr1 << " evaluates to " << val1 << ",\n"
      << expr2 << " evaluates to " << val2 << ", and\n"
      << abs_error_expr << " evaluates to " << abs_error << ".";
}

template <typename RawType>
AssertionResult FloatingPointLE(const char* expr1,
                                const char* expr2,
                                RawType val1,
                                RawType val2) {

  if (val1 < val2) {
    return AssertionSuccess();
  }

  const FloatingPoint<RawType> lhs(val1), rhs(val2);
  if (lhs.AlmostEquals(rhs)) {
    return AssertionSuccess();
  }

  ::std::stringstream val1_ss;
  val1_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
          << val1;

  ::std::stringstream val2_ss;
  val2_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
          << val2;

  return AssertionFailure()
      << "Expected: (" << expr1 << ") <= (" << expr2 << ")\n"
      << "  Actual: " << StringStreamToString(&val1_ss) << " vs "
      << StringStreamToString(&val2_ss);
}

}

AssertionResult FloatLE(const char* expr1, const char* expr2,
                        float val1, float val2) {
  return internal::FloatingPointLE<float>(expr1, expr2, val1, val2);
}

AssertionResult DoubleLE(const char* expr1, const char* expr2,
                         double val1, double val2) {
  return internal::FloatingPointLE<double>(expr1, expr2, val1, val2);
}

namespace internal {

AssertionResult CmpHelperSTREQ(const char* lhs_expression,
                               const char* rhs_expression,
                               const char* lhs,
                               const char* rhs) {
  if (String::CStringEquals(lhs, rhs)) {
    return AssertionSuccess();
  }

  return EqFailure(lhs_expression,
                   rhs_expression,
                   PrintToString(lhs),
                   PrintToString(rhs),
                   false);
}

AssertionResult CmpHelperSTRCASEEQ(const char* lhs_expression,
                                   const char* rhs_expression,
                                   const char* lhs,
                                   const char* rhs) {
  if (String::CaseInsensitiveCStringEquals(lhs, rhs)) {
    return AssertionSuccess();
  }

  return EqFailure(lhs_expression,
                   rhs_expression,
                   PrintToString(lhs),
                   PrintToString(rhs),
                   true);
}

AssertionResult CmpHelperSTRNE(const char* s1_expression,
                               const char* s2_expression,
                               const char* s1,
                               const char* s2) {
  if (!String::CStringEquals(s1, s2)) {
    return AssertionSuccess();
  } else {
    return AssertionFailure() << "Expected: (" << s1_expression << ") != ("
                              << s2_expression << "), actual: \""
                              << s1 << "\" vs \"" << s2 << "\"";
  }
}

AssertionResult CmpHelperSTRCASENE(const char* s1_expression,
                                   const char* s2_expression,
                                   const char* s1,
                                   const char* s2) {
  if (!String::CaseInsensitiveCStringEquals(s1, s2)) {
    return AssertionSuccess();
  } else {
    return AssertionFailure()
        << "Expected: (" << s1_expression << ") != ("
        << s2_expression << ") (ignoring case), actual: \""
        << s1 << "\" vs \"" << s2 << "\"";
  }
}

}

namespace {

bool IsSubstringPred(const char* needle, const char* haystack) {
  if (needle == nullptr || haystack == nullptr) return needle == haystack;

  return strstr(haystack, needle) != nullptr;
}

bool IsSubstringPred(const wchar_t* needle, const wchar_t* haystack) {
  if (needle == nullptr || haystack == nullptr) return needle == haystack;

  return wcsstr(haystack, needle) != nullptr;
}

template <typename StringType>
bool IsSubstringPred(const StringType& needle,
                     const StringType& haystack) {
  return haystack.find(needle) != StringType::npos;
}

template <typename StringType>
AssertionResult IsSubstringImpl(
    bool expected_to_be_substring,
    const char* needle_expr, const char* haystack_expr,
    const StringType& needle, const StringType& haystack) {
  if (IsSubstringPred(needle, haystack) == expected_to_be_substring)
    return AssertionSuccess();

  const bool is_wide_string = sizeof(needle[0]) > 1;
  const char* const begin_string_quote = is_wide_string ? "L\"" : "\"";
  return AssertionFailure()
      << "Value of: " << needle_expr << "\n"
      << "  Actual: " << begin_string_quote << needle << "\"\n"
      << "Expected: " << (expected_to_be_substring ? "" : "not ")
      << "a substring of " << haystack_expr << "\n"
      << "Which is: " << begin_string_quote << haystack << "\"";
}

}

AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const char* needle, const char* haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const wchar_t* needle, const wchar_t* haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const char* needle, const char* haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const wchar_t* needle, const wchar_t* haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::string& needle, const ::std::string& haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::string& needle, const ::std::string& haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}

#if GTEST_HAS_STD_WSTRING
AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::wstring& needle, const ::std::wstring& haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::wstring& needle, const ::std::wstring& haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}
#endif

namespace internal {

#if GTEST_OS_WINDOWS

namespace {

AssertionResult HRESULTFailureHelper(const char* expr,
                                     const char* expected,
                                     long hr) {
# if GTEST_OS_WINDOWS_MOBILE || GTEST_OS_WINDOWS_TV_TITLE

  const char error_text[] = "";

# else

  const DWORD kFlags = FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD kBufSize = 4096;

  char error_text[kBufSize] = { '\0' };
  DWORD message_length = ::FormatMessageA(kFlags,
                                          0,
                                          static_cast<DWORD>(hr),
                                          0,
                                          error_text,
                                          kBufSize,
                                          nullptr);

  for (; message_length && IsSpace(error_text[message_length - 1]);
          --message_length) {
    error_text[message_length - 1] = '\0';
  }

# endif

  const std::string error_hex("0x" + String::FormatHexInt(hr));
  return ::testing::AssertionFailure()
      << "Expected: " << expr << " " << expected << ".\n"
      << "  Actual: " << error_hex << " " << error_text << "\n";
}

}

AssertionResult IsHRESULTSuccess(const char* expr, long hr) {
  if (SUCCEEDED(hr)) {
    return AssertionSuccess();
  }
  return HRESULTFailureHelper(expr, "succeeds", hr);
}

AssertionResult IsHRESULTFailure(const char* expr, long hr) {
  if (FAILED(hr)) {
    return AssertionSuccess();
  }
  return HRESULTFailureHelper(expr, "fails", hr);
}

#endif

constexpr uint32_t kMaxCodePoint1 = (static_cast<uint32_t>(1) <<  7) - 1;

constexpr uint32_t kMaxCodePoint2 = (static_cast<uint32_t>(1) << (5 + 6)) - 1;

constexpr uint32_t kMaxCodePoint3 = (static_cast<uint32_t>(1) << (4 + 2*6)) - 1;

constexpr uint32_t kMaxCodePoint4 = (static_cast<uint32_t>(1) << (3 + 3*6)) - 1;

inline uint32_t ChopLowBits(uint32_t* bits, int n) {
  const uint32_t low_bits = *bits & ((static_cast<uint32_t>(1) << n) - 1);
  *bits >>= n;
  return low_bits;
}

std::string CodePointToUtf8(uint32_t code_point) {
  if (code_point > kMaxCodePoint4) {
    return "(Invalid Unicode 0x" + String::FormatHexUInt32(code_point) + ")";
  }

  char str[5];
  if (code_point <= kMaxCodePoint1) {
    str[1] = '\0';
    str[0] = static_cast<char>(code_point);
  } else if (code_point <= kMaxCodePoint2) {
    str[2] = '\0';
    str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));
    str[0] = static_cast<char>(0xC0 | code_point);
  } else if (code_point <= kMaxCodePoint3) {
    str[3] = '\0';
    str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));
    str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));
    str[0] = static_cast<char>(0xE0 | code_point);
  } else {
    str[4] = '\0';
    str[3] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));
    str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));
    str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));
    str[0] = static_cast<char>(0xF0 | code_point);
  }
  return str;
}

inline bool IsUtf16SurrogatePair(wchar_t first, wchar_t second) {
  return sizeof(wchar_t) == 2 &&
      (first & 0xFC00) == 0xD800 && (second & 0xFC00) == 0xDC00;
}

inline uint32_t CreateCodePointFromUtf16SurrogatePair(wchar_t first,
                                                      wchar_t second) {
  const auto first_u = static_cast<uint32_t>(first);
  const auto second_u = static_cast<uint32_t>(second);
  const uint32_t mask = (1 << 10) - 1;
  return (sizeof(wchar_t) == 2)
             ? (((first_u & mask) << 10) | (second_u & mask)) + 0x10000
             :

             first_u;
}

std::string WideStringToUtf8(const wchar_t* str, int num_chars) {
  if (num_chars == -1)
    num_chars = static_cast<int>(wcslen(str));

  ::std::stringstream stream;
  for (int i = 0; i < num_chars; ++i) {
    uint32_t unicode_code_point;

    if (str[i] == L'\0') {
      break;
    } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
      unicode_code_point = CreateCodePointFromUtf16SurrogatePair(str[i],
                                                                 str[i + 1]);
      i++;
    } else {
      unicode_code_point = static_cast<uint32_t>(str[i]);
    }

    stream << CodePointToUtf8(unicode_code_point);
  }
  return StringStreamToString(&stream);
}

std::string String::ShowWideCString(const wchar_t * wide_c_str) {
  if (wide_c_str == nullptr) return "(null)";

  return internal::WideStringToUtf8(wide_c_str, -1);
}

bool String::WideCStringEquals(const wchar_t * lhs, const wchar_t * rhs) {
  if (lhs == nullptr) return rhs == nullptr;

  if (rhs == nullptr) return false;

  return wcscmp(lhs, rhs) == 0;
}

AssertionResult CmpHelperSTREQ(const char* lhs_expression,
                               const char* rhs_expression,
                               const wchar_t* lhs,
                               const wchar_t* rhs) {
  if (String::WideCStringEquals(lhs, rhs)) {
    return AssertionSuccess();
  }

  return EqFailure(lhs_expression,
                   rhs_expression,
                   PrintToString(lhs),
                   PrintToString(rhs),
                   false);
}

AssertionResult CmpHelperSTRNE(const char* s1_expression,
                               const char* s2_expression,
                               const wchar_t* s1,
                               const wchar_t* s2) {
  if (!String::WideCStringEquals(s1, s2)) {
    return AssertionSuccess();
  }

  return AssertionFailure() << "Expected: (" << s1_expression << ") != ("
                            << s2_expression << "), actual: "
                            << PrintToString(s1)
                            << " vs " << PrintToString(s2);
}

bool String::CaseInsensitiveCStringEquals(const char * lhs, const char * rhs) {
  if (lhs == nullptr) return rhs == nullptr;
  if (rhs == nullptr) return false;
  return posix::StrCaseCmp(lhs, rhs) == 0;
}

bool String::CaseInsensitiveWideCStringEquals(const wchar_t* lhs,
                                              const wchar_t* rhs) {
  if (lhs == nullptr) return rhs == nullptr;

  if (rhs == nullptr) return false;

#if GTEST_OS_WINDOWS
  return _wcsicmp(lhs, rhs) == 0;
#elif GTEST_OS_LINUX && !GTEST_OS_LINUX_ANDROID
  return wcscasecmp(lhs, rhs) == 0;
#else

  wint_t left, right;
  do {
    left = towlower(static_cast<wint_t>(*lhs++));
    right = towlower(static_cast<wint_t>(*rhs++));
  } while (left && left == right);
  return left == right;
#endif
}

bool String::EndsWithCaseInsensitive(
    const std::string& str, const std::string& suffix) {
  const size_t str_len = str.length();
  const size_t suffix_len = suffix.length();
  return (str_len >= suffix_len) &&
         CaseInsensitiveCStringEquals(str.c_str() + str_len - suffix_len,
                                      suffix.c_str());
}

std::string String::FormatIntWidth2(int value) {
  return FormatIntWidthN(value, 2);
}

std::string String::FormatIntWidthN(int value, int width) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(width) << value;
  return ss.str();
}

std::string String::FormatHexUInt32(uint32_t value) {
  std::stringstream ss;
  ss << std::hex << std::uppercase << value;
  return ss.str();
}

std::string String::FormatHexInt(int value) {
  return FormatHexUInt32(static_cast<uint32_t>(value));
}

std::string String::FormatByte(unsigned char value) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
     << static_cast<unsigned int>(value);
  return ss.str();
}

std::string StringStreamToString(::std::stringstream* ss) {
  const ::std::string& str = ss->str();
  const char* const start = str.c_str();
  const char* const end = start + str.length();

  std::string result;
  result.reserve(static_cast<size_t>(2 * (end - start)));
  for (const char* ch = start; ch != end; ++ch) {
    if (*ch == '\0') {
      result += "\\0";
    } else {
      result += *ch;
    }
  }

  return result;
}

std::string AppendUserMessage(const std::string& gtest_msg,
                              const Message& user_msg) {

  const std::string user_msg_string = user_msg.GetString();
  if (user_msg_string.empty()) {
    return gtest_msg;
  }
  if (gtest_msg.empty()) {
    return user_msg_string;
  }
  return gtest_msg + "\n" + user_msg_string;
}

}

TestResult::TestResult()
    : death_test_count_(0), start_timestamp_(0), elapsed_time_(0) {}

TestResult::~TestResult() {
}

const TestPartResult& TestResult::GetTestPartResult(int i) const {
  if (i < 0 || i >= total_part_count())
    internal::posix::Abort();
  return test_part_results_.at(static_cast<size_t>(i));
}

const TestProperty& TestResult::GetTestProperty(int i) const {
  if (i < 0 || i >= test_property_count())
    internal::posix::Abort();
  return test_properties_.at(static_cast<size_t>(i));
}

void TestResult::ClearTestPartResults() {
  test_part_results_.clear();
}

void TestResult::AddTestPartResult(const TestPartResult& test_part_result) {
  test_part_results_.push_back(test_part_result);
}

void TestResult::RecordProperty(const std::string& xml_element,
                                const TestProperty& test_property) {
  if (!ValidateTestProperty(xml_element, test_property)) {
    return;
  }
  internal::MutexLock lock(&test_properties_mutex_);
  const std::vector<TestProperty>::iterator property_with_matching_key =
      std::find_if(test_properties_.begin(), test_properties_.end(),
                   internal::TestPropertyKeyIs(test_property.key()));
  if (property_with_matching_key == test_properties_.end()) {
    test_properties_.push_back(test_property);
    return;
  }
  property_with_matching_key->SetValue(test_property.value());
}

static const char* const kReservedTestSuitesAttributes[] = {
  "disabled",
  "errors",
  "failures",
  "name",
  "random_seed",
  "tests",
  "time",
  "timestamp"
};

static const char* const kReservedTestSuiteAttributes[] = {
    "disabled", "errors", "failures",  "name",
    "tests",    "time",   "timestamp", "skipped"};

static const char* const kReservedTestCaseAttributes[] = {
    "classname",   "name", "status", "time",  "type_param",
    "value_param", "file", "line"};

static const char* const kReservedOutputTestCaseAttributes[] = {
    "classname",   "name", "status", "time",   "type_param",
    "value_param", "file", "line",   "result", "timestamp"};

template <size_t kSize>
std::vector<std::string> ArrayAsVector(const char* const (&array)[kSize]) {
  return std::vector<std::string>(array, array + kSize);
}

static std::vector<std::string> GetReservedAttributesForElement(
    const std::string& xml_element) {
  if (xml_element == "testsuites") {
    return ArrayAsVector(kReservedTestSuitesAttributes);
  } else if (xml_element == "testsuite") {
    return ArrayAsVector(kReservedTestSuiteAttributes);
  } else if (xml_element == "testcase") {
    return ArrayAsVector(kReservedTestCaseAttributes);
  } else {
    GTEST_CHECK_(false) << "Unrecognized xml_element provided: " << xml_element;
  }

  return std::vector<std::string>();
}

static std::vector<std::string> GetReservedOutputAttributesForElement(
    const std::string& xml_element) {
  if (xml_element == "testsuites") {
    return ArrayAsVector(kReservedTestSuitesAttributes);
  } else if (xml_element == "testsuite") {
    return ArrayAsVector(kReservedTestSuiteAttributes);
  } else if (xml_element == "testcase") {
    return ArrayAsVector(kReservedOutputTestCaseAttributes);
  } else {
    GTEST_CHECK_(false) << "Unrecognized xml_element provided: " << xml_element;
  }

  return std::vector<std::string>();
}

static std::string FormatWordList(const std::vector<std::string>& words) {
  Message word_list;
  for (size_t i = 0; i < words.size(); ++i) {
    if (i > 0 && words.size() > 2) {
      word_list << ", ";
    }
    if (i == words.size() - 1) {
      word_list << "and ";
    }
    word_list << "'" << words[i] << "'";
  }
  return word_list.GetString();
}

static bool ValidateTestPropertyName(
    const std::string& property_name,
    const std::vector<std::string>& reserved_names) {
  if (std::find(reserved_names.begin(), reserved_names.end(), property_name) !=
          reserved_names.end()) {
    ADD_FAILURE() << "Reserved key used in RecordProperty(): " << property_name
                  << " (" << FormatWordList(reserved_names)
                  << " are reserved by " << GTEST_NAME_ << ")";
    return false;
  }
  return true;
}

bool TestResult::ValidateTestProperty(const std::string& xml_element,
                                      const TestProperty& test_property) {
  return ValidateTestPropertyName(test_property.key(),
                                  GetReservedAttributesForElement(xml_element));
}

void TestResult::Clear() {
  test_part_results_.clear();
  test_properties_.clear();
  death_test_count_ = 0;
  elapsed_time_ = 0;
}

static bool TestPartSkipped(const TestPartResult& result) {
  return result.skipped();
}

bool TestResult::Skipped() const {
  return !Failed() && CountIf(test_part_results_, TestPartSkipped) > 0;
}

bool TestResult::Failed() const {
  for (int i = 0; i < total_part_count(); ++i) {
    if (GetTestPartResult(i).failed())
      return true;
  }
  return false;
}

static bool TestPartFatallyFailed(const TestPartResult& result) {
  return result.fatally_failed();
}

bool TestResult::HasFatalFailure() const {
  return CountIf(test_part_results_, TestPartFatallyFailed) > 0;
}

static bool TestPartNonfatallyFailed(const TestPartResult& result) {
  return result.nonfatally_failed();
}

bool TestResult::HasNonfatalFailure() const {
  return CountIf(test_part_results_, TestPartNonfatallyFailed) > 0;
}

int TestResult::total_part_count() const {
  return static_cast<int>(test_part_results_.size());
}

int TestResult::test_property_count() const {
  return static_cast<int>(test_properties_.size());
}

Test::Test()
    : gtest_flag_saver_(new GTEST_FLAG_SAVER_) {
}

Test::~Test() {
}

void Test::SetUp() {
}

void Test::TearDown() {
}

void Test::RecordProperty(const std::string& key, const std::string& value) {
  UnitTest::GetInstance()->RecordProperty(key, value);
}

void Test::RecordProperty(const std::string& key, int value) {
  Message value_message;
  value_message << value;
  RecordProperty(key, value_message.GetString().c_str());
}

namespace internal {

void ReportFailureInUnknownLocation(TestPartResult::Type result_type,
                                    const std::string& message) {

  UnitTest::GetInstance()->AddTestPartResult(
      result_type,
      nullptr,
      -1,
      message,
      "");
}

}

bool Test::HasSameFixtureClass() {
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  const TestSuite* const test_suite = impl->current_test_suite();

  const TestInfo* const first_test_info = test_suite->test_info_list()[0];
  const internal::TypeId first_fixture_id = first_test_info->fixture_class_id_;
  const char* const first_test_name = first_test_info->name();

  const TestInfo* const this_test_info = impl->current_test_info();
  const internal::TypeId this_fixture_id = this_test_info->fixture_class_id_;
  const char* const this_test_name = this_test_info->name();

  if (this_fixture_id != first_fixture_id) {

    const bool first_is_TEST = first_fixture_id == internal::GetTestTypeId();

    const bool this_is_TEST = this_fixture_id == internal::GetTestTypeId();

    if (first_is_TEST || this_is_TEST) {

      const char* const TEST_name =
          first_is_TEST ? first_test_name : this_test_name;
      const char* const TEST_F_name =
          first_is_TEST ? this_test_name : first_test_name;

      ADD_FAILURE()
          << "All tests in the same test suite must use the same test fixture\n"
          << "class, so mixing TEST_F and TEST in the same test suite is\n"
          << "illegal.  In test suite " << this_test_info->test_suite_name()
          << ",\n"
          << "test " << TEST_F_name << " is defined using TEST_F but\n"
          << "test " << TEST_name << " is defined using TEST.  You probably\n"
          << "want to change the TEST to TEST_F or move it to another test\n"
          << "case.";
    } else {

      ADD_FAILURE()
          << "All tests in the same test suite must use the same test fixture\n"
          << "class.  However, in test suite "
          << this_test_info->test_suite_name() << ",\n"
          << "you defined test " << first_test_name << " and test "
          << this_test_name << "\n"
          << "using two different test fixture classes.  This can happen if\n"
          << "the two classes are from different namespaces or translation\n"
          << "units and have the same name.  You should probably rename one\n"
          << "of the classes to put the tests into different test suites.";
    }
    return false;
  }

  return true;
}

#if GTEST_HAS_SEH

static std::string* FormatSehExceptionMessage(DWORD exception_code,
                                              const char* location) {
  Message message;
  message << "SEH exception with code 0x" << std::setbase(16) <<
    exception_code << std::setbase(10) << " thrown in " << location << ".";

  return new std::string(message.GetString());
}

#endif

namespace internal {

#if GTEST_HAS_EXCEPTIONS

static std::string FormatCxxExceptionMessage(const char* description,
                                             const char* location) {
  Message message;
  if (description != nullptr) {
    message << "C++ exception with description \"" << description << "\"";
  } else {
    message << "Unknown C++ exception";
  }
  message << " thrown in " << location << ".";

  return message.GetString();
}

static std::string PrintTestPartResultToString(
    const TestPartResult& test_part_result);

GoogleTestFailureException::GoogleTestFailureException(
    const TestPartResult& failure)
    : ::std::runtime_error(PrintTestPartResultToString(failure).c_str()) {}

#endif

template <class T, typename Result>
Result HandleSehExceptionsInMethodIfSupported(
    T* object, Result (T::*method)(), const char* location) {
#if GTEST_HAS_SEH
  __try {
    return (object->*method)();
  } __except (internal::UnitTestOptions::GTestShouldProcessSEH(
      GetExceptionCode())) {

    std::string* exception_message = FormatSehExceptionMessage(
        GetExceptionCode(), location);
    internal::ReportFailureInUnknownLocation(TestPartResult::kFatalFailure,
                                             *exception_message);
    delete exception_message;
    return static_cast<Result>(0);
  }
#else
  (void)location;
  return (object->*method)();
#endif
}

template <class T, typename Result>
Result HandleExceptionsInMethodIfSupported(
    T* object, Result (T::*method)(), const char* location) {

  if (internal::GetUnitTestImpl()->catch_exceptions()) {
#if GTEST_HAS_EXCEPTIONS
    try {
      return HandleSehExceptionsInMethodIfSupported(object, method, location);
    } catch (const AssertionException&) {

    } catch (const internal::GoogleTestFailureException&) {

      throw;
    } catch (const std::exception& e) {
      internal::ReportFailureInUnknownLocation(
          TestPartResult::kFatalFailure,
          FormatCxxExceptionMessage(e.what(), location));
    } catch (...) {
      internal::ReportFailureInUnknownLocation(
          TestPartResult::kFatalFailure,
          FormatCxxExceptionMessage(nullptr, location));
    }
    return static_cast<Result>(0);
#else
    return HandleSehExceptionsInMethodIfSupported(object, method, location);
#endif
  } else {
    return (object->*method)();
  }
}

}

void Test::Run() {
  if (!HasSameFixtureClass()) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->os_stack_trace_getter()->UponLeavingGTest();
  internal::HandleExceptionsInMethodIfSupported(this, &Test::SetUp, "SetUp()");

  if (!HasFatalFailure() && !IsSkipped()) {
    impl->os_stack_trace_getter()->UponLeavingGTest();
    internal::HandleExceptionsInMethodIfSupported(
        this, &Test::TestBody, "the test body");
  }

  impl->os_stack_trace_getter()->UponLeavingGTest();
  internal::HandleExceptionsInMethodIfSupported(
      this, &Test::TearDown, "TearDown()");
}

bool Test::HasFatalFailure() {
  return internal::GetUnitTestImpl()->current_test_result()->HasFatalFailure();
}

bool Test::HasNonfatalFailure() {
  return internal::GetUnitTestImpl()->current_test_result()->
      HasNonfatalFailure();
}

bool Test::IsSkipped() {
  return internal::GetUnitTestImpl()->current_test_result()->Skipped();
}

TestInfo::TestInfo(const std::string& a_test_suite_name,
                   const std::string& a_name, const char* a_type_param,
                   const char* a_value_param,
                   internal::CodeLocation a_code_location,
                   internal::TypeId fixture_class_id,
                   internal::TestFactoryBase* factory)
    : test_suite_name_(a_test_suite_name),
      name_(a_name),
      type_param_(a_type_param ? new std::string(a_type_param) : nullptr),
      value_param_(a_value_param ? new std::string(a_value_param) : nullptr),
      location_(a_code_location),
      fixture_class_id_(fixture_class_id),
      should_run_(false),
      is_disabled_(false),
      matches_filter_(false),
      is_in_another_shard_(false),
      factory_(factory),
      result_() {}

TestInfo::~TestInfo() { delete factory_; }

namespace internal {

TestInfo* MakeAndRegisterTestInfo(
    const char* test_suite_name, const char* name, const char* type_param,
    const char* value_param, CodeLocation code_location,
    TypeId fixture_class_id, SetUpTestSuiteFunc set_up_tc,
    TearDownTestSuiteFunc tear_down_tc, TestFactoryBase* factory) {
  TestInfo* const test_info =
      new TestInfo(test_suite_name, name, type_param, value_param,
                   code_location, fixture_class_id, factory);
  GetUnitTestImpl()->AddTestInfo(set_up_tc, tear_down_tc, test_info);
  return test_info;
}

void ReportInvalidTestSuiteType(const char* test_suite_name,
                                CodeLocation code_location) {
  Message errors;
  errors
      << "Attempted redefinition of test suite " << test_suite_name << ".\n"
      << "All tests in the same test suite must use the same test fixture\n"
      << "class.  However, in test suite " << test_suite_name << ", you tried\n"
      << "to define a test using a fixture class different from the one\n"
      << "used earlier. This can happen if the two fixture classes are\n"
      << "from different namespaces and have the same name. You should\n"
      << "probably rename one of the classes to put the tests into different\n"
      << "test suites.";

  GTEST_LOG_(ERROR) << FormatFileLocation(code_location.file.c_str(),
                                          code_location.line)
                    << " " << errors.GetString();
}
}

namespace {

class TestNameIs {
 public:

  explicit TestNameIs(const char* name)
      : name_(name) {}

  bool operator()(const TestInfo * test_info) const {
    return test_info && test_info->name() == name_;
  }

 private:
  std::string name_;
};

}

namespace internal {

void UnitTestImpl::RegisterParameterizedTests() {
  if (!parameterized_tests_registered_) {
    parameterized_test_registry_.RegisterTests();
    type_parameterized_test_registry_.CheckForInstantiations();
    parameterized_tests_registered_ = true;
  }
}

}

void TestInfo::Run() {
  if (!should_run_) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_info(this);

  TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

  repeater->OnTestStart(*this);

  result_.set_start_timestamp(internal::GetTimeInMillis());
  internal::Timer timer;

  impl->os_stack_trace_getter()->UponLeavingGTest();

  Test* const test = internal::HandleExceptionsInMethodIfSupported(
      factory_, &internal::TestFactoryBase::CreateTest,
      "the test fixture's constructor");

  if (!Test::HasFatalFailure() && !Test::IsSkipped()) {

    test->Run();
  }

  if (test != nullptr) {

    impl->os_stack_trace_getter()->UponLeavingGTest();
    internal::HandleExceptionsInMethodIfSupported(
        test, &Test::DeleteSelf_, "the test fixture's destructor");
  }

  result_.set_elapsed_time(timer.Elapsed());

  repeater->OnTestEnd(*this);

  impl->set_current_test_info(nullptr);
}

void TestInfo::Skip() {
  if (!should_run_) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_info(this);

  TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

  repeater->OnTestStart(*this);

  const TestPartResult test_part_result =
      TestPartResult(TestPartResult::kSkip, this->file(), this->line(), "");
  impl->GetTestPartResultReporterForCurrentThread()->ReportTestPartResult(
      test_part_result);

  repeater->OnTestEnd(*this);
  impl->set_current_test_info(nullptr);
}

int TestSuite::successful_test_count() const {
  return CountIf(test_info_list_, TestPassed);
}

int TestSuite::skipped_test_count() const {
  return CountIf(test_info_list_, TestSkipped);
}

int TestSuite::failed_test_count() const {
  return CountIf(test_info_list_, TestFailed);
}

int TestSuite::reportable_disabled_test_count() const {
  return CountIf(test_info_list_, TestReportableDisabled);
}

int TestSuite::disabled_test_count() const {
  return CountIf(test_info_list_, TestDisabled);
}

int TestSuite::reportable_test_count() const {
  return CountIf(test_info_list_, TestReportable);
}

int TestSuite::test_to_run_count() const {
  return CountIf(test_info_list_, ShouldRunTest);
}

int TestSuite::total_test_count() const {
  return static_cast<int>(test_info_list_.size());
}

TestSuite::TestSuite(const char* a_name, const char* a_type_param,
                     internal::SetUpTestSuiteFunc set_up_tc,
                     internal::TearDownTestSuiteFunc tear_down_tc)
    : name_(a_name),
      type_param_(a_type_param ? new std::string(a_type_param) : nullptr),
      set_up_tc_(set_up_tc),
      tear_down_tc_(tear_down_tc),
      should_run_(false),
      start_timestamp_(0),
      elapsed_time_(0) {}

TestSuite::~TestSuite() {

  ForEach(test_info_list_, internal::Delete<TestInfo>);
}

const TestInfo* TestSuite::GetTestInfo(int i) const {
  const int index = GetElementOr(test_indices_, i, -1);
  return index < 0 ? nullptr : test_info_list_[static_cast<size_t>(index)];
}

TestInfo* TestSuite::GetMutableTestInfo(int i) {
  const int index = GetElementOr(test_indices_, i, -1);
  return index < 0 ? nullptr : test_info_list_[static_cast<size_t>(index)];
}

void TestSuite::AddTestInfo(TestInfo* test_info) {
  test_info_list_.push_back(test_info);
  test_indices_.push_back(static_cast<int>(test_indices_.size()));
}

void TestSuite::Run() {
  if (!should_run_) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_suite(this);

  TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

  repeater->OnTestSuiteStart(*this);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  repeater->OnTestCaseStart(*this);
#endif

  impl->os_stack_trace_getter()->UponLeavingGTest();
  internal::HandleExceptionsInMethodIfSupported(
      this, &TestSuite::RunSetUpTestSuite, "SetUpTestSuite()");

  start_timestamp_ = internal::GetTimeInMillis();
  internal::Timer timer;
  for (int i = 0; i < total_test_count(); i++) {
    GetMutableTestInfo(i)->Run();
    if (GTEST_FLAG(fail_fast) && GetMutableTestInfo(i)->result()->Failed()) {
      for (int j = i + 1; j < total_test_count(); j++) {
        GetMutableTestInfo(j)->Skip();
      }
      break;
    }
  }
  elapsed_time_ = timer.Elapsed();

  impl->os_stack_trace_getter()->UponLeavingGTest();
  internal::HandleExceptionsInMethodIfSupported(
      this, &TestSuite::RunTearDownTestSuite, "TearDownTestSuite()");

  repeater->OnTestSuiteEnd(*this);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  repeater->OnTestCaseEnd(*this);
#endif

  impl->set_current_test_suite(nullptr);
}

void TestSuite::Skip() {
  if (!should_run_) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_suite(this);

  TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

  repeater->OnTestSuiteStart(*this);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  repeater->OnTestCaseStart(*this);
#endif

  for (int i = 0; i < total_test_count(); i++) {
    GetMutableTestInfo(i)->Skip();
  }

  repeater->OnTestSuiteEnd(*this);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  repeater->OnTestCaseEnd(*this);
#endif

  impl->set_current_test_suite(nullptr);
}

void TestSuite::ClearResult() {
  ad_hoc_test_result_.Clear();
  ForEach(test_info_list_, TestInfo::ClearTestResult);
}

void TestSuite::ShuffleTests(internal::Random* random) {
  Shuffle(random, &test_indices_);
}

void TestSuite::UnshuffleTests() {
  for (size_t i = 0; i < test_indices_.size(); i++) {
    test_indices_[i] = static_cast<int>(i);
  }
}

static std::string FormatCountableNoun(int count,
                                       const char * singular_form,
                                       const char * plural_form) {
  return internal::StreamableToString(count) + " " +
      (count == 1 ? singular_form : plural_form);
}

static std::string FormatTestCount(int test_count) {
  return FormatCountableNoun(test_count, "test", "tests");
}

static std::string FormatTestSuiteCount(int test_suite_count) {
  return FormatCountableNoun(test_suite_count, "test suite", "test suites");
}

static const char * TestPartResultTypeToString(TestPartResult::Type type) {
  switch (type) {
    case TestPartResult::kSkip:
      return "Skipped\n";
    case TestPartResult::kSuccess:
      return "Success";

    case TestPartResult::kNonFatalFailure:
    case TestPartResult::kFatalFailure:
#ifdef _MSC_VER
      return "error: ";
#else
      return "Failure\n";
#endif
    default:
      return "Unknown result type";
  }
}

namespace internal {
namespace {
enum class GTestColor { kDefault, kRed, kGreen, kYellow };
}

static std::string PrintTestPartResultToString(
    const TestPartResult& test_part_result) {
  return (Message()
          << internal::FormatFileLocation(test_part_result.file_name(),
                                          test_part_result.line_number())
          << " " << TestPartResultTypeToString(test_part_result.type())
          << test_part_result.message()).GetString();
}

static void PrintTestPartResult(const TestPartResult& test_part_result) {
  const std::string& result =
      PrintTestPartResultToString(test_part_result);
  printf("%s\n", result.c_str());
  fflush(stdout);

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE

  ::OutputDebugStringA(result.c_str());
  ::OutputDebugStringA("\n");
#endif
}

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE && \
    !GTEST_OS_WINDOWS_PHONE && !GTEST_OS_WINDOWS_RT && !GTEST_OS_WINDOWS_MINGW

static WORD GetColorAttribute(GTestColor color) {
  switch (color) {
    case GTestColor::kRed:
      return FOREGROUND_RED;
    case GTestColor::kGreen:
      return FOREGROUND_GREEN;
    case GTestColor::kYellow:
      return FOREGROUND_RED | FOREGROUND_GREEN;
    default:           return 0;
  }
}

static int GetBitOffset(WORD color_mask) {
  if (color_mask == 0) return 0;

  int bitOffset = 0;
  while ((color_mask & 1) == 0) {
    color_mask >>= 1;
    ++bitOffset;
  }
  return bitOffset;
}

static WORD GetNewColor(GTestColor color, WORD old_color_attrs) {

  static const WORD background_mask = BACKGROUND_BLUE | BACKGROUND_GREEN |
                                      BACKGROUND_RED | BACKGROUND_INTENSITY;
  static const WORD foreground_mask = FOREGROUND_BLUE | FOREGROUND_GREEN |
                                      FOREGROUND_RED | FOREGROUND_INTENSITY;
  const WORD existing_bg = old_color_attrs & background_mask;

  WORD new_color =
      GetColorAttribute(color) | existing_bg | FOREGROUND_INTENSITY;
  static const int bg_bitOffset = GetBitOffset(background_mask);
  static const int fg_bitOffset = GetBitOffset(foreground_mask);

  if (((new_color & background_mask) >> bg_bitOffset) ==
      ((new_color & foreground_mask) >> fg_bitOffset)) {
    new_color ^= FOREGROUND_INTENSITY;
  }
  return new_color;
}

#else

static const char* GetAnsiColorCode(GTestColor color) {
  switch (color) {
    case GTestColor::kRed:
      return "1";
    case GTestColor::kGreen:
      return "2";
    case GTestColor::kYellow:
      return "3";
    default:
      return nullptr;
  }
}

#endif

bool ShouldUseColor(bool stdout_is_tty) {
  const char* const gtest_color = GTEST_FLAG(color).c_str();

  if (String::CaseInsensitiveCStringEquals(gtest_color, "auto")) {
#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MINGW

    return stdout_is_tty;
#else

    const char* const term = posix::GetEnv("TERM");
    const bool term_supports_color =
        String::CStringEquals(term, "xterm") ||
        String::CStringEquals(term, "xterm-color") ||
        String::CStringEquals(term, "xterm-256color") ||
        String::CStringEquals(term, "screen") ||
        String::CStringEquals(term, "screen-256color") ||
        String::CStringEquals(term, "tmux") ||
        String::CStringEquals(term, "tmux-256color") ||
        String::CStringEquals(term, "rxvt-unicode") ||
        String::CStringEquals(term, "rxvt-unicode-256color") ||
        String::CStringEquals(term, "linux") ||
        String::CStringEquals(term, "cygwin");
    return stdout_is_tty && term_supports_color;
#endif
  }

  return String::CaseInsensitiveCStringEquals(gtest_color, "yes") ||
      String::CaseInsensitiveCStringEquals(gtest_color, "true") ||
      String::CaseInsensitiveCStringEquals(gtest_color, "t") ||
      String::CStringEquals(gtest_color, "1");

}

GTEST_ATTRIBUTE_PRINTF_(2, 3)
static void ColoredPrintf(GTestColor color, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

#if GTEST_OS_WINDOWS_MOBILE || GTEST_OS_ZOS || GTEST_OS_IOS || \
    GTEST_OS_WINDOWS_PHONE || GTEST_OS_WINDOWS_RT || defined(ESP_PLATFORM)
  const bool use_color = AlwaysFalse();
#else
  static const bool in_color_mode =
      ShouldUseColor(posix::IsATTY(posix::FileNo(stdout)) != 0);
  const bool use_color = in_color_mode && (color != GTestColor::kDefault);
#endif

  if (!use_color) {
    vprintf(fmt, args);
    va_end(args);
    return;
  }

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE && \
    !GTEST_OS_WINDOWS_PHONE && !GTEST_OS_WINDOWS_RT && !GTEST_OS_WINDOWS_MINGW
  const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
  GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
  const WORD old_color_attrs = buffer_info.wAttributes;
  const WORD new_color = GetNewColor(color, old_color_attrs);

  fflush(stdout);
  SetConsoleTextAttribute(stdout_handle, new_color);

  vprintf(fmt, args);

  fflush(stdout);

  SetConsoleTextAttribute(stdout_handle, old_color_attrs);
#else
  printf("\033[0;3%sm", GetAnsiColorCode(color));
  vprintf(fmt, args);
  printf("\033[m");
#endif
  va_end(args);
}

static const char kTypeParamLabel[] = "TypeParam";
static const char kValueParamLabel[] = "GetParam()";

static void PrintFullTestCommentIfPresent(const TestInfo& test_info) {
  const char* const type_param = test_info.type_param();
  const char* const value_param = test_info.value_param();

  if (type_param != nullptr || value_param != nullptr) {
    printf(", where ");
    if (type_param != nullptr) {
      printf("%s = %s", kTypeParamLabel, type_param);
      if (value_param != nullptr) printf(" and ");
    }
    if (value_param != nullptr) {
      printf("%s = %s", kValueParamLabel, value_param);
    }
  }
}

class PrettyUnitTestResultPrinter : public TestEventListener {
 public:
  PrettyUnitTestResultPrinter() {}
  static void PrintTestName(const char* test_suite, const char* test) {
    printf("%s.%s", test_suite, test);
  }

  void OnTestProgramStart(const UnitTest&  ) override {}
  void OnTestIterationStart(const UnitTest& unit_test, int iteration) override;
  void OnEnvironmentsSetUpStart(const UnitTest& unit_test) override;
  void OnEnvironmentsSetUpEnd(const UnitTest&  ) override {}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseStart(const TestCase& test_case) override;
#else
  void OnTestSuiteStart(const TestSuite& test_suite) override;
#endif

  void OnTestStart(const TestInfo& test_info) override;

  void OnTestPartResult(const TestPartResult& result) override;
  void OnTestEnd(const TestInfo& test_info) override;
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseEnd(const TestCase& test_case) override;
#else
  void OnTestSuiteEnd(const TestSuite& test_suite) override;
#endif

  void OnEnvironmentsTearDownStart(const UnitTest& unit_test) override;
  void OnEnvironmentsTearDownEnd(const UnitTest&  ) override {}
  void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
  void OnTestProgramEnd(const UnitTest&  ) override {}

 private:
  static void PrintFailedTests(const UnitTest& unit_test);
  static void PrintFailedTestSuites(const UnitTest& unit_test);
  static void PrintSkippedTests(const UnitTest& unit_test);
};

void PrettyUnitTestResultPrinter::OnTestIterationStart(
    const UnitTest& unit_test, int iteration) {
  if (GTEST_FLAG(repeat) != 1)
    printf("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);

  const char* const filter = GTEST_FLAG(filter).c_str();

  if (!String::CStringEquals(filter, kUniversalFilter)) {
    ColoredPrintf(GTestColor::kYellow, "Note: %s filter = %s\n", GTEST_NAME_,
                  filter);
  }

  if (internal::ShouldShard(kTestTotalShards, kTestShardIndex, false)) {
    const int32_t shard_index = Int32FromEnvOrDie(kTestShardIndex, -1);
    ColoredPrintf(GTestColor::kYellow, "Note: This is test shard %d of %s.\n",
                  static_cast<int>(shard_index) + 1,
                  internal::posix::GetEnv(kTestTotalShards));
  }

  if (GTEST_FLAG(shuffle)) {
    ColoredPrintf(GTestColor::kYellow,
                  "Note: Randomizing tests' orders with a seed of %d .\n",
                  unit_test.random_seed());
  }

  ColoredPrintf(GTestColor::kGreen, "[==========] ");
  printf("Running %s from %s.\n",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestSuiteCount(unit_test.test_suite_to_run_count()).c_str());
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnEnvironmentsSetUpStart(
    const UnitTest&  ) {
  ColoredPrintf(GTestColor::kGreen, "[----------] ");
  printf("Global test environment set-up.\n");
  fflush(stdout);
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void PrettyUnitTestResultPrinter::OnTestCaseStart(const TestCase& test_case) {
  const std::string counts =
      FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
  ColoredPrintf(GTestColor::kGreen, "[----------] ");
  printf("%s from %s", counts.c_str(), test_case.name());
  if (test_case.type_param() == nullptr) {
    printf("\n");
  } else {
    printf(", where %s = %s\n", kTypeParamLabel, test_case.type_param());
  }
  fflush(stdout);
}
#else
void PrettyUnitTestResultPrinter::OnTestSuiteStart(
    const TestSuite& test_suite) {
  const std::string counts =
      FormatCountableNoun(test_suite.test_to_run_count(), "test", "tests");
  ColoredPrintf(GTestColor::kGreen, "[----------] ");
  printf("%s from %s", counts.c_str(), test_suite.name());
  if (test_suite.type_param() == nullptr) {
    printf("\n");
  } else {
    printf(", where %s = %s\n", kTypeParamLabel, test_suite.type_param());
  }
  fflush(stdout);
}
#endif

void PrettyUnitTestResultPrinter::OnTestStart(const TestInfo& test_info) {
  ColoredPrintf(GTestColor::kGreen, "[ RUN      ] ");
  PrintTestName(test_info.test_suite_name(), test_info.name());
  printf("\n");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestPartResult(
    const TestPartResult& result) {
  switch (result.type()) {

    case TestPartResult::kSuccess:
      return;
    default:

      PrintTestPartResult(result);
      fflush(stdout);
  }
}

void PrettyUnitTestResultPrinter::OnTestEnd(const TestInfo& test_info) {
  if (test_info.result()->Passed()) {
    ColoredPrintf(GTestColor::kGreen, "[       OK ] ");
  } else if (test_info.result()->Skipped()) {
    ColoredPrintf(GTestColor::kGreen, "[  SKIPPED ] ");
  } else {
    ColoredPrintf(GTestColor::kRed, "[  FAILED  ] ");
  }
  PrintTestName(test_info.test_suite_name(), test_info.name());
  if (test_info.result()->Failed())
    PrintFullTestCommentIfPresent(test_info);

  if (GTEST_FLAG(print_time)) {
    printf(" (%s ms)\n", internal::StreamableToString(
           test_info.result()->elapsed_time()).c_str());
  } else {
    printf("\n");
  }
  fflush(stdout);
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
void PrettyUnitTestResultPrinter::OnTestCaseEnd(const TestCase& test_case) {
  if (!GTEST_FLAG(print_time)) return;

  const std::string counts =
      FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
  ColoredPrintf(GTestColor::kGreen, "[----------] ");
  printf("%s from %s (%s ms total)\n\n", counts.c_str(), test_case.name(),
         internal::StreamableToString(test_case.elapsed_time()).c_str());
  fflush(stdout);
}
#else
void PrettyUnitTestResultPrinter::OnTestSuiteEnd(const TestSuite& test_suite) {
  if (!GTEST_FLAG(print_time)) return;

  const std::string counts =
      FormatCountableNoun(test_suite.test_to_run_count(), "test", "tests");
  ColoredPrintf(GTestColor::kGreen, "[----------] ");
  printf("%s from %s (%s ms total)\n\n", counts.c_str(), test_suite.name(),
         internal::StreamableToString(test_suite.elapsed_time()).c_str());
  fflush(stdout);
}
#endif

void PrettyUnitTestResultPrinter::OnEnvironmentsTearDownStart(
    const UnitTest&  ) {
  ColoredPrintf(GTestColor::kGreen, "[----------] ");
  printf("Global test environment tear-down\n");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::PrintFailedTests(const UnitTest& unit_test) {
  const int failed_test_count = unit_test.failed_test_count();
  ColoredPrintf(GTestColor::kRed, "[  FAILED  ] ");
  printf("%s, listed below:\n", FormatTestCount(failed_test_count).c_str());

  for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
    const TestSuite& test_suite = *unit_test.GetTestSuite(i);
    if (!test_suite.should_run() || (test_suite.failed_test_count() == 0)) {
      continue;
    }
    for (int j = 0; j < test_suite.total_test_count(); ++j) {
      const TestInfo& test_info = *test_suite.GetTestInfo(j);
      if (!test_info.should_run() || !test_info.result()->Failed()) {
        continue;
      }
      ColoredPrintf(GTestColor::kRed, "[  FAILED  ] ");
      printf("%s.%s", test_suite.name(), test_info.name());
      PrintFullTestCommentIfPresent(test_info);
      printf("\n");
    }
  }
  printf("\n%2d FAILED %s\n", failed_test_count,
         failed_test_count == 1 ? "TEST" : "TESTS");
}

void PrettyUnitTestResultPrinter::PrintFailedTestSuites(
    const UnitTest& unit_test) {
  int suite_failure_count = 0;
  for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
    const TestSuite& test_suite = *unit_test.GetTestSuite(i);
    if (!test_suite.should_run()) {
      continue;
    }
    if (test_suite.ad_hoc_test_result().Failed()) {
      ColoredPrintf(GTestColor::kRed, "[  FAILED  ] ");
      printf("%s: SetUpTestSuite or TearDownTestSuite\n", test_suite.name());
      ++suite_failure_count;
    }
  }
  if (suite_failure_count > 0) {
    printf("\n%2d FAILED TEST %s\n", suite_failure_count,
           suite_failure_count == 1 ? "SUITE" : "SUITES");
  }
}

void PrettyUnitTestResultPrinter::PrintSkippedTests(const UnitTest& unit_test) {
  const int skipped_test_count = unit_test.skipped_test_count();
  if (skipped_test_count == 0) {
    return;
  }

  for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
    const TestSuite& test_suite = *unit_test.GetTestSuite(i);
    if (!test_suite.should_run() || (test_suite.skipped_test_count() == 0)) {
      continue;
    }
    for (int j = 0; j < test_suite.total_test_count(); ++j) {
      const TestInfo& test_info = *test_suite.GetTestInfo(j);
      if (!test_info.should_run() || !test_info.result()->Skipped()) {
        continue;
      }
      ColoredPrintf(GTestColor::kGreen, "[  SKIPPED ] ");
      printf("%s.%s", test_suite.name(), test_info.name());
      printf("\n");
    }
  }
}

void PrettyUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                     int  ) {
  ColoredPrintf(GTestColor::kGreen, "[==========] ");
  printf("%s from %s ran.",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestSuiteCount(unit_test.test_suite_to_run_count()).c_str());
  if (GTEST_FLAG(print_time)) {
    printf(" (%s ms total)",
           internal::StreamableToString(unit_test.elapsed_time()).c_str());
  }
  printf("\n");
  ColoredPrintf(GTestColor::kGreen, "[  PASSED  ] ");
  printf("%s.\n", FormatTestCount(unit_test.successful_test_count()).c_str());

  const int skipped_test_count = unit_test.skipped_test_count();
  if (skipped_test_count > 0) {
    ColoredPrintf(GTestColor::kGreen, "[  SKIPPED ] ");
    printf("%s, listed below:\n", FormatTestCount(skipped_test_count).c_str());
    PrintSkippedTests(unit_test);
  }

  if (!unit_test.Passed()) {
    PrintFailedTests(unit_test);
    PrintFailedTestSuites(unit_test);
  }

  int num_disabled = unit_test.reportable_disabled_test_count();
  if (num_disabled && !GTEST_FLAG(also_run_disabled_tests)) {
    if (unit_test.Passed()) {
      printf("\n");
    }
    ColoredPrintf(GTestColor::kYellow, "  YOU HAVE %d DISABLED %s\n\n",
                  num_disabled, num_disabled == 1 ? "TEST" : "TESTS");
  }

  fflush(stdout);
}

class BriefUnitTestResultPrinter : public TestEventListener {
 public:
  BriefUnitTestResultPrinter() {}
  static void PrintTestName(const char* test_suite, const char* test) {
    printf("%s.%s", test_suite, test);
  }

  void OnTestProgramStart(const UnitTest&  ) override {}
  void OnTestIterationStart(const UnitTest&  ,
                            int  ) override {}
  void OnEnvironmentsSetUpStart(const UnitTest&  ) override {}
  void OnEnvironmentsSetUpEnd(const UnitTest&  ) override {}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseStart(const TestCase&  ) override {}
#else
  void OnTestSuiteStart(const TestSuite&  ) override {}
#endif

  void OnTestStart(const TestInfo&  ) override {}

  void OnTestPartResult(const TestPartResult& result) override;
  void OnTestEnd(const TestInfo& test_info) override;
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseEnd(const TestCase&  ) override {}
#else
  void OnTestSuiteEnd(const TestSuite&  ) override {}
#endif

  void OnEnvironmentsTearDownStart(const UnitTest&  ) override {}
  void OnEnvironmentsTearDownEnd(const UnitTest&  ) override {}
  void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
  void OnTestProgramEnd(const UnitTest&  ) override {}
};

void BriefUnitTestResultPrinter::OnTestPartResult(
    const TestPartResult& result) {
  switch (result.type()) {

    case TestPartResult::kSuccess:
      return;
    default:

      PrintTestPartResult(result);
      fflush(stdout);
  }
}

void BriefUnitTestResultPrinter::OnTestEnd(const TestInfo& test_info) {
  if (test_info.result()->Failed()) {
    ColoredPrintf(GTestColor::kRed, "[  FAILED  ] ");
    PrintTestName(test_info.test_suite_name(), test_info.name());
    PrintFullTestCommentIfPresent(test_info);

    if (GTEST_FLAG(print_time)) {
      printf(" (%s ms)\n",
             internal::StreamableToString(test_info.result()->elapsed_time())
                 .c_str());
    } else {
      printf("\n");
    }
    fflush(stdout);
  }
}

void BriefUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                    int  ) {
  ColoredPrintf(GTestColor::kGreen, "[==========] ");
  printf("%s from %s ran.",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestSuiteCount(unit_test.test_suite_to_run_count()).c_str());
  if (GTEST_FLAG(print_time)) {
    printf(" (%s ms total)",
           internal::StreamableToString(unit_test.elapsed_time()).c_str());
  }
  printf("\n");
  ColoredPrintf(GTestColor::kGreen, "[  PASSED  ] ");
  printf("%s.\n", FormatTestCount(unit_test.successful_test_count()).c_str());

  const int skipped_test_count = unit_test.skipped_test_count();
  if (skipped_test_count > 0) {
    ColoredPrintf(GTestColor::kGreen, "[  SKIPPED ] ");
    printf("%s.\n", FormatTestCount(skipped_test_count).c_str());
  }

  int num_disabled = unit_test.reportable_disabled_test_count();
  if (num_disabled && !GTEST_FLAG(also_run_disabled_tests)) {
    if (unit_test.Passed()) {
      printf("\n");
    }
    ColoredPrintf(GTestColor::kYellow, "  YOU HAVE %d DISABLED %s\n\n",
                  num_disabled, num_disabled == 1 ? "TEST" : "TESTS");
  }

  fflush(stdout);
}

class TestEventRepeater : public TestEventListener {
 public:
  TestEventRepeater() : forwarding_enabled_(true) {}
  ~TestEventRepeater() override;
  void Append(TestEventListener *listener);
  TestEventListener* Release(TestEventListener* listener);

  bool forwarding_enabled() const { return forwarding_enabled_; }
  void set_forwarding_enabled(bool enable) { forwarding_enabled_ = enable; }

  void OnTestProgramStart(const UnitTest& unit_test) override;
  void OnTestIterationStart(const UnitTest& unit_test, int iteration) override;
  void OnEnvironmentsSetUpStart(const UnitTest& unit_test) override;
  void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) override;

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseStart(const TestSuite& parameter) override;
#endif
  void OnTestSuiteStart(const TestSuite& parameter) override;
  void OnTestStart(const TestInfo& test_info) override;
  void OnTestPartResult(const TestPartResult& result) override;
  void OnTestEnd(const TestInfo& test_info) override;

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseEnd(const TestCase& parameter) override;
#endif
  void OnTestSuiteEnd(const TestSuite& parameter) override;
  void OnEnvironmentsTearDownStart(const UnitTest& unit_test) override;
  void OnEnvironmentsTearDownEnd(const UnitTest& unit_test) override;
  void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
  void OnTestProgramEnd(const UnitTest& unit_test) override;

 private:

  bool forwarding_enabled_;

  std::vector<TestEventListener*> listeners_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestEventRepeater);
};

TestEventRepeater::~TestEventRepeater() {
  ForEach(listeners_, Delete<TestEventListener>);
}

void TestEventRepeater::Append(TestEventListener *listener) {
  listeners_.push_back(listener);
}

TestEventListener* TestEventRepeater::Release(TestEventListener *listener) {
  for (size_t i = 0; i < listeners_.size(); ++i) {
    if (listeners_[i] == listener) {
      listeners_.erase(listeners_.begin() + static_cast<int>(i));
      return listener;
    }
  }

  return nullptr;
}

#define GTEST_REPEATER_METHOD_(Name, Type) \
void TestEventRepeater::Name(const Type& parameter) { \
  if (forwarding_enabled_) { \
    for (size_t i = 0; i < listeners_.size(); i++) { \
      listeners_[i]->Name(parameter); \
    } \
  } \
}

#define GTEST_REVERSE_REPEATER_METHOD_(Name, Type)      \
  void TestEventRepeater::Name(const Type& parameter) { \
    if (forwarding_enabled_) {                          \
      for (size_t i = listeners_.size(); i != 0; i--) { \
        listeners_[i - 1]->Name(parameter);             \
      }                                                 \
    }                                                   \
  }

GTEST_REPEATER_METHOD_(OnTestProgramStart, UnitTest)
GTEST_REPEATER_METHOD_(OnEnvironmentsSetUpStart, UnitTest)

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
GTEST_REPEATER_METHOD_(OnTestCaseStart, TestSuite)
#endif
GTEST_REPEATER_METHOD_(OnTestSuiteStart, TestSuite)
GTEST_REPEATER_METHOD_(OnTestStart, TestInfo)
GTEST_REPEATER_METHOD_(OnTestPartResult, TestPartResult)
GTEST_REPEATER_METHOD_(OnEnvironmentsTearDownStart, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsSetUpEnd, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsTearDownEnd, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnTestEnd, TestInfo)

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
GTEST_REVERSE_REPEATER_METHOD_(OnTestCaseEnd, TestSuite)
#endif
GTEST_REVERSE_REPEATER_METHOD_(OnTestSuiteEnd, TestSuite)
GTEST_REVERSE_REPEATER_METHOD_(OnTestProgramEnd, UnitTest)

#undef GTEST_REPEATER_METHOD_
#undef GTEST_REVERSE_REPEATER_METHOD_

void TestEventRepeater::OnTestIterationStart(const UnitTest& unit_test,
                                             int iteration) {
  if (forwarding_enabled_) {
    for (size_t i = 0; i < listeners_.size(); i++) {
      listeners_[i]->OnTestIterationStart(unit_test, iteration);
    }
  }
}

void TestEventRepeater::OnTestIterationEnd(const UnitTest& unit_test,
                                           int iteration) {
  if (forwarding_enabled_) {
    for (size_t i = listeners_.size(); i > 0; i--) {
      listeners_[i - 1]->OnTestIterationEnd(unit_test, iteration);
    }
  }
}

class XmlUnitTestResultPrinter : public EmptyTestEventListener {
 public:
  explicit XmlUnitTestResultPrinter(const char* output_file);

  void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
  void ListTestsMatchingFilter(const std::vector<TestSuite*>& test_suites);

  static void PrintXmlTestsList(std::ostream* stream,
                                const std::vector<TestSuite*>& test_suites);

 private:

  static bool IsNormalizableWhitespace(char c) {
    return c == 0x9 || c == 0xA || c == 0xD;
  }

  static bool IsValidXmlCharacter(char c) {
    return IsNormalizableWhitespace(c) || c >= 0x20;
  }

  static std::string EscapeXml(const std::string& str, bool is_attribute);

  static std::string RemoveInvalidXmlCharacters(const std::string& str);

  static std::string EscapeXmlAttribute(const std::string& str) {
    return EscapeXml(str, true);
  }

  static std::string EscapeXmlText(const char* str) {
    return EscapeXml(str, false);
  }

  static void OutputXmlAttribute(std::ostream* stream,
                                 const std::string& element_name,
                                 const std::string& name,
                                 const std::string& value);

  static void OutputXmlCDataSection(::std::ostream* stream, const char* data);

  static void OutputXmlTestSuiteForTestResult(::std::ostream* stream,
                                              const TestResult& result);

  static void OutputXmlTestResult(::std::ostream* stream,
                                  const TestResult& result);

  static void OutputXmlTestInfo(::std::ostream* stream,
                                const char* test_suite_name,
                                const TestInfo& test_info);

  static void PrintXmlTestSuite(::std::ostream* stream,
                                const TestSuite& test_suite);

  static void PrintXmlUnitTest(::std::ostream* stream,
                               const UnitTest& unit_test);

  static std::string TestPropertiesAsXmlAttributes(const TestResult& result);

  static void OutputXmlTestProperties(std::ostream* stream,
                                      const TestResult& result);

  const std::string output_file_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(XmlUnitTestResultPrinter);
};

XmlUnitTestResultPrinter::XmlUnitTestResultPrinter(const char* output_file)
    : output_file_(output_file) {
  if (output_file_.empty()) {
    GTEST_LOG_(FATAL) << "XML output file may not be null";
  }
}

void XmlUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                  int  ) {
  FILE* xmlout = OpenFileForWriting(output_file_);
  std::stringstream stream;
  PrintXmlUnitTest(&stream, unit_test);
  fprintf(xmlout, "%s", StringStreamToString(&stream).c_str());
  fclose(xmlout);
}

void XmlUnitTestResultPrinter::ListTestsMatchingFilter(
    const std::vector<TestSuite*>& test_suites) {
  FILE* xmlout = OpenFileForWriting(output_file_);
  std::stringstream stream;
  PrintXmlTestsList(&stream, test_suites);
  fprintf(xmlout, "%s", StringStreamToString(&stream).c_str());
  fclose(xmlout);
}

std::string XmlUnitTestResultPrinter::EscapeXml(
    const std::string& str, bool is_attribute) {
  Message m;

  for (size_t i = 0; i < str.size(); ++i) {
    const char ch = str[i];
    switch (ch) {
      case '<':
        m << "&lt;";
        break;
      case '>':
        m << "&gt;";
        break;
      case '&':
        m << "&amp;";
        break;
      case '\'':
        if (is_attribute)
          m << "&apos;";
        else
          m << '\'';
        break;
      case '"':
        if (is_attribute)
          m << "&quot;";
        else
          m << '"';
        break;
      default:
        if (IsValidXmlCharacter(ch)) {
          if (is_attribute && IsNormalizableWhitespace(ch))
            m << "&#x" << String::FormatByte(static_cast<unsigned char>(ch))
              << ";";
          else
            m << ch;
        }
        break;
    }
  }

  return m.GetString();
}

std::string XmlUnitTestResultPrinter::RemoveInvalidXmlCharacters(
    const std::string& str) {
  std::string output;
  output.reserve(str.size());
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    if (IsValidXmlCharacter(*it))
      output.push_back(*it);

  return output;
}

std::string FormatTimeInMillisAsSeconds(TimeInMillis ms) {
  ::std::stringstream ss;
  ss << (static_cast<double>(ms) * 1e-3);
  return ss.str();
}

static bool PortableLocaltime(time_t seconds, struct tm* out) {
#if defined(_MSC_VER)
  return localtime_s(out, &seconds) == 0;
#elif defined(__MINGW32__) || defined(__MINGW64__)

  struct tm* tm_ptr = localtime(&seconds);
  if (tm_ptr == nullptr) return false;
  *out = *tm_ptr;
  return true;
#elif defined(__STDC_LIB_EXT1__)

  return localtime_s(&seconds, out) != nullptr;
#else
  return localtime_r(&seconds, out) != nullptr;
#endif
}

std::string FormatEpochTimeInMillisAsIso8601(TimeInMillis ms) {
  struct tm time_struct;
  if (!PortableLocaltime(static_cast<time_t>(ms / 1000), &time_struct))
    return "";

  return StreamableToString(time_struct.tm_year + 1900) + "-" +
      String::FormatIntWidth2(time_struct.tm_mon + 1) + "-" +
      String::FormatIntWidth2(time_struct.tm_mday) + "T" +
      String::FormatIntWidth2(time_struct.tm_hour) + ":" +
      String::FormatIntWidth2(time_struct.tm_min) + ":" +
      String::FormatIntWidth2(time_struct.tm_sec) + "." +
      String::FormatIntWidthN(static_cast<int>(ms % 1000), 3);
}

void XmlUnitTestResultPrinter::OutputXmlCDataSection(::std::ostream* stream,
                                                     const char* data) {
  const char* segment = data;
  *stream << "<![CDATA[";
  for (;;) {
    const char* const next_segment = strstr(segment, "]]>");
    if (next_segment != nullptr) {
      stream->write(
          segment, static_cast<std::streamsize>(next_segment - segment));
      *stream << "]]>]]&gt;<![CDATA[";
      segment = next_segment + strlen("]]>");
    } else {
      *stream << segment;
      break;
    }
  }
  *stream << "]]>";
}

void XmlUnitTestResultPrinter::OutputXmlAttribute(
    std::ostream* stream,
    const std::string& element_name,
    const std::string& name,
    const std::string& value) {
  const std::vector<std::string>& allowed_names =
      GetReservedOutputAttributesForElement(element_name);

  GTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) !=
                   allowed_names.end())
      << "Attribute " << name << " is not allowed for element <" << element_name
      << ">.";

  *stream << " " << name << "=\"" << EscapeXmlAttribute(value) << "\"";
}

void XmlUnitTestResultPrinter::OutputXmlTestSuiteForTestResult(
    ::std::ostream* stream, const TestResult& result) {

  *stream << "  <testsuite";
  OutputXmlAttribute(stream, "testsuite", "name", "NonTestSuiteFailure");
  OutputXmlAttribute(stream, "testsuite", "tests", "1");
  OutputXmlAttribute(stream, "testsuite", "failures", "1");
  OutputXmlAttribute(stream, "testsuite", "disabled", "0");
  OutputXmlAttribute(stream, "testsuite", "skipped", "0");
  OutputXmlAttribute(stream, "testsuite", "errors", "0");
  OutputXmlAttribute(stream, "testsuite", "time",
                     FormatTimeInMillisAsSeconds(result.elapsed_time()));
  OutputXmlAttribute(
      stream, "testsuite", "timestamp",
      FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));
  *stream << ">";

  *stream << "    <testcase";
  OutputXmlAttribute(stream, "testcase", "name", "");
  OutputXmlAttribute(stream, "testcase", "status", "run");
  OutputXmlAttribute(stream, "testcase", "result", "completed");
  OutputXmlAttribute(stream, "testcase", "classname", "");
  OutputXmlAttribute(stream, "testcase", "time",
                     FormatTimeInMillisAsSeconds(result.elapsed_time()));
  OutputXmlAttribute(
      stream, "testcase", "timestamp",
      FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));

  OutputXmlTestResult(stream, result);

  *stream << "  </testsuite>\n";
}

void XmlUnitTestResultPrinter::OutputXmlTestInfo(::std::ostream* stream,
                                                 const char* test_suite_name,
                                                 const TestInfo& test_info) {
  const TestResult& result = *test_info.result();
  const std::string kTestsuite = "testcase";

  if (test_info.is_in_another_shard()) {
    return;
  }

  *stream << "    <testcase";
  OutputXmlAttribute(stream, kTestsuite, "name", test_info.name());

  if (test_info.value_param() != nullptr) {
    OutputXmlAttribute(stream, kTestsuite, "value_param",
                       test_info.value_param());
  }
  if (test_info.type_param() != nullptr) {
    OutputXmlAttribute(stream, kTestsuite, "type_param",
                       test_info.type_param());
  }
  if (GTEST_FLAG(list_tests)) {
    OutputXmlAttribute(stream, kTestsuite, "file", test_info.file());
    OutputXmlAttribute(stream, kTestsuite, "line",
                       StreamableToString(test_info.line()));
    *stream << " />\n";
    return;
  }

  OutputXmlAttribute(stream, kTestsuite, "status",
                     test_info.should_run() ? "run" : "notrun");
  OutputXmlAttribute(stream, kTestsuite, "result",
                     test_info.should_run()
                         ? (result.Skipped() ? "skipped" : "completed")
                         : "suppressed");
  OutputXmlAttribute(stream, kTestsuite, "time",
                     FormatTimeInMillisAsSeconds(result.elapsed_time()));
  OutputXmlAttribute(
      stream, kTestsuite, "timestamp",
      FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));
  OutputXmlAttribute(stream, kTestsuite, "classname", test_suite_name);

  OutputXmlTestResult(stream, result);
}

void XmlUnitTestResultPrinter::OutputXmlTestResult(::std::ostream* stream,
                                                   const TestResult& result) {
  int failures = 0;
  int skips = 0;
  for (int i = 0; i < result.total_part_count(); ++i) {
    const TestPartResult& part = result.GetTestPartResult(i);
    if (part.failed()) {
      if (++failures == 1 && skips == 0) {
        *stream << ">\n";
      }
      const std::string location =
          internal::FormatCompilerIndependentFileLocation(part.file_name(),
                                                          part.line_number());
      const std::string summary = location + "\n" + part.summary();
      *stream << "      <failure message=\""
              << EscapeXmlAttribute(summary)
              << "\" type=\"\">";
      const std::string detail = location + "\n" + part.message();
      OutputXmlCDataSection(stream, RemoveInvalidXmlCharacters(detail).c_str());
      *stream << "</failure>\n";
    } else if (part.skipped()) {
      if (++skips == 1 && failures == 0) {
        *stream << ">\n";
      }
      const std::string location =
          internal::FormatCompilerIndependentFileLocation(part.file_name(),
                                                          part.line_number());
      const std::string summary = location + "\n" + part.summary();
      *stream << "      <skipped message=\""
              << EscapeXmlAttribute(summary.c_str()) << "\">";
      const std::string detail = location + "\n" + part.message();
      OutputXmlCDataSection(stream, RemoveInvalidXmlCharacters(detail).c_str());
      *stream << "</skipped>\n";
    }
  }

  if (failures == 0 && skips == 0 && result.test_property_count() == 0) {
    *stream << " />\n";
  } else {
    if (failures == 0 && skips == 0) {
      *stream << ">\n";
    }
    OutputXmlTestProperties(stream, result);
    *stream << "    </testcase>\n";
  }
}

void XmlUnitTestResultPrinter::PrintXmlTestSuite(std::ostream* stream,
                                                 const TestSuite& test_suite) {
  const std::string kTestsuite = "testsuite";
  *stream << "  <" << kTestsuite;
  OutputXmlAttribute(stream, kTestsuite, "name", test_suite.name());
  OutputXmlAttribute(stream, kTestsuite, "tests",
                     StreamableToString(test_suite.reportable_test_count()));
  if (!GTEST_FLAG(list_tests)) {
    OutputXmlAttribute(stream, kTestsuite, "failures",
                       StreamableToString(test_suite.failed_test_count()));
    OutputXmlAttribute(
        stream, kTestsuite, "disabled",
        StreamableToString(test_suite.reportable_disabled_test_count()));
    OutputXmlAttribute(stream, kTestsuite, "skipped",
                       StreamableToString(test_suite.skipped_test_count()));

    OutputXmlAttribute(stream, kTestsuite, "errors", "0");

    OutputXmlAttribute(stream, kTestsuite, "time",
                       FormatTimeInMillisAsSeconds(test_suite.elapsed_time()));
    OutputXmlAttribute(
        stream, kTestsuite, "timestamp",
        FormatEpochTimeInMillisAsIso8601(test_suite.start_timestamp()));
    *stream << TestPropertiesAsXmlAttributes(test_suite.ad_hoc_test_result());
  }
  *stream << ">\n";
  for (int i = 0; i < test_suite.total_test_count(); ++i) {
    if (test_suite.GetTestInfo(i)->is_reportable())
      OutputXmlTestInfo(stream, test_suite.name(), *test_suite.GetTestInfo(i));
  }
  *stream << "  </" << kTestsuite << ">\n";
}

void XmlUnitTestResultPrinter::PrintXmlUnitTest(std::ostream* stream,
                                                const UnitTest& unit_test) {
  const std::string kTestsuites = "testsuites";

  *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  *stream << "<" << kTestsuites;

  OutputXmlAttribute(stream, kTestsuites, "tests",
                     StreamableToString(unit_test.reportable_test_count()));
  OutputXmlAttribute(stream, kTestsuites, "failures",
                     StreamableToString(unit_test.failed_test_count()));
  OutputXmlAttribute(
      stream, kTestsuites, "disabled",
      StreamableToString(unit_test.reportable_disabled_test_count()));
  OutputXmlAttribute(stream, kTestsuites, "errors", "0");
  OutputXmlAttribute(stream, kTestsuites, "time",
                     FormatTimeInMillisAsSeconds(unit_test.elapsed_time()));
  OutputXmlAttribute(
      stream, kTestsuites, "timestamp",
      FormatEpochTimeInMillisAsIso8601(unit_test.start_timestamp()));

  if (GTEST_FLAG(shuffle)) {
    OutputXmlAttribute(stream, kTestsuites, "random_seed",
                       StreamableToString(unit_test.random_seed()));
  }
  *stream << TestPropertiesAsXmlAttributes(unit_test.ad_hoc_test_result());

  OutputXmlAttribute(stream, kTestsuites, "name", "AllTests");
  *stream << ">\n";

  for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
    if (unit_test.GetTestSuite(i)->reportable_test_count() > 0)
      PrintXmlTestSuite(stream, *unit_test.GetTestSuite(i));
  }

  if (unit_test.ad_hoc_test_result().Failed()) {
    OutputXmlTestSuiteForTestResult(stream, unit_test.ad_hoc_test_result());
  }

  *stream << "</" << kTestsuites << ">\n";
}

void XmlUnitTestResultPrinter::PrintXmlTestsList(
    std::ostream* stream, const std::vector<TestSuite*>& test_suites) {
  const std::string kTestsuites = "testsuites";

  *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  *stream << "<" << kTestsuites;

  int total_tests = 0;
  for (auto test_suite : test_suites) {
    total_tests += test_suite->total_test_count();
  }
  OutputXmlAttribute(stream, kTestsuites, "tests",
                     StreamableToString(total_tests));
  OutputXmlAttribute(stream, kTestsuites, "name", "AllTests");
  *stream << ">\n";

  for (auto test_suite : test_suites) {
    PrintXmlTestSuite(stream, *test_suite);
  }
  *stream << "</" << kTestsuites << ">\n";
}

std::string XmlUnitTestResultPrinter::TestPropertiesAsXmlAttributes(
    const TestResult& result) {
  Message attributes;
  for (int i = 0; i < result.test_property_count(); ++i) {
    const TestProperty& property = result.GetTestProperty(i);
    attributes << " " << property.key() << "="
        << "\"" << EscapeXmlAttribute(property.value()) << "\"";
  }
  return attributes.GetString();
}

void XmlUnitTestResultPrinter::OutputXmlTestProperties(
    std::ostream* stream, const TestResult& result) {
  const std::string kProperties = "properties";
  const std::string kProperty = "property";

  if (result.test_property_count() <= 0) {
    return;
  }

  *stream << "<" << kProperties << ">\n";
  for (int i = 0; i < result.test_property_count(); ++i) {
    const TestProperty& property = result.GetTestProperty(i);
    *stream << "<" << kProperty;
    *stream << " name=\"" << EscapeXmlAttribute(property.key()) << "\"";
    *stream << " value=\"" << EscapeXmlAttribute(property.value()) << "\"";
    *stream << "/>\n";
  }
  *stream << "</" << kProperties << ">\n";
}

class JsonUnitTestResultPrinter : public EmptyTestEventListener {
 public:
  explicit JsonUnitTestResultPrinter(const char* output_file);

  void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;

  static void PrintJsonTestList(::std::ostream* stream,
                                const std::vector<TestSuite*>& test_suites);

 private:

  static std::string EscapeJson(const std::string& str);

  static void OutputJsonKey(std::ostream* stream,
                            const std::string& element_name,
                            const std::string& name,
                            const std::string& value,
                            const std::string& indent,
                            bool comma = true);
  static void OutputJsonKey(std::ostream* stream,
                            const std::string& element_name,
                            const std::string& name,
                            int value,
                            const std::string& indent,
                            bool comma = true);

  static void OutputJsonTestSuiteForTestResult(::std::ostream* stream,
                                               const TestResult& result);

  static void OutputJsonTestResult(::std::ostream* stream,
                                   const TestResult& result);

  static void OutputJsonTestInfo(::std::ostream* stream,
                                 const char* test_suite_name,
                                 const TestInfo& test_info);

  static void PrintJsonTestSuite(::std::ostream* stream,
                                 const TestSuite& test_suite);

  static void PrintJsonUnitTest(::std::ostream* stream,
                                const UnitTest& unit_test);

  static std::string TestPropertiesAsJson(const TestResult& result,
                                          const std::string& indent);

  const std::string output_file_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(JsonUnitTestResultPrinter);
};

JsonUnitTestResultPrinter::JsonUnitTestResultPrinter(const char* output_file)
    : output_file_(output_file) {
  if (output_file_.empty()) {
    GTEST_LOG_(FATAL) << "JSON output file may not be null";
  }
}

void JsonUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                  int  ) {
  FILE* jsonout = OpenFileForWriting(output_file_);
  std::stringstream stream;
  PrintJsonUnitTest(&stream, unit_test);
  fprintf(jsonout, "%s", StringStreamToString(&stream).c_str());
  fclose(jsonout);
}

std::string JsonUnitTestResultPrinter::EscapeJson(const std::string& str) {
  Message m;

  for (size_t i = 0; i < str.size(); ++i) {
    const char ch = str[i];
    switch (ch) {
      case '\\':
      case '"':
      case '/':
        m << '\\' << ch;
        break;
      case '\b':
        m << "\\b";
        break;
      case '\t':
        m << "\\t";
        break;
      case '\n':
        m << "\\n";
        break;
      case '\f':
        m << "\\f";
        break;
      case '\r':
        m << "\\r";
        break;
      default:
        if (ch < ' ') {
          m << "\\u00" << String::FormatByte(static_cast<unsigned char>(ch));
        } else {
          m << ch;
        }
        break;
    }
  }

  return m.GetString();
}

static std::string FormatTimeInMillisAsDuration(TimeInMillis ms) {
  ::std::stringstream ss;
  ss << (static_cast<double>(ms) * 1e-3) << "s";
  return ss.str();
}

static std::string FormatEpochTimeInMillisAsRFC3339(TimeInMillis ms) {
  struct tm time_struct;
  if (!PortableLocaltime(static_cast<time_t>(ms / 1000), &time_struct))
    return "";

  return StreamableToString(time_struct.tm_year + 1900) + "-" +
      String::FormatIntWidth2(time_struct.tm_mon + 1) + "-" +
      String::FormatIntWidth2(time_struct.tm_mday) + "T" +
      String::FormatIntWidth2(time_struct.tm_hour) + ":" +
      String::FormatIntWidth2(time_struct.tm_min) + ":" +
      String::FormatIntWidth2(time_struct.tm_sec) + "Z";
}

static inline std::string Indent(size_t width) {
  return std::string(width, ' ');
}

void JsonUnitTestResultPrinter::OutputJsonKey(
    std::ostream* stream,
    const std::string& element_name,
    const std::string& name,
    const std::string& value,
    const std::string& indent,
    bool comma) {
  const std::vector<std::string>& allowed_names =
      GetReservedOutputAttributesForElement(element_name);

  GTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) !=
                   allowed_names.end())
      << "Key \"" << name << "\" is not allowed for value \"" << element_name
      << "\".";

  *stream << indent << "\"" << name << "\": \"" << EscapeJson(value) << "\"";
  if (comma)
    *stream << ",\n";
}

void JsonUnitTestResultPrinter::OutputJsonKey(
    std::ostream* stream,
    const std::string& element_name,
    const std::string& name,
    int value,
    const std::string& indent,
    bool comma) {
  const std::vector<std::string>& allowed_names =
      GetReservedOutputAttributesForElement(element_name);

  GTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) !=
                   allowed_names.end())
      << "Key \"" << name << "\" is not allowed for value \"" << element_name
      << "\".";

  *stream << indent << "\"" << name << "\": " << StreamableToString(value);
  if (comma)
    *stream << ",\n";
}

void JsonUnitTestResultPrinter::OutputJsonTestSuiteForTestResult(
    ::std::ostream* stream, const TestResult& result) {

  *stream << Indent(4) << "{\n";
  OutputJsonKey(stream, "testsuite", "name", "NonTestSuiteFailure", Indent(6));
  OutputJsonKey(stream, "testsuite", "tests", 1, Indent(6));
  if (!GTEST_FLAG(list_tests)) {
    OutputJsonKey(stream, "testsuite", "failures", 1, Indent(6));
    OutputJsonKey(stream, "testsuite", "disabled", 0, Indent(6));
    OutputJsonKey(stream, "testsuite", "skipped", 0, Indent(6));
    OutputJsonKey(stream, "testsuite", "errors", 0, Indent(6));
    OutputJsonKey(stream, "testsuite", "time",
                  FormatTimeInMillisAsDuration(result.elapsed_time()),
                  Indent(6));
    OutputJsonKey(stream, "testsuite", "timestamp",
                  FormatEpochTimeInMillisAsRFC3339(result.start_timestamp()),
                  Indent(6));
  }
  *stream << Indent(6) << "\"testsuite\": [\n";

  *stream << Indent(8) << "{\n";
  OutputJsonKey(stream, "testcase", "name", "", Indent(10));
  OutputJsonKey(stream, "testcase", "status", "RUN", Indent(10));
  OutputJsonKey(stream, "testcase", "result", "COMPLETED", Indent(10));
  OutputJsonKey(stream, "testcase", "timestamp",
                FormatEpochTimeInMillisAsRFC3339(result.start_timestamp()),
                Indent(10));
  OutputJsonKey(stream, "testcase", "time",
                FormatTimeInMillisAsDuration(result.elapsed_time()),
                Indent(10));
  OutputJsonKey(stream, "testcase", "classname", "", Indent(10), false);
  *stream << TestPropertiesAsJson(result, Indent(10));

  OutputJsonTestResult(stream, result);

  *stream << "\n" << Indent(6) << "]\n" << Indent(4) << "}";
}

void JsonUnitTestResultPrinter::OutputJsonTestInfo(::std::ostream* stream,
                                                   const char* test_suite_name,
                                                   const TestInfo& test_info) {
  const TestResult& result = *test_info.result();
  const std::string kTestsuite = "testcase";
  const std::string kIndent = Indent(10);

  *stream << Indent(8) << "{\n";
  OutputJsonKey(stream, kTestsuite, "name", test_info.name(), kIndent);

  if (test_info.value_param() != nullptr) {
    OutputJsonKey(stream, kTestsuite, "value_param", test_info.value_param(),
                  kIndent);
  }
  if (test_info.type_param() != nullptr) {
    OutputJsonKey(stream, kTestsuite, "type_param", test_info.type_param(),
                  kIndent);
  }
  if (GTEST_FLAG(list_tests)) {
    OutputJsonKey(stream, kTestsuite, "file", test_info.file(), kIndent);
    OutputJsonKey(stream, kTestsuite, "line", test_info.line(), kIndent, false);
    *stream << "\n" << Indent(8) << "}";
    return;
  }

  OutputJsonKey(stream, kTestsuite, "status",
                test_info.should_run() ? "RUN" : "NOTRUN", kIndent);
  OutputJsonKey(stream, kTestsuite, "result",
                test_info.should_run()
                    ? (result.Skipped() ? "SKIPPED" : "COMPLETED")
                    : "SUPPRESSED",
                kIndent);
  OutputJsonKey(stream, kTestsuite, "timestamp",
                FormatEpochTimeInMillisAsRFC3339(result.start_timestamp()),
                kIndent);
  OutputJsonKey(stream, kTestsuite, "time",
                FormatTimeInMillisAsDuration(result.elapsed_time()), kIndent);
  OutputJsonKey(stream, kTestsuite, "classname", test_suite_name, kIndent,
                false);
  *stream << TestPropertiesAsJson(result, kIndent);

  OutputJsonTestResult(stream, result);
}

void JsonUnitTestResultPrinter::OutputJsonTestResult(::std::ostream* stream,
                                                     const TestResult& result) {
  const std::string kIndent = Indent(10);

  int failures = 0;
  for (int i = 0; i < result.total_part_count(); ++i) {
    const TestPartResult& part = result.GetTestPartResult(i);
    if (part.failed()) {
      *stream << ",\n";
      if (++failures == 1) {
        *stream << kIndent << "\"" << "failures" << "\": [\n";
      }
      const std::string location =
          internal::FormatCompilerIndependentFileLocation(part.file_name(),
                                                          part.line_number());
      const std::string message = EscapeJson(location + "\n" + part.message());
      *stream << kIndent << "  {\n"
              << kIndent << "    \"failure\": \"" << message << "\",\n"
              << kIndent << "    \"type\": \"\"\n"
              << kIndent << "  }";
    }
  }

  if (failures > 0)
    *stream << "\n" << kIndent << "]";
  *stream << "\n" << Indent(8) << "}";
}

void JsonUnitTestResultPrinter::PrintJsonTestSuite(
    std::ostream* stream, const TestSuite& test_suite) {
  const std::string kTestsuite = "testsuite";
  const std::string kIndent = Indent(6);

  *stream << Indent(4) << "{\n";
  OutputJsonKey(stream, kTestsuite, "name", test_suite.name(), kIndent);
  OutputJsonKey(stream, kTestsuite, "tests", test_suite.reportable_test_count(),
                kIndent);
  if (!GTEST_FLAG(list_tests)) {
    OutputJsonKey(stream, kTestsuite, "failures",
                  test_suite.failed_test_count(), kIndent);
    OutputJsonKey(stream, kTestsuite, "disabled",
                  test_suite.reportable_disabled_test_count(), kIndent);
    OutputJsonKey(stream, kTestsuite, "errors", 0, kIndent);
    OutputJsonKey(
        stream, kTestsuite, "timestamp",
        FormatEpochTimeInMillisAsRFC3339(test_suite.start_timestamp()),
        kIndent);
    OutputJsonKey(stream, kTestsuite, "time",
                  FormatTimeInMillisAsDuration(test_suite.elapsed_time()),
                  kIndent, false);
    *stream << TestPropertiesAsJson(test_suite.ad_hoc_test_result(), kIndent)
            << ",\n";
  }

  *stream << kIndent << "\"" << kTestsuite << "\": [\n";

  bool comma = false;
  for (int i = 0; i < test_suite.total_test_count(); ++i) {
    if (test_suite.GetTestInfo(i)->is_reportable()) {
      if (comma) {
        *stream << ",\n";
      } else {
        comma = true;
      }
      OutputJsonTestInfo(stream, test_suite.name(), *test_suite.GetTestInfo(i));
    }
  }
  *stream << "\n" << kIndent << "]\n" << Indent(4) << "}";
}

void JsonUnitTestResultPrinter::PrintJsonUnitTest(std::ostream* stream,
                                                  const UnitTest& unit_test) {
  const std::string kTestsuites = "testsuites";
  const std::string kIndent = Indent(2);
  *stream << "{\n";

  OutputJsonKey(stream, kTestsuites, "tests", unit_test.reportable_test_count(),
                kIndent);
  OutputJsonKey(stream, kTestsuites, "failures", unit_test.failed_test_count(),
                kIndent);
  OutputJsonKey(stream, kTestsuites, "disabled",
                unit_test.reportable_disabled_test_count(), kIndent);
  OutputJsonKey(stream, kTestsuites, "errors", 0, kIndent);
  if (GTEST_FLAG(shuffle)) {
    OutputJsonKey(stream, kTestsuites, "random_seed", unit_test.random_seed(),
                  kIndent);
  }
  OutputJsonKey(stream, kTestsuites, "timestamp",
                FormatEpochTimeInMillisAsRFC3339(unit_test.start_timestamp()),
                kIndent);
  OutputJsonKey(stream, kTestsuites, "time",
                FormatTimeInMillisAsDuration(unit_test.elapsed_time()), kIndent,
                false);

  *stream << TestPropertiesAsJson(unit_test.ad_hoc_test_result(), kIndent)
          << ",\n";

  OutputJsonKey(stream, kTestsuites, "name", "AllTests", kIndent);
  *stream << kIndent << "\"" << kTestsuites << "\": [\n";

  bool comma = false;
  for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
    if (unit_test.GetTestSuite(i)->reportable_test_count() > 0) {
      if (comma) {
        *stream << ",\n";
      } else {
        comma = true;
      }
      PrintJsonTestSuite(stream, *unit_test.GetTestSuite(i));
    }
  }

  if (unit_test.ad_hoc_test_result().Failed()) {
    OutputJsonTestSuiteForTestResult(stream, unit_test.ad_hoc_test_result());
  }

  *stream << "\n" << kIndent << "]\n" << "}\n";
}

void JsonUnitTestResultPrinter::PrintJsonTestList(
    std::ostream* stream, const std::vector<TestSuite*>& test_suites) {
  const std::string kTestsuites = "testsuites";
  const std::string kIndent = Indent(2);
  *stream << "{\n";
  int total_tests = 0;
  for (auto test_suite : test_suites) {
    total_tests += test_suite->total_test_count();
  }
  OutputJsonKey(stream, kTestsuites, "tests", total_tests, kIndent);

  OutputJsonKey(stream, kTestsuites, "name", "AllTests", kIndent);
  *stream << kIndent << "\"" << kTestsuites << "\": [\n";

  for (size_t i = 0; i < test_suites.size(); ++i) {
    if (i != 0) {
      *stream << ",\n";
    }
    PrintJsonTestSuite(stream, *test_suites[i]);
  }

  *stream << "\n"
          << kIndent << "]\n"
          << "}\n";
}

std::string JsonUnitTestResultPrinter::TestPropertiesAsJson(
    const TestResult& result, const std::string& indent) {
  Message attributes;
  for (int i = 0; i < result.test_property_count(); ++i) {
    const TestProperty& property = result.GetTestProperty(i);
    attributes << ",\n" << indent << "\"" << property.key() << "\": "
               << "\"" << EscapeJson(property.value()) << "\"";
  }
  return attributes.GetString();
}

#if GTEST_CAN_STREAM_RESULTS_

std::string StreamingListener::UrlEncode(const char* str) {
  std::string result;
  result.reserve(strlen(str) + 1);
  for (char ch = *str; ch != '\0'; ch = *++str) {
    switch (ch) {
      case '%':
      case '=':
      case '&':
      case '\n':
        result.append("%" + String::FormatByte(static_cast<unsigned char>(ch)));
        break;
      default:
        result.push_back(ch);
        break;
    }
  }
  return result;
}

void StreamingListener::SocketWriter::MakeConnection() {
  GTEST_CHECK_(sockfd_ == -1)
      << "MakeConnection() can't be called when there is already a connection.";

  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  addrinfo* servinfo = nullptr;

  const int error_num = getaddrinfo(
      host_name_.c_str(), port_num_.c_str(), &hints, &servinfo);
  if (error_num != 0) {
    GTEST_LOG_(WARNING) << "stream_result_to: getaddrinfo() failed: "
                        << gai_strerror(error_num);
  }

  for (addrinfo* cur_addr = servinfo; sockfd_ == -1 && cur_addr != nullptr;
       cur_addr = cur_addr->ai_next) {
    sockfd_ = socket(
        cur_addr->ai_family, cur_addr->ai_socktype, cur_addr->ai_protocol);
    if (sockfd_ != -1) {

      if (connect(sockfd_, cur_addr->ai_addr, cur_addr->ai_addrlen) == -1) {
        close(sockfd_);
        sockfd_ = -1;
      }
    }
  }

  freeaddrinfo(servinfo);

  if (sockfd_ == -1) {
    GTEST_LOG_(WARNING) << "stream_result_to: failed to connect to "
                        << host_name_ << ":" << port_num_;
  }
}

#endif

const char* const OsStackTraceGetterInterface::kElidedFramesMarker =
    "... " GTEST_NAME_ " internal frames ...";

std::string OsStackTraceGetter::CurrentStackTrace(int max_depth, int skip_count)
    GTEST_LOCK_EXCLUDED_(mutex_) {
#if GTEST_HAS_ABSL
  std::string result;

  if (max_depth <= 0) {
    return result;
  }

  max_depth = std::min(max_depth, kMaxStackTraceDepth);

  std::vector<void*> raw_stack(max_depth);

  const int raw_stack_size =
      absl::GetStackTrace(&raw_stack[0], max_depth, skip_count + 1);

  void* caller_frame = nullptr;
  {
    MutexLock lock(&mutex_);
    caller_frame = caller_frame_;
  }

  for (int i = 0; i < raw_stack_size; ++i) {
    if (raw_stack[i] == caller_frame &&
        !GTEST_FLAG(show_internal_stack_frames)) {

      absl::StrAppend(&result, kElidedFramesMarker, "\n");
      break;
    }

    char tmp[1024];
    const char* symbol = "(unknown)";
    if (absl::Symbolize(raw_stack[i], tmp, sizeof(tmp))) {
      symbol = tmp;
    }

    char line[1024];
    snprintf(line, sizeof(line), "  %p: %s\n", raw_stack[i], symbol);
    result += line;
  }

  return result;

#else
  static_cast<void>(max_depth);
  static_cast<void>(skip_count);
  return "";
#endif
}

void OsStackTraceGetter::UponLeavingGTest() GTEST_LOCK_EXCLUDED_(mutex_) {
#if GTEST_HAS_ABSL
  void* caller_frame = nullptr;
  if (absl::GetStackTrace(&caller_frame, 1, 3) <= 0) {
    caller_frame = nullptr;
  }

  MutexLock lock(&mutex_);
  caller_frame_ = caller_frame;
#endif
}

class ScopedPrematureExitFile {
 public:
  explicit ScopedPrematureExitFile(const char* premature_exit_filepath)
      : premature_exit_filepath_(premature_exit_filepath ?
                                 premature_exit_filepath : "") {

    if (!premature_exit_filepath_.empty()) {

      FILE* pfile = posix::FOpen(premature_exit_filepath, "w");
      fwrite("0", 1, 1, pfile);
      fclose(pfile);
    }
  }

  ~ScopedPrematureExitFile() {
#if !defined GTEST_OS_ESP8266
    if (!premature_exit_filepath_.empty()) {
      int retval = remove(premature_exit_filepath_.c_str());
      if (retval) {
        GTEST_LOG_(ERROR) << "Failed to remove premature exit filepath \""
                          << premature_exit_filepath_ << "\" with error "
                          << retval;
      }
    }
#endif
  }

 private:
  const std::string premature_exit_filepath_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ScopedPrematureExitFile);
};

}

TestEventListeners::TestEventListeners()
    : repeater_(new internal::TestEventRepeater()),
      default_result_printer_(nullptr),
      default_xml_generator_(nullptr) {}

TestEventListeners::~TestEventListeners() { delete repeater_; }

void TestEventListeners::Append(TestEventListener* listener) {
  repeater_->Append(listener);
}

TestEventListener* TestEventListeners::Release(TestEventListener* listener) {
  if (listener == default_result_printer_)
    default_result_printer_ = nullptr;
  else if (listener == default_xml_generator_)
    default_xml_generator_ = nullptr;
  return repeater_->Release(listener);
}

TestEventListener* TestEventListeners::repeater() { return repeater_; }

void TestEventListeners::SetDefaultResultPrinter(TestEventListener* listener) {
  if (default_result_printer_ != listener) {

    delete Release(default_result_printer_);
    default_result_printer_ = listener;
    if (listener != nullptr) Append(listener);
  }
}

void TestEventListeners::SetDefaultXmlGenerator(TestEventListener* listener) {
  if (default_xml_generator_ != listener) {

    delete Release(default_xml_generator_);
    default_xml_generator_ = listener;
    if (listener != nullptr) Append(listener);
  }
}

bool TestEventListeners::EventForwardingEnabled() const {
  return repeater_->forwarding_enabled();
}

void TestEventListeners::SuppressEventForwarding() {
  repeater_->set_forwarding_enabled(false);
}

UnitTest* UnitTest::GetInstance() {

#if defined(__BORLANDC__)
  static UnitTest* const instance = new UnitTest;
  return instance;
#else
  static UnitTest instance;
  return &instance;
#endif
}

int UnitTest::successful_test_suite_count() const {
  return impl()->successful_test_suite_count();
}

int UnitTest::failed_test_suite_count() const {
  return impl()->failed_test_suite_count();
}

int UnitTest::total_test_suite_count() const {
  return impl()->total_test_suite_count();
}

int UnitTest::test_suite_to_run_count() const {
  return impl()->test_suite_to_run_count();
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
int UnitTest::successful_test_case_count() const {
  return impl()->successful_test_suite_count();
}
int UnitTest::failed_test_case_count() const {
  return impl()->failed_test_suite_count();
}
int UnitTest::total_test_case_count() const {
  return impl()->total_test_suite_count();
}
int UnitTest::test_case_to_run_count() const {
  return impl()->test_suite_to_run_count();
}
#endif

int UnitTest::successful_test_count() const {
  return impl()->successful_test_count();
}

int UnitTest::skipped_test_count() const {
  return impl()->skipped_test_count();
}

int UnitTest::failed_test_count() const { return impl()->failed_test_count(); }

int UnitTest::reportable_disabled_test_count() const {
  return impl()->reportable_disabled_test_count();
}

int UnitTest::disabled_test_count() const {
  return impl()->disabled_test_count();
}

int UnitTest::reportable_test_count() const {
  return impl()->reportable_test_count();
}

int UnitTest::total_test_count() const { return impl()->total_test_count(); }

int UnitTest::test_to_run_count() const { return impl()->test_to_run_count(); }

internal::TimeInMillis UnitTest::start_timestamp() const {
    return impl()->start_timestamp();
}

internal::TimeInMillis UnitTest::elapsed_time() const {
  return impl()->elapsed_time();
}

bool UnitTest::Passed() const { return impl()->Passed(); }

bool UnitTest::Failed() const { return impl()->Failed(); }

const TestSuite* UnitTest::GetTestSuite(int i) const {
  return impl()->GetTestSuite(i);
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
const TestCase* UnitTest::GetTestCase(int i) const {
  return impl()->GetTestCase(i);
}
#endif

const TestResult& UnitTest::ad_hoc_test_result() const {
  return *impl()->ad_hoc_test_result();
}

TestSuite* UnitTest::GetMutableTestSuite(int i) {
  return impl()->GetMutableSuiteCase(i);
}

TestEventListeners& UnitTest::listeners() {
  return *impl()->listeners();
}

Environment* UnitTest::AddEnvironment(Environment* env) {
  if (env == nullptr) {
    return nullptr;
  }

  impl_->environments().push_back(env);
  return env;
}

void UnitTest::AddTestPartResult(
    TestPartResult::Type result_type,
    const char* file_name,
    int line_number,
    const std::string& message,
    const std::string& os_stack_trace) GTEST_LOCK_EXCLUDED_(mutex_) {
  Message msg;
  msg << message;

  internal::MutexLock lock(&mutex_);
  if (impl_->gtest_trace_stack().size() > 0) {
    msg << "\n" << GTEST_NAME_ << " trace:";

    for (size_t i = impl_->gtest_trace_stack().size(); i > 0; --i) {
      const internal::TraceInfo& trace = impl_->gtest_trace_stack()[i - 1];
      msg << "\n" << internal::FormatFileLocation(trace.file, trace.line)
          << " " << trace.message;
    }
  }

  if (os_stack_trace.c_str() != nullptr && !os_stack_trace.empty()) {
    msg << internal::kStackTraceMarker << os_stack_trace;
  }

  const TestPartResult result = TestPartResult(
      result_type, file_name, line_number, msg.GetString().c_str());
  impl_->GetTestPartResultReporterForCurrentThread()->
      ReportTestPartResult(result);

  if (result_type != TestPartResult::kSuccess &&
      result_type != TestPartResult::kSkip) {

    if (GTEST_FLAG(break_on_failure)) {
#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_PHONE && !GTEST_OS_WINDOWS_RT

      DebugBreak();
#elif (!defined(__native_client__)) &&            \
    ((defined(__clang__) || defined(__GNUC__)) && \
     (defined(__x86_64__) || defined(__i386__)))

      asm("int3");
#else

      *static_cast<volatile int*>(nullptr) = 1;
#endif
    } else if (GTEST_FLAG(throw_on_failure)) {
#if GTEST_HAS_EXCEPTIONS
      throw internal::GoogleTestFailureException(result);
#else

      exit(1);
#endif
    }
  }
}

void UnitTest::RecordProperty(const std::string& key,
                              const std::string& value) {
  impl_->RecordProperty(TestProperty(key, value));
}

int UnitTest::Run() {
  const bool in_death_test_child_process =
      internal::GTEST_FLAG(internal_run_death_test).length() > 0;

  const internal::ScopedPrematureExitFile premature_exit_file(
      in_death_test_child_process
          ? nullptr
          : internal::posix::GetEnv("TEST_PREMATURE_EXIT_FILE"));

  impl()->set_catch_exceptions(GTEST_FLAG(catch_exceptions));

#if GTEST_OS_WINDOWS

  if (impl()->catch_exceptions() || in_death_test_child_process) {
# if !GTEST_OS_WINDOWS_MOBILE && !GTEST_OS_WINDOWS_PHONE && !GTEST_OS_WINDOWS_RT

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT |
                 SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
# endif

# if (defined(_MSC_VER) || GTEST_OS_WINDOWS_MINGW) && !GTEST_OS_WINDOWS_MOBILE

    _set_error_mode(_OUT_TO_STDERR);
# endif

# if defined(_MSC_VER) && !GTEST_OS_WINDOWS_MOBILE

    if (!GTEST_FLAG(break_on_failure))
      _set_abort_behavior(
          0x0,
          _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    if (!IsDebuggerPresent()) {
      (void)_CrtSetReportMode(_CRT_ASSERT,
                              _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
      (void)_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    }
# endif
  }
#endif

  return internal::HandleExceptionsInMethodIfSupported(
      impl(),
      &internal::UnitTestImpl::RunAllTests,
      "auxiliary test code (environments or event listeners)") ? 0 : 1;
}

const char* UnitTest::original_working_dir() const {
  return impl_->original_working_dir_.c_str();
}

const TestSuite* UnitTest::current_test_suite() const
    GTEST_LOCK_EXCLUDED_(mutex_) {
  internal::MutexLock lock(&mutex_);
  return impl_->current_test_suite();
}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
const TestCase* UnitTest::current_test_case() const
    GTEST_LOCK_EXCLUDED_(mutex_) {
  internal::MutexLock lock(&mutex_);
  return impl_->current_test_suite();
}
#endif

const TestInfo* UnitTest::current_test_info() const
    GTEST_LOCK_EXCLUDED_(mutex_) {
  internal::MutexLock lock(&mutex_);
  return impl_->current_test_info();
}

int UnitTest::random_seed() const { return impl_->random_seed(); }

internal::ParameterizedTestSuiteRegistry&
UnitTest::parameterized_test_registry() GTEST_LOCK_EXCLUDED_(mutex_) {
  return impl_->parameterized_test_registry();
}

UnitTest::UnitTest() {
  impl_ = new internal::UnitTestImpl(this);
}

UnitTest::~UnitTest() {
  delete impl_;
}

void UnitTest::PushGTestTrace(const internal::TraceInfo& trace)
    GTEST_LOCK_EXCLUDED_(mutex_) {
  internal::MutexLock lock(&mutex_);
  impl_->gtest_trace_stack().push_back(trace);
}

void UnitTest::PopGTestTrace()
    GTEST_LOCK_EXCLUDED_(mutex_) {
  internal::MutexLock lock(&mutex_);
  impl_->gtest_trace_stack().pop_back();
}

namespace internal {

UnitTestImpl::UnitTestImpl(UnitTest* parent)
    : parent_(parent),
      GTEST_DISABLE_MSC_WARNINGS_PUSH_(4355  )
          default_global_test_part_result_reporter_(this),
      default_per_thread_test_part_result_reporter_(this),
      GTEST_DISABLE_MSC_WARNINGS_POP_() global_test_part_result_repoter_(
          &default_global_test_part_result_reporter_),
      per_thread_test_part_result_reporter_(
          &default_per_thread_test_part_result_reporter_),
      parameterized_test_registry_(),
      parameterized_tests_registered_(false),
      last_death_test_suite_(-1),
      current_test_suite_(nullptr),
      current_test_info_(nullptr),
      ad_hoc_test_result_(),
      os_stack_trace_getter_(nullptr),
      post_flag_parse_init_performed_(false),
      random_seed_(0),
      random_(0),
      start_timestamp_(0),
      elapsed_time_(0),
#if GTEST_HAS_DEATH_TEST
      death_test_factory_(new DefaultDeathTestFactory),
#endif

      catch_exceptions_(false) {
  listeners()->SetDefaultResultPrinter(new PrettyUnitTestResultPrinter);
}

UnitTestImpl::~UnitTestImpl() {

  ForEach(test_suites_, internal::Delete<TestSuite>);

  ForEach(environments_, internal::Delete<Environment>);

  delete os_stack_trace_getter_;
}

void UnitTestImpl::RecordProperty(const TestProperty& test_property) {
  std::string xml_element;
  TestResult* test_result;

  if (current_test_info_ != nullptr) {
    xml_element = "testcase";
    test_result = &(current_test_info_->result_);
  } else if (current_test_suite_ != nullptr) {
    xml_element = "testsuite";
    test_result = &(current_test_suite_->ad_hoc_test_result_);
  } else {
    xml_element = "testsuites";
    test_result = &ad_hoc_test_result_;
  }
  test_result->RecordProperty(xml_element, test_property);
}

#if GTEST_HAS_DEATH_TEST

void UnitTestImpl::SuppressTestEventsIfInSubprocess() {
  if (internal_run_death_test_flag_.get() != nullptr)
    listeners()->SuppressEventForwarding();
}
#endif

void UnitTestImpl::ConfigureXmlOutput() {
  const std::string& output_format = UnitTestOptions::GetOutputFormat();
  if (output_format == "xml") {
    listeners()->SetDefaultXmlGenerator(new XmlUnitTestResultPrinter(
        UnitTestOptions::GetAbsolutePathToOutputFile().c_str()));
  } else if (output_format == "json") {
    listeners()->SetDefaultXmlGenerator(new JsonUnitTestResultPrinter(
        UnitTestOptions::GetAbsolutePathToOutputFile().c_str()));
  } else if (output_format != "") {
    GTEST_LOG_(WARNING) << "WARNING: unrecognized output format \""
                        << output_format << "\" ignored.";
  }
}

#if GTEST_CAN_STREAM_RESULTS_

void UnitTestImpl::ConfigureStreamingOutput() {
  const std::string& target = GTEST_FLAG(stream_result_to);
  if (!target.empty()) {
    const size_t pos = target.find(':');
    if (pos != std::string::npos) {
      listeners()->Append(new StreamingListener(target.substr(0, pos),
                                                target.substr(pos+1)));
    } else {
      GTEST_LOG_(WARNING) << "unrecognized streaming target \"" << target
                          << "\" ignored.";
    }
  }
}
#endif

void UnitTestImpl::PostFlagParsingInit() {

  if (!post_flag_parse_init_performed_) {
    post_flag_parse_init_performed_ = true;

#if defined(GTEST_CUSTOM_TEST_EVENT_LISTENER_)

    listeners()->Append(new GTEST_CUSTOM_TEST_EVENT_LISTENER_());
#endif

#if GTEST_HAS_DEATH_TEST
    InitDeathTestSubprocessControlInfo();
    SuppressTestEventsIfInSubprocess();
#endif

    RegisterParameterizedTests();

    ConfigureXmlOutput();

    if (GTEST_FLAG(brief)) {
      listeners()->SetDefaultResultPrinter(new BriefUnitTestResultPrinter);
    }

#if GTEST_CAN_STREAM_RESULTS_

    ConfigureStreamingOutput();
#endif

#if GTEST_HAS_ABSL
    if (GTEST_FLAG(install_failure_signal_handler)) {
      absl::FailureSignalHandlerOptions options;
      absl::InstallFailureSignalHandler(options);
    }
#endif
  }
}

class TestSuiteNameIs {
 public:

  explicit TestSuiteNameIs(const std::string& name) : name_(name) {}

  bool operator()(const TestSuite* test_suite) const {
    return test_suite != nullptr &&
           strcmp(test_suite->name(), name_.c_str()) == 0;
  }

 private:
  std::string name_;
};

TestSuite* UnitTestImpl::GetTestSuite(
    const char* test_suite_name, const char* type_param,
    internal::SetUpTestSuiteFunc set_up_tc,
    internal::TearDownTestSuiteFunc tear_down_tc) {

  const auto test_suite =
      std::find_if(test_suites_.rbegin(), test_suites_.rend(),
                   TestSuiteNameIs(test_suite_name));

  if (test_suite != test_suites_.rend()) return *test_suite;

  auto* const new_test_suite =
      new TestSuite(test_suite_name, type_param, set_up_tc, tear_down_tc);

  if (internal::UnitTestOptions::MatchesFilter(test_suite_name,
                                               kDeathTestSuiteFilter)) {

    ++last_death_test_suite_;
    test_suites_.insert(test_suites_.begin() + last_death_test_suite_,
                        new_test_suite);
  } else {

    test_suites_.push_back(new_test_suite);
  }

  test_suite_indices_.push_back(static_cast<int>(test_suite_indices_.size()));
  return new_test_suite;
}

static void SetUpEnvironment(Environment* env) { env->SetUp(); }
static void TearDownEnvironment(Environment* env) { env->TearDown(); }

bool UnitTestImpl::RunAllTests() {

  const bool gtest_is_initialized_before_run_all_tests = GTestIsInitialized();

  if (g_help_flag)
    return true;

  PostFlagParsingInit();

  internal::WriteToShardStatusFileIfNeeded();

  bool in_subprocess_for_death_test = false;

#if GTEST_HAS_DEATH_TEST
  in_subprocess_for_death_test =
      (internal_run_death_test_flag_.get() != nullptr);
# if defined(GTEST_EXTRA_DEATH_TEST_CHILD_SETUP_)
  if (in_subprocess_for_death_test) {
    GTEST_EXTRA_DEATH_TEST_CHILD_SETUP_();
  }
# endif
#endif

  const bool should_shard = ShouldShard(kTestTotalShards, kTestShardIndex,
                                        in_subprocess_for_death_test);

  const bool has_tests_to_run = FilterTests(should_shard
                                              ? HONOR_SHARDING_PROTOCOL
                                              : IGNORE_SHARDING_PROTOCOL) > 0;

  if (GTEST_FLAG(list_tests)) {

    ListTestsMatchingFilter();
    return true;
  }

  random_seed_ = GTEST_FLAG(shuffle) ?
      GetRandomSeedFromFlag(GTEST_FLAG(random_seed)) : 0;

  bool failed = false;

  TestEventListener* repeater = listeners()->repeater();

  start_timestamp_ = GetTimeInMillis();
  repeater->OnTestProgramStart(*parent_);

  const int repeat = in_subprocess_for_death_test ? 1 : GTEST_FLAG(repeat);

  const bool gtest_repeat_forever = repeat < 0;
  for (int i = 0; gtest_repeat_forever || i != repeat; i++) {

    ClearNonAdHocTestResult();

    Timer timer;

    if (has_tests_to_run && GTEST_FLAG(shuffle)) {
      random()->Reseed(static_cast<uint32_t>(random_seed_));

      ShuffleTests();
    }

    repeater->OnTestIterationStart(*parent_, i);

    if (has_tests_to_run) {

      repeater->OnEnvironmentsSetUpStart(*parent_);
      ForEach(environments_, SetUpEnvironment);
      repeater->OnEnvironmentsSetUpEnd(*parent_);

      if (Test::IsSkipped()) {

        TestResult& test_result =
            *internal::GetUnitTestImpl()->current_test_result();
        for (int j = 0; j < test_result.total_part_count(); ++j) {
          const TestPartResult& test_part_result =
              test_result.GetTestPartResult(j);
          if (test_part_result.type() == TestPartResult::kSkip) {
            const std::string& result = test_part_result.message();
            printf("%s\n", result.c_str());
          }
        }
        fflush(stdout);
      } else if (!Test::HasFatalFailure()) {
        for (int test_index = 0; test_index < total_test_suite_count();
             test_index++) {
          GetMutableSuiteCase(test_index)->Run();
          if (GTEST_FLAG(fail_fast) &&
              GetMutableSuiteCase(test_index)->Failed()) {
            for (int j = test_index + 1; j < total_test_suite_count(); j++) {
              GetMutableSuiteCase(j)->Skip();
            }
            break;
          }
        }
      } else if (Test::HasFatalFailure()) {

        for (int test_index = 0; test_index < total_test_suite_count();
             test_index++) {
          GetMutableSuiteCase(test_index)->Skip();
        }
      }

      repeater->OnEnvironmentsTearDownStart(*parent_);
      std::for_each(environments_.rbegin(), environments_.rend(),
                    TearDownEnvironment);
      repeater->OnEnvironmentsTearDownEnd(*parent_);
    }

    elapsed_time_ = timer.Elapsed();

    repeater->OnTestIterationEnd(*parent_, i);

    if (!Passed()) {
      failed = true;
    }

    UnshuffleTests();

    if (GTEST_FLAG(shuffle)) {

      random_seed_ = GetNextRandomSeed(random_seed_);
    }
  }

  repeater->OnTestProgramEnd(*parent_);

  if (!gtest_is_initialized_before_run_all_tests) {
    ColoredPrintf(
        GTestColor::kRed,
        "\nIMPORTANT NOTICE - DO NOT IGNORE:\n"
        "This test program did NOT call " GTEST_INIT_GOOGLE_TEST_NAME_
        "() before calling RUN_ALL_TESTS(). This is INVALID. Soon " GTEST_NAME_
        " will start to enforce the valid usage. "
        "Please fix it ASAP, or IT WILL START TO FAIL.\n");
#if GTEST_FOR_GOOGLE_
    ColoredPrintf(GTestColor::kRed,
                  "For more details, see http://wiki/Main/ValidGUnitMain.\n");
#endif
  }

  return !failed;
}

void WriteToShardStatusFileIfNeeded() {
  const char* const test_shard_file = posix::GetEnv(kTestShardStatusFile);
  if (test_shard_file != nullptr) {
    FILE* const file = posix::FOpen(test_shard_file, "w");
    if (file == nullptr) {
      ColoredPrintf(GTestColor::kRed,
                    "Could not write to the test shard status file \"%s\" "
                    "specified by the %s environment variable.\n",
                    test_shard_file, kTestShardStatusFile);
      fflush(stdout);
      exit(EXIT_FAILURE);
    }
    fclose(file);
  }
}

bool ShouldShard(const char* total_shards_env,
                 const char* shard_index_env,
                 bool in_subprocess_for_death_test) {
  if (in_subprocess_for_death_test) {
    return false;
  }

  const int32_t total_shards = Int32FromEnvOrDie(total_shards_env, -1);
  const int32_t shard_index = Int32FromEnvOrDie(shard_index_env, -1);

  if (total_shards == -1 && shard_index == -1) {
    return false;
  } else if (total_shards == -1 && shard_index != -1) {
    const Message msg = Message()
      << "Invalid environment variables: you have "
      << kTestShardIndex << " = " << shard_index
      << ", but have left " << kTestTotalShards << " unset.\n";
    ColoredPrintf(GTestColor::kRed, "%s", msg.GetString().c_str());
    fflush(stdout);
    exit(EXIT_FAILURE);
  } else if (total_shards != -1 && shard_index == -1) {
    const Message msg = Message()
      << "Invalid environment variables: you have "
      << kTestTotalShards << " = " << total_shards
      << ", but have left " << kTestShardIndex << " unset.\n";
    ColoredPrintf(GTestColor::kRed, "%s", msg.GetString().c_str());
    fflush(stdout);
    exit(EXIT_FAILURE);
  } else if (shard_index < 0 || shard_index >= total_shards) {
    const Message msg = Message()
      << "Invalid environment variables: we require 0 <= "
      << kTestShardIndex << " < " << kTestTotalShards
      << ", but you have " << kTestShardIndex << "=" << shard_index
      << ", " << kTestTotalShards << "=" << total_shards << ".\n";
    ColoredPrintf(GTestColor::kRed, "%s", msg.GetString().c_str());
    fflush(stdout);
    exit(EXIT_FAILURE);
  }

  return total_shards > 1;
}

int32_t Int32FromEnvOrDie(const char* var, int32_t default_val) {
  const char* str_val = posix::GetEnv(var);
  if (str_val == nullptr) {
    return default_val;
  }

  int32_t result;
  if (!ParseInt32(Message() << "The value of environment variable " << var,
                  str_val, &result)) {
    exit(EXIT_FAILURE);
  }
  return result;
}

bool ShouldRunTestOnShard(int total_shards, int shard_index, int test_id) {
  return (test_id % total_shards) == shard_index;
}

int UnitTestImpl::FilterTests(ReactionToSharding shard_tests) {
  const int32_t total_shards = shard_tests == HONOR_SHARDING_PROTOCOL ?
      Int32FromEnvOrDie(kTestTotalShards, -1) : -1;
  const int32_t shard_index = shard_tests == HONOR_SHARDING_PROTOCOL ?
      Int32FromEnvOrDie(kTestShardIndex, -1) : -1;

  int num_runnable_tests = 0;
  int num_selected_tests = 0;
  for (auto* test_suite : test_suites_) {
    const std::string& test_suite_name = test_suite->name();
    test_suite->set_should_run(false);

    for (size_t j = 0; j < test_suite->test_info_list().size(); j++) {
      TestInfo* const test_info = test_suite->test_info_list()[j];
      const std::string test_name(test_info->name());

      const bool is_disabled = internal::UnitTestOptions::MatchesFilter(
                                   test_suite_name, kDisableTestFilter) ||
                               internal::UnitTestOptions::MatchesFilter(
                                   test_name, kDisableTestFilter);
      test_info->is_disabled_ = is_disabled;

      const bool matches_filter = internal::UnitTestOptions::FilterMatchesTest(
          test_suite_name, test_name);
      test_info->matches_filter_ = matches_filter;

      const bool is_runnable =
          (GTEST_FLAG(also_run_disabled_tests) || !is_disabled) &&
          matches_filter;

      const bool is_in_another_shard =
          shard_tests != IGNORE_SHARDING_PROTOCOL &&
          !ShouldRunTestOnShard(total_shards, shard_index, num_runnable_tests);
      test_info->is_in_another_shard_ = is_in_another_shard;
      const bool is_selected = is_runnable && !is_in_another_shard;

      num_runnable_tests += is_runnable;
      num_selected_tests += is_selected;

      test_info->should_run_ = is_selected;
      test_suite->set_should_run(test_suite->should_run() || is_selected);
    }
  }
  return num_selected_tests;
}

static void PrintOnOneLine(const char* str, int max_length) {
  if (str != nullptr) {
    for (int i = 0; *str != '\0'; ++str) {
      if (i >= max_length) {
        printf("...");
        break;
      }
      if (*str == '\n') {
        printf("\\n");
        i += 2;
      } else {
        printf("%c", *str);
        ++i;
      }
    }
  }
}

void UnitTestImpl::ListTestsMatchingFilter() {

  const int kMaxParamLength = 250;

  for (auto* test_suite : test_suites_) {
    bool printed_test_suite_name = false;

    for (size_t j = 0; j < test_suite->test_info_list().size(); j++) {
      const TestInfo* const test_info = test_suite->test_info_list()[j];
      if (test_info->matches_filter_) {
        if (!printed_test_suite_name) {
          printed_test_suite_name = true;
          printf("%s.", test_suite->name());
          if (test_suite->type_param() != nullptr) {
            printf("  # %s = ", kTypeParamLabel);

            PrintOnOneLine(test_suite->type_param(), kMaxParamLength);
          }
          printf("\n");
        }
        printf("  %s", test_info->name());
        if (test_info->value_param() != nullptr) {
          printf("  # %s = ", kValueParamLabel);

          PrintOnOneLine(test_info->value_param(), kMaxParamLength);
        }
        printf("\n");
      }
    }
  }
  fflush(stdout);
  const std::string& output_format = UnitTestOptions::GetOutputFormat();
  if (output_format == "xml" || output_format == "json") {
    FILE* fileout = OpenFileForWriting(
        UnitTestOptions::GetAbsolutePathToOutputFile().c_str());
    std::stringstream stream;
    if (output_format == "xml") {
      XmlUnitTestResultPrinter(
          UnitTestOptions::GetAbsolutePathToOutputFile().c_str())
          .PrintXmlTestsList(&stream, test_suites_);
    } else if (output_format == "json") {
      JsonUnitTestResultPrinter(
          UnitTestOptions::GetAbsolutePathToOutputFile().c_str())
          .PrintJsonTestList(&stream, test_suites_);
    }
    fprintf(fileout, "%s", StringStreamToString(&stream).c_str());
    fclose(fileout);
  }
}

void UnitTestImpl::set_os_stack_trace_getter(
    OsStackTraceGetterInterface* getter) {
  if (os_stack_trace_getter_ != getter) {
    delete os_stack_trace_getter_;
    os_stack_trace_getter_ = getter;
  }
}

OsStackTraceGetterInterface* UnitTestImpl::os_stack_trace_getter() {
  if (os_stack_trace_getter_ == nullptr) {
#ifdef GTEST_OS_STACK_TRACE_GETTER_
    os_stack_trace_getter_ = new GTEST_OS_STACK_TRACE_GETTER_;
#else
    os_stack_trace_getter_ = new OsStackTraceGetter;
#endif
  }

  return os_stack_trace_getter_;
}

TestResult* UnitTestImpl::current_test_result() {
  if (current_test_info_ != nullptr) {
    return &current_test_info_->result_;
  }
  if (current_test_suite_ != nullptr) {
    return &current_test_suite_->ad_hoc_test_result_;
  }
  return &ad_hoc_test_result_;
}

void UnitTestImpl::ShuffleTests() {

  ShuffleRange(random(), 0, last_death_test_suite_ + 1, &test_suite_indices_);

  ShuffleRange(random(), last_death_test_suite_ + 1,
               static_cast<int>(test_suites_.size()), &test_suite_indices_);

  for (auto& test_suite : test_suites_) {
    test_suite->ShuffleTests(random());
  }
}

void UnitTestImpl::UnshuffleTests() {
  for (size_t i = 0; i < test_suites_.size(); i++) {

    test_suites_[i]->UnshuffleTests();

    test_suite_indices_[i] = static_cast<int>(i);
  }
}

std::string GetCurrentOsStackTraceExceptTop(UnitTest*  ,
                                            int skip_count) {

  return GetUnitTestImpl()->CurrentOsStackTraceExceptTop(skip_count + 1);
}

namespace {
class ClassUniqueToAlwaysTrue {};
}

bool IsTrue(bool condition) { return condition; }

bool AlwaysTrue() {
#if GTEST_HAS_EXCEPTIONS

  if (IsTrue(false))
    throw ClassUniqueToAlwaysTrue();
#endif
  return true;
}

bool SkipPrefix(const char* prefix, const char** pstr) {
  const size_t prefix_len = strlen(prefix);
  if (strncmp(*pstr, prefix, prefix_len) == 0) {
    *pstr += prefix_len;
    return true;
  }
  return false;
}

static const char* ParseFlagValue(const char* str, const char* flag,
                                  bool def_optional) {

  if (str == nullptr || flag == nullptr) return nullptr;

  const std::string flag_str = std::string("--") + GTEST_FLAG_PREFIX_ + flag;
  const size_t flag_len = flag_str.length();
  if (strncmp(str, flag_str.c_str(), flag_len) != 0) return nullptr;

  const char* flag_end = str + flag_len;

  if (def_optional && (flag_end[0] == '\0')) {
    return flag_end;
  }

  if (flag_end[0] != '=') return nullptr;

  return flag_end + 1;
}

static bool ParseBoolFlag(const char* str, const char* flag, bool* value) {

  const char* const value_str = ParseFlagValue(str, flag, true);

  if (value_str == nullptr) return false;

  *value = !(*value_str == '0' || *value_str == 'f' || *value_str == 'F');
  return true;
}

bool ParseInt32Flag(const char* str, const char* flag, int32_t* value) {

  const char* const value_str = ParseFlagValue(str, flag, false);

  if (value_str == nullptr) return false;

  return ParseInt32(Message() << "The value of flag --" << flag,
                    value_str, value);
}

template <typename String>
static bool ParseStringFlag(const char* str, const char* flag, String* value) {

  const char* const value_str = ParseFlagValue(str, flag, false);

  if (value_str == nullptr) return false;

  *value = value_str;
  return true;
}

static bool HasGoogleTestFlagPrefix(const char* str) {
  return (SkipPrefix("--", &str) ||
          SkipPrefix("-", &str) ||
          SkipPrefix("/", &str)) &&
         !SkipPrefix(GTEST_FLAG_PREFIX_ "internal_", &str) &&
         (SkipPrefix(GTEST_FLAG_PREFIX_, &str) ||
          SkipPrefix(GTEST_FLAG_PREFIX_DASH_, &str));
}

static void PrintColorEncoded(const char* str) {
  GTestColor color = GTestColor::kDefault;

  for (;;) {
    const char* p = strchr(str, '@');
    if (p == nullptr) {
      ColoredPrintf(color, "%s", str);
      return;
    }

    ColoredPrintf(color, "%s", std::string(str, p).c_str());

    const char ch = p[1];
    str = p + 2;
    if (ch == '@') {
      ColoredPrintf(color, "@");
    } else if (ch == 'D') {
      color = GTestColor::kDefault;
    } else if (ch == 'R') {
      color = GTestColor::kRed;
    } else if (ch == 'G') {
      color = GTestColor::kGreen;
    } else if (ch == 'Y') {
      color = GTestColor::kYellow;
    } else {
      --str;
    }
  }
}

static const char kColorEncodedHelpMessage[] =
    "This program contains tests written using " GTEST_NAME_
    ". You can use the\n"
    "following command line flags to control its behavior:\n"
    "\n"
    "Test Selection:\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "list_tests@D\n"
    "      List the names of all tests instead of running them. The name of\n"
    "      TEST(Foo, Bar) is \"Foo.Bar\".\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "filter=@YPOSITIVE_PATTERNS"
    "[@G-@YNEGATIVE_PATTERNS]@D\n"
    "      Run only the tests whose name matches one of the positive patterns "
    "but\n"
    "      none of the negative patterns. '?' matches any single character; "
    "'*'\n"
    "      matches any substring; ':' separates two patterns.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "also_run_disabled_tests@D\n"
    "      Run all disabled tests too.\n"
    "\n"
    "Test Execution:\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "repeat=@Y[COUNT]@D\n"
    "      Run the tests repeatedly; use a negative count to repeat forever.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "shuffle@D\n"
    "      Randomize tests' orders on every iteration.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "random_seed=@Y[NUMBER]@D\n"
    "      Random number seed to use for shuffling test orders (between 1 and\n"
    "      99999, or 0 to use a seed based on the current time).\n"
    "\n"
    "Test Output:\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "color=@Y(@Gyes@Y|@Gno@Y|@Gauto@Y)@D\n"
    "      Enable/disable colored output. The default is @Gauto@D.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "brief=1@D\n"
    "      Only print test failures.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "print_time=0@D\n"
    "      Don't print the elapsed time of each test.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "output=@Y(@Gjson@Y|@Gxml@Y)[@G:@YDIRECTORY_PATH@G" GTEST_PATH_SEP_
    "@Y|@G:@YFILE_PATH]@D\n"
    "      Generate a JSON or XML report in the given directory or with the "
    "given\n"
    "      file name. @YFILE_PATH@D defaults to @Gtest_detail.xml@D.\n"
# if GTEST_CAN_STREAM_RESULTS_
    "  @G--" GTEST_FLAG_PREFIX_
    "stream_result_to=@YHOST@G:@YPORT@D\n"
    "      Stream test results to the given server.\n"
# endif
    "\n"
    "Assertion Behavior:\n"
# if GTEST_HAS_DEATH_TEST && !GTEST_OS_WINDOWS
    "  @G--" GTEST_FLAG_PREFIX_
    "death_test_style=@Y(@Gfast@Y|@Gthreadsafe@Y)@D\n"
    "      Set the default death test style.\n"
# endif
    "  @G--" GTEST_FLAG_PREFIX_
    "break_on_failure@D\n"
    "      Turn assertion failures into debugger break-points.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "throw_on_failure@D\n"
    "      Turn assertion failures into C++ exceptions for use by an external\n"
    "      test framework.\n"
    "  @G--" GTEST_FLAG_PREFIX_
    "catch_exceptions=0@D\n"
    "      Do not report exceptions as test failures. Instead, allow them\n"
    "      to crash the program or throw a pop-up (on Windows).\n"
    "\n"
    "Except for @G--" GTEST_FLAG_PREFIX_
    "list_tests@D, you can alternatively set "
    "the corresponding\n"
    "environment variable of a flag (all letters in upper-case). For example, "
    "to\n"
    "disable colored text output, you can either specify "
    "@G--" GTEST_FLAG_PREFIX_
    "color=no@D or set\n"
    "the @G" GTEST_FLAG_PREFIX_UPPER_
    "COLOR@D environment variable to @Gno@D.\n"
    "\n"
    "For more information, please read the " GTEST_NAME_
    " documentation at\n"
    "@G" GTEST_PROJECT_URL_ "@D. If you find a bug in " GTEST_NAME_
    "\n"
    "(not one in your own code or tests), please report it to\n"
    "@G<" GTEST_DEV_EMAIL_ ">@D.\n";

static bool ParseGoogleTestFlag(const char* const arg) {
  return ParseBoolFlag(arg, kAlsoRunDisabledTestsFlag,
                       &GTEST_FLAG(also_run_disabled_tests)) ||
         ParseBoolFlag(arg, kBreakOnFailureFlag,
                       &GTEST_FLAG(break_on_failure)) ||
         ParseBoolFlag(arg, kCatchExceptionsFlag,
                       &GTEST_FLAG(catch_exceptions)) ||
         ParseStringFlag(arg, kColorFlag, &GTEST_FLAG(color)) ||
         ParseStringFlag(arg, kDeathTestStyleFlag,
                         &GTEST_FLAG(death_test_style)) ||
         ParseBoolFlag(arg, kDeathTestUseFork,
                       &GTEST_FLAG(death_test_use_fork)) ||
         ParseBoolFlag(arg, kFailFast, &GTEST_FLAG(fail_fast)) ||
         ParseStringFlag(arg, kFilterFlag, &GTEST_FLAG(filter)) ||
         ParseStringFlag(arg, kInternalRunDeathTestFlag,
                         &GTEST_FLAG(internal_run_death_test)) ||
         ParseBoolFlag(arg, kListTestsFlag, &GTEST_FLAG(list_tests)) ||
         ParseStringFlag(arg, kOutputFlag, &GTEST_FLAG(output)) ||
         ParseBoolFlag(arg, kBriefFlag, &GTEST_FLAG(brief)) ||
         ParseBoolFlag(arg, kPrintTimeFlag, &GTEST_FLAG(print_time)) ||
         ParseBoolFlag(arg, kPrintUTF8Flag, &GTEST_FLAG(print_utf8)) ||
         ParseInt32Flag(arg, kRandomSeedFlag, &GTEST_FLAG(random_seed)) ||
         ParseInt32Flag(arg, kRepeatFlag, &GTEST_FLAG(repeat)) ||
         ParseBoolFlag(arg, kShuffleFlag, &GTEST_FLAG(shuffle)) ||
         ParseInt32Flag(arg, kStackTraceDepthFlag,
                        &GTEST_FLAG(stack_trace_depth)) ||
         ParseStringFlag(arg, kStreamResultToFlag,
                         &GTEST_FLAG(stream_result_to)) ||
         ParseBoolFlag(arg, kThrowOnFailureFlag, &GTEST_FLAG(throw_on_failure));
}

#if GTEST_USE_OWN_FLAGFILE_FLAG_
static void LoadFlagsFromFile(const std::string& path) {
  FILE* flagfile = posix::FOpen(path.c_str(), "r");
  if (!flagfile) {
    GTEST_LOG_(FATAL) << "Unable to open file \"" << GTEST_FLAG(flagfile)
                      << "\"";
  }
  std::string contents(ReadEntireFile(flagfile));
  posix::FClose(flagfile);
  std::vector<std::string> lines;
  SplitString(contents, '\n', &lines);
  for (size_t i = 0; i < lines.size(); ++i) {
    if (lines[i].empty())
      continue;
    if (!ParseGoogleTestFlag(lines[i].c_str()))
      g_help_flag = true;
  }
}
#endif

template <typename CharType>
void ParseGoogleTestFlagsOnlyImpl(int* argc, CharType** argv) {
  for (int i = 1; i < *argc; i++) {
    const std::string arg_string = StreamableToString(argv[i]);
    const char* const arg = arg_string.c_str();

    using internal::ParseBoolFlag;
    using internal::ParseInt32Flag;
    using internal::ParseStringFlag;

    bool remove_flag = false;
    if (ParseGoogleTestFlag(arg)) {
      remove_flag = true;
#if GTEST_USE_OWN_FLAGFILE_FLAG_
    } else if (ParseStringFlag(arg, kFlagfileFlag, &GTEST_FLAG(flagfile))) {
      LoadFlagsFromFile(GTEST_FLAG(flagfile));
      remove_flag = true;
#endif
    } else if (arg_string == "--help" || arg_string == "-h" ||
               arg_string == "-?" || arg_string == "/?" ||
               HasGoogleTestFlagPrefix(arg)) {

      g_help_flag = true;
    }

    if (remove_flag) {

      for (int j = i; j != *argc; j++) {
        argv[j] = argv[j + 1];
      }

      (*argc)--;

      i--;
    }
  }

  if (g_help_flag) {

    PrintColorEncoded(kColorEncodedHelpMessage);
  }
}

void ParseGoogleTestFlagsOnly(int* argc, char** argv) {
  ParseGoogleTestFlagsOnlyImpl(argc, argv);

#if GTEST_OS_MAC
#ifndef GTEST_OS_IOS
  if (*_NSGetArgv() == argv) {
    *_NSGetArgc() = *argc;
  }
#endif
#endif
}
void ParseGoogleTestFlagsOnly(int* argc, wchar_t** argv) {
  ParseGoogleTestFlagsOnlyImpl(argc, argv);
}

template <typename CharType>
void InitGoogleTestImpl(int* argc, CharType** argv) {

  if (GTestIsInitialized()) return;

  if (*argc <= 0) return;

  g_argvs.clear();
  for (int i = 0; i != *argc; i++) {
    g_argvs.push_back(StreamableToString(argv[i]));
  }

#if GTEST_HAS_ABSL
  absl::InitializeSymbolizer(g_argvs[0].c_str());
#endif

  ParseGoogleTestFlagsOnly(argc, argv);
  GetUnitTestImpl()->PostFlagParsingInit();
}

}

void InitGoogleTest(int* argc, char** argv) {
#if defined(GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
  GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_(argc, argv);
#else
  internal::InitGoogleTestImpl(argc, argv);
#endif
}

void InitGoogleTest(int* argc, wchar_t** argv) {
#if defined(GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
  GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_(argc, argv);
#else
  internal::InitGoogleTestImpl(argc, argv);
#endif
}

void InitGoogleTest() {

  int argc = 1;
  const auto arg0 = "dummy";
  char* argv0 = const_cast<char*>(arg0);
  char** argv = &argv0;

#if defined(GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_)
  GTEST_CUSTOM_INIT_GOOGLE_TEST_FUNCTION_(&argc, argv);
#else
  internal::InitGoogleTestImpl(&argc, argv);
#endif
}

std::string TempDir() {
#if defined(GTEST_CUSTOM_TEMPDIR_FUNCTION_)
  return GTEST_CUSTOM_TEMPDIR_FUNCTION_();
#elif GTEST_OS_WINDOWS_MOBILE
  return "\\temp\\";
#elif GTEST_OS_WINDOWS
  const char* temp_dir = internal::posix::GetEnv("TEMP");
  if (temp_dir == nullptr || temp_dir[0] == '\0') {
    return "\\temp\\";
  } else if (temp_dir[strlen(temp_dir) - 1] == '\\') {
    return temp_dir;
  } else {
    return std::string(temp_dir) + "\\";
  }
#elif GTEST_OS_LINUX_ANDROID
  const char* temp_dir = internal::posix::GetEnv("TEST_TMPDIR");
  if (temp_dir == nullptr || temp_dir[0] == '\0') {
    return "/data/local/tmp/";
  } else {
    return temp_dir;
  }
#elif GTEST_OS_LINUX
  const char* temp_dir = internal::posix::GetEnv("TEST_TMPDIR");
  if (temp_dir == nullptr || temp_dir[0] == '\0') {
    return "/tmp/";
  } else {
    return temp_dir;
  }
#else
  return "/tmp/";
#endif
}

void ScopedTrace::PushTrace(const char* file, int line, std::string message) {
  internal::TraceInfo trace;
  trace.file = file;
  trace.line = line;
  trace.message.swap(message);

  UnitTest::GetInstance()->PushGTestTrace(trace);
}

ScopedTrace::~ScopedTrace()
    GTEST_LOCK_EXCLUDED_(&UnitTest::mutex_) {
  UnitTest::GetInstance()->PopGTestTrace();
}

}

#include <functional>
#include <utility>

#if GTEST_HAS_DEATH_TEST

# if GTEST_OS_MAC
#  include <crt_externs.h>
# endif

# include <errno.h>
# include <fcntl.h>
# include <limits.h>

# if GTEST_OS_LINUX
#  include <signal.h>
# endif

# include <stdarg.h>

# if GTEST_OS_WINDOWS
#  include <windows.h>
# else
#  include <sys/mman.h>
#  include <sys/wait.h>
# endif

# if GTEST_OS_QNX
#  include <spawn.h>
# endif

# if GTEST_OS_FUCHSIA
#  include <lib/fdio/fd.h>
#  include <lib/fdio/io.h>
#  include <lib/fdio/spawn.h>
#  include <lib/zx/channel.h>
#  include <lib/zx/port.h>
#  include <lib/zx/process.h>
#  include <lib/zx/socket.h>
#  include <zircon/processargs.h>
#  include <zircon/syscalls.h>
#  include <zircon/syscalls/policy.h>
#  include <zircon/syscalls/port.h>
# endif

#endif

namespace testing {

static const char kDefaultDeathTestStyle[] = GTEST_DEFAULT_DEATH_TEST_STYLE;

GTEST_DEFINE_string_(
    death_test_style,
    internal::StringFromGTestEnv("death_test_style", kDefaultDeathTestStyle),
    "Indicates how to run a death test in a forked child process: "
    "\"threadsafe\" (child process re-executes the test binary "
    "from the beginning, running only the specific death test) or "
    "\"fast\" (child process runs the death test immediately "
    "after forking).");

GTEST_DEFINE_bool_(
    death_test_use_fork,
    internal::BoolFromGTestEnv("death_test_use_fork", false),
    "Instructs to use fork()/_exit() instead of clone() in death tests. "
    "Ignored and always uses fork() on POSIX systems where clone() is not "
    "implemented. Useful when running under valgrind or similar tools if "
    "those do not support clone(). Valgrind 3.3.1 will just fail if "
    "it sees an unsupported combination of clone() flags. "
    "It is not recommended to use this flag w/o valgrind though it will "
    "work in 99% of the cases. Once valgrind is fixed, this flag will "
    "most likely be removed.");

namespace internal {
GTEST_DEFINE_string_(
    internal_run_death_test, "",
    "Indicates the file, line number, temporal index of "
    "the single death test to run, and a file descriptor to "
    "which a success code may be sent, all separated by "
    "the '|' characters.  This flag is specified if and only if the "
    "current process is a sub-process launched for running a thread-safe "
    "death test.  FOR INTERNAL USE ONLY.");
}

#if GTEST_HAS_DEATH_TEST

namespace internal {

# if !GTEST_OS_WINDOWS && !GTEST_OS_FUCHSIA
static bool g_in_fast_death_test_child = false;
# endif

bool InDeathTestChild() {
# if GTEST_OS_WINDOWS || GTEST_OS_FUCHSIA

  return !GTEST_FLAG(internal_run_death_test).empty();

# else

  if (GTEST_FLAG(death_test_style) == "threadsafe")
    return !GTEST_FLAG(internal_run_death_test).empty();
  else
    return g_in_fast_death_test_child;
#endif
}

}

ExitedWithCode::ExitedWithCode(int exit_code) : exit_code_(exit_code) {
}

bool ExitedWithCode::operator()(int exit_status) const {
# if GTEST_OS_WINDOWS || GTEST_OS_FUCHSIA

  return exit_status == exit_code_;

# else

  return WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == exit_code_;

# endif
}

# if !GTEST_OS_WINDOWS && !GTEST_OS_FUCHSIA

KilledBySignal::KilledBySignal(int signum) : signum_(signum) {
}

bool KilledBySignal::operator()(int exit_status) const {
#  if defined(GTEST_KILLED_BY_SIGNAL_OVERRIDE_)
  {
    bool result;
    if (GTEST_KILLED_BY_SIGNAL_OVERRIDE_(signum_, exit_status, &result)) {
      return result;
    }
  }
#  endif
  return WIFSIGNALED(exit_status) && WTERMSIG(exit_status) == signum_;
}
# endif

namespace internal {

static std::string ExitSummary(int exit_code) {
  Message m;

# if GTEST_OS_WINDOWS || GTEST_OS_FUCHSIA

  m << "Exited with exit status " << exit_code;

# else

  if (WIFEXITED(exit_code)) {
    m << "Exited with exit status " << WEXITSTATUS(exit_code);
  } else if (WIFSIGNALED(exit_code)) {
    m << "Terminated by signal " << WTERMSIG(exit_code);
  }
#  ifdef WCOREDUMP
  if (WCOREDUMP(exit_code)) {
    m << " (core dumped)";
  }
#  endif
# endif

  return m.GetString();
}

bool ExitedUnsuccessfully(int exit_status) {
  return !ExitedWithCode(0)(exit_status);
}

# if !GTEST_OS_WINDOWS && !GTEST_OS_FUCHSIA

static std::string DeathTestThreadWarning(size_t thread_count) {
  Message msg;
  msg << "Death tests use fork(), which is unsafe particularly"
      << " in a threaded context. For this test, " << GTEST_NAME_ << " ";
  if (thread_count == 0) {
    msg << "couldn't detect the number of threads.";
  } else {
    msg << "detected " << thread_count << " threads.";
  }
  msg << " See "
         "https://github.com/google/googletest/blob/master/docs/"
         "advanced.md#death-tests-and-threads"
      << " for more explanation and suggested solutions, especially if"
      << " this is the last message you see before your test times out.";
  return msg.GetString();
}
# endif

static const char kDeathTestLived = 'L';
static const char kDeathTestReturned = 'R';
static const char kDeathTestThrew = 'T';
static const char kDeathTestInternalError = 'I';

#if GTEST_OS_FUCHSIA

static const int kFuchsiaReadPipeFd = 3;

#endif

enum DeathTestOutcome { IN_PROGRESS, DIED, LIVED, RETURNED, THREW };

static void DeathTestAbort(const std::string& message) {

  const InternalRunDeathTestFlag* const flag =
      GetUnitTestImpl()->internal_run_death_test_flag();
  if (flag != nullptr) {
    FILE* parent = posix::FDOpen(flag->write_fd(), "w");
    fputc(kDeathTestInternalError, parent);
    fprintf(parent, "%s", message.c_str());
    fflush(parent);
    _exit(1);
  } else {
    fprintf(stderr, "%s", message.c_str());
    fflush(stderr);
    posix::Abort();
  }
}

# define GTEST_DEATH_TEST_CHECK_(expression) \
  do { \
    if (!::testing::internal::IsTrue(expression)) { \
      DeathTestAbort( \
          ::std::string("CHECK failed: File ") + __FILE__ +  ", line " \
          + ::testing::internal::StreamableToString(__LINE__) + ": " \
          + #expression); \
    } \
  } while (::testing::internal::AlwaysFalse())

# define GTEST_DEATH_TEST_CHECK_SYSCALL_(expression) \
  do { \
    int gtest_retval; \
    do { \
      gtest_retval = (expression); \
    } while (gtest_retval == -1 && errno == EINTR); \
    if (gtest_retval == -1) { \
      DeathTestAbort( \
          ::std::string("CHECK failed: File ") + __FILE__ + ", line " \
          + ::testing::internal::StreamableToString(__LINE__) + ": " \
          + #expression + " != -1"); \
    } \
  } while (::testing::internal::AlwaysFalse())

std::string GetLastErrnoDescription() {
    return errno == 0 ? "" : posix::StrError(errno);
}

static void FailFromInternalError(int fd) {
  Message error;
  char buffer[256];
  int num_read;

  do {
    while ((num_read = posix::Read(fd, buffer, 255)) > 0) {
      buffer[num_read] = '\0';
      error << buffer;
    }
  } while (num_read == -1 && errno == EINTR);

  if (num_read == 0) {
    GTEST_LOG_(FATAL) << error.GetString();
  } else {
    const int last_error = errno;
    GTEST_LOG_(FATAL) << "Error while reading death test internal: "
                      << GetLastErrnoDescription() << " [" << last_error << "]";
  }
}

DeathTest::DeathTest() {
  TestInfo* const info = GetUnitTestImpl()->current_test_info();
  if (info == nullptr) {
    DeathTestAbort("Cannot run a death test outside of a TEST or "
                   "TEST_F construct");
  }
}

bool DeathTest::Create(const char* statement,
                       Matcher<const std::string&> matcher, const char* file,
                       int line, DeathTest** test) {
  return GetUnitTestImpl()->death_test_factory()->Create(
      statement, std::move(matcher), file, line, test);
}

const char* DeathTest::LastMessage() {
  return last_death_test_message_.c_str();
}

void DeathTest::set_last_death_test_message(const std::string& message) {
  last_death_test_message_ = message;
}

std::string DeathTest::last_death_test_message_;

class DeathTestImpl : public DeathTest {
 protected:
  DeathTestImpl(const char* a_statement, Matcher<const std::string&> matcher)
      : statement_(a_statement),
        matcher_(std::move(matcher)),
        spawned_(false),
        status_(-1),
        outcome_(IN_PROGRESS),
        read_fd_(-1),
        write_fd_(-1) {}

  ~DeathTestImpl() override { GTEST_DEATH_TEST_CHECK_(read_fd_ == -1); }

  void Abort(AbortReason reason) override;
  bool Passed(bool status_ok) override;

  const char* statement() const { return statement_; }
  bool spawned() const { return spawned_; }
  void set_spawned(bool is_spawned) { spawned_ = is_spawned; }
  int status() const { return status_; }
  void set_status(int a_status) { status_ = a_status; }
  DeathTestOutcome outcome() const { return outcome_; }
  void set_outcome(DeathTestOutcome an_outcome) { outcome_ = an_outcome; }
  int read_fd() const { return read_fd_; }
  void set_read_fd(int fd) { read_fd_ = fd; }
  int write_fd() const { return write_fd_; }
  void set_write_fd(int fd) { write_fd_ = fd; }

  void ReadAndInterpretStatusByte();

  virtual std::string GetErrorLogs();

 private:

  const char* const statement_;

  Matcher<const std::string&> matcher_;

  bool spawned_;

  int status_;

  DeathTestOutcome outcome_;

  int read_fd_;

  int write_fd_;
};

void DeathTestImpl::ReadAndInterpretStatusByte() {
  char flag;
  int bytes_read;

  do {
    bytes_read = posix::Read(read_fd(), &flag, 1);
  } while (bytes_read == -1 && errno == EINTR);

  if (bytes_read == 0) {
    set_outcome(DIED);
  } else if (bytes_read == 1) {
    switch (flag) {
      case kDeathTestReturned:
        set_outcome(RETURNED);
        break;
      case kDeathTestThrew:
        set_outcome(THREW);
        break;
      case kDeathTestLived:
        set_outcome(LIVED);
        break;
      case kDeathTestInternalError:
        FailFromInternalError(read_fd());
        break;
      default:
        GTEST_LOG_(FATAL) << "Death test child process reported "
                          << "unexpected status byte ("
                          << static_cast<unsigned int>(flag) << ")";
    }
  } else {
    GTEST_LOG_(FATAL) << "Read from death test child process failed: "
                      << GetLastErrnoDescription();
  }
  GTEST_DEATH_TEST_CHECK_SYSCALL_(posix::Close(read_fd()));
  set_read_fd(-1);
}

std::string DeathTestImpl::GetErrorLogs() {
  return GetCapturedStderr();
}

void DeathTestImpl::Abort(AbortReason reason) {

  const char status_ch =
      reason == TEST_DID_NOT_DIE ? kDeathTestLived :
      reason == TEST_THREW_EXCEPTION ? kDeathTestThrew : kDeathTestReturned;

  GTEST_DEATH_TEST_CHECK_SYSCALL_(posix::Write(write_fd(), &status_ch, 1));

  _exit(1);
}

static ::std::string FormatDeathTestOutput(const ::std::string& output) {
  ::std::string ret;
  for (size_t at = 0; ; ) {
    const size_t line_end = output.find('\n', at);
    ret += "[  DEATH   ] ";
    if (line_end == ::std::string::npos) {
      ret += output.substr(at);
      break;
    }
    ret += output.substr(at, line_end + 1 - at);
    at = line_end + 1;
  }
  return ret;
}

bool DeathTestImpl::Passed(bool status_ok) {
  if (!spawned())
    return false;

  const std::string error_message = GetErrorLogs();

  bool success = false;
  Message buffer;

  buffer << "Death test: " << statement() << "\n";
  switch (outcome()) {
    case LIVED:
      buffer << "    Result: failed to die.\n"
             << " Error msg:\n" << FormatDeathTestOutput(error_message);
      break;
    case THREW:
      buffer << "    Result: threw an exception.\n"
             << " Error msg:\n" << FormatDeathTestOutput(error_message);
      break;
    case RETURNED:
      buffer << "    Result: illegal return in test statement.\n"
             << " Error msg:\n" << FormatDeathTestOutput(error_message);
      break;
    case DIED:
      if (status_ok) {
        if (matcher_.Matches(error_message)) {
          success = true;
        } else {
          std::ostringstream stream;
          matcher_.DescribeTo(&stream);
          buffer << "    Result: died but not with expected error.\n"
                 << "  Expected: " << stream.str() << "\n"
                 << "Actual msg:\n"
                 << FormatDeathTestOutput(error_message);
        }
      } else {
        buffer << "    Result: died but not with expected exit code:\n"
               << "            " << ExitSummary(status()) << "\n"
               << "Actual msg:\n" << FormatDeathTestOutput(error_message);
      }
      break;
    case IN_PROGRESS:
    default:
      GTEST_LOG_(FATAL)
          << "DeathTest::Passed somehow called before conclusion of test";
  }

  DeathTest::set_last_death_test_message(buffer.GetString());
  return success;
}

# if GTEST_OS_WINDOWS

class WindowsDeathTest : public DeathTestImpl {
 public:
  WindowsDeathTest(const char* a_statement, Matcher<const std::string&> matcher,
                   const char* file, int line)
      : DeathTestImpl(a_statement, std::move(matcher)),
        file_(file),
        line_(line) {}

  virtual int Wait();
  virtual TestRole AssumeRole();

 private:

  const char* const file_;

  const int line_;

  AutoHandle write_handle_;

  AutoHandle child_handle_;

  AutoHandle event_handle_;
};

int WindowsDeathTest::Wait() {
  if (!spawned())
    return 0;

  const HANDLE wait_handles[2] = { child_handle_.Get(), event_handle_.Get() };
  switch (::WaitForMultipleObjects(2,
                                   wait_handles,
                                   FALSE,
                                   INFINITE)) {
    case WAIT_OBJECT_0:
    case WAIT_OBJECT_0 + 1:
      break;
    default:
      GTEST_DEATH_TEST_CHECK_(false);
  }

  write_handle_.Reset();
  event_handle_.Reset();

  ReadAndInterpretStatusByte();

  GTEST_DEATH_TEST_CHECK_(
      WAIT_OBJECT_0 == ::WaitForSingleObject(child_handle_.Get(),
                                             INFINITE));
  DWORD status_code;
  GTEST_DEATH_TEST_CHECK_(
      ::GetExitCodeProcess(child_handle_.Get(), &status_code) != FALSE);
  child_handle_.Reset();
  set_status(static_cast<int>(status_code));
  return status();
}

DeathTest::TestRole WindowsDeathTest::AssumeRole() {
  const UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const TestInfo* const info = impl->current_test_info();
  const int death_test_index = info->result()->death_test_count();

  if (flag != nullptr) {

    set_write_fd(flag->write_fd());
    return EXECUTE_TEST;
  }

  SECURITY_ATTRIBUTES handles_are_inheritable = {sizeof(SECURITY_ATTRIBUTES),
                                                 nullptr, TRUE};
  HANDLE read_handle, write_handle;
  GTEST_DEATH_TEST_CHECK_(
      ::CreatePipe(&read_handle, &write_handle, &handles_are_inheritable,
                   0)
      != FALSE);
  set_read_fd(::_open_osfhandle(reinterpret_cast<intptr_t>(read_handle),
                                O_RDONLY));
  write_handle_.Reset(write_handle);
  event_handle_.Reset(::CreateEvent(
      &handles_are_inheritable,
      TRUE,
      FALSE,
      nullptr));
  GTEST_DEATH_TEST_CHECK_(event_handle_.Get() != nullptr);
  const std::string filter_flag = std::string("--") + GTEST_FLAG_PREFIX_ +
                                  kFilterFlag + "=" + info->test_suite_name() +
                                  "." + info->name();
  const std::string internal_flag =
      std::string("--") + GTEST_FLAG_PREFIX_ + kInternalRunDeathTestFlag +
      "=" + file_ + "|" + StreamableToString(line_) + "|" +
      StreamableToString(death_test_index) + "|" +
      StreamableToString(static_cast<unsigned int>(::GetCurrentProcessId())) +

      "|" + StreamableToString(reinterpret_cast<size_t>(write_handle)) +
      "|" + StreamableToString(reinterpret_cast<size_t>(event_handle_.Get()));

  char executable_path[_MAX_PATH + 1];
  GTEST_DEATH_TEST_CHECK_(_MAX_PATH + 1 != ::GetModuleFileNameA(nullptr,
                                                                executable_path,
                                                                _MAX_PATH));

  std::string command_line =
      std::string(::GetCommandLineA()) + " " + filter_flag + " \"" +
      internal_flag + "\"";

  DeathTest::set_last_death_test_message("");

  CaptureStderr();

  FlushInfoLog();

  STARTUPINFOA startup_info;
  memset(&startup_info, 0, sizeof(STARTUPINFO));
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
  startup_info.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
  startup_info.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);

  PROCESS_INFORMATION process_info;
  GTEST_DEATH_TEST_CHECK_(
      ::CreateProcessA(
          executable_path, const_cast<char*>(command_line.c_str()),
          nullptr,
          nullptr,
          TRUE,
          0x0,
          nullptr,
          UnitTest::GetInstance()->original_working_dir(), &startup_info,
          &process_info) != FALSE);
  child_handle_.Reset(process_info.hProcess);
  ::CloseHandle(process_info.hThread);
  set_spawned(true);
  return OVERSEE_TEST;
}

# elif GTEST_OS_FUCHSIA

class FuchsiaDeathTest : public DeathTestImpl {
 public:
  FuchsiaDeathTest(const char* a_statement, Matcher<const std::string&> matcher,
                   const char* file, int line)
      : DeathTestImpl(a_statement, std::move(matcher)),
        file_(file),
        line_(line) {}

  int Wait() override;
  TestRole AssumeRole() override;
  std::string GetErrorLogs() override;

 private:

  const char* const file_;

  const int line_;

  std::string captured_stderr_;

  zx::process child_process_;
  zx::channel exception_channel_;
  zx::socket stderr_socket_;
};

class Arguments {
 public:
  Arguments() { args_.push_back(nullptr); }

  ~Arguments() {
    for (std::vector<char*>::iterator i = args_.begin(); i != args_.end();
         ++i) {
      free(*i);
    }
  }
  void AddArgument(const char* argument) {
    args_.insert(args_.end() - 1, posix::StrDup(argument));
  }

  template <typename Str>
  void AddArguments(const ::std::vector<Str>& arguments) {
    for (typename ::std::vector<Str>::const_iterator i = arguments.begin();
         i != arguments.end();
         ++i) {
      args_.insert(args_.end() - 1, posix::StrDup(i->c_str()));
    }
  }
  char* const* Argv() {
    return &args_[0];
  }

  int size() {
    return static_cast<int>(args_.size()) - 1;
  }

 private:
  std::vector<char*> args_;
};

int FuchsiaDeathTest::Wait() {
  const int kProcessKey = 0;
  const int kSocketKey = 1;
  const int kExceptionKey = 2;

  if (!spawned())
    return 0;

  zx_status_t status_zx;
  zx::port port;
  status_zx = zx::port::create(0, &port);
  GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);

  status_zx = child_process_.wait_async(
      port, kProcessKey, ZX_PROCESS_TERMINATED, 0);
  GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);

  status_zx = stderr_socket_.wait_async(
      port, kSocketKey, ZX_SOCKET_READABLE | ZX_SOCKET_PEER_CLOSED, 0);
  GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);

  status_zx = exception_channel_.wait_async(
      port, kExceptionKey, ZX_CHANNEL_READABLE, 0);
  GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);

  bool process_terminated = false;
  bool socket_closed = false;
  do {
    zx_port_packet_t packet = {};
    status_zx = port.wait(zx::time::infinite(), &packet);
    GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);

    if (packet.key == kExceptionKey) {

      status_zx = child_process_.kill();
      GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);
    } else if (packet.key == kProcessKey) {

      GTEST_DEATH_TEST_CHECK_(ZX_PKT_IS_SIGNAL_ONE(packet.type));
      GTEST_DEATH_TEST_CHECK_(packet.signal.observed & ZX_PROCESS_TERMINATED);
      process_terminated = true;
    } else if (packet.key == kSocketKey) {
      GTEST_DEATH_TEST_CHECK_(ZX_PKT_IS_SIGNAL_ONE(packet.type));
      if (packet.signal.observed & ZX_SOCKET_READABLE) {

        constexpr size_t kBufferSize = 1024;
        do {
          size_t old_length = captured_stderr_.length();
          size_t bytes_read = 0;
          captured_stderr_.resize(old_length + kBufferSize);
          status_zx = stderr_socket_.read(
              0, &captured_stderr_.front() + old_length, kBufferSize,
              &bytes_read);
          captured_stderr_.resize(old_length + bytes_read);
        } while (status_zx == ZX_OK);
        if (status_zx == ZX_ERR_PEER_CLOSED) {
          socket_closed = true;
        } else {
          GTEST_DEATH_TEST_CHECK_(status_zx == ZX_ERR_SHOULD_WAIT);
          status_zx = stderr_socket_.wait_async(
              port, kSocketKey, ZX_SOCKET_READABLE | ZX_SOCKET_PEER_CLOSED, 0);
          GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);
        }
      } else {
        GTEST_DEATH_TEST_CHECK_(packet.signal.observed & ZX_SOCKET_PEER_CLOSED);
        socket_closed = true;
      }
    }
  } while (!process_terminated && !socket_closed);

  ReadAndInterpretStatusByte();

  zx_info_process_v2_t buffer;
  status_zx = child_process_.get_info(
      ZX_INFO_PROCESS_V2, &buffer, sizeof(buffer), nullptr, nullptr);
  GTEST_DEATH_TEST_CHECK_(status_zx == ZX_OK);

  GTEST_DEATH_TEST_CHECK_(buffer.flags & ZX_INFO_PROCESS_FLAG_EXITED);
  set_status(static_cast<int>(buffer.return_code));
  return status();
}

DeathTest::TestRole FuchsiaDeathTest::AssumeRole() {
  const UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const TestInfo* const info = impl->current_test_info();
  const int death_test_index = info->result()->death_test_count();

  if (flag != nullptr) {

    set_write_fd(kFuchsiaReadPipeFd);
    return EXECUTE_TEST;
  }

  FlushInfoLog();

  const std::string filter_flag = std::string("--") + GTEST_FLAG_PREFIX_ +
                                  kFilterFlag + "=" + info->test_suite_name() +
                                  "." + info->name();
  const std::string internal_flag =
      std::string("--") + GTEST_FLAG_PREFIX_ + kInternalRunDeathTestFlag + "="
      + file_ + "|"
      + StreamableToString(line_) + "|"
      + StreamableToString(death_test_index);
  Arguments args;
  args.AddArguments(GetInjectableArgvs());
  args.AddArgument(filter_flag.c_str());
  args.AddArgument(internal_flag.c_str());

  zx_status_t status;
  zx_handle_t child_pipe_handle;
  int child_pipe_fd;
  status = fdio_pipe_half(&child_pipe_fd, &child_pipe_handle);
  GTEST_DEATH_TEST_CHECK_(status == ZX_OK);
  set_read_fd(child_pipe_fd);

  fdio_spawn_action_t spawn_actions[2] = {};
  fdio_spawn_action_t* add_handle_action = &spawn_actions[0];
  add_handle_action->action = FDIO_SPAWN_ACTION_ADD_HANDLE;
  add_handle_action->h.id = PA_HND(PA_FD, kFuchsiaReadPipeFd);
  add_handle_action->h.handle = child_pipe_handle;

  zx::socket stderr_producer_socket;
  status =
      zx::socket::create(0, &stderr_producer_socket, &stderr_socket_);
  GTEST_DEATH_TEST_CHECK_(status >= 0);
  int stderr_producer_fd = -1;
  status =
      fdio_fd_create(stderr_producer_socket.release(), &stderr_producer_fd);
  GTEST_DEATH_TEST_CHECK_(status >= 0);

  GTEST_DEATH_TEST_CHECK_(fcntl(stderr_producer_fd, F_SETFL, 0) == 0);

  fdio_spawn_action_t* add_stderr_action = &spawn_actions[1];
  add_stderr_action->action = FDIO_SPAWN_ACTION_CLONE_FD;
  add_stderr_action->fd.local_fd = stderr_producer_fd;
  add_stderr_action->fd.target_fd = STDERR_FILENO;

  zx_handle_t child_job = ZX_HANDLE_INVALID;
  status = zx_job_create(zx_job_default(), 0, & child_job);
  GTEST_DEATH_TEST_CHECK_(status == ZX_OK);
  zx_policy_basic_t policy;
  policy.condition = ZX_POL_NEW_ANY;
  policy.policy = ZX_POL_ACTION_ALLOW;
  status = zx_job_set_policy(
      child_job, ZX_JOB_POL_RELATIVE, ZX_JOB_POL_BASIC, &policy, 1);
  GTEST_DEATH_TEST_CHECK_(status == ZX_OK);

  status =
      zx_task_create_exception_channel(
          child_job, 0, exception_channel_.reset_and_get_address());
  GTEST_DEATH_TEST_CHECK_(status == ZX_OK);

  status = fdio_spawn_etc(
      child_job, FDIO_SPAWN_CLONE_ALL, args.Argv()[0], args.Argv(), nullptr,
      2, spawn_actions, child_process_.reset_and_get_address(), nullptr);
  GTEST_DEATH_TEST_CHECK_(status == ZX_OK);

  set_spawned(true);
  return OVERSEE_TEST;
}

std::string FuchsiaDeathTest::GetErrorLogs() {
  return captured_stderr_;
}

#else

class ForkingDeathTest : public DeathTestImpl {
 public:
  ForkingDeathTest(const char* statement, Matcher<const std::string&> matcher);

  int Wait() override;

 protected:
  void set_child_pid(pid_t child_pid) { child_pid_ = child_pid; }

 private:

  pid_t child_pid_;
};

ForkingDeathTest::ForkingDeathTest(const char* a_statement,
                                   Matcher<const std::string&> matcher)
    : DeathTestImpl(a_statement, std::move(matcher)), child_pid_(-1) {}

int ForkingDeathTest::Wait() {
  if (!spawned())
    return 0;

  ReadAndInterpretStatusByte();

  int status_value;
  GTEST_DEATH_TEST_CHECK_SYSCALL_(waitpid(child_pid_, &status_value, 0));
  set_status(status_value);
  return status_value;
}

class NoExecDeathTest : public ForkingDeathTest {
 public:
  NoExecDeathTest(const char* a_statement, Matcher<const std::string&> matcher)
      : ForkingDeathTest(a_statement, std::move(matcher)) {}
  TestRole AssumeRole() override;
};

DeathTest::TestRole NoExecDeathTest::AssumeRole() {
  const size_t thread_count = GetThreadCount();
  if (thread_count != 1) {
    GTEST_LOG_(WARNING) << DeathTestThreadWarning(thread_count);
  }

  int pipe_fd[2];
  GTEST_DEATH_TEST_CHECK_(pipe(pipe_fd) != -1);

  DeathTest::set_last_death_test_message("");
  CaptureStderr();

  FlushInfoLog();

  const pid_t child_pid = fork();
  GTEST_DEATH_TEST_CHECK_(child_pid != -1);
  set_child_pid(child_pid);
  if (child_pid == 0) {
    GTEST_DEATH_TEST_CHECK_SYSCALL_(close(pipe_fd[0]));
    set_write_fd(pipe_fd[1]);

    LogToStderr();

    GetUnitTestImpl()->listeners()->SuppressEventForwarding();
    g_in_fast_death_test_child = true;
    return EXECUTE_TEST;
  } else {
    GTEST_DEATH_TEST_CHECK_SYSCALL_(close(pipe_fd[1]));
    set_read_fd(pipe_fd[0]);
    set_spawned(true);
    return OVERSEE_TEST;
  }
}

class ExecDeathTest : public ForkingDeathTest {
 public:
  ExecDeathTest(const char* a_statement, Matcher<const std::string&> matcher,
                const char* file, int line)
      : ForkingDeathTest(a_statement, std::move(matcher)),
        file_(file),
        line_(line) {}
  TestRole AssumeRole() override;

 private:
  static ::std::vector<std::string> GetArgvsForDeathTestChildProcess() {
    ::std::vector<std::string> args = GetInjectableArgvs();
#  if defined(GTEST_EXTRA_DEATH_TEST_COMMAND_LINE_ARGS_)
    ::std::vector<std::string> extra_args =
        GTEST_EXTRA_DEATH_TEST_COMMAND_LINE_ARGS_();
    args.insert(args.end(), extra_args.begin(), extra_args.end());
#  endif
    return args;
  }

  const char* const file_;

  const int line_;
};

class Arguments {
 public:
  Arguments() { args_.push_back(nullptr); }

  ~Arguments() {
    for (std::vector<char*>::iterator i = args_.begin(); i != args_.end();
         ++i) {
      free(*i);
    }
  }
  void AddArgument(const char* argument) {
    args_.insert(args_.end() - 1, posix::StrDup(argument));
  }

  template <typename Str>
  void AddArguments(const ::std::vector<Str>& arguments) {
    for (typename ::std::vector<Str>::const_iterator i = arguments.begin();
         i != arguments.end();
         ++i) {
      args_.insert(args_.end() - 1, posix::StrDup(i->c_str()));
    }
  }
  char* const* Argv() {
    return &args_[0];
  }

 private:
  std::vector<char*> args_;
};

struct ExecDeathTestArgs {
  char* const* argv;
  int close_fd;
};

#  if GTEST_OS_QNX
extern "C" char** environ;
#  else

static int ExecDeathTestChildMain(void* child_arg) {
  ExecDeathTestArgs* const args = static_cast<ExecDeathTestArgs*>(child_arg);
  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(args->close_fd));

  const char* const original_dir =
      UnitTest::GetInstance()->original_working_dir();

  if (chdir(original_dir) != 0) {
    DeathTestAbort(std::string("chdir(\"") + original_dir + "\") failed: " +
                   GetLastErrnoDescription());
    return EXIT_FAILURE;
  }

  execv(args->argv[0], args->argv);
  DeathTestAbort(std::string("execv(") + args->argv[0] + ", ...) in " +
                 original_dir + " failed: " +
                 GetLastErrnoDescription());
  return EXIT_FAILURE;
}
#  endif

#  if GTEST_HAS_CLONE

static void StackLowerThanAddress(const void* ptr,
                                  bool* result) GTEST_NO_INLINE_;

GTEST_ATTRIBUTE_NO_SANITIZE_ADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_HWADDRESS_
static void StackLowerThanAddress(const void* ptr, bool* result) {
  int dummy = 0;
  *result = std::less<const void*>()(&dummy, ptr);
}

GTEST_ATTRIBUTE_NO_SANITIZE_ADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_HWADDRESS_
static bool StackGrowsDown() {
  int dummy = 0;
  bool result;
  StackLowerThanAddress(&dummy, &result);
  return result;
}
#  endif

static pid_t ExecDeathTestSpawnChild(char* const* argv, int close_fd) {
  ExecDeathTestArgs args = { argv, close_fd };
  pid_t child_pid = -1;

#  if GTEST_OS_QNX

  const int cwd_fd = open(".", O_RDONLY);
  GTEST_DEATH_TEST_CHECK_(cwd_fd != -1);
  GTEST_DEATH_TEST_CHECK_SYSCALL_(fcntl(cwd_fd, F_SETFD, FD_CLOEXEC));

  const char* const original_dir =
      UnitTest::GetInstance()->original_working_dir();

  if (chdir(original_dir) != 0) {
    DeathTestAbort(std::string("chdir(\"") + original_dir + "\") failed: " +
                   GetLastErrnoDescription());
    return EXIT_FAILURE;
  }

  int fd_flags;

  GTEST_DEATH_TEST_CHECK_SYSCALL_(fd_flags = fcntl(close_fd, F_GETFD));
  GTEST_DEATH_TEST_CHECK_SYSCALL_(fcntl(close_fd, F_SETFD,
                                        fd_flags | FD_CLOEXEC));
  struct inheritance inherit = {0};

  child_pid = spawn(args.argv[0], 0, nullptr, &inherit, args.argv, environ);

  GTEST_DEATH_TEST_CHECK_(fchdir(cwd_fd) != -1);
  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(cwd_fd));

#  else
#   if GTEST_OS_LINUX

  struct sigaction saved_sigprof_action;
  struct sigaction ignore_sigprof_action;
  memset(&ignore_sigprof_action, 0, sizeof(ignore_sigprof_action));
  sigemptyset(&ignore_sigprof_action.sa_mask);
  ignore_sigprof_action.sa_handler = SIG_IGN;
  GTEST_DEATH_TEST_CHECK_SYSCALL_(sigaction(
      SIGPROF, &ignore_sigprof_action, &saved_sigprof_action));
#   endif

#   if GTEST_HAS_CLONE
  const bool use_fork = GTEST_FLAG(death_test_use_fork);

  if (!use_fork) {
    static const bool stack_grows_down = StackGrowsDown();
    const auto stack_size = static_cast<size_t>(getpagesize() * 2);

    void* const stack = mmap(nullptr, stack_size, PROT_READ | PROT_WRITE,
                             MAP_ANON | MAP_PRIVATE, -1, 0);
    GTEST_DEATH_TEST_CHECK_(stack != MAP_FAILED);

    const size_t kMaxStackAlignment = 64;
    void* const stack_top =
        static_cast<char*>(stack) +
            (stack_grows_down ? stack_size - kMaxStackAlignment : 0);
    GTEST_DEATH_TEST_CHECK_(
        static_cast<size_t>(stack_size) > kMaxStackAlignment &&
        reinterpret_cast<uintptr_t>(stack_top) % kMaxStackAlignment == 0);

    child_pid = clone(&ExecDeathTestChildMain, stack_top, SIGCHLD, &args);

    GTEST_DEATH_TEST_CHECK_(munmap(stack, stack_size) != -1);
  }
#   else
  const bool use_fork = true;
#   endif

  if (use_fork && (child_pid = fork()) == 0) {
      ExecDeathTestChildMain(&args);
      _exit(0);
  }
#  endif
#  if GTEST_OS_LINUX
  GTEST_DEATH_TEST_CHECK_SYSCALL_(
      sigaction(SIGPROF, &saved_sigprof_action, nullptr));
#  endif

  GTEST_DEATH_TEST_CHECK_(child_pid != -1);
  return child_pid;
}

DeathTest::TestRole ExecDeathTest::AssumeRole() {
  const UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const TestInfo* const info = impl->current_test_info();
  const int death_test_index = info->result()->death_test_count();

  if (flag != nullptr) {
    set_write_fd(flag->write_fd());
    return EXECUTE_TEST;
  }

  int pipe_fd[2];
  GTEST_DEATH_TEST_CHECK_(pipe(pipe_fd) != -1);

  GTEST_DEATH_TEST_CHECK_(fcntl(pipe_fd[1], F_SETFD, 0) != -1);

  const std::string filter_flag = std::string("--") + GTEST_FLAG_PREFIX_ +
                                  kFilterFlag + "=" + info->test_suite_name() +
                                  "." + info->name();
  const std::string internal_flag =
      std::string("--") + GTEST_FLAG_PREFIX_ + kInternalRunDeathTestFlag + "="
      + file_ + "|" + StreamableToString(line_) + "|"
      + StreamableToString(death_test_index) + "|"
      + StreamableToString(pipe_fd[1]);
  Arguments args;
  args.AddArguments(GetArgvsForDeathTestChildProcess());
  args.AddArgument(filter_flag.c_str());
  args.AddArgument(internal_flag.c_str());

  DeathTest::set_last_death_test_message("");

  CaptureStderr();

  FlushInfoLog();

  const pid_t child_pid = ExecDeathTestSpawnChild(args.Argv(), pipe_fd[0]);
  GTEST_DEATH_TEST_CHECK_SYSCALL_(close(pipe_fd[1]));
  set_child_pid(child_pid);
  set_read_fd(pipe_fd[0]);
  set_spawned(true);
  return OVERSEE_TEST;
}

# endif

bool DefaultDeathTestFactory::Create(const char* statement,
                                     Matcher<const std::string&> matcher,
                                     const char* file, int line,
                                     DeathTest** test) {
  UnitTestImpl* const impl = GetUnitTestImpl();
  const InternalRunDeathTestFlag* const flag =
      impl->internal_run_death_test_flag();
  const int death_test_index = impl->current_test_info()
      ->increment_death_test_count();

  if (flag != nullptr) {
    if (death_test_index > flag->index()) {
      DeathTest::set_last_death_test_message(
          "Death test count (" + StreamableToString(death_test_index)
          + ") somehow exceeded expected maximum ("
          + StreamableToString(flag->index()) + ")");
      return false;
    }

    if (!(flag->file() == file && flag->line() == line &&
          flag->index() == death_test_index)) {
      *test = nullptr;
      return true;
    }
  }

# if GTEST_OS_WINDOWS

  if (GTEST_FLAG(death_test_style) == "threadsafe" ||
      GTEST_FLAG(death_test_style) == "fast") {
    *test = new WindowsDeathTest(statement, std::move(matcher), file, line);
  }

# elif GTEST_OS_FUCHSIA

  if (GTEST_FLAG(death_test_style) == "threadsafe" ||
      GTEST_FLAG(death_test_style) == "fast") {
    *test = new FuchsiaDeathTest(statement, std::move(matcher), file, line);
  }

# else

  if (GTEST_FLAG(death_test_style) == "threadsafe") {
    *test = new ExecDeathTest(statement, std::move(matcher), file, line);
  } else if (GTEST_FLAG(death_test_style) == "fast") {
    *test = new NoExecDeathTest(statement, std::move(matcher));
  }

# endif

  else {
    DeathTest::set_last_death_test_message(
        "Unknown death test style \"" + GTEST_FLAG(death_test_style)
        + "\" encountered");
    return false;
  }

  return true;
}

# if GTEST_OS_WINDOWS

static int GetStatusFileDescriptor(unsigned int parent_process_id,
                            size_t write_handle_as_size_t,
                            size_t event_handle_as_size_t) {
  AutoHandle parent_process_handle(::OpenProcess(PROCESS_DUP_HANDLE,
                                                   FALSE,
                                                   parent_process_id));
  if (parent_process_handle.Get() == INVALID_HANDLE_VALUE) {
    DeathTestAbort("Unable to open parent process " +
                   StreamableToString(parent_process_id));
  }

  GTEST_CHECK_(sizeof(HANDLE) <= sizeof(size_t));

  const HANDLE write_handle =
      reinterpret_cast<HANDLE>(write_handle_as_size_t);
  HANDLE dup_write_handle;

  if (!::DuplicateHandle(parent_process_handle.Get(), write_handle,
                         ::GetCurrentProcess(), &dup_write_handle,
                         0x0,

                         FALSE,
                         DUPLICATE_SAME_ACCESS)) {
    DeathTestAbort("Unable to duplicate the pipe handle " +
                   StreamableToString(write_handle_as_size_t) +
                   " from the parent process " +
                   StreamableToString(parent_process_id));
  }

  const HANDLE event_handle = reinterpret_cast<HANDLE>(event_handle_as_size_t);
  HANDLE dup_event_handle;

  if (!::DuplicateHandle(parent_process_handle.Get(), event_handle,
                         ::GetCurrentProcess(), &dup_event_handle,
                         0x0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS)) {
    DeathTestAbort("Unable to duplicate the event handle " +
                   StreamableToString(event_handle_as_size_t) +
                   " from the parent process " +
                   StreamableToString(parent_process_id));
  }

  const int write_fd =
      ::_open_osfhandle(reinterpret_cast<intptr_t>(dup_write_handle), O_APPEND);
  if (write_fd == -1) {
    DeathTestAbort("Unable to convert pipe handle " +
                   StreamableToString(write_handle_as_size_t) +
                   " to a file descriptor");
  }

  ::SetEvent(dup_event_handle);

  return write_fd;
}
# endif

InternalRunDeathTestFlag* ParseInternalRunDeathTestFlag() {
  if (GTEST_FLAG(internal_run_death_test) == "") return nullptr;

  int line = -1;
  int index = -1;
  ::std::vector< ::std::string> fields;
  SplitString(GTEST_FLAG(internal_run_death_test).c_str(), '|', &fields);
  int write_fd = -1;

# if GTEST_OS_WINDOWS

  unsigned int parent_process_id = 0;
  size_t write_handle_as_size_t = 0;
  size_t event_handle_as_size_t = 0;

  if (fields.size() != 6
      || !ParseNaturalNumber(fields[1], &line)
      || !ParseNaturalNumber(fields[2], &index)
      || !ParseNaturalNumber(fields[3], &parent_process_id)
      || !ParseNaturalNumber(fields[4], &write_handle_as_size_t)
      || !ParseNaturalNumber(fields[5], &event_handle_as_size_t)) {
    DeathTestAbort("Bad --gtest_internal_run_death_test flag: " +
                   GTEST_FLAG(internal_run_death_test));
  }
  write_fd = GetStatusFileDescriptor(parent_process_id,
                                     write_handle_as_size_t,
                                     event_handle_as_size_t);

# elif GTEST_OS_FUCHSIA

  if (fields.size() != 3
      || !ParseNaturalNumber(fields[1], &line)
      || !ParseNaturalNumber(fields[2], &index)) {
    DeathTestAbort("Bad --gtest_internal_run_death_test flag: "
        + GTEST_FLAG(internal_run_death_test));
  }

# else

  if (fields.size() != 4
      || !ParseNaturalNumber(fields[1], &line)
      || !ParseNaturalNumber(fields[2], &index)
      || !ParseNaturalNumber(fields[3], &write_fd)) {
    DeathTestAbort("Bad --gtest_internal_run_death_test flag: "
        + GTEST_FLAG(internal_run_death_test));
  }

# endif

  return new InternalRunDeathTestFlag(fields[0], line, index, write_fd);
}

}

#endif

}

#include <stdlib.h>

#if GTEST_OS_WINDOWS_MOBILE
# include <windows.h>
#elif GTEST_OS_WINDOWS
# include <direct.h>
# include <io.h>
#else
# include <limits.h>
# include <climits>
#endif

#if GTEST_OS_WINDOWS
# define GTEST_PATH_MAX_ _MAX_PATH
#elif defined(PATH_MAX)
# define GTEST_PATH_MAX_ PATH_MAX
#elif defined(_XOPEN_PATH_MAX)
# define GTEST_PATH_MAX_ _XOPEN_PATH_MAX
#else
# define GTEST_PATH_MAX_ _POSIX_PATH_MAX
#endif

namespace testing {
namespace internal {

#if GTEST_OS_WINDOWS

const char kPathSeparator = '\\';
const char kAlternatePathSeparator = '/';
const char kAlternatePathSeparatorString[] = "/";
# if GTEST_OS_WINDOWS_MOBILE

const char kCurrentDirectoryString[] = "\\";

const DWORD kInvalidFileAttributes = 0xffffffff;
# else
const char kCurrentDirectoryString[] = ".\\";
# endif
#else
const char kPathSeparator = '/';
const char kCurrentDirectoryString[] = "./";
#endif

static bool IsPathSeparator(char c) {
#if GTEST_HAS_ALT_PATH_SEP_
  return (c == kPathSeparator) || (c == kAlternatePathSeparator);
#else
  return c == kPathSeparator;
#endif
}

FilePath FilePath::GetCurrentDir() {
#if GTEST_OS_WINDOWS_MOBILE || GTEST_OS_WINDOWS_PHONE ||         \
    GTEST_OS_WINDOWS_RT || GTEST_OS_ESP8266 || GTEST_OS_ESP32 || \
    GTEST_OS_XTENSA

  return FilePath(kCurrentDirectoryString);
#elif GTEST_OS_WINDOWS
  char cwd[GTEST_PATH_MAX_ + 1] = { '\0' };
  return FilePath(_getcwd(cwd, sizeof(cwd)) == nullptr ? "" : cwd);
#else
  char cwd[GTEST_PATH_MAX_ + 1] = { '\0' };
  char* result = getcwd(cwd, sizeof(cwd));
# if GTEST_OS_NACL

  return FilePath(result == nullptr ? kCurrentDirectoryString : cwd);
# endif
  return FilePath(result == nullptr ? "" : cwd);
#endif
}

FilePath FilePath::RemoveExtension(const char* extension) const {
  const std::string dot_extension = std::string(".") + extension;
  if (String::EndsWithCaseInsensitive(pathname_, dot_extension)) {
    return FilePath(pathname_.substr(
        0, pathname_.length() - dot_extension.length()));
  }
  return *this;
}

const char* FilePath::FindLastPathSeparator() const {
  const char* const last_sep = strrchr(c_str(), kPathSeparator);
#if GTEST_HAS_ALT_PATH_SEP_
  const char* const last_alt_sep = strrchr(c_str(), kAlternatePathSeparator);

  if (last_alt_sep != nullptr &&
      (last_sep == nullptr || last_alt_sep > last_sep)) {
    return last_alt_sep;
  }
#endif
  return last_sep;
}

FilePath FilePath::RemoveDirectoryName() const {
  const char* const last_sep = FindLastPathSeparator();
  return last_sep ? FilePath(last_sep + 1) : *this;
}

FilePath FilePath::RemoveFileName() const {
  const char* const last_sep = FindLastPathSeparator();
  std::string dir;
  if (last_sep) {
    dir = std::string(c_str(), static_cast<size_t>(last_sep + 1 - c_str()));
  } else {
    dir = kCurrentDirectoryString;
  }
  return FilePath(dir);
}

FilePath FilePath::MakeFileName(const FilePath& directory,
                                const FilePath& base_name,
                                int number,
                                const char* extension) {
  std::string file;
  if (number == 0) {
    file = base_name.string() + "." + extension;
  } else {
    file = base_name.string() + "_" + StreamableToString(number)
        + "." + extension;
  }
  return ConcatPaths(directory, FilePath(file));
}

FilePath FilePath::ConcatPaths(const FilePath& directory,
                               const FilePath& relative_path) {
  if (directory.IsEmpty())
    return relative_path;
  const FilePath dir(directory.RemoveTrailingPathSeparator());
  return FilePath(dir.string() + kPathSeparator + relative_path.string());
}

bool FilePath::FileOrDirectoryExists() const {
#if GTEST_OS_WINDOWS_MOBILE
  LPCWSTR unicode = String::AnsiToUtf16(pathname_.c_str());
  const DWORD attributes = GetFileAttributes(unicode);
  delete [] unicode;
  return attributes != kInvalidFileAttributes;
#else
  posix::StatStruct file_stat;
  return posix::Stat(pathname_.c_str(), &file_stat) == 0;
#endif
}

bool FilePath::DirectoryExists() const {
  bool result = false;
#if GTEST_OS_WINDOWS

  const FilePath& path(IsRootDirectory() ? *this :
                                           RemoveTrailingPathSeparator());
#else
  const FilePath& path(*this);
#endif

#if GTEST_OS_WINDOWS_MOBILE
  LPCWSTR unicode = String::AnsiToUtf16(path.c_str());
  const DWORD attributes = GetFileAttributes(unicode);
  delete [] unicode;
  if ((attributes != kInvalidFileAttributes) &&
      (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
    result = true;
  }
#else
  posix::StatStruct file_stat;
  result = posix::Stat(path.c_str(), &file_stat) == 0 &&
      posix::IsDir(file_stat);
#endif

  return result;
}

bool FilePath::IsRootDirectory() const {
#if GTEST_OS_WINDOWS
  return pathname_.length() == 3 && IsAbsolutePath();
#else
  return pathname_.length() == 1 && IsPathSeparator(pathname_.c_str()[0]);
#endif
}

bool FilePath::IsAbsolutePath() const {
  const char* const name = pathname_.c_str();
#if GTEST_OS_WINDOWS
  return pathname_.length() >= 3 &&
     ((name[0] >= 'a' && name[0] <= 'z') ||
      (name[0] >= 'A' && name[0] <= 'Z')) &&
     name[1] == ':' &&
     IsPathSeparator(name[2]);
#else
  return IsPathSeparator(name[0]);
#endif
}

FilePath FilePath::GenerateUniqueFileName(const FilePath& directory,
                                          const FilePath& base_name,
                                          const char* extension) {
  FilePath full_pathname;
  int number = 0;
  do {
    full_pathname.Set(MakeFileName(directory, base_name, number++, extension));
  } while (full_pathname.FileOrDirectoryExists());
  return full_pathname;
}

bool FilePath::IsDirectory() const {
  return !pathname_.empty() &&
         IsPathSeparator(pathname_.c_str()[pathname_.length() - 1]);
}

bool FilePath::CreateDirectoriesRecursively() const {
  if (!this->IsDirectory()) {
    return false;
  }

  if (pathname_.length() == 0 || this->DirectoryExists()) {
    return true;
  }

  const FilePath parent(this->RemoveTrailingPathSeparator().RemoveFileName());
  return parent.CreateDirectoriesRecursively() && this->CreateFolder();
}

bool FilePath::CreateFolder() const {
#if GTEST_OS_WINDOWS_MOBILE
  FilePath removed_sep(this->RemoveTrailingPathSeparator());
  LPCWSTR unicode = String::AnsiToUtf16(removed_sep.c_str());
  int result = CreateDirectory(unicode, nullptr) ? 0 : -1;
  delete [] unicode;
#elif GTEST_OS_WINDOWS
  int result = _mkdir(pathname_.c_str());
#elif GTEST_OS_ESP8266 || GTEST_OS_XTENSA

  int result = 0;
#else
  int result = mkdir(pathname_.c_str(), 0777);
#endif

  if (result == -1) {
    return this->DirectoryExists();
  }
  return true;
}

FilePath FilePath::RemoveTrailingPathSeparator() const {
  return IsDirectory()
      ? FilePath(pathname_.substr(0, pathname_.length() - 1))
      : *this;
}

void FilePath::Normalize() {
  auto out = pathname_.begin();

  for (const char character : pathname_) {
    if (!IsPathSeparator(character)) {
      *(out++) = character;
    } else if (out == pathname_.begin() || *std::prev(out) != kPathSeparator) {
      *(out++) = kPathSeparator;
    } else {
      continue;
    }
  }

  pathname_.erase(out, pathname_.end());
}

}
}

#include <string>

namespace testing {

Matcher<const std::string&>::Matcher(const std::string& s) { *this = Eq(s); }

Matcher<const std::string&>::Matcher(const char* s) {
  *this = Eq(std::string(s));
}

Matcher<std::string>::Matcher(const std::string& s) { *this = Eq(s); }

Matcher<std::string>::Matcher(const char* s) { *this = Eq(std::string(s)); }

#if GTEST_INTERNAL_HAS_STRING_VIEW

Matcher<const internal::StringView&>::Matcher(const std::string& s) {
  *this = Eq(s);
}

Matcher<const internal::StringView&>::Matcher(const char* s) {
  *this = Eq(std::string(s));
}

Matcher<const internal::StringView&>::Matcher(internal::StringView s) {
  *this = Eq(std::string(s));
}

Matcher<internal::StringView>::Matcher(const std::string& s) { *this = Eq(s); }

Matcher<internal::StringView>::Matcher(const char* s) {
  *this = Eq(std::string(s));
}

Matcher<internal::StringView>::Matcher(internal::StringView s) {
  *this = Eq(std::string(s));
}
#endif

}

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <fstream>
#include <memory>

#if GTEST_OS_WINDOWS
# include <windows.h>
# include <io.h>
# include <sys/stat.h>
# include <map>
# ifdef _MSC_VER
#  include <crtdbg.h>
# endif
#else
# include <unistd.h>
#endif

#if GTEST_OS_MAC
# include <mach/mach_init.h>
# include <mach/task.h>
# include <mach/vm_map.h>
#endif

#if GTEST_OS_DRAGONFLY || GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD || \
    GTEST_OS_NETBSD || GTEST_OS_OPENBSD
# include <sys/sysctl.h>
# if GTEST_OS_DRAGONFLY || GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD
#  include <sys/user.h>
# endif
#endif

#if GTEST_OS_QNX
# include <devctl.h>
# include <fcntl.h>
# include <sys/procfs.h>
#endif

#if GTEST_OS_AIX
# include <procinfo.h>
# include <sys/types.h>
#endif

#if GTEST_OS_FUCHSIA
# include <zircon/process.h>
# include <zircon/syscalls.h>
#endif

namespace testing {
namespace internal {

#if defined(_MSC_VER) || defined(__BORLANDC__)

const int kStdOutFileno = 1;
const int kStdErrFileno = 2;
#else
const int kStdOutFileno = STDOUT_FILENO;
const int kStdErrFileno = STDERR_FILENO;
#endif

#if GTEST_OS_LINUX

namespace {
template <typename T>
T ReadProcFileField(const std::string& filename, int field) {
  std::string dummy;
  std::ifstream file(filename.c_str());
  while (field-- > 0) {
    file >> dummy;
  }
  T output = 0;
  file >> output;
  return output;
}
}

size_t GetThreadCount() {
  const std::string filename =
      (Message() << "/proc/" << getpid() << "/stat").GetString();
  return ReadProcFileField<size_t>(filename, 19);
}

#elif GTEST_OS_MAC

size_t GetThreadCount() {
  const task_t task = mach_task_self();
  mach_msg_type_number_t thread_count;
  thread_act_array_t thread_list;
  const kern_return_t status = task_threads(task, &thread_list, &thread_count);
  if (status == KERN_SUCCESS) {

    vm_deallocate(task,
                  reinterpret_cast<vm_address_t>(thread_list),
                  sizeof(thread_t) * thread_count);
    return static_cast<size_t>(thread_count);
  } else {
    return 0;
  }
}

#elif GTEST_OS_DRAGONFLY || GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD || \
      GTEST_OS_NETBSD

#if GTEST_OS_NETBSD
#undef KERN_PROC
#define KERN_PROC KERN_PROC2
#define kinfo_proc kinfo_proc2
#endif

#if GTEST_OS_DRAGONFLY
#define KP_NLWP(kp) (kp.kp_nthreads)
#elif GTEST_OS_FREEBSD || GTEST_OS_GNU_KFREEBSD
#define KP_NLWP(kp) (kp.ki_numthreads)
#elif GTEST_OS_NETBSD
#define KP_NLWP(kp) (kp.p_nlwps)
#endif

size_t GetThreadCount() {
  int mib[] = {
    CTL_KERN,
    KERN_PROC,
    KERN_PROC_PID,
    getpid(),
#if GTEST_OS_NETBSD
    sizeof(struct kinfo_proc),
    1,
#endif
  };
  u_int miblen = sizeof(mib) / sizeof(mib[0]);
  struct kinfo_proc info;
  size_t size = sizeof(info);
  if (sysctl(mib, miblen, &info, &size, NULL, 0)) {
    return 0;
  }
  return static_cast<size_t>(KP_NLWP(info));
}
#elif GTEST_OS_OPENBSD

size_t GetThreadCount() {
  int mib[] = {
    CTL_KERN,
    KERN_PROC,
    KERN_PROC_PID | KERN_PROC_SHOW_THREADS,
    getpid(),
    sizeof(struct kinfo_proc),
    0,
  };
  u_int miblen = sizeof(mib) / sizeof(mib[0]);

  size_t size;
  if (sysctl(mib, miblen, NULL, &size, NULL, 0)) {
    return 0;
  }

  mib[5] = static_cast<int>(size / static_cast<size_t>(mib[4]));

  struct kinfo_proc info[mib[5]];
  if (sysctl(mib, miblen, &info, &size, NULL, 0)) {
    return 0;
  }

  size_t nthreads = 0;
  for (size_t i = 0; i < size / static_cast<size_t>(mib[4]); i++) {
    if (info[i].p_tid != -1)
      nthreads++;
  }
  return nthreads;
}

#elif GTEST_OS_QNX

size_t GetThreadCount() {
  const int fd = open("/proc/self/as", O_RDONLY);
  if (fd < 0) {
    return 0;
  }
  procfs_info process_info;
  const int status =
      devctl(fd, DCMD_PROC_INFO, &process_info, sizeof(process_info), nullptr);
  close(fd);
  if (status == EOK) {
    return static_cast<size_t>(process_info.num_threads);
  } else {
    return 0;
  }
}

#elif GTEST_OS_AIX

size_t GetThreadCount() {
  struct procentry64 entry;
  pid_t pid = getpid();
  int status = getprocs64(&entry, sizeof(entry), nullptr, 0, &pid, 1);
  if (status == 1) {
    return entry.pi_thcount;
  } else {
    return 0;
  }
}

#elif GTEST_OS_FUCHSIA

size_t GetThreadCount() {
  int dummy_buffer;
  size_t avail;
  zx_status_t status = zx_object_get_info(
      zx_process_self(),
      ZX_INFO_PROCESS_THREADS,
      &dummy_buffer,
      0,
      nullptr,
      &avail);
  if (status == ZX_OK) {
    return avail;
  } else {
    return 0;
  }
}

#else

size_t GetThreadCount() {

  return 0;
}

#endif

#if GTEST_IS_THREADSAFE && GTEST_OS_WINDOWS

void SleepMilliseconds(int n) {
  ::Sleep(static_cast<DWORD>(n));
}

AutoHandle::AutoHandle()
    : handle_(INVALID_HANDLE_VALUE) {}

AutoHandle::AutoHandle(Handle handle)
    : handle_(handle) {}

AutoHandle::~AutoHandle() {
  Reset();
}

AutoHandle::Handle AutoHandle::Get() const {
  return handle_;
}

void AutoHandle::Reset() {
  Reset(INVALID_HANDLE_VALUE);
}

void AutoHandle::Reset(HANDLE handle) {

  if (handle_ != handle) {
    if (IsCloseable()) {
      ::CloseHandle(handle_);
    }
    handle_ = handle;
  } else {
    GTEST_CHECK_(!IsCloseable())
        << "Resetting a valid handle to itself is likely a programmer error "
            "and thus not allowed.";
  }
}

bool AutoHandle::IsCloseable() const {

  return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
}

Notification::Notification()
    : event_(::CreateEvent(nullptr,
                           TRUE,
                           FALSE,
                           nullptr)) {
  GTEST_CHECK_(event_.Get() != nullptr);
}

void Notification::Notify() {
  GTEST_CHECK_(::SetEvent(event_.Get()) != FALSE);
}

void Notification::WaitForNotification() {
  GTEST_CHECK_(
      ::WaitForSingleObject(event_.Get(), INFINITE) == WAIT_OBJECT_0);
}

Mutex::Mutex()
    : owner_thread_id_(0),
      type_(kDynamic),
      critical_section_init_phase_(0),
      critical_section_(new CRITICAL_SECTION) {
  ::InitializeCriticalSection(critical_section_);
}

Mutex::~Mutex() {

  if (type_ == kDynamic) {
    ::DeleteCriticalSection(critical_section_);
    delete critical_section_;
    critical_section_ = nullptr;
  }
}

void Mutex::Lock() {
  ThreadSafeLazyInit();
  ::EnterCriticalSection(critical_section_);
  owner_thread_id_ = ::GetCurrentThreadId();
}

void Mutex::Unlock() {
  ThreadSafeLazyInit();

  owner_thread_id_ = 0;
  ::LeaveCriticalSection(critical_section_);
}

void Mutex::AssertHeld() {
  ThreadSafeLazyInit();
  GTEST_CHECK_(owner_thread_id_ == ::GetCurrentThreadId())
      << "The current thread is not holding the mutex @" << this;
}

namespace {

#ifdef _MSC_VER

class MemoryIsNotDeallocated
{
 public:
  MemoryIsNotDeallocated() : old_crtdbg_flag_(0) {
    old_crtdbg_flag_ = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    _CrtSetDbgFlag(old_crtdbg_flag_ & ~_CRTDBG_ALLOC_MEM_DF);
  }

  ~MemoryIsNotDeallocated() {

    _CrtSetDbgFlag(old_crtdbg_flag_);
  }

 private:
  int old_crtdbg_flag_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(MemoryIsNotDeallocated);
};
#endif

}

void Mutex::ThreadSafeLazyInit() {

  if (type_ == kStatic) {
    switch (
        ::InterlockedCompareExchange(&critical_section_init_phase_, 1L, 0L)) {
      case 0:

        owner_thread_id_ = 0;
        {

#ifdef _MSC_VER
          MemoryIsNotDeallocated memory_is_not_deallocated;
#endif
          critical_section_ = new CRITICAL_SECTION;
        }
        ::InitializeCriticalSection(critical_section_);

        GTEST_CHECK_(::InterlockedCompareExchange(
                          &critical_section_init_phase_, 2L, 1L) ==
                      1L);
        break;
      case 1:

        while (::InterlockedCompareExchange(&critical_section_init_phase_,
                                            2L,
                                            2L) != 2L) {

          ::Sleep(0);
        }
        break;

      case 2:
        break;

      default:
        GTEST_CHECK_(false)
            << "Unexpected value of critical_section_init_phase_ "
            << "while initializing a static mutex.";
    }
  }
}

namespace {

class ThreadWithParamSupport : public ThreadWithParamBase {
 public:
  static HANDLE CreateThread(Runnable* runnable,
                             Notification* thread_can_start) {
    ThreadMainParam* param = new ThreadMainParam(runnable, thread_can_start);
    DWORD thread_id;
    HANDLE thread_handle = ::CreateThread(
        nullptr,
        0,
        &ThreadWithParamSupport::ThreadMain,
        param,
        0x0,
        &thread_id);
    GTEST_CHECK_(thread_handle != nullptr)
        << "CreateThread failed with error " << ::GetLastError() << ".";
    if (thread_handle == nullptr) {
      delete param;
    }
    return thread_handle;
  }

 private:
  struct ThreadMainParam {
    ThreadMainParam(Runnable* runnable, Notification* thread_can_start)
        : runnable_(runnable),
          thread_can_start_(thread_can_start) {
    }
    std::unique_ptr<Runnable> runnable_;

    Notification* thread_can_start_;
  };

  static DWORD WINAPI ThreadMain(void* ptr) {

    std::unique_ptr<ThreadMainParam> param(static_cast<ThreadMainParam*>(ptr));
    if (param->thread_can_start_ != nullptr)
      param->thread_can_start_->WaitForNotification();
    param->runnable_->Run();
    return 0;
  }

  ThreadWithParamSupport();

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ThreadWithParamSupport);
};

}

ThreadWithParamBase::ThreadWithParamBase(Runnable *runnable,
                                         Notification* thread_can_start)
      : thread_(ThreadWithParamSupport::CreateThread(runnable,
                                                     thread_can_start)) {
}

ThreadWithParamBase::~ThreadWithParamBase() {
  Join();
}

void ThreadWithParamBase::Join() {
  GTEST_CHECK_(::WaitForSingleObject(thread_.Get(), INFINITE) == WAIT_OBJECT_0)
      << "Failed to join the thread with error " << ::GetLastError() << ".";
}

class ThreadLocalRegistryImpl {
 public:

  static ThreadLocalValueHolderBase* GetValueOnCurrentThread(
      const ThreadLocalBase* thread_local_instance) {
#ifdef _MSC_VER
    MemoryIsNotDeallocated memory_is_not_deallocated;
#endif
    DWORD current_thread = ::GetCurrentThreadId();
    MutexLock lock(&mutex_);
    ThreadIdToThreadLocals* const thread_to_thread_locals =
        GetThreadLocalsMapLocked();
    ThreadIdToThreadLocals::iterator thread_local_pos =
        thread_to_thread_locals->find(current_thread);
    if (thread_local_pos == thread_to_thread_locals->end()) {
      thread_local_pos = thread_to_thread_locals->insert(
          std::make_pair(current_thread, ThreadLocalValues())).first;
      StartWatcherThreadFor(current_thread);
    }
    ThreadLocalValues& thread_local_values = thread_local_pos->second;
    ThreadLocalValues::iterator value_pos =
        thread_local_values.find(thread_local_instance);
    if (value_pos == thread_local_values.end()) {
      value_pos =
          thread_local_values
              .insert(std::make_pair(
                  thread_local_instance,
                  std::shared_ptr<ThreadLocalValueHolderBase>(
                      thread_local_instance->NewValueForCurrentThread())))
              .first;
    }
    return value_pos->second.get();
  }

  static void OnThreadLocalDestroyed(
      const ThreadLocalBase* thread_local_instance) {
    std::vector<std::shared_ptr<ThreadLocalValueHolderBase> > value_holders;

    {
      MutexLock lock(&mutex_);
      ThreadIdToThreadLocals* const thread_to_thread_locals =
          GetThreadLocalsMapLocked();
      for (ThreadIdToThreadLocals::iterator it =
          thread_to_thread_locals->begin();
          it != thread_to_thread_locals->end();
          ++it) {
        ThreadLocalValues& thread_local_values = it->second;
        ThreadLocalValues::iterator value_pos =
            thread_local_values.find(thread_local_instance);
        if (value_pos != thread_local_values.end()) {
          value_holders.push_back(value_pos->second);
          thread_local_values.erase(value_pos);

        }
      }
    }

  }

  static void OnThreadExit(DWORD thread_id) {
    GTEST_CHECK_(thread_id != 0) << ::GetLastError();
    std::vector<std::shared_ptr<ThreadLocalValueHolderBase> > value_holders;

    {
      MutexLock lock(&mutex_);
      ThreadIdToThreadLocals* const thread_to_thread_locals =
          GetThreadLocalsMapLocked();
      ThreadIdToThreadLocals::iterator thread_local_pos =
          thread_to_thread_locals->find(thread_id);
      if (thread_local_pos != thread_to_thread_locals->end()) {
        ThreadLocalValues& thread_local_values = thread_local_pos->second;
        for (ThreadLocalValues::iterator value_pos =
            thread_local_values.begin();
            value_pos != thread_local_values.end();
            ++value_pos) {
          value_holders.push_back(value_pos->second);
        }
        thread_to_thread_locals->erase(thread_local_pos);
      }
    }

  }

 private:

  typedef std::map<const ThreadLocalBase*,
                   std::shared_ptr<ThreadLocalValueHolderBase> >
      ThreadLocalValues;

  typedef std::map<DWORD, ThreadLocalValues> ThreadIdToThreadLocals;

  typedef std::pair<DWORD, HANDLE> ThreadIdAndHandle;

  static void StartWatcherThreadFor(DWORD thread_id) {

    HANDLE thread = ::OpenThread(SYNCHRONIZE | THREAD_QUERY_INFORMATION,
                                 FALSE,
                                 thread_id);
    GTEST_CHECK_(thread != nullptr);

    DWORD watcher_thread_id;
    HANDLE watcher_thread = ::CreateThread(
        nullptr,
        0,
        &ThreadLocalRegistryImpl::WatcherThreadFunc,
        reinterpret_cast<LPVOID>(new ThreadIdAndHandle(thread_id, thread)),
        CREATE_SUSPENDED, &watcher_thread_id);
    GTEST_CHECK_(watcher_thread != nullptr);

    ::SetThreadPriority(watcher_thread,
                        ::GetThreadPriority(::GetCurrentThread()));
    ::ResumeThread(watcher_thread);
    ::CloseHandle(watcher_thread);
  }

  static DWORD WINAPI WatcherThreadFunc(LPVOID param) {
    const ThreadIdAndHandle* tah =
        reinterpret_cast<const ThreadIdAndHandle*>(param);
    GTEST_CHECK_(
        ::WaitForSingleObject(tah->second, INFINITE) == WAIT_OBJECT_0);
    OnThreadExit(tah->first);
    ::CloseHandle(tah->second);
    delete tah;
    return 0;
  }

  static ThreadIdToThreadLocals* GetThreadLocalsMapLocked() {
    mutex_.AssertHeld();
#ifdef _MSC_VER
    MemoryIsNotDeallocated memory_is_not_deallocated;
#endif
    static ThreadIdToThreadLocals* map = new ThreadIdToThreadLocals();
    return map;
  }

  static Mutex mutex_;

  static Mutex thread_map_mutex_;
};

Mutex ThreadLocalRegistryImpl::mutex_(Mutex::kStaticMutex);
Mutex ThreadLocalRegistryImpl::thread_map_mutex_(Mutex::kStaticMutex);

ThreadLocalValueHolderBase* ThreadLocalRegistry::GetValueOnCurrentThread(
      const ThreadLocalBase* thread_local_instance) {
  return ThreadLocalRegistryImpl::GetValueOnCurrentThread(
      thread_local_instance);
}

void ThreadLocalRegistry::OnThreadLocalDestroyed(
      const ThreadLocalBase* thread_local_instance) {
  ThreadLocalRegistryImpl::OnThreadLocalDestroyed(thread_local_instance);
}

#endif

#if GTEST_USES_POSIX_RE

RE::~RE() {
  if (is_valid_) {

    regfree(&partial_regex_);
    regfree(&full_regex_);
  }
  free(const_cast<char*>(pattern_));
}

bool RE::FullMatch(const char* str, const RE& re) {
  if (!re.is_valid_) return false;

  regmatch_t match;
  return regexec(&re.full_regex_, str, 1, &match, 0) == 0;
}

bool RE::PartialMatch(const char* str, const RE& re) {
  if (!re.is_valid_) return false;

  regmatch_t match;
  return regexec(&re.partial_regex_, str, 1, &match, 0) == 0;
}

void RE::Init(const char* regex) {
  pattern_ = posix::StrDup(regex);

  const size_t full_regex_len = strlen(regex) + 10;
  char* const full_pattern = new char[full_regex_len];

  snprintf(full_pattern, full_regex_len, "^(%s)$", regex);
  is_valid_ = regcomp(&full_regex_, full_pattern, REG_EXTENDED) == 0;

  if (is_valid_) {
    const char* const partial_regex = (*regex == '\0') ? "()" : regex;
    is_valid_ = regcomp(&partial_regex_, partial_regex, REG_EXTENDED) == 0;
  }
  EXPECT_TRUE(is_valid_)
      << "Regular expression \"" << regex
      << "\" is not a valid POSIX Extended regular expression.";

  delete[] full_pattern;
}

#elif GTEST_USES_SIMPLE_RE

bool IsInSet(char ch, const char* str) {
  return ch != '\0' && strchr(str, ch) != nullptr;
}

bool IsAsciiDigit(char ch) { return '0' <= ch && ch <= '9'; }
bool IsAsciiPunct(char ch) {
  return IsInSet(ch, "^-!\"#$%&'()*+,./:;<=>?@[\\]_`{|}~");
}
bool IsRepeat(char ch) { return IsInSet(ch, "?*+"); }
bool IsAsciiWhiteSpace(char ch) { return IsInSet(ch, " \f\n\r\t\v"); }
bool IsAsciiWordChar(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') ||
      ('0' <= ch && ch <= '9') || ch == '_';
}

bool IsValidEscape(char c) {
  return (IsAsciiPunct(c) || IsInSet(c, "dDfnrsStvwW"));
}

bool AtomMatchesChar(bool escaped, char pattern_char, char ch) {
  if (escaped) {
    switch (pattern_char) {
      case 'd': return IsAsciiDigit(ch);
      case 'D': return !IsAsciiDigit(ch);
      case 'f': return ch == '\f';
      case 'n': return ch == '\n';
      case 'r': return ch == '\r';
      case 's': return IsAsciiWhiteSpace(ch);
      case 'S': return !IsAsciiWhiteSpace(ch);
      case 't': return ch == '\t';
      case 'v': return ch == '\v';
      case 'w': return IsAsciiWordChar(ch);
      case 'W': return !IsAsciiWordChar(ch);
    }
    return IsAsciiPunct(pattern_char) && pattern_char == ch;
  }

  return (pattern_char == '.' && ch != '\n') || pattern_char == ch;
}

static std::string FormatRegexSyntaxError(const char* regex, int index) {
  return (Message() << "Syntax error at index " << index
          << " in simple regular expression \"" << regex << "\": ").GetString();
}

bool ValidateRegex(const char* regex) {
  if (regex == nullptr) {
    ADD_FAILURE() << "NULL is not a valid simple regular expression.";
    return false;
  }

  bool is_valid = true;

  bool prev_repeatable = false;
  for (int i = 0; regex[i]; i++) {
    if (regex[i] == '\\') {
      i++;
      if (regex[i] == '\0') {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i - 1)
                      << "'\\' cannot appear at the end.";
        return false;
      }

      if (!IsValidEscape(regex[i])) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i - 1)
                      << "invalid escape sequence \"\\" << regex[i] << "\".";
        is_valid = false;
      }
      prev_repeatable = true;
    } else {
      const char ch = regex[i];

      if (ch == '^' && i > 0) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'^' can only appear at the beginning.";
        is_valid = false;
      } else if (ch == '$' && regex[i + 1] != '\0') {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'$' can only appear at the end.";
        is_valid = false;
      } else if (IsInSet(ch, "()[]{}|")) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'" << ch << "' is unsupported.";
        is_valid = false;
      } else if (IsRepeat(ch) && !prev_repeatable) {
        ADD_FAILURE() << FormatRegexSyntaxError(regex, i)
                      << "'" << ch << "' can only follow a repeatable token.";
        is_valid = false;
      }

      prev_repeatable = !IsInSet(ch, "^$?*+");
    }
  }

  return is_valid;
}

bool MatchRepetitionAndRegexAtHead(
    bool escaped, char c, char repeat, const char* regex,
    const char* str) {
  const size_t min_count = (repeat == '+') ? 1 : 0;
  const size_t max_count = (repeat == '?') ? 1 :
      static_cast<size_t>(-1) - 1;

  for (size_t i = 0; i <= max_count; ++i) {

    if (i >= min_count && MatchRegexAtHead(regex, str + i)) {

      return true;
    }
    if (str[i] == '\0' || !AtomMatchesChar(escaped, c, str[i]))
      return false;
  }
  return false;
}

bool MatchRegexAtHead(const char* regex, const char* str) {
  if (*regex == '\0')
    return true;

  if (*regex == '$')
    return *str == '\0';

  const bool escaped = *regex == '\\';
  if (escaped)
    ++regex;
  if (IsRepeat(regex[1])) {

    return MatchRepetitionAndRegexAtHead(
        escaped, regex[0], regex[1], regex + 2, str);
  } else {

    return (*str != '\0') && AtomMatchesChar(escaped, *regex, *str) &&
        MatchRegexAtHead(regex + 1, str + 1);
  }
}

bool MatchRegexAnywhere(const char* regex, const char* str) {
  if (regex == nullptr || str == nullptr) return false;

  if (*regex == '^')
    return MatchRegexAtHead(regex + 1, str);

  do {
    if (MatchRegexAtHead(regex, str))
      return true;
  } while (*str++ != '\0');
  return false;
}

RE::~RE() {
  free(const_cast<char*>(pattern_));
  free(const_cast<char*>(full_pattern_));
}

bool RE::FullMatch(const char* str, const RE& re) {
  return re.is_valid_ && MatchRegexAnywhere(re.full_pattern_, str);
}

bool RE::PartialMatch(const char* str, const RE& re) {
  return re.is_valid_ && MatchRegexAnywhere(re.pattern_, str);
}

void RE::Init(const char* regex) {
  pattern_ = full_pattern_ = nullptr;
  if (regex != nullptr) {
    pattern_ = posix::StrDup(regex);
  }

  is_valid_ = ValidateRegex(regex);
  if (!is_valid_) {

    return;
  }

  const size_t len = strlen(regex);

  char* buffer = static_cast<char*>(malloc(len + 3));
  full_pattern_ = buffer;

  if (*regex != '^')
    *buffer++ = '^';

  memcpy(buffer, regex, len);
  buffer += len;

  if (len == 0 || regex[len - 1] != '$')
    *buffer++ = '$';

  *buffer = '\0';
}

#endif

const char kUnknownFile[] = "unknown file";

GTEST_API_ ::std::string FormatFileLocation(const char* file, int line) {
  const std::string file_name(file == nullptr ? kUnknownFile : file);

  if (line < 0) {
    return file_name + ":";
  }
#ifdef _MSC_VER
  return file_name + "(" + StreamableToString(line) + "):";
#else
  return file_name + ":" + StreamableToString(line) + ":";
#endif
}

GTEST_API_ ::std::string FormatCompilerIndependentFileLocation(
    const char* file, int line) {
  const std::string file_name(file == nullptr ? kUnknownFile : file);

  if (line < 0)
    return file_name;
  else
    return file_name + ":" + StreamableToString(line);
}

GTestLog::GTestLog(GTestLogSeverity severity, const char* file, int line)
    : severity_(severity) {
  const char* const marker =
      severity == GTEST_INFO ?    "[  INFO ]" :
      severity == GTEST_WARNING ? "[WARNING]" :
      severity == GTEST_ERROR ?   "[ ERROR ]" : "[ FATAL ]";
  GetStream() << ::std::endl << marker << " "
              << FormatFileLocation(file, line).c_str() << ": ";
}

GTestLog::~GTestLog() {
  GetStream() << ::std::endl;
  if (severity_ == GTEST_FATAL) {
    fflush(stderr);
    posix::Abort();
  }
}

GTEST_DISABLE_MSC_DEPRECATED_PUSH_()

#if GTEST_HAS_STREAM_REDIRECTION

class CapturedStream {
 public:

  explicit CapturedStream(int fd) : fd_(fd), uncaptured_fd_(dup(fd)) {
# if GTEST_OS_WINDOWS
    char temp_dir_path[MAX_PATH + 1] = { '\0' };
    char temp_file_path[MAX_PATH + 1] = { '\0' };

    ::GetTempPathA(sizeof(temp_dir_path), temp_dir_path);
    const UINT success = ::GetTempFileNameA(temp_dir_path,
                                            "gtest_redir",
                                            0,
                                            temp_file_path);
    GTEST_CHECK_(success != 0)
        << "Unable to create a temporary file in " << temp_dir_path;
    const int captured_fd = creat(temp_file_path, _S_IREAD | _S_IWRITE);
    GTEST_CHECK_(captured_fd != -1) << "Unable to open temporary file "
                                    << temp_file_path;
    filename_ = temp_file_path;
# else

#  if GTEST_OS_LINUX_ANDROID

    char name_template[] = "/data/local/tmp/gtest_captured_stream.XXXXXX";
#  else
    char name_template[] = "/tmp/captured_stream.XXXXXX";
#  endif
    const int captured_fd = mkstemp(name_template);
    if (captured_fd == -1) {
      GTEST_LOG_(WARNING)
          << "Failed to create tmp file " << name_template
          << " for test; does the test have access to the /tmp directory?";
    }
    filename_ = name_template;
# endif
    fflush(nullptr);
    dup2(captured_fd, fd_);
    close(captured_fd);
  }

  ~CapturedStream() {
    remove(filename_.c_str());
  }

  std::string GetCapturedString() {
    if (uncaptured_fd_ != -1) {

      fflush(nullptr);
      dup2(uncaptured_fd_, fd_);
      close(uncaptured_fd_);
      uncaptured_fd_ = -1;
    }

    FILE* const file = posix::FOpen(filename_.c_str(), "r");
    if (file == nullptr) {
      GTEST_LOG_(FATAL) << "Failed to open tmp file " << filename_
                        << " for capturing stream.";
    }
    const std::string content = ReadEntireFile(file);
    posix::FClose(file);
    return content;
  }

 private:
  const int fd_;
  int uncaptured_fd_;

  ::std::string filename_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(CapturedStream);
};

GTEST_DISABLE_MSC_DEPRECATED_POP_()

static CapturedStream* g_captured_stderr = nullptr;
static CapturedStream* g_captured_stdout = nullptr;

static void CaptureStream(int fd, const char* stream_name,
                          CapturedStream** stream) {
  if (*stream != nullptr) {
    GTEST_LOG_(FATAL) << "Only one " << stream_name
                      << " capturer can exist at a time.";
  }
  *stream = new CapturedStream(fd);
}

static std::string GetCapturedStream(CapturedStream** captured_stream) {
  const std::string content = (*captured_stream)->GetCapturedString();

  delete *captured_stream;
  *captured_stream = nullptr;

  return content;
}

void CaptureStdout() {
  CaptureStream(kStdOutFileno, "stdout", &g_captured_stdout);
}

void CaptureStderr() {
  CaptureStream(kStdErrFileno, "stderr", &g_captured_stderr);
}

std::string GetCapturedStdout() {
  return GetCapturedStream(&g_captured_stdout);
}

std::string GetCapturedStderr() {
  return GetCapturedStream(&g_captured_stderr);
}

#endif

size_t GetFileSize(FILE* file) {
  fseek(file, 0, SEEK_END);
  return static_cast<size_t>(ftell(file));
}

std::string ReadEntireFile(FILE* file) {
  const size_t file_size = GetFileSize(file);
  char* const buffer = new char[file_size];

  size_t bytes_last_read = 0;
  size_t bytes_read = 0;

  fseek(file, 0, SEEK_SET);

  do {
    bytes_last_read = fread(buffer+bytes_read, 1, file_size-bytes_read, file);
    bytes_read += bytes_last_read;
  } while (bytes_last_read > 0 && bytes_read < file_size);

  const std::string content(buffer, bytes_read);
  delete[] buffer;

  return content;
}

#if GTEST_HAS_DEATH_TEST
static const std::vector<std::string>* g_injected_test_argvs =
    nullptr;

std::vector<std::string> GetInjectableArgvs() {
  if (g_injected_test_argvs != nullptr) {
    return *g_injected_test_argvs;
  }
  return GetArgvs();
}

void SetInjectableArgvs(const std::vector<std::string>* new_argvs) {
  if (g_injected_test_argvs != new_argvs) delete g_injected_test_argvs;
  g_injected_test_argvs = new_argvs;
}

void SetInjectableArgvs(const std::vector<std::string>& new_argvs) {
  SetInjectableArgvs(
      new std::vector<std::string>(new_argvs.begin(), new_argvs.end()));
}

void ClearInjectableArgvs() {
  delete g_injected_test_argvs;
  g_injected_test_argvs = nullptr;
}
#endif

#if GTEST_OS_WINDOWS_MOBILE
namespace posix {
void Abort() {
  DebugBreak();
  TerminateProcess(GetCurrentProcess(), 1);
}
}
#endif

static std::string FlagToEnvVar(const char* flag) {
  const std::string full_flag =
      (Message() << GTEST_FLAG_PREFIX_ << flag).GetString();

  Message env_var;
  for (size_t i = 0; i != full_flag.length(); i++) {
    env_var << ToUpper(full_flag.c_str()[i]);
  }

  return env_var.GetString();
}

bool ParseInt32(const Message& src_text, const char* str, int32_t* value) {

  char* end = nullptr;
  const long long_value = strtol(str, &end, 10);

  if (*end != '\0') {

    Message msg;
    msg << "WARNING: " << src_text
        << " is expected to be a 32-bit integer, but actually"
        << " has value \"" << str << "\".\n";
    printf("%s", msg.GetString().c_str());
    fflush(stdout);
    return false;
  }

  const auto result = static_cast<int32_t>(long_value);
  if (long_value == LONG_MAX || long_value == LONG_MIN ||

      result != long_value

      ) {
    Message msg;
    msg << "WARNING: " << src_text
        << " is expected to be a 32-bit integer, but actually"
        << " has value " << str << ", which overflows.\n";
    printf("%s", msg.GetString().c_str());
    fflush(stdout);
    return false;
  }

  *value = result;
  return true;
}

bool BoolFromGTestEnv(const char* flag, bool default_value) {
#if defined(GTEST_GET_BOOL_FROM_ENV_)
  return GTEST_GET_BOOL_FROM_ENV_(flag, default_value);
#else
  const std::string env_var = FlagToEnvVar(flag);
  const char* const string_value = posix::GetEnv(env_var.c_str());
  return string_value == nullptr ? default_value
                                 : strcmp(string_value, "0") != 0;
#endif
}

int32_t Int32FromGTestEnv(const char* flag, int32_t default_value) {
#if defined(GTEST_GET_INT32_FROM_ENV_)
  return GTEST_GET_INT32_FROM_ENV_(flag, default_value);
#else
  const std::string env_var = FlagToEnvVar(flag);
  const char* const string_value = posix::GetEnv(env_var.c_str());
  if (string_value == nullptr) {

    return default_value;
  }

  int32_t result = default_value;
  if (!ParseInt32(Message() << "Environment variable " << env_var,
                  string_value, &result)) {
    printf("The default value %s is used.\n",
           (Message() << default_value).GetString().c_str());
    fflush(stdout);
    return default_value;
  }

  return result;
#endif
}

std::string OutputFlagAlsoCheckEnvVar(){
  std::string default_value_for_output_flag = "";
  const char* xml_output_file_env = posix::GetEnv("XML_OUTPUT_FILE");
  if (nullptr != xml_output_file_env) {
    default_value_for_output_flag = std::string("xml:") + xml_output_file_env;
  }
  return default_value_for_output_flag;
}

const char* StringFromGTestEnv(const char* flag, const char* default_value) {
#if defined(GTEST_GET_STRING_FROM_ENV_)
  return GTEST_GET_STRING_FROM_ENV_(flag, default_value);
#else
  const std::string env_var = FlagToEnvVar(flag);
  const char* const value = posix::GetEnv(env_var.c_str());
  return value == nullptr ? default_value : value;
#endif
}

}
}

#include <stdio.h>

#include <cctype>
#include <cstdint>
#include <cwchar>
#include <ostream>
#include <string>
#include <type_traits>

namespace testing {

namespace {

using ::std::ostream;

GTEST_ATTRIBUTE_NO_SANITIZE_MEMORY_
GTEST_ATTRIBUTE_NO_SANITIZE_ADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_HWADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_THREAD_
void PrintByteSegmentInObjectTo(const unsigned char* obj_bytes, size_t start,
                                size_t count, ostream* os) {
  char text[5] = "";
  for (size_t i = 0; i != count; i++) {
    const size_t j = start + i;
    if (i != 0) {

      if ((j % 2) == 0)
        *os << ' ';
      else
        *os << '-';
    }
    GTEST_SNPRINTF_(text, sizeof(text), "%02X", obj_bytes[j]);
    *os << text;
  }
}

void PrintBytesInObjectToImpl(const unsigned char* obj_bytes, size_t count,
                              ostream* os) {

  *os << count << "-byte object <";

  const size_t kThreshold = 132;
  const size_t kChunkSize = 64;

  if (count < kThreshold) {
    PrintByteSegmentInObjectTo(obj_bytes, 0, count, os);
  } else {
    PrintByteSegmentInObjectTo(obj_bytes, 0, kChunkSize, os);
    *os << " ... ";

    const size_t resume_pos = (count - kChunkSize + 1)/2*2;
    PrintByteSegmentInObjectTo(obj_bytes, resume_pos, count - resume_pos, os);
  }
  *os << ">";
}

template <typename CharType>
char32_t ToChar32(CharType in) {
  return static_cast<char32_t>(
      static_cast<typename std::make_unsigned<CharType>::type>(in));
}

}

namespace internal {

void PrintBytesInObjectTo(const unsigned char* obj_bytes, size_t count,
                          ostream* os) {
  PrintBytesInObjectToImpl(obj_bytes, count, os);
}

enum CharFormat {
  kAsIs,
  kHexEscape,
  kSpecialEscape
};

inline bool IsPrintableAscii(char32_t c) { return 0x20 <= c && c <= 0x7E; }

template <typename Char>
static CharFormat PrintAsCharLiteralTo(Char c, ostream* os) {
  const char32_t u_c = ToChar32(c);
  switch (u_c) {
    case L'\0':
      *os << "\\0";
      break;
    case L'\'':
      *os << "\\'";
      break;
    case L'\\':
      *os << "\\\\";
      break;
    case L'\a':
      *os << "\\a";
      break;
    case L'\b':
      *os << "\\b";
      break;
    case L'\f':
      *os << "\\f";
      break;
    case L'\n':
      *os << "\\n";
      break;
    case L'\r':
      *os << "\\r";
      break;
    case L'\t':
      *os << "\\t";
      break;
    case L'\v':
      *os << "\\v";
      break;
    default:
      if (IsPrintableAscii(u_c)) {
        *os << static_cast<char>(c);
        return kAsIs;
      } else {
        ostream::fmtflags flags = os->flags();
        *os << "\\x" << std::hex << std::uppercase << static_cast<int>(u_c);
        os->flags(flags);
        return kHexEscape;
      }
  }
  return kSpecialEscape;
}

static CharFormat PrintAsStringLiteralTo(char32_t c, ostream* os) {
  switch (c) {
    case L'\'':
      *os << "'";
      return kAsIs;
    case L'"':
      *os << "\\\"";
      return kSpecialEscape;
    default:
      return PrintAsCharLiteralTo(c, os);
  }
}

static const char* GetCharWidthPrefix(char) {
  return "";
}

static const char* GetCharWidthPrefix(signed char) {
  return "";
}

static const char* GetCharWidthPrefix(unsigned char) {
  return "";
}

#ifdef __cpp_char8_t
static const char* GetCharWidthPrefix(char8_t) {
  return "u8";
}
#endif

static const char* GetCharWidthPrefix(char16_t) {
  return "u";
}

static const char* GetCharWidthPrefix(char32_t) {
  return "U";
}

static const char* GetCharWidthPrefix(wchar_t) {
  return "L";
}

static CharFormat PrintAsStringLiteralTo(char c, ostream* os) {
  return PrintAsStringLiteralTo(ToChar32(c), os);
}

#ifdef __cpp_char8_t
static CharFormat PrintAsStringLiteralTo(char8_t c, ostream* os) {
  return PrintAsStringLiteralTo(ToChar32(c), os);
}
#endif

static CharFormat PrintAsStringLiteralTo(char16_t c, ostream* os) {
  return PrintAsStringLiteralTo(ToChar32(c), os);
}

static CharFormat PrintAsStringLiteralTo(wchar_t c, ostream* os) {
  return PrintAsStringLiteralTo(ToChar32(c), os);
}

template <typename Char>
void PrintCharAndCodeTo(Char c, ostream* os) {

  *os << GetCharWidthPrefix(c) << "'";
  const CharFormat format = PrintAsCharLiteralTo(c, os);
  *os << "'";

  if (c == 0)
    return;
  *os << " (" << static_cast<int>(c);

  if (format == kHexEscape || (1 <= c && c <= 9)) {

  } else {
    *os << ", 0x" << String::FormatHexInt(static_cast<int>(c));
  }
  *os << ")";
}

void PrintTo(unsigned char c, ::std::ostream* os) { PrintCharAndCodeTo(c, os); }
void PrintTo(signed char c, ::std::ostream* os) { PrintCharAndCodeTo(c, os); }

void PrintTo(wchar_t wc, ostream* os) { PrintCharAndCodeTo(wc, os); }

void PrintTo(char32_t c, ::std::ostream* os) {
  *os << std::hex << "U+" << std::uppercase << std::setfill('0') << std::setw(4)
      << static_cast<uint32_t>(c);
}

template <typename CharType>
GTEST_ATTRIBUTE_NO_SANITIZE_MEMORY_
GTEST_ATTRIBUTE_NO_SANITIZE_ADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_HWADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_THREAD_
static CharFormat PrintCharsAsStringTo(
    const CharType* begin, size_t len, ostream* os) {
  const char* const quote_prefix = GetCharWidthPrefix(*begin);
  *os << quote_prefix << "\"";
  bool is_previous_hex = false;
  CharFormat print_format = kAsIs;
  for (size_t index = 0; index < len; ++index) {
    const CharType cur = begin[index];
    if (is_previous_hex && IsXDigit(cur)) {

      *os << "\" " << quote_prefix << "\"";
    }
    is_previous_hex = PrintAsStringLiteralTo(cur, os) == kHexEscape;

    if (is_previous_hex) {
      print_format = kHexEscape;
    }
  }
  *os << "\"";
  return print_format;
}

template <typename CharType>
GTEST_ATTRIBUTE_NO_SANITIZE_MEMORY_
GTEST_ATTRIBUTE_NO_SANITIZE_ADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_HWADDRESS_
GTEST_ATTRIBUTE_NO_SANITIZE_THREAD_
static void UniversalPrintCharArray(
    const CharType* begin, size_t len, ostream* os) {

  if (len > 0 && begin[len - 1] == '\0') {
    PrintCharsAsStringTo(begin, len - 1, os);
    return;
  }

  PrintCharsAsStringTo(begin, len, os);
  *os << " (no terminating NUL)";
}

void UniversalPrintArray(const char* begin, size_t len, ostream* os) {
  UniversalPrintCharArray(begin, len, os);
}

#ifdef __cpp_char8_t

void UniversalPrintArray(const char8_t* begin, size_t len, ostream* os) {
  UniversalPrintCharArray(begin, len, os);
}
#endif

void UniversalPrintArray(const char16_t* begin, size_t len, ostream* os) {
  UniversalPrintCharArray(begin, len, os);
}

void UniversalPrintArray(const char32_t* begin, size_t len, ostream* os) {
  UniversalPrintCharArray(begin, len, os);
}

void UniversalPrintArray(const wchar_t* begin, size_t len, ostream* os) {
  UniversalPrintCharArray(begin, len, os);
}

namespace {

template <typename Char>
void PrintCStringTo(const Char* s, ostream* os) {
  if (s == nullptr) {
    *os << "NULL";
  } else {
    *os << ImplicitCast_<const void*>(s) << " pointing to ";
    PrintCharsAsStringTo(s, std::char_traits<Char>::length(s), os);
  }
}

}

void PrintTo(const char* s, ostream* os) { PrintCStringTo(s, os); }

#ifdef __cpp_char8_t
void PrintTo(const char8_t* s, ostream* os) { PrintCStringTo(s, os); }
#endif

void PrintTo(const char16_t* s, ostream* os) { PrintCStringTo(s, os); }

void PrintTo(const char32_t* s, ostream* os) { PrintCStringTo(s, os); }

#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)

void PrintTo(const wchar_t* s, ostream* os) { PrintCStringTo(s, os); }
#endif

namespace {

bool ContainsUnprintableControlCodes(const char* str, size_t length) {
  const unsigned char *s = reinterpret_cast<const unsigned char *>(str);

  for (size_t i = 0; i < length; i++) {
    unsigned char ch = *s++;
    if (std::iscntrl(ch)) {
        switch (ch) {
        case '\t':
        case '\n':
        case '\r':
          break;
        default:
          return true;
        }
      }
  }
  return false;
}

bool IsUTF8TrailByte(unsigned char t) { return 0x80 <= t && t<= 0xbf; }

bool IsValidUTF8(const char* str, size_t length) {
  const unsigned char *s = reinterpret_cast<const unsigned char *>(str);

  for (size_t i = 0; i < length;) {
    unsigned char lead = s[i++];

    if (lead <= 0x7f) {
      continue;
    }
    if (lead < 0xc2) {
      return false;
    } else if (lead <= 0xdf && (i + 1) <= length && IsUTF8TrailByte(s[i])) {
      ++i;
    } else if (0xe0 <= lead && lead <= 0xef && (i + 2) <= length &&
               IsUTF8TrailByte(s[i]) &&
               IsUTF8TrailByte(s[i + 1]) &&

               (lead != 0xe0 || s[i] >= 0xa0) &&
               (lead != 0xed || s[i] < 0xa0)) {
      i += 2;
    } else if (0xf0 <= lead && lead <= 0xf4 && (i + 3) <= length &&
               IsUTF8TrailByte(s[i]) &&
               IsUTF8TrailByte(s[i + 1]) &&
               IsUTF8TrailByte(s[i + 2]) &&

               (lead != 0xf0 || s[i] >= 0x90) &&
               (lead != 0xf4 || s[i] < 0x90)) {
      i += 3;
    } else {
      return false;
    }
  }
  return true;
}

void ConditionalPrintAsText(const char* str, size_t length, ostream* os) {
  if (!ContainsUnprintableControlCodes(str, length) &&
      IsValidUTF8(str, length)) {
    *os << "\n    As Text: \"" << str << "\"";
  }
}

}

void PrintStringTo(const ::std::string& s, ostream* os) {
  if (PrintCharsAsStringTo(s.data(), s.size(), os) == kHexEscape) {
    if (GTEST_FLAG(print_utf8)) {
      ConditionalPrintAsText(s.data(), s.size(), os);
    }
  }
}

#ifdef __cpp_char8_t
void PrintU8StringTo(const ::std::u8string& s, ostream* os) {
  PrintCharsAsStringTo(s.data(), s.size(), os);
}
#endif

void PrintU16StringTo(const ::std::u16string& s, ostream* os) {
  PrintCharsAsStringTo(s.data(), s.size(), os);
}

void PrintU32StringTo(const ::std::u32string& s, ostream* os) {
  PrintCharsAsStringTo(s.data(), s.size(), os);
}

#if GTEST_HAS_STD_WSTRING
void PrintWideStringTo(const ::std::wstring& s, ostream* os) {
  PrintCharsAsStringTo(s.data(), s.size(), os);
}
#endif

}

}

namespace testing {

using internal::GetUnitTestImpl;

std::string TestPartResult::ExtractSummary(const char* message) {
  const char* const stack_trace = strstr(message, internal::kStackTraceMarker);
  return stack_trace == nullptr ? message : std::string(message, stack_trace);
}

std::ostream& operator<<(std::ostream& os, const TestPartResult& result) {
  return os << internal::FormatFileLocation(result.file_name(),
                                            result.line_number())
            << " "
            << (result.type() == TestPartResult::kSuccess
                    ? "Success"
                    : result.type() == TestPartResult::kSkip
                          ? "Skipped"
                          : result.type() == TestPartResult::kFatalFailure
                                ? "Fatal failure"
                                : "Non-fatal failure")
            << ":\n"
            << result.message() << std::endl;
}

void TestPartResultArray::Append(const TestPartResult& result) {
  array_.push_back(result);
}

const TestPartResult& TestPartResultArray::GetTestPartResult(int index) const {
  if (index < 0 || index >= size()) {
    printf("\nInvalid index (%d) into TestPartResultArray.\n", index);
    internal::posix::Abort();
  }

  return array_[static_cast<size_t>(index)];
}

int TestPartResultArray::size() const {
  return static_cast<int>(array_.size());
}

namespace internal {

HasNewFatalFailureHelper::HasNewFatalFailureHelper()
    : has_new_fatal_failure_(false),
      original_reporter_(GetUnitTestImpl()->
                         GetTestPartResultReporterForCurrentThread()) {
  GetUnitTestImpl()->SetTestPartResultReporterForCurrentThread(this);
}

HasNewFatalFailureHelper::~HasNewFatalFailureHelper() {
  GetUnitTestImpl()->SetTestPartResultReporterForCurrentThread(
      original_reporter_);
}

void HasNewFatalFailureHelper::ReportTestPartResult(
    const TestPartResult& result) {
  if (result.fatally_failed())
    has_new_fatal_failure_ = true;
  original_reporter_->ReportTestPartResult(result);
}

}

}

namespace testing {
namespace internal {

static const char* SkipSpaces(const char* str) {
  while (IsSpace(*str))
    str++;
  return str;
}

static std::vector<std::string> SplitIntoTestNames(const char* src) {
  std::vector<std::string> name_vec;
  src = SkipSpaces(src);
  for (; src != nullptr; src = SkipComma(src)) {
    name_vec.push_back(StripTrailingSpaces(GetPrefixUntilComma(src)));
  }
  return name_vec;
}

const char* TypedTestSuitePState::VerifyRegisteredTestNames(
    const char* test_suite_name, const char* file, int line,
    const char* registered_tests) {
  RegisterTypeParameterizedTestSuite(test_suite_name, CodeLocation(file, line));

  typedef RegisteredTestsMap::const_iterator RegisteredTestIter;
  registered_ = true;

  std::vector<std::string> name_vec = SplitIntoTestNames(registered_tests);

  Message errors;

  std::set<std::string> tests;
  for (std::vector<std::string>::const_iterator name_it = name_vec.begin();
       name_it != name_vec.end(); ++name_it) {
    const std::string& name = *name_it;
    if (tests.count(name) != 0) {
      errors << "Test " << name << " is listed more than once.\n";
      continue;
    }

    if (registered_tests_.count(name) != 0) {
      tests.insert(name);
    } else {
      errors << "No test named " << name
             << " can be found in this test suite.\n";
    }
  }

  for (RegisteredTestIter it = registered_tests_.begin();
       it != registered_tests_.end();
       ++it) {
    if (tests.count(it->first) == 0) {
      errors << "You forgot to list test " << it->first << ".\n";
    }
  }

  const std::string& errors_str = errors.GetString();
  if (errors_str != "") {
    fprintf(stderr, "%s %s", FormatFileLocation(file, line).c_str(),
            errors_str.c_str());
    fflush(stderr);
    posix::Abort();
  }

  return registered_tests;
}

}
}

#include "gmock/gmock.h"

#include <limits.h>
#include <ostream>
#include <sstream>
#include <string>

namespace testing {

namespace {

class BetweenCardinalityImpl : public CardinalityInterface {
 public:
  BetweenCardinalityImpl(int min, int max)
      : min_(min >= 0 ? min : 0),
        max_(max >= min_ ? max : min_) {
    std::stringstream ss;
    if (min < 0) {
      ss << "The invocation lower bound must be >= 0, "
         << "but is actually " << min << ".";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    } else if (max < 0) {
      ss << "The invocation upper bound must be >= 0, "
         << "but is actually " << max << ".";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    } else if (min > max) {
      ss << "The invocation upper bound (" << max
         << ") must be >= the invocation lower bound (" << min
         << ").";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    }
  }

  int ConservativeLowerBound() const override { return min_; }
  int ConservativeUpperBound() const override { return max_; }

  bool IsSatisfiedByCallCount(int call_count) const override {
    return min_ <= call_count && call_count <= max_;
  }

  bool IsSaturatedByCallCount(int call_count) const override {
    return call_count >= max_;
  }

  void DescribeTo(::std::ostream* os) const override;

 private:
  const int min_;
  const int max_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(BetweenCardinalityImpl);
};

inline std::string FormatTimes(int n) {
  if (n == 1) {
    return "once";
  } else if (n == 2) {
    return "twice";
  } else {
    std::stringstream ss;
    ss << n << " times";
    return ss.str();
  }
}

void BetweenCardinalityImpl::DescribeTo(::std::ostream* os) const {
  if (min_ == 0) {
    if (max_ == 0) {
      *os << "never called";
    } else if (max_ == INT_MAX) {
      *os << "called any number of times";
    } else {
      *os << "called at most " << FormatTimes(max_);
    }
  } else if (min_ == max_) {
    *os << "called " << FormatTimes(min_);
  } else if (max_ == INT_MAX) {
    *os << "called at least " << FormatTimes(min_);
  } else {

    *os << "called between " << min_ << " and " << max_ << " times";
  }
}

}

void Cardinality::DescribeActualCallCountTo(int actual_call_count,
                                            ::std::ostream* os) {
  if (actual_call_count > 0) {
    *os << "called " << FormatTimes(actual_call_count);
  } else {
    *os << "never called";
  }
}

GTEST_API_ Cardinality AtLeast(int n) { return Between(n, INT_MAX); }

GTEST_API_ Cardinality AtMost(int n) { return Between(0, n); }

GTEST_API_ Cardinality AnyNumber() { return AtLeast(0); }

GTEST_API_ Cardinality Between(int min, int max) {
  return Cardinality(new BetweenCardinalityImpl(min, max));
}

GTEST_API_ Cardinality Exactly(int n) { return Between(n, n); }

}

#include <ctype.h>
#include <ostream>
#include <string>

namespace testing {
namespace internal {

GTEST_API_ std::string JoinAsTuple(const Strings& fields) {
  switch (fields.size()) {
    case 0:
      return "";
    case 1:
      return fields[0];
    default:
      std::string result = "(" + fields[0];
      for (size_t i = 1; i < fields.size(); i++) {
        result += ", ";
        result += fields[i];
      }
      result += ")";
      return result;
  }
}

GTEST_API_ std::string ConvertIdentifierNameToWords(const char* id_name) {
  std::string result;
  char prev_char = '\0';
  for (const char* p = id_name; *p != '\0'; prev_char = *(p++)) {

    const bool starts_new_word = IsUpper(*p) ||
        (!IsAlpha(prev_char) && IsLower(*p)) ||
        (!IsDigit(prev_char) && IsDigit(*p));

    if (IsAlNum(*p)) {
      if (starts_new_word && result != "")
        result += ' ';
      result += ToLower(*p);
    }
  }
  return result;
}

class GoogleTestFailureReporter : public FailureReporterInterface {
 public:
  void ReportFailure(FailureType type, const char* file, int line,
                     const std::string& message) override {
    AssertHelper(type == kFatal ?
                 TestPartResult::kFatalFailure :
                 TestPartResult::kNonFatalFailure,
                 file,
                 line,
                 message.c_str()) = Message();
    if (type == kFatal) {
      posix::Abort();
    }
  }
};

GTEST_API_ FailureReporterInterface* GetFailureReporter() {

  static FailureReporterInterface* const failure_reporter =
      new GoogleTestFailureReporter();
  return failure_reporter;
}

static GTEST_DEFINE_STATIC_MUTEX_(g_log_mutex);

GTEST_API_ bool LogIsVisible(LogSeverity severity) {
  if (GMOCK_FLAG(verbose) == kInfoVerbosity) {

    return true;
  } else if (GMOCK_FLAG(verbose) == kErrorVerbosity) {

    return false;
  } else {

    return severity == kWarning;
  }
}

GTEST_API_ void Log(LogSeverity severity, const std::string& message,
                    int stack_frames_to_skip) {
  if (!LogIsVisible(severity))
    return;

  MutexLock l(&g_log_mutex);

  if (severity == kWarning) {

    std::cout << "\nGMOCK WARNING:";
  }

  if (message.empty() || message[0] != '\n') {
    std::cout << "\n";
  }
  std::cout << message;
  if (stack_frames_to_skip >= 0) {
#ifdef NDEBUG

    const int actual_to_skip = 0;
#else

    const int actual_to_skip = stack_frames_to_skip + 1;
#endif

    if (!message.empty() && *message.rbegin() != '\n') {
      std::cout << "\n";
    }
    std::cout << "Stack trace:\n"
         << ::testing::internal::GetCurrentOsStackTraceExceptTop(
             ::testing::UnitTest::GetInstance(), actual_to_skip);
  }
  std::cout << ::std::flush;
}

GTEST_API_ WithoutMatchers GetWithoutMatchers() { return WithoutMatchers(); }

GTEST_API_ void IllegalDoDefault(const char* file, int line) {
  internal::Assert(
      false, file, line,
      "You are using DoDefault() inside a composite action like "
      "DoAll() or WithArgs().  This is not supported for technical "
      "reasons.  Please instead spell out the default action, or "
      "assign the default action to an Action variable and use "
      "the variable in various places.");
}

}
}

#include <string.h>
#include <iostream>
#include <sstream>
#include <string>

namespace testing {
namespace internal {

GTEST_API_ std::string FormatMatcherDescription(bool negation,
                                                const char* matcher_name,
                                                const Strings& param_values) {
  std::string result = ConvertIdentifierNameToWords(matcher_name);
  if (param_values.size() >= 1) result += " " + JoinAsTuple(param_values);
  return negation ? "not (" + result + ")" : result;
}

class MaxBipartiteMatchState {
 public:
  explicit MaxBipartiteMatchState(const MatchMatrix& graph)
      : graph_(&graph),
        left_(graph_->LhsSize(), kUnused),
        right_(graph_->RhsSize(), kUnused) {}

  ElementMatcherPairs Compute() {

    ::std::vector<char> seen;

    for (size_t ilhs = 0; ilhs < graph_->LhsSize(); ++ilhs) {

      GTEST_CHECK_(left_[ilhs] == kUnused)
          << "ilhs: " << ilhs << ", left_[ilhs]: " << left_[ilhs];

      seen.assign(graph_->RhsSize(), 0);
      TryAugment(ilhs, &seen);
    }
    ElementMatcherPairs result;
    for (size_t ilhs = 0; ilhs < left_.size(); ++ilhs) {
      size_t irhs = left_[ilhs];
      if (irhs == kUnused) continue;
      result.push_back(ElementMatcherPair(ilhs, irhs));
    }
    return result;
  }

 private:
  static const size_t kUnused = static_cast<size_t>(-1);

  bool TryAugment(size_t ilhs, ::std::vector<char>* seen) {
    for (size_t irhs = 0; irhs < graph_->RhsSize(); ++irhs) {
      if ((*seen)[irhs]) continue;
      if (!graph_->HasEdge(ilhs, irhs)) continue;

      (*seen)[irhs] = 1;

      if (right_[irhs] == kUnused || TryAugment(right_[irhs], seen)) {

        left_[ilhs] = irhs;
        right_[irhs] = ilhs;
        return true;
      }
    }
    return false;
  }

  const MatchMatrix* graph_;

  ::std::vector<size_t> left_;
  ::std::vector<size_t> right_;
};

const size_t MaxBipartiteMatchState::kUnused;

GTEST_API_ ElementMatcherPairs FindMaxBipartiteMatching(const MatchMatrix& g) {
  return MaxBipartiteMatchState(g).Compute();
}

static void LogElementMatcherPairVec(const ElementMatcherPairs& pairs,
                                     ::std::ostream* stream) {
  typedef ElementMatcherPairs::const_iterator Iter;
  ::std::ostream& os = *stream;
  os << "{";
  const char* sep = "";
  for (Iter it = pairs.begin(); it != pairs.end(); ++it) {
    os << sep << "\n  ("
       << "element #" << it->first << ", "
       << "matcher #" << it->second << ")";
    sep = ",";
  }
  os << "\n}";
}

bool MatchMatrix::NextGraph() {
  for (size_t ilhs = 0; ilhs < LhsSize(); ++ilhs) {
    for (size_t irhs = 0; irhs < RhsSize(); ++irhs) {
      char& b = matched_[SpaceIndex(ilhs, irhs)];
      if (!b) {
        b = 1;
        return true;
      }
      b = 0;
    }
  }
  return false;
}

void MatchMatrix::Randomize() {
  for (size_t ilhs = 0; ilhs < LhsSize(); ++ilhs) {
    for (size_t irhs = 0; irhs < RhsSize(); ++irhs) {
      char& b = matched_[SpaceIndex(ilhs, irhs)];
      b = static_cast<char>(rand() & 1);
    }
  }
}

std::string MatchMatrix::DebugString() const {
  ::std::stringstream ss;
  const char* sep = "";
  for (size_t i = 0; i < LhsSize(); ++i) {
    ss << sep;
    for (size_t j = 0; j < RhsSize(); ++j) {
      ss << HasEdge(i, j);
    }
    sep = ";";
  }
  return ss.str();
}

void UnorderedElementsAreMatcherImplBase::DescribeToImpl(
    ::std::ostream* os) const {
  switch (match_flags()) {
    case UnorderedMatcherRequire::ExactMatch:
      if (matcher_describers_.empty()) {
        *os << "is empty";
        return;
      }
      if (matcher_describers_.size() == 1) {
        *os << "has " << Elements(1) << " and that element ";
        matcher_describers_[0]->DescribeTo(os);
        return;
      }
      *os << "has " << Elements(matcher_describers_.size())
          << " and there exists some permutation of elements such that:\n";
      break;
    case UnorderedMatcherRequire::Superset:
      *os << "a surjection from elements to requirements exists such that:\n";
      break;
    case UnorderedMatcherRequire::Subset:
      *os << "an injection from elements to requirements exists such that:\n";
      break;
  }

  const char* sep = "";
  for (size_t i = 0; i != matcher_describers_.size(); ++i) {
    *os << sep;
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      *os << " - element #" << i << " ";
    } else {
      *os << " - an element ";
    }
    matcher_describers_[i]->DescribeTo(os);
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      sep = ", and\n";
    } else {
      sep = "\n";
    }
  }
}

void UnorderedElementsAreMatcherImplBase::DescribeNegationToImpl(
    ::std::ostream* os) const {
  switch (match_flags()) {
    case UnorderedMatcherRequire::ExactMatch:
      if (matcher_describers_.empty()) {
        *os << "isn't empty";
        return;
      }
      if (matcher_describers_.size() == 1) {
        *os << "doesn't have " << Elements(1) << ", or has " << Elements(1)
            << " that ";
        matcher_describers_[0]->DescribeNegationTo(os);
        return;
      }
      *os << "doesn't have " << Elements(matcher_describers_.size())
          << ", or there exists no permutation of elements such that:\n";
      break;
    case UnorderedMatcherRequire::Superset:
      *os << "no surjection from elements to requirements exists such that:\n";
      break;
    case UnorderedMatcherRequire::Subset:
      *os << "no injection from elements to requirements exists such that:\n";
      break;
  }
  const char* sep = "";
  for (size_t i = 0; i != matcher_describers_.size(); ++i) {
    *os << sep;
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      *os << " - element #" << i << " ";
    } else {
      *os << " - an element ";
    }
    matcher_describers_[i]->DescribeTo(os);
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      sep = ", and\n";
    } else {
      sep = "\n";
    }
  }
}

bool UnorderedElementsAreMatcherImplBase::VerifyMatchMatrix(
    const ::std::vector<std::string>& element_printouts,
    const MatchMatrix& matrix, MatchResultListener* listener) const {
  bool result = true;
  ::std::vector<char> element_matched(matrix.LhsSize(), 0);
  ::std::vector<char> matcher_matched(matrix.RhsSize(), 0);

  for (size_t ilhs = 0; ilhs < matrix.LhsSize(); ilhs++) {
    for (size_t irhs = 0; irhs < matrix.RhsSize(); irhs++) {
      char matched = matrix.HasEdge(ilhs, irhs);
      element_matched[ilhs] |= matched;
      matcher_matched[irhs] |= matched;
    }
  }

  if (match_flags() & UnorderedMatcherRequire::Superset) {
    const char* sep =
        "where the following matchers don't match any elements:\n";
    for (size_t mi = 0; mi < matcher_matched.size(); ++mi) {
      if (matcher_matched[mi]) continue;
      result = false;
      if (listener->IsInterested()) {
        *listener << sep << "matcher #" << mi << ": ";
        matcher_describers_[mi]->DescribeTo(listener->stream());
        sep = ",\n";
      }
    }
  }

  if (match_flags() & UnorderedMatcherRequire::Subset) {
    const char* sep =
        "where the following elements don't match any matchers:\n";
    const char* outer_sep = "";
    if (!result) {
      outer_sep = "\nand ";
    }
    for (size_t ei = 0; ei < element_matched.size(); ++ei) {
      if (element_matched[ei]) continue;
      result = false;
      if (listener->IsInterested()) {
        *listener << outer_sep << sep << "element #" << ei << ": "
                  << element_printouts[ei];
        sep = ",\n";
        outer_sep = "";
      }
    }
  }
  return result;
}

bool UnorderedElementsAreMatcherImplBase::FindPairing(
    const MatchMatrix& matrix, MatchResultListener* listener) const {
  ElementMatcherPairs matches = FindMaxBipartiteMatching(matrix);

  size_t max_flow = matches.size();
  if ((match_flags() & UnorderedMatcherRequire::Superset) &&
      max_flow < matrix.RhsSize()) {
    if (listener->IsInterested()) {
      *listener << "where no permutation of the elements can satisfy all "
                   "matchers, and the closest match is "
                << max_flow << " of " << matrix.RhsSize()
                << " matchers with the pairings:\n";
      LogElementMatcherPairVec(matches, listener->stream());
    }
    return false;
  }
  if ((match_flags() & UnorderedMatcherRequire::Subset) &&
      max_flow < matrix.LhsSize()) {
    if (listener->IsInterested()) {
      *listener
          << "where not all elements can be matched, and the closest match is "
          << max_flow << " of " << matrix.RhsSize()
          << " matchers with the pairings:\n";
      LogElementMatcherPairVec(matches, listener->stream());
    }
    return false;
  }

  if (matches.size() > 1) {
    if (listener->IsInterested()) {
      const char* sep = "where:\n";
      for (size_t mi = 0; mi < matches.size(); ++mi) {
        *listener << sep << " - element #" << matches[mi].first
                  << " is matched by matcher #" << matches[mi].second;
        sep = ",\n";
      }
    }
  }
  return true;
}

}
}

#include <stdlib.h>

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#if GTEST_OS_CYGWIN || GTEST_OS_LINUX || GTEST_OS_MAC
# include <unistd.h>
#endif

#ifdef _MSC_VER
#if _MSC_VER == 1900
#  pragma warning(push)
#  pragma warning(disable:4800)
#endif
#endif

namespace testing {
namespace internal {

GTEST_API_ GTEST_DEFINE_STATIC_MUTEX_(g_gmock_mutex);

GTEST_API_ void LogWithLocation(testing::internal::LogSeverity severity,
                                const char* file, int line,
                                const std::string& message) {
  ::std::ostringstream s;
  s << internal::FormatFileLocation(file, line) << " " << message
    << ::std::endl;
  Log(severity, s.str(), 0);
}

ExpectationBase::ExpectationBase(const char* a_file, int a_line,
                                 const std::string& a_source_text)
    : file_(a_file),
      line_(a_line),
      source_text_(a_source_text),
      cardinality_specified_(false),
      cardinality_(Exactly(1)),
      call_count_(0),
      retired_(false),
      extra_matcher_specified_(false),
      repeated_action_specified_(false),
      retires_on_saturation_(false),
      last_clause_(kNone),
      action_count_checked_(false) {}

ExpectationBase::~ExpectationBase() {}

void ExpectationBase::SpecifyCardinality(const Cardinality& a_cardinality) {
  cardinality_specified_ = true;
  cardinality_ = a_cardinality;
}

void ExpectationBase::RetireAllPreRequisites()
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  if (is_retired()) {

    return;
  }

  ::std::vector<ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      ExpectationBase* next = it->expectation_base().get();
      if (!next->is_retired()) {
        next->Retire();
        expectations.push_back(next);
      }
    }
  }
}

bool ExpectationBase::AllPrerequisitesAreSatisfied() const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  ::std::vector<const ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    const ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      const ExpectationBase* next = it->expectation_base().get();
      if (!next->IsSatisfied()) return false;
      expectations.push_back(next);
    }
  }
  return true;
}

void ExpectationBase::FindUnsatisfiedPrerequisites(ExpectationSet* result) const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  ::std::vector<const ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    const ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      const ExpectationBase* next = it->expectation_base().get();

      if (next->IsSatisfied()) {

        if (next->call_count_ == 0) {
          expectations.push_back(next);
        }
      } else {

        *result += *it;
      }
    }
  }
}

void ExpectationBase::DescribeCallCountTo(::std::ostream* os) const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();

  *os << "         Expected: to be ";
  cardinality().DescribeTo(os);
  *os << "\n           Actual: ";
  Cardinality::DescribeActualCallCountTo(call_count(), os);

  *os << " - " << (IsOverSaturated() ? "over-saturated" :
                   IsSaturated() ? "saturated" :
                   IsSatisfied() ? "satisfied" : "unsatisfied")
      << " and "
      << (is_retired() ? "retired" : "active");
}

void ExpectationBase::CheckActionCountIfNotDone() const
    GTEST_LOCK_EXCLUDED_(mutex_) {
  bool should_check = false;
  {
    MutexLock l(&mutex_);
    if (!action_count_checked_) {
      action_count_checked_ = true;
      should_check = true;
    }
  }

  if (should_check) {
    if (!cardinality_specified_) {

      return;
    }

    const int action_count = static_cast<int>(untyped_actions_.size());
    const int upper_bound = cardinality().ConservativeUpperBound();
    const int lower_bound = cardinality().ConservativeLowerBound();
    bool too_many;

    if (action_count > upper_bound ||
        (action_count == upper_bound && repeated_action_specified_)) {
      too_many = true;
    } else if (0 < action_count && action_count < lower_bound &&
               !repeated_action_specified_) {
      too_many = false;
    } else {
      return;
    }

    ::std::stringstream ss;
    DescribeLocationTo(&ss);
    ss << "Too " << (too_many ? "many" : "few")
       << " actions specified in " << source_text() << "...\n"
       << "Expected to be ";
    cardinality().DescribeTo(&ss);
    ss << ", but has " << (too_many ? "" : "only ")
       << action_count << " WillOnce()"
       << (action_count == 1 ? "" : "s");
    if (repeated_action_specified_) {
      ss << " and a WillRepeatedly()";
    }
    ss << ".";
    Log(kWarning, ss.str(), -1);
  }
}

void ExpectationBase::UntypedTimes(const Cardinality& a_cardinality) {
  if (last_clause_ == kTimes) {
    ExpectSpecProperty(false,
                       ".Times() cannot appear "
                       "more than once in an EXPECT_CALL().");
  } else {
    ExpectSpecProperty(last_clause_ < kTimes,
                       ".Times() cannot appear after "
                       ".InSequence(), .WillOnce(), .WillRepeatedly(), "
                       "or .RetiresOnSaturation().");
  }
  last_clause_ = kTimes;

  SpecifyCardinality(a_cardinality);
}

GTEST_API_ ThreadLocal<Sequence*> g_gmock_implicit_sequence;

void ReportUninterestingCall(CallReaction reaction, const std::string& msg) {

  const int stack_frames_to_skip =
      GMOCK_FLAG(verbose) == kInfoVerbosity ? 3 : -1;
  switch (reaction) {
    case kAllow:
      Log(kInfo, msg, stack_frames_to_skip);
      break;
    case kWarn:
      Log(kWarning,
          msg +
              "\nNOTE: You can safely ignore the above warning unless this "
              "call should not happen.  Do not suppress it by blindly adding "
              "an EXPECT_CALL() if you don't mean to enforce the call.  "
              "See "
              "https://github.com/google/googletest/blob/master/docs/"
              "gmock_cook_book.md#"
              "knowing-when-to-expect for details.\n",
          stack_frames_to_skip);
      break;
    default:
      Expect(false, nullptr, -1, msg);
  }
}

UntypedFunctionMockerBase::UntypedFunctionMockerBase()
    : mock_obj_(nullptr), name_("") {}

UntypedFunctionMockerBase::~UntypedFunctionMockerBase() {}

void UntypedFunctionMockerBase::RegisterOwner(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  {
    MutexLock l(&g_gmock_mutex);
    mock_obj_ = mock_obj;
  }
  Mock::Register(mock_obj, this);
}

void UntypedFunctionMockerBase::SetOwnerAndName(const void* mock_obj,
                                                const char* name)
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {

  MutexLock l(&g_gmock_mutex);
  mock_obj_ = mock_obj;
  name_ = name;
}

const void* UntypedFunctionMockerBase::MockObject() const
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  const void* mock_obj;
  {

    MutexLock l(&g_gmock_mutex);
    Assert(mock_obj_ != nullptr, __FILE__, __LINE__,
           "MockObject() must not be called before RegisterOwner() or "
           "SetOwnerAndName() has been called.");
    mock_obj = mock_obj_;
  }
  return mock_obj;
}

const char* UntypedFunctionMockerBase::Name() const
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  const char* name;
  {

    MutexLock l(&g_gmock_mutex);
    Assert(name_ != nullptr, __FILE__, __LINE__,
           "Name() must not be called before SetOwnerAndName() has "
           "been called.");
    name = name_;
  }
  return name;
}

UntypedActionResultHolderBase* UntypedFunctionMockerBase::UntypedInvokeWith(
    void* const untyped_args) GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {

  if (untyped_expectations_.size() == 0) {

    const CallReaction reaction =
        Mock::GetReactionOnUninterestingCalls(MockObject());

    const bool need_to_report_uninteresting_call =

        reaction == kAllow ? LogIsVisible(kInfo) :

            reaction == kWarn
                ? LogIsVisible(kWarning)
                :

                true;

    if (!need_to_report_uninteresting_call) {

      return this->UntypedPerformDefaultAction(
          untyped_args, "Function call: " + std::string(Name()));
    }

    ::std::stringstream ss;
    this->UntypedDescribeUninterestingCall(untyped_args, &ss);

    UntypedActionResultHolderBase* const result =
        this->UntypedPerformDefaultAction(untyped_args, ss.str());

    if (result != nullptr) result->PrintAsActionResult(&ss);

    ReportUninterestingCall(reaction, ss.str());
    return result;
  }

  bool is_excessive = false;
  ::std::stringstream ss;
  ::std::stringstream why;
  ::std::stringstream loc;
  const void* untyped_action = nullptr;

  const ExpectationBase* const untyped_expectation =
      this->UntypedFindMatchingExpectation(untyped_args, &untyped_action,
                                           &is_excessive, &ss, &why);
  const bool found = untyped_expectation != nullptr;

  const bool need_to_report_call =
      !found || is_excessive || LogIsVisible(kInfo);
  if (!need_to_report_call) {

    return untyped_action == nullptr
               ? this->UntypedPerformDefaultAction(untyped_args, "")
               : this->UntypedPerformAction(untyped_action, untyped_args);
  }

  ss << "    Function call: " << Name();
  this->UntypedPrintArgs(untyped_args, &ss);

  if (found && !is_excessive) {
    untyped_expectation->DescribeLocationTo(&loc);
  }

  UntypedActionResultHolderBase* result = nullptr;

  auto perform_action = [&] {
    return untyped_action == nullptr
               ? this->UntypedPerformDefaultAction(untyped_args, ss.str())
               : this->UntypedPerformAction(untyped_action, untyped_args);
  };
  auto handle_failures = [&] {
    ss << "\n" << why.str();

    if (!found) {

      Expect(false, nullptr, -1, ss.str());
    } else if (is_excessive) {

      Expect(false, untyped_expectation->file(), untyped_expectation->line(),
             ss.str());
    } else {

      Log(kInfo, loc.str() + ss.str(), 2);
    }
  };
#if GTEST_HAS_EXCEPTIONS
  try {
    result = perform_action();
  } catch (...) {
    handle_failures();
    throw;
  }
#else
  result = perform_action();
#endif

  if (result != nullptr) result->PrintAsActionResult(&ss);
  handle_failures();
  return result;
}

Expectation UntypedFunctionMockerBase::GetHandleOf(ExpectationBase* exp) {

  for (UntypedExpectations::const_iterator it =
           untyped_expectations_.begin();
       it != untyped_expectations_.end(); ++it) {
    if (it->get() == exp) {
      return Expectation(*it);
    }
  }

  Assert(false, __FILE__, __LINE__, "Cannot find expectation.");
  return Expectation();

}

bool UntypedFunctionMockerBase::VerifyAndClearExpectationsLocked()
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  bool expectations_met = true;
  for (UntypedExpectations::const_iterator it =
           untyped_expectations_.begin();
       it != untyped_expectations_.end(); ++it) {
    ExpectationBase* const untyped_expectation = it->get();
    if (untyped_expectation->IsOverSaturated()) {

      expectations_met = false;
    } else if (!untyped_expectation->IsSatisfied()) {
      expectations_met = false;
      ::std::stringstream ss;
      ss  << "Actual function call count doesn't match "
          << untyped_expectation->source_text() << "...\n";

      untyped_expectation->MaybeDescribeExtraMatcherTo(&ss);
      untyped_expectation->DescribeCallCountTo(&ss);
      Expect(false, untyped_expectation->file(),
             untyped_expectation->line(), ss.str());
    }
  }

  UntypedExpectations expectations_to_delete;
  untyped_expectations_.swap(expectations_to_delete);

  g_gmock_mutex.Unlock();
  expectations_to_delete.clear();
  g_gmock_mutex.Lock();

  return expectations_met;
}

CallReaction intToCallReaction(int mock_behavior) {
  if (mock_behavior >= kAllow && mock_behavior <= kFail) {
    return static_cast<internal::CallReaction>(mock_behavior);
  }
  return kWarn;
}

}

namespace {

typedef std::set<internal::UntypedFunctionMockerBase*> FunctionMockers;

struct MockObjectState {
  MockObjectState()
      : first_used_file(nullptr), first_used_line(-1), leakable(false) {}

  const char* first_used_file;
  int first_used_line;
  ::std::string first_used_test_suite;
  ::std::string first_used_test;
  bool leakable;
  FunctionMockers function_mockers;
};

class MockObjectRegistry {
 public:

  typedef std::map<const void*, MockObjectState> StateMap;

  ~MockObjectRegistry() {
    if (!GMOCK_FLAG(catch_leaked_mocks))
      return;

    int leaked_count = 0;
    for (StateMap::const_iterator it = states_.begin(); it != states_.end();
         ++it) {
      if (it->second.leakable)
        continue;

      std::cout << "\n";
      const MockObjectState& state = it->second;
      std::cout << internal::FormatFileLocation(state.first_used_file,
                                                state.first_used_line);
      std::cout << " ERROR: this mock object";
      if (state.first_used_test != "") {
        std::cout << " (used in test " << state.first_used_test_suite << "."
                  << state.first_used_test << ")";
      }
      std::cout << " should be deleted but never is. Its address is @"
           << it->first << ".";
      leaked_count++;
    }
    if (leaked_count > 0) {
      std::cout << "\nERROR: " << leaked_count << " leaked mock "
                << (leaked_count == 1 ? "object" : "objects")
                << " found at program exit. Expectations on a mock object are "
                   "verified when the object is destructed. Leaking a mock "
                   "means that its expectations aren't verified, which is "
                   "usually a test bug. If you really intend to leak a mock, "
                   "you can suppress this error using "
                   "testing::Mock::AllowLeak(mock_object), or you may use a "
                   "fake or stub instead of a mock.\n";
      std::cout.flush();
      ::std::cerr.flush();

      _exit(1);

    }
  }

  StateMap& states() { return states_; }

 private:
  StateMap states_;
};

MockObjectRegistry g_mock_object_registry;

std::map<const void*, internal::CallReaction> g_uninteresting_call_reaction;

void SetReactionOnUninterestingCalls(const void* mock_obj,
                                     internal::CallReaction reaction)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_uninteresting_call_reaction[mock_obj] = reaction;
}

}

void Mock::AllowUninterestingCalls(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kAllow);
}

void Mock::WarnUninterestingCalls(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kWarn);
}

void Mock::FailUninterestingCalls(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kFail);
}

void Mock::UnregisterCallReaction(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_uninteresting_call_reaction.erase(mock_obj);
}

internal::CallReaction Mock::GetReactionOnUninterestingCalls(
    const void* mock_obj)
        GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return (g_uninteresting_call_reaction.count(mock_obj) == 0) ?
      internal::intToCallReaction(GMOCK_FLAG(default_mock_behavior)) :
      g_uninteresting_call_reaction[mock_obj];
}

void Mock::AllowLeak(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].leakable = true;
}

bool Mock::VerifyAndClearExpectations(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

bool Mock::VerifyAndClear(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  ClearDefaultActionsLocked(mock_obj);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

bool Mock::VerifyAndClearExpectationsLocked(void* mock_obj)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();
  if (g_mock_object_registry.states().count(mock_obj) == 0) {

    return true;
  }

  bool expectations_met = true;
  FunctionMockers& mockers =
      g_mock_object_registry.states()[mock_obj].function_mockers;
  for (FunctionMockers::const_iterator it = mockers.begin();
       it != mockers.end(); ++it) {
    if (!(*it)->VerifyAndClearExpectationsLocked()) {
      expectations_met = false;
    }
  }

  return expectations_met;
}

bool Mock::IsNaggy(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kWarn;
}
bool Mock::IsNice(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kAllow;
}
bool Mock::IsStrict(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kFail;
}

void Mock::Register(const void* mock_obj,
                    internal::UntypedFunctionMockerBase* mocker)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].function_mockers.insert(mocker);
}

void Mock::RegisterUseByOnCallOrExpectCall(const void* mock_obj,
                                           const char* file, int line)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  MockObjectState& state = g_mock_object_registry.states()[mock_obj];
  if (state.first_used_file == nullptr) {
    state.first_used_file = file;
    state.first_used_line = line;
    const TestInfo* const test_info =
        UnitTest::GetInstance()->current_test_info();
    if (test_info != nullptr) {
      state.first_used_test_suite = test_info->test_suite_name();
      state.first_used_test = test_info->name();
    }
  }
}

void Mock::UnregisterLocked(internal::UntypedFunctionMockerBase* mocker)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();
  for (MockObjectRegistry::StateMap::iterator it =
           g_mock_object_registry.states().begin();
       it != g_mock_object_registry.states().end(); ++it) {
    FunctionMockers& mockers = it->second.function_mockers;
    if (mockers.erase(mocker) > 0) {

      if (mockers.empty()) {
        g_mock_object_registry.states().erase(it);
      }
      return;
    }
  }
}

void Mock::ClearDefaultActionsLocked(void* mock_obj)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();

  if (g_mock_object_registry.states().count(mock_obj) == 0) {

    return;
  }

  FunctionMockers& mockers =
      g_mock_object_registry.states()[mock_obj].function_mockers;
  for (FunctionMockers::const_iterator it = mockers.begin();
       it != mockers.end(); ++it) {
    (*it)->ClearDefaultActionsLocked();
  }

}

Expectation::Expectation() {}

Expectation::Expectation(
    const std::shared_ptr<internal::ExpectationBase>& an_expectation_base)
    : expectation_base_(an_expectation_base) {}

Expectation::~Expectation() {}

void Sequence::AddExpectation(const Expectation& expectation) const {
  if (*last_expectation_ != expectation) {
    if (last_expectation_->expectation_base() != nullptr) {
      expectation.expectation_base()->immediate_prerequisites_
          += *last_expectation_;
    }
    *last_expectation_ = expectation;
  }
}

InSequence::InSequence() {
  if (internal::g_gmock_implicit_sequence.get() == nullptr) {
    internal::g_gmock_implicit_sequence.set(new Sequence);
    sequence_created_ = true;
  } else {
    sequence_created_ = false;
  }
}

InSequence::~InSequence() {
  if (sequence_created_) {
    delete internal::g_gmock_implicit_sequence.get();
    internal::g_gmock_implicit_sequence.set(nullptr);
  }
}

}

#ifdef _MSC_VER
#if _MSC_VER == 1900
#  pragma warning(pop)
#endif
#endif

namespace testing {

GMOCK_DEFINE_bool_(catch_leaked_mocks, true,
                   "true if and only if Google Mock should report leaked "
                   "mock objects as failures.");

GMOCK_DEFINE_string_(verbose, internal::kWarningVerbosity,
                     "Controls how verbose Google Mock's output is."
                     "  Valid values:\n"
                     "  info    - prints all messages.\n"
                     "  warning - prints warnings and errors.\n"
                     "  error   - prints errors only.");

GMOCK_DEFINE_int32_(default_mock_behavior, 1,
                    "Controls the default behavior of mocks."
                    "  Valid values:\n"
                    "  0 - by default, mocks act as NiceMocks.\n"
                    "  1 - by default, mocks act as NaggyMocks.\n"
                    "  2 - by default, mocks act as StrictMocks.");

namespace internal {

static const char* ParseGoogleMockFlagValue(const char* str,
                                            const char* flag,
                                            bool def_optional) {

  if (str == nullptr || flag == nullptr) return nullptr;

  const std::string flag_str = std::string("--gmock_") + flag;
  const size_t flag_len = flag_str.length();
  if (strncmp(str, flag_str.c_str(), flag_len) != 0) return nullptr;

  const char* flag_end = str + flag_len;

  if (def_optional && (flag_end[0] == '\0')) {
    return flag_end;
  }

  if (flag_end[0] != '=') return nullptr;

  return flag_end + 1;
}

static bool ParseGoogleMockBoolFlag(const char* str, const char* flag,
                                    bool* value) {

  const char* const value_str = ParseGoogleMockFlagValue(str, flag, true);

  if (value_str == nullptr) return false;

  *value = !(*value_str == '0' || *value_str == 'f' || *value_str == 'F');
  return true;
}

template <typename String>
static bool ParseGoogleMockStringFlag(const char* str, const char* flag,
                                      String* value) {

  const char* const value_str = ParseGoogleMockFlagValue(str, flag, false);

  if (value_str == nullptr) return false;

  *value = value_str;
  return true;
}

static bool ParseGoogleMockIntFlag(const char* str, const char* flag,
                                   int32_t* value) {

  const char* const value_str = ParseGoogleMockFlagValue(str, flag, true);

  if (value_str == nullptr) return false;

  return ParseInt32(Message() << "The value of flag --" << flag,
                    value_str, value);
}

template <typename CharType>
void InitGoogleMockImpl(int* argc, CharType** argv) {

  InitGoogleTest(argc, argv);
  if (*argc <= 0) return;

  for (int i = 1; i != *argc; i++) {
    const std::string arg_string = StreamableToString(argv[i]);
    const char* const arg = arg_string.c_str();

    if (ParseGoogleMockBoolFlag(arg, "catch_leaked_mocks",
                                &GMOCK_FLAG(catch_leaked_mocks)) ||
        ParseGoogleMockStringFlag(arg, "verbose", &GMOCK_FLAG(verbose)) ||
        ParseGoogleMockIntFlag(arg, "default_mock_behavior",
                               &GMOCK_FLAG(default_mock_behavior))) {

      for (int j = i; j != *argc; j++) {
        argv[j] = argv[j + 1];
      }

      (*argc)--;

      i--;
    }
  }
}

}

GTEST_API_ void InitGoogleMock(int* argc, char** argv) {
  internal::InitGoogleMockImpl(argc, argv);
}

GTEST_API_ void InitGoogleMock(int* argc, wchar_t** argv) {
  internal::InitGoogleMockImpl(argc, argv);
}

GTEST_API_ void InitGoogleMock() {

  int argc = 1;
  const auto arg0 = "dummy";
  char* argv0 = const_cast<char*>(arg0);
  char** argv = &argv0;

  internal::InitGoogleMockImpl(&argc, argv);
}

}
