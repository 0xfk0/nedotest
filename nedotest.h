/* vim: set et sts=4 sw=4: */
#ifndef _NT_HEADER

/* Below is listed definitions, which you can use directly for creating
 * your unit tests. If you have issue with conflicting names, you
 * can define macro _NT_USE_PREFIX and then all definitions must
 * be prefixed with _NT_ prefix (except of _LT_, _GT_ and other
 * binary operations, which must have undescore as prefix and suffix). */

#if !_NT_USE_PREFIX

/* Define new test function, example:
 *
 * TEST(suite1, func1)
 * {
 *    int x = func();
 *    ...
 *    CHECK(x EQ 42);
 * }
 *
 * In test function body you may use CHECK, REQUIRE and similar assertions
 * to check conditions and report test failures.
 *
 * Suite name and test name must be valid C identifiers (i.e. start from
 * letter or underscore, and consist of letters, underscores and digits).
 */
#define TEST(suite, name)       _NT_TEST(suite, name)

/* Define test fixture type, example:
 *
 * struct FIXTURE(suite1)
 * {
 *      int x;
 *      char y;
 *      ...
 * };
 *
 * Fixture is the structure in which you may store  some arbitrary data.
 * You may define "setup" and "teardown" functions (see below) which
 * if defined, will be called before and after each of the tests belonging to
 * particular suite. These functions will get pointer to fixture as an
 * argument.
 *
 * Fixture must be initialized by setup function and all necessary cleanup
 * actions must be performed by teardown function. Fixture not exists and
 * not preserves it's state after exiting from teardown function!
 *
 * Setup and teardown functions, and fixture might be defined
 * for each separate suite.
 */
#define FIXTURE(suite)          _NT_FIXTURE(suite)

/* Gets fixture within setup, teardown functions  and the test function. */
#define GET_FIXTURE()           _NT_GET_FIXTURE()

/* Define setup function, which will be called for each test belonged to the
 * specified suite, before starting the test itself. Example:
 *
 * TEST_SETUP(suite1)
 * {
 *     struct FIXTURE(suite1) *thiz = GET_FIXTURE();
 *     thiz->mem = malloc(100500);
 *     ...
 * }
 *
 * Each setup and teardown function receive "hidden" argument "thiz", which
 * points to fixture associated with particular test suite.
 *
 */
#define TEST_SETUP(suite)       _NT_TEST_SETUP(suite)

/* Define teardown function, which will be called after completion of each
 * test belonging to specified suite. Teardown function is called regardless
 * of test is failed or successfully completed. Example:
 *
 * TEST_TEARDOWN(suite1)
 * {
 *     struct FIXTURE(suite1) *thiz = GET_FIXTURE();
 *     free(thiz->mem);
 *     ...
 * }
 */
#define TEST_TEARDOWN(suite)    _NT_TEST_TEARDOWN(suite)

/* Define assertion for test functions. There are basically two types of
 * assertions: CHECK and REQUIRE. If any of these failed, when written logical
 * condition is not satisfied, the test will be considered as failed.
 * But if CHECK assertion failed, test function will continue to run and
 * might generate more error messages. And if REQUIRE fails, test function
 * will be terminated immediately (via longjmp function call).
 *
 * Both, CHECK and REQUIRE expected one of the binary operations listed
 * below (LT, EQ, ..., STREQ...), example:
 *
 * TEST(suite1, func1)
 * {
 *     int x = func();
 *     CHECK(x EQ 42);
 *     const char *y = func2();
 *     REQUIRE(y STREQ "exact match");
 *     ...
 *     struct FIXTURE(suite1) *thiz = GET_FIXTURE();
 *     thiz->mem = xxx;
 *     ...
 * }
 *
 * You must must avoid usingo of regular C operators like ==, !=, etc,
 * as in case of test failure test system will be unable to print actual
 * variable values (which is done, when you are using named binary operations
 * in place of C operators).
 *
 * You can't write complex expression by using named binary operatinos,
 * like this: CHECK(x EQ z && y NE 10). If you need complex expressions,
 * which consists of multiple operators, you need or split it to series
 * of simple operations:
 *
 *   CHECK(x EQ z), CHECK(y NE 10);
 *
 * In this case if tests fails you will get reasonable error messages:
 *
 *   test failed: CHECK expression (x == z)
 *   where "x" == 4, and "z" == 5.
 *
 * Or you need to write complex expression by using C-operators:
 *
 *   CHECK(x == z && y != 10);
 *
 * But as stated above, in case of failure you will get obscure message like:
 *
 *   test failed: CHECK expression (x == z && y != 10)
 *   where "x == z && y != 10" = 0.
 *
 */
#define CHECK(expr)             _NT_CHECK(expr)
#define REQUIRE(expr)           _NT_REQUIRE(expr)

/* These assertions require, that logical condition must be false,
 * so expression like CHECK_FALSE(x EQ 5) fails when x is equal to 5. */
#define CHECK_FALSE(expr)       _NT_CHECK_FALSE(expr)
#define REQUIRE_FALSE(expr)     _NT_REQUIRE_FALSE(expr)

/* These assertions allow to print a diagnostic message in a case, if given
 * condition evaluates to  false. These keywords can be used in place of `if`
 * and `if..else` operators in source code. */
#define CHECKED_IF(...)         _NT_CHECKED_IF(__VA_ARGS__)
#define CHECKED_ELSE(...)       _NT_CHECKED_ELSE(__VA_ARGS__)

/* This allows to print a  diagnostic message in a  case, if the given condition
 * is evaluated  to false (same as `REQUIRE` or `CHECK` do). The difference is
 * that the test will not be marked as failed in this case, and test continues. */
#define CHECK_NOFAIL(...)       _NT_CHECK_NOFAIL(__VA_ARGS__)

/* These similar to REQUIRE and REQUIRE_FALSE, but both require strictly
 * compile time constant as argument. These assertions might be used
 * in place of "static_assert" (by using these assertions compile time
 * error can be turned into test error in the run time). */
#define STATIC_REQUIRE(...)         _NT_STATIC_REQUIRE(__VA_ARGS__)
#define STATIC_REQUIRE_FALSE(...)   _NT_STATIC_REQUIRE_FALSE(__VA_ARGS__)


/* Binary operations to use with CHECK and REQUIRE assertions,
 * following operations applicable to scalar arithmetic data types
 * and to pointers: */
#define LT       _LT_           /* less than (<)        */
#define GT       _GT_           /* greater than (>)     */
#define LE       _LE_           /* less or equal (<=)   */
#define GE       _GE_           /* great or equal (>=)  */
#define EQ       _EQ_           /* equal (==)           */
#define NE       _NE_           /* not equal (!=)       */


/* Binary opearations to use with CHECK and REQUIRE assertions,
 * it is assumed, both operands must be valid C-strings: */

#define STREQ    _STREQ_        /* Both strings must be same. */
#define STRNE    _STRNE_        /* Strings must be different. */

/* This operation checks, that left string contains right one, in other words,
 * right string must be a substring of the left string. */
#define CONTAINS _CONTAINS_

#define NOT_CONTAINS _NOT_CONTAINS_

/* This operation checks, that left string matches to the pattern given in
 * right string. Comparison performed in case insensetive way (only for
 * ASCII symbols, locale not matters). For all symbols pattern matched
 * char to char, but asterisk ('*') might replace any number of other
 * characters. Example:
 *
 *   CHECK("Test string" MATCHES "test *ing");  // okay
 *   CHECK("other string" MATCHES "string");    // fail
 *   CHECK("third string" MATCHES "*str*");     // okay
 */
#define MATCHES  _MATCHES_

#define NOT_MATCHES  _NOT_MATCHES_

/* This macro causes test failure with the given message. 
 * This and following macro accepts arbitrary arguments, which
 * might be strings, numbers, or a pointers. All values will be
 * concatenated with spaces. */
#define FAIL(...)               _NT_FAIL(__VA_ARGS__)

/* This only marks the test as failed, but test execution continues. */
#define FAIL_CHECK(...)         _NT_FAIL_CHECK(__VA_ARGS__)

/* This macro unconditionally prints warning message. */
#define WARN(...)               _NT_WARN(__VA_ARGS__)

/* This macro prints a message only in a case if this test fails
 * in future. If test not fails, message will not be printed. */
#define INFO(...)               _NT_INFO(__VA_ARGS__)

/* Capture variable values and if in future test fails, these
 * values will be printed. The difference with INFO macro is that
 * this macro also prints variable names (arguments of a macro
 * must be variables). */
#define CAPTURE(...)            _NT_CAPTURE(__VA_ARGS__)

/* Skip currently running test with a message (test execution aborted). */
#define SKIP_TEST(...)          _NT_SKIP(__VA_ARGS__)

/* Mark current test as passed with a message (test execution aborted). */
#define SUCCEED(...)            _NT_SUCCEED(__VA_ARGS__)

/* This macro allows to check if currently running test failed. */
#define TEST_FAILED()           _NT_TEST_FAILED()


/* Following macros allows you define mocked function. Test system will provide
 * you definition of the function with specified return type and argument types.
 * Function name will be "__wrap_<func_name>".
 *
 * Later you may change return value for this function, may get calls count,
 * you may supply own function which must be called in place of mocked function.
 *
 * If you want to mock already existing function, for example library function,
 * or other function already existing in your code, you need "wrap" somehow
 * call to real function, to the call to fake function defined by MOCK_FUNCTION
 * macros. Basically, there are two ways for it:
 *
 *   1) First method works only if all code available in source code form:
 *      pass -Dfunc=__wrap_func option to the compiler when compiling
 *      your code which uses mocked function, then pass -D__real_func=func
 *      when compiling the test itself.
 *
 *   2) use --wrap=func option with gcc/clang compilers
 *      (this works for libraries too).
 *
 * Also you might find useful option of Microsoft's Visual C compiler:
 * "/alternatename:__real_func=func".
 */
#define MOCK_FUNCTION(result_type, func_name, ...) \
    _NT_MOCK_FUNCTION(result_type, func_name, __VA_ARGS__)

#define MOCK_OVERRIDE(result_type, func_name, ...) \
    _NT_MOCK_OVERRIDE(result_type, func_name, __VA_ARGS__)


/* Reset call counter, result and callback function for given mocked function
 * `func`. I mocked function was defined via MOCK_FUNCTION, it will start to
 * return some default (initialized by zeros) return value. If mock was defined
 * by MOCK_EXISTING_FUNC, calls to mocked function will be directed to original
 * function which mock can override. */
#define MOCK_RESET(func)                _NT_MOCK_RESET(func)

/* This expression returns number of times mocked function was called. */
#define MOCK_COUNT(func)                _NT_MOCK_COUNT(func)

/* This sets result value for mocked function. All subsequent calls to mocked
 * function will return specified value. */
#define MOCK_SET_RESULT(func, result)   _NT_MOCK_SET_RESULT(func, result)

/* This allows redirect call to mocked function to other function defined
 * in your code. You must use this possibility when you need to implement
 * complex behaviour of the mocked function. */
#define MOCK_SET_FUNC(func, func_ptr)   _NT_MOCK_SET_FUNC(func, func_ptr)

#endif  /* !_NT_PREFIX */

/* 
 * Public header finished at this point, you must not use the definitins found below directly!
 */

#ifdef __GNUC__
#define _NT_UNUSED(var) var __attribute__((unused))
#else
#define _NT_UNUSED(var) var
#endif

#ifdef __cplusplus
#define _NT_NULL 0
#else
#define _NT_NULL ((void*)0)
#endif

/* Note, following macroses will declare _nt_OP_char functions
 * which actually never be used (as char is expanded to int due
 * to integral promotion), but for simplicity those functions
 * retained as is. */

/* Declare all known, distinguishable data types for which binary operations
 * (like less, great, equal, etc...) can be applied */
#define _NT_TYPES(APPLY, ...)                                \
    APPLY(Char,             char,               __VA_ARGS__) \
    APPLY(signed_char,      signed char,        __VA_ARGS__) \
    APPLY(unsigned_char,    unsigned char,      __VA_ARGS__) \
    APPLY(any_signed,       long long,          __VA_ARGS__) \
    APPLY(any_unsigned,     unsigned long long, __VA_ARGS__) \
    APPLY(any_float,        long double,        __VA_ARGS__) \
    APPLY(any_pointer,      const void *,       __VA_ARGS__) \
    APPLY(cstring,          const char *,       __VA_ARGS__)

/* Define mapping between native types, the types specified above,
 * and functions which able to print value of corresponding type.
 * One exception is generic pointer, it is handled separately
 * (as default value in _Generic selection). */
#define _NT_TYPEMAP(APPLY, ...)                                                       \
    APPLY(                char, Char,           char,                    __VA_ARGS__) \
    APPLY(         signed char, signed_char,    signed_char,             __VA_ARGS__) \
    APPLY(       unsigned char, unsigned_char,  unsigned_char,           __VA_ARGS__) \
    APPLY(      unsigned short, any_unsigned,   unsigned_short,          __VA_ARGS__) \
    APPLY(            unsigned, any_unsigned,   unsigned,                __VA_ARGS__) \
    APPLY(       unsigned long, any_unsigned,   unsigned_long,           __VA_ARGS__) \
    APPLY(  unsigned long long, any_unsigned,   unsigned_long_long,      __VA_ARGS__) \
    APPLY(               short, any_signed,     short,                   __VA_ARGS__) \
    APPLY(                 int, any_signed,     int,                     __VA_ARGS__) \
    APPLY(                long, any_signed,     long,                    __VA_ARGS__) \
    APPLY(           long long, any_signed,     long_long,               __VA_ARGS__) \
    APPLY(               float, any_float,      float,                   __VA_ARGS__) \
    APPLY(              double, any_float,      double,                  __VA_ARGS__) \
    APPLY(         long double, any_float,      long_double,             __VA_ARGS__) \
    APPLY(         const char*, cstring,        const_char_ptr,          __VA_ARGS__) \
    APPLY(               char*, cstring,        char_ptr,                __VA_ARGS__) \
    APPLY(  const signed char*, cstring,        const_signed_char_ptr,   __VA_ARGS__) \
    APPLY(        signed char*, cstring,        signed_char_ptr,         __VA_ARGS__) \
    APPLY(const unsigned char*, cstring,        const_unsigned_char_ptr, __VA_ARGS__) \
    APPLY(      unsigned char*, cstring,        unsigned_char_ptr,       __VA_ARGS__)

/* This macros puts given argument to double quotes: str -> "str". */
#define _NT_STRINGIFY(str)  _NT_STRINGIFY_(str)
#define _NT_STRINGIFY_(str) #str

/* This macros contatenates two given arguments. */
#define _NT_CONCAT(a, b)    _NT_CONCAT_(a, b)
#define _NT_CONCAT_(a, b)   a##b

/* This macros allows to get expression of that type which is common for both
 * arguments in sense, that both must be converted to that type before applying
 * binary operation. Note, that this macros also performs integral promotion,
 * so type information will be lost for char-types. */
#define _NT_EXPR_TYPE(left, right) (0 ? (0 ? left : right) : 0)

/* This macros transforms symbolic name of the binary operation to the
 * corresponding operator in C language. Some operations, which not existing
 * in C transformed to && operator. Later this is used only to check, that
 * resulting expression is compilable. To perform expression evaluation
 * _NT_FUNC macros is used. */
#define _NT_OP(op)  _NT_CONCAT(_NT, op)
#define _NT_OP_LT_              <
#define _NT_OP_GT_              >
#define _NT_OP_LE_              <=
#define _NT_OP_GE_              >=
#define _NT_OP_EQ_              ==
#define _NT_OP_NE_              !=
#define _NT_OP_STREQ_           !=
#define _NT_OP_STRNE_           !=
#define _NT_OP_CONTAINS_        !=
#define _NT_OP_NOT_CONTAINS_    !=
#define _NT_OP_MATCHES_         !=
#define _NT_OP_NOT_MATCHES_     !=

/* This macros transform symbolic name of the binary operation to the
 * string which represents operation on assertion text. */
#define _NT_OP_NAME(op)     _NT_CONCAT(_NT_NAME, op)
#define _NT_NAME_OP_LT_             "<"
#define _NT_NAME_OP_GT_             ">"
#define _NT_NAME_OP_GE_             ">="
#define _NT_NAME_OP_LE_             "<="
#define _NT_NAME_OP_EQ_             "=="
#define _NT_NAME_OP_NE_             "!="
#define _NT_NAME_OP_STREQ_          "STREQ"
#define _NT_NAME_OP_STRNE_          "STRNE"
#define _NT_NAME_OP_CONTAINS_       "CONTAINS"
#define _NT_NAME_OP_NOT_CONTAINS_   "NOT_CONTAINS"
#define _NT_NAME_OP_MATCHES_        "MATCHES"
#define _NT_NAME_OP_NOT_MATCHES_    "NOT_MATCHES"

/* Following set of definitions used as symbolic names of the binary
 * operations applicable with CHECK and REQUIRE macroses. */
#define _LT_            ,_OP_LT_,
#define _GT_            ,_OP_GT_,
#define _LE_            ,_OP_LE_,
#define _GE_            ,_OP_GE_,
#define _EQ_            ,_OP_EQ_,
#define _NE_            ,_OP_NE_,
#define _STREQ_         ,_OP_STREQ_,
#define _STRNE_         ,_OP_STRNE_,
#define _CONTAINS_      ,_OP_CONTAINS_,
#define _NOT_CONTAINS_  ,_OP_NOT_CONTAINS_,
#define _MATCHES_       ,_OP_MATCHES_,
#define _NOT_MATCHES_   ,_OP_NOT_MATCHES_,

/* Define names of the known binary operations. */
#define _NT_BINOPS_SCALAR(APPLY, ...) \
    APPLY(_OP_LT_,  __VA_ARGS__)      \
    APPLY(_OP_GT_,  __VA_ARGS__)      \
    APPLY(_OP_LE_,  __VA_ARGS__)      \
    APPLY(_OP_GE_,  __VA_ARGS__)      \
    APPLY(_OP_EQ_,  __VA_ARGS__)      \
    APPLY(_OP_NE_,  __VA_ARGS__)

#define _NT_BINOPS_STRING(APPLY, ...)       \
    APPLY(_OP_STREQ_,       __VA_ARGS__)    \
    APPLY(_OP_STRNE_,       __VA_ARGS__)    \
    APPLY(_OP_CONTAINS_,    __VA_ARGS__)    \
    APPLY(_OP_NOT_CONTAINS_, __VA_ARGS__)   \
    APPLY(_OP_MATCHES_,     __VA_ARGS__)    \
    APPLY(_OP_NOT_MATCHES_,  __VA_ARGS__)

#define _NT_BINOPS_ALL(APPLY, ...)          \
    _NT_BINOPS_SCALAR(APPLY, __VA_ARGS__)   \
    _NT_BINOPS_STRING(APPLY, __VA_ARGS__)

/* This macros transforms symbolic name of the binary operation and arguments
 * type name to the name of the function performing such binary operation.
 * The name of this function has the form: _nt_OP_TYPE, there OP is operation
 * name (like LT, GT, etc...), and TYPE is one of the types defined by
 * _NT_TYPES macros (_nt_any_signed, _nt_float, etc...) */
#define _NT_FUNC(op, type) _NT_CONCAT(_NT_CONCAT(_nt, op), type)

/* Represent given names, as listed in _NT_TYPES, in C type name. */
#define _NT_TYPENAME(type) _NT_CONCAT(_nt_, type)

/* Forward declaration of required C types. */
#define _NT_DECL_TYPE(name, type, ...) typedef type _NT_TYPENAME(name);
_NT_TYPES(_NT_DECL_TYPE, dummy)

/* Define union to hold temporary variable representing any printed value. */
#define _NT_UNION(name, type, ...) _NT_TYPENAME(name) name;
typedef union {
    struct {
        _NT_TYPES(_NT_UNION, dummy)
    };
    char data[1];
} _nt_value_t;

struct _nt_typeinfo;
typedef const struct _nt_typeinfo* _nt_typeinfo_t;

/* Flags for comparison operation. */
enum {
    _NT_NEG_OP = 1,
    _NT_NOASSERT = 2
};

/* This macros generates declaration of the function which checks boolean
 * operation, prints assertion if needed, and returns result of the operation. */
#define _NT_DECL_FUNC(op_name, type)                                                    \
    int _NT_FUNC(op_name, type)(int dummy,                                              \
        const char *name, const char *file, unsigned line,                              \
        const char *op, int flags, int narg,                                            \
        const char *left_expr,  const _nt_value_t* left_val,  _nt_typeinfo_t left_type, \
        const char *right_expr, const _nt_value_t* right_val, _nt_typeinfo_t right_type)

/* Forward declaration of all check functions, for all types and binary
 * operations applicatble to scalar arguments. */
//#define _NT_CDAR(a, b, c) b
#define _NT_DECL_FUNC_BINOP(op, type)   _NT_DECL_FUNC(op, type);
#define _NT_DECL_ALL_FUNC(type, ...)    _NT_BINOPS_ALL(_NT_DECL_FUNC_BINOP, type)
_NT_TYPES(_NT_DECL_ALL_FUNC, dummy)

/* Define name of symbol containing type info for given type. */
#define _NT_TYPEINFO(type) _NT_CONCAT(_nt_typeinfo_, type)

/* Forward declaration of all typeinfo structures. */
#define _NT_DECL_TYPEINFO(type, ...) extern const struct _nt_typeinfo _NT_TYPEINFO(type);
_NT_TYPES(_NT_DECL_TYPEINFO, dummy)

/* Define mapping between all primitive types and corresponding
 * comparator functions. */
#define _NT_GENERIC(type, result, tname, op) type: _NT_FUNC(op, result),

/* Define mapping between all primitive types and typeinfo structures. */
#define _NT_TYPE_SEL(type, result, name, ...)  type: &_NT_TYPEINFO(result),

/* Select printer function appropriate to type of the given expression. */
#define _NT_SEL_TYPE(expr) \
    _Generic((expr), _NT_TYPEMAP(_NT_TYPE_SEL, dummy) \
                    default: &_NT_TYPEINFO(any_pointer))

/* Define name of a function which makes temporary from assertion argument. */
#define _NT_TYPE_TEMP_FN(type) _NT_CONCAT(_nt_maketemp_, type)

/* Define a functions to capture assertion arguments in temporary variables. */
#define _NT_DECL_TEMP(type, ctype, tname, ...) \
    inline static _nt_value_t _NT_TYPE_TEMP_FN(tname)(type val) {  \
        _nt_value_t temp; temp.ctype = (_NT_TYPENAME(ctype))val; return temp; \
    }

_NT_TYPEMAP(_NT_DECL_TEMP, dummy)
_NT_DECL_TEMP(const void*, any_pointer, any_pointer, dummy)

/* Apply function to create temporary value and returns its address. */
#define _NT_MAKETEMP(val) \
    ((const _nt_value_t*)(long long)_Generic((val), _NT_TYPEMAP(_NT_GEN_TEMP, dummy)  \
                default: _NT_TYPE_TEMP_FN(any_pointer))(val).data)

#define _NT_GEN_TEMP(type, ctype, tname, ...) type: _NT_TYPE_TEMP_FN(tname),

/* Generic, type-depending assertyion function. */
#define _NT_ASSERT(name, left, op, right, flags, narg, continuation) (      \
    _Generic( _NT_EXPR_TYPE((left), (right)),                               \
        _NT_TYPEMAP(_NT_GENERIC, op)                                        \
        default: _NT_FUNC(op, any_pointer) )                                \
           (sizeof((left) _NT_OP(op) (right)),                              \
            name, __FILE__, __LINE__, _NT_OP_NAME(op), flags, narg,         \
            _NT_STRINGIFY(left), _NT_MAKETEMP(left), _NT_SEL_TYPE(left),    \
            _NT_STRINGIFY(right), _NT_MAKETEMP(right), _NT_SEL_TYPE(right)) \
                ? 1 : (_nt_trap(), continuation, 0))

/* Function at which breakpoint can be set. */
void _nt_trap(void);

/* Mark test as failed, abort test execution. */
void _nt_abort(void);

/* Mark test as failed, but continue test execution. */
void _nt_assert(void);

void _nt_fail(const char *file, unsigned line, unsigned nargs, ...);

void _nt_fail_check(const char *file, unsigned line, unsigned nargs, ...);

void _nt_skip(const char *file, unsigned line, unsigned nargs, ...);

void _nt_success(const char *file, unsigned line, unsigned nargs, ...);

void _nt_warn(const char *file, unsigned line, unsigned nargs, ...);

typedef const char* _nt_uniq_t;

void _nt_info(_nt_uniq_t, const char *file, unsigned line, unsigned nargs, ...);

void _nt_capture(_nt_uniq_t, const char *file, unsigned line, unsigned nargs, ...);
void _nt_scope_reset(void);

#define _NT_MESSAGE(func, ...) \
    func(__FILE__, __LINE__, _NT_NARGS(__VA_ARGS__) _NT_FOREACH(_NT_VALUE, dummy, __VA_ARGS__))

#define _NT_FAIL(...)       _NT_MESSAGE(_nt_fail,       __VA_ARGS__)
#define _NT_FAIL_CHECK(...) _NT_MESSAGE(_nt_fail_check, __VA_ARGS__)
#define _NT_WARN(...)       _NT_MESSAGE(_nt_warn,       __VA_ARGS__)
#define _NT_SKIP(...)       _NT_MESSAGE(_nt_skip,       __VA_ARGS__)
#define _NT_SUCCEED(...)    _NT_MESSAGE(_nt_success,    __VA_ARGS__)

#define _NT_TEST_FAILED()       _nt_is_fail()

#define _NT_INFO(...)   \
    _nt_info(_NT_UNIQ_LIT(), __FILE__, __LINE__, \
        _NT_NARGS(__VA_ARGS__) _NT_FOREACH(_NT_VALUE, dummy, __VA_ARGS__))

#define _NT_CAPTURE(...) \
    _nt_capture(_NT_UNIQ_LIT(), __FILE__, __LINE__, \
        _NT_NARGS(__VA_ARGS__) _NT_FOREACH(_NT_VARIABLE, dummy, __VA_ARGS__))

#define _NT_VARIABLE(p, v) , _NT_STRINGIFY(v), _NT_MAKETEMP(v), _NT_SEL_TYPE(v)

#define _NT_VALUE(p, v) , _NT_MAKETEMP(v), _NT_SEL_TYPE(v)

#define _NT_UNIQ_LIT() __FILE__ ":" _NT_STRINGIFY(__LINE__)


#define _NT_STATIC_REQUIRE(...)  (_NT_STATIC_CONSTEVAL(__VA_ARGS__), _NT_REQUIRE(__VA_ARGS__))
#define _NT_STATIC_REQUIRE_FALSE(...)  (_NT_STATIC_CONSTEVAL(__VA_ARGS__), _NT_REQUIRE_FALSE(__VA_ARGS__))

#define _NT_STATIC_CONSTEVAL(...)  _NT_APPLY(_NT_CHECK_CONSTEVAL, __VA_ARGS__ _NE_ 0,, 1,)

#define _NT_CHECK_CONSTEVAL(left, op, right, ...) (void)sizeof(struct{ \
    _Static_assert(_Generic(0 ? (int*)0 : (void*)((size_t)(left _NT_OP(op) right)*0), int*: 1, default: 0), \
    "(" _NT_STRINGIFY(left) " " _NT_OP_NAME(op) " " _NT_STRINGIFY(right) "): need compile time constant expression"); \
    int dummy; })


/* Split arguments and call generic assertion function. */
#define _NT_CHECK(...) \
    (!_NT_APPLY(_NT_ANY_CHECK, "CHECK", _nt_assert(),  0, __VA_ARGS__ _NE_ 0,, 1,))

#define _NT_REQUIRE(...) \
    (void)_NT_APPLY(_NT_ANY_CHECK, "REQUIRE", _nt_abort(),   0, __VA_ARGS__ _NE_ 0,, 1,)

#define _NT_CHECK_FALSE(...) \
    (!_NT_APPLY(_NT_ANY_CHECK, "CHECK_FALSE", _nt_assert(), _NT_NEG_OP, __VA_ARGS__ _NE_ 0,, 1,))

#define _NT_REQUIRE_FALSE(...) \
    (void)_NT_APPLY(_NT_ANY_CHECK, "REQUIRE_FALSE", _nt_abort(), _NT_NEG_OP, __VA_ARGS__ _NE_ 0,, 1,)

#define _NT_CHECK_NOFAIL(...) \
    (void)_NT_APPLY(_NT_ANY_CHECK, "CHECK_NOFAIL", (void)0,  _NT_NOASSERT, __VA_ARGS__ _NE_ 0,, 1,)

#define _NT_CHECKED_IF(...) \
    if (_NT_APPLY(_NT_ANY_CHECK, "CHECKED_IF", (void)0,  _NT_NOASSERT, __VA_ARGS__ _NE_ 0,, 1,))

#define _NT_CHECKED_ELSE(...) \
    if (!_NT_APPLY(_NT_ANY_CHECK, "CHECKED_ELSE", (void)0,  _NT_NOASSERT, __VA_ARGS__ _NE_ 0,, 1,))

#define _NT_ANY_CHECK(name, handle, flags, left, op, right, op2, narg, ...) \
    _NT_ASSERT(name, left, op, right, flags, narg, handle)

/* Combine test name from suite name and particular test name. */
#define _NT_TEST_NAME(suite, name) _NT_CONCAT(_NT_CONCAT(suite, _), name)

/* Generate signature and name of function which registers test within system. */
#define _NT_REGISTER_SIG(prefix, name) \
    static __attribute__((constructor)) void _NT_CONCAT(prefix, name)(void)

/* Create name of the function implementing particular test. */
#define _NT_TEST_FUNC(suite, name) _NT_CONCAT(_nt_test_, _NT_TEST_NAME(suite, name))
#define _NT_RUNNER_FUNC(suite, name)  _NT_CONCAT(_nt_runner_, _NT_TEST_NAME(suite, name))

/* Generate signature of the function which performs the test. */
#define _NT_SETUP_FUNC(suite)       _NT_CONCAT(_nt_setup_, suite)
#define _NT_TEARDOWN_FUNC(suite)    _NT_CONCAT(_nt_teardown_, suite)
#define _NT_SETUP_HELPER(suite)     _NT_CONCAT(_nt_setup_helper_, suite)
#define _NT_TEARDOWN_HELPER(suite)  _NT_CONCAT(_nt_teardown_helper_, suite)

/* Generate function which registers the test. */
#define _NT_FIXTURE(suite)          _NT_CONCAT(_nt_fixture_, suite)

typedef struct { int tag; } _nt_fixture_tag;

struct _nt_suite {
    const char *const name;
    unsigned fixture_size;
    void (*setup)(_nt_fixture_tag *);
    void (*teardown)(_nt_fixture_tag *);
    struct _nt_suite *next;
};

struct _nt_scope_msg;

struct _nt_fail_info {
    const char *fail_file;
    unsigned fail_line;
    char *fail_msg;
};
struct _nt_test {
    const char *const file;
    const unsigned line;
    void (*const func)(_nt_fixture_tag *);
    const char *const name;
    void *jmpbuf;
    const struct _nt_suite *suite;
    _nt_fixture_tag *fixture;
    struct _nt_scope_msg *scope_msg;
    struct _nt_fail_info first, last;
    struct _nt_test *next;
};

void _nt_register_test(struct _nt_test *);

const struct _nt_suite* _nt_setup_suite(struct _nt_suite *suite);

#define _NT_GET_FIXTURE()   (0 ? _fixture : _fixture)

/* Define test function. 
 * TODO: pass fixture to each test. */
#define _NT_TEST(Suite, Name)                                               \
    struct _NT_FIXTURE(Suite);                                              \
    static void _NT_TEST_FUNC(Suite, Name)(struct _NT_FIXTURE(Suite) *);    \
    static void _NT_RUNNER_FUNC(Suite, Name)(_nt_fixture_tag *fixture) {    \
        _NT_TEST_FUNC(Suite, Name)((struct _NT_FIXTURE(Suite)*)fixture);    \
    }                                                                       \
    _NT_REGISTER_SIG(_nt_register_test_, _NT_TEST_NAME(Suite, Name)) {      \
        static struct _nt_suite smem = {                                    \
            _NT_STRINGIFY(Suite), 0, _NT_NULL, _NT_NULL, _NT_NULL           \
        };                                                                  \
        const struct _nt_suite *s_ptr = _nt_setup_suite(&smem);             \
        static struct _nt_test test = {                                     \
            __FILE__, __LINE__,                                             \
            _NT_RUNNER_FUNC(Suite, Name),				    \
            _NT_STRINGIFY(Suite) "/" _NT_STRINGIFY(Name),                   \
            _NT_NULL, _NT_NULL, _NT_NULL, _NT_NULL,                         \
            {_NT_NULL, 0, _NT_NULL},                                        \
            {_NT_NULL, 0, _NT_NULL},                                        \
            _NT_NULL                                                        \
        };                                                                  \
        test.suite = s_ptr;                                                 \
        _nt_register_test(&test);                                           \
    }                                                                       \
    static void _NT_TEST_FUNC(Suite, Name)                                  \
                (struct _NT_FIXTURE(Suite) *_NT_UNUSED(_fixture))


#define _NT_FIXTURE_TEMPLATE(suite, prefix, helper, func, setup, teardown)  \
    struct _NT_FIXTURE(suite);                                              \
    static void func(struct _NT_FIXTURE(suite) *);                          \
    static void helper(_nt_fixture_tag *ptr) {                              \
        func((struct _NT_FIXTURE(suite)*)ptr);                              \
    }                                                                       \
    _NT_REGISTER_SIG(prefix, suite) {                                       \
        static struct _nt_suite var = {                                     \
            _NT_STRINGIFY(suite),                                           \
            sizeof(struct _NT_FIXTURE(suite)),                              \
            setup, teardown, _NT_NULL                                       \
        };                                                                  \
        _nt_setup_suite(&var);                                              \
    }                                                                       \
    static void func(struct _NT_FIXTURE(suite) *_fixture)


#define _NT_TEST_SETUP(suite) \
    _NT_FIXTURE_TEMPLATE(suite, _nt_register_setup_, _NT_SETUP_HELPER(suite), \
        _NT_SETUP_FUNC(suite), _NT_SETUP_HELPER(suite), _NT_NULL)

#define _NT_TEST_TEARDOWN(suite) \
    _NT_FIXTURE_TEMPLATE(suite, _nt_register_teardown_, _NT_TEARDOWN_HELPER(suite), \
        _NT_TEARDOWN_FUNC(suite), _NT_NULL, _NT_TEARDOWN_HELPER(suite))


/* This macros count number of arguments in range [1..20]. */
#define _NT_NARGS(...) \
    _NT_APPLY(_NT_COUNT_IMPL, __VA_ARGS__, _NT_NUMBERS, dummy)

#define _NT_COUNT_IMPL(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
    a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, N, ...) N

#define _NT_APPLY(F, ...) F(__VA_ARGS__)

#define _NT_NUMBERS \
    20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1

#define _NT_CAR(v, ...) v
#define _NT_CDR(v, ...) __VA_ARGS__

#define _NT_JOIN(F, N, X, Y) _NT_CONCAT(_NT_JOIN_, N)(F, X, Y)

#define _NT_JOIN_1(F, X, Y) F(_NT_CAR X, _NT_CAR Y)
#define _NT_JOIN_2(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_1(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_3(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_2(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_4(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_3(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_5(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_4(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_6(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_5(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_7(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_6(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_8(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_7(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_9(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_8(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_10(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_9(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_11(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_10(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_12(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_11(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_13(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_12(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_14(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_13(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_15(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_14(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_16(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_15(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_17(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_16(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_18(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_17(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_19(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_18(F, (_NT_CDR X), (_NT_CDR Y))
#define _NT_JOIN_20(F, X, Y) _NT_JOIN_1(F, X, Y), _NT_JOIN_19(F, (_NT_CDR X), (_NT_CDR Y))

#define _NT_MOCK_DECL_ARG(x, y) x _NT_MOCK_ARG_NAME(x, y)

#define _NT_MOCK_ARG_NAME(x, y) _NT_CONCAT(_, y)

#define _NT_MOCK_DECL_ARGS(APPLY, ...) \
    _NT_JOIN(APPLY, _NT_NARGS(__VA_ARGS__), (__VA_ARGS__, dummy), (_NT_NUMBERS, dummy))

#define _NT_MOCK_DECL(...) _NT_MOCK_DECL_ARGS(_NT_MOCK_DECL_ARG, __VA_ARGS__)

#define _NT_MOCK_ARGS(...) _NT_MOCK_DECL_ARGS(_NT_MOCK_ARG_NAME, __VA_ARGS__)


#define _NT_NVOID(type, expr) \
    _NT_APPLY(_NT_EMPTY(_NT_VOID, _NT_EXPAND, _NT_CONCAT(_NT_EAT_, type)), expr)

#define _NT_EAT_void
#define _NT_EXPAND(...) __VA_ARGS__
#define _NT_VOID(...)

#define _NT_EMPTY(True, False, arg) \
    _NT_APPLY(_NT_CDR, _NT_APPLY(_NT_CDR, _NT_EMPTY_EVAL arg (True), )) \
    _NT_APPLY(_NT_CAR, _NT_APPLY(_NT_CDR, _NT_COMMA_FUNC arg (), False))

#define _NT_EMPTY_EVAL(expr) , expr
#define _NT_COMMA_FUNC() ,


#define _NT_MOCK_TYPE(func) struct _NT_CONCAT(_nt_mock_type_, func)
#define _NT_MOCK(func) (_NT_CONCAT(_nt_mock_get_, func)())

struct _nt_mock {
    void (*const func)(void);
    struct _nt_mock *next;
};

void _nt_register_mock(struct _nt_mock *);

/* Arguments must be function argument types.
 * TODO reset all mocks at end of each tests. */
#define _NT_MOCK_ANY_FUNCTION(Real, Wrap, Result, Func, ...)        \
    _NT_MOCK_TYPE(Func) {                                           \
            unsigned long count;                                    \
            Result (*func)(__VA_ARGS__);                            \
            union {                                                 \
                int dummy;                                          \
                _NT_NVOID(Result, Result result);                   \
            };                                                      \
    };                                                              \
    _NT_MOCK_TYPE(Func) *_NT_CONCAT(_nt_mock_get_, Func)(void) {    \
        static _NT_MOCK_TYPE(Func) mock = {0, _NT_NULL, {0}};       \
        return &mock;                                               \
    }                                                               \
    void _NT_CONCAT(_nt_mock_reset_, Func)(void) {                  \
        static const _NT_MOCK_TYPE(Func) init = {0, _NT_NULL, {0}}; \
        *_NT_MOCK(Func) = init;                                     \
    }                                                               \
    unsigned long _NT_CONCAT(_nt_mock_count_, Func)(void) {         \
        return _NT_MOCK(Func)->count;                               \
    }                                                               \
    Result Real(__VA_ARGS__);                                       \
    Result _NT_CONCAT(_nt_fake_, Func)(_NT_MOCK_DECL(__VA_ARGS__)){ \
        if (0) _NT_MOCK(Func)->func(_NT_MOCK_ARGS(__VA_ARGS__));    \
        return _NT_NVOID(Result, _NT_MOCK(Func)->result);           \
    }                                                               \
    Result Wrap(_NT_MOCK_DECL(__VA_ARGS__))                         \
    {                                                               \
        _NT_MOCK_TYPE(Func) *mock = _NT_MOCK(Func);                 \
        mock->count++;                                              \
        return (mock->func ? mock->func : Real)                     \
                    (_NT_MOCK_ARGS(__VA_ARGS__));                   \
    }                                                               \
    _NT_REGISTER_SIG(_nt_register_mock_, Func) {                    \
        static struct _nt_mock mock = {                             \
            _NT_CONCAT(_nt_mock_reset_, Func),                      \
            _NT_NULL                                                \
        };                                                          \
        _nt_register_mock(&mock);                                   \
    }

#define _NT_MOCK_FUNCTION(result, func, ...) \
    _NT_MOCK_ANY_FUNCTION(_NT_CONCAT(_nt_fake_, func), func, result, func, __VA_ARGS__)

#define _NT_MOCK_OVERRIDE(result, func, ...) \
    _NT_MOCK_ANY_FUNCTION(_NT_CONCAT(__real_, func), _NT_CONCAT(__wrap_, func), \
                            result, func, __VA_ARGS__)

#define _NT_MOCK_RESET(func) (_NT_CONCAT(_nt_mock_reset_, func)())

// TODO expand to readable expression...
#define _NT_MOCK_COUNT(func) _NT_CONCAT(_nt_mock_count_, func)()

#define _NT_MOCK_SET_RESULT(Func, val) \
    (_NT_MOCK(Func)->func = _NT_CONCAT(_nt_fake_, Func), _NT_MOCK(Func)->result = (val))

#define _NT_MOCK_SET_FUNC(Func, func_ptr) (_NT_MOCK(Func)->func = (func_ptr))


#define _NT_FOREACH(F, param, ...) \
    _NT_APPLY(_NT_CONCAT(_NT_FOREACH_, _NT_NARGS(__VA_ARGS__)), F, param, __VA_ARGS__)

#define _NT_FOREACH_1(F, p, v) F(p, v)
#define _NT_FOREACH_2(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_1(F, p, __VA_ARGS__)
#define _NT_FOREACH_3(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_2(F, p, __VA_ARGS__)
#define _NT_FOREACH_4(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_3(F, p, __VA_ARGS__)
#define _NT_FOREACH_5(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_4(F, p, __VA_ARGS__)
#define _NT_FOREACH_6(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_5(F, p, __VA_ARGS__)
#define _NT_FOREACH_7(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_6(F, p, __VA_ARGS__)
#define _NT_FOREACH_8(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_7(F, p, __VA_ARGS__)
#define _NT_FOREACH_9(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_8(F, p, __VA_ARGS__)
#define _NT_FOREACH_10(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_9(F, p, __VA_ARGS__)
#define _NT_FOREACH_11(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_10(F, p, __VA_ARGS__)
#define _NT_FOREACH_12(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_11(F, p, __VA_ARGS__)
#define _NT_FOREACH_13(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_12(F, p, __VA_ARGS__)
#define _NT_FOREACH_14(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_13(F, p, __VA_ARGS__)
#define _NT_FOREACH_15(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_14(F, p, __VA_ARGS__)
#define _NT_FOREACH_16(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_15(F, p, __VA_ARGS__)
#define _NT_FOREACH_17(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_16(F, p, __VA_ARGS__)
#define _NT_FOREACH_18(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_17(F, p, __VA_ARGS__)
#define _NT_FOREACH_19(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_18(F, p, __VA_ARGS__)
#define _NT_FOREACH_20(F, p, v, ...) _NT_FOREACH_1(F, p, v) _NT_FOREACH_19(F, p, __VA_ARGS__)

#endif  /* _NT_HEADER */
