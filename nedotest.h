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

/* Define setup function, which will be called for each test belonged to the
 * specified suite, before starting the test itself. Example:
 *
 * TEST_SETUP(suite1)
 * {
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


#endif


/* Public header finished at this point, you must not use the definitins
 * found below directly! */



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
#define _NT_TYPEMAP(APPLY, ...)                              \
    APPLY(                char, Char,           __VA_ARGS__) \
    APPLY(         signed char, signed_char,    __VA_ARGS__) \
    APPLY(       unsigned char, unsigned_char,  __VA_ARGS__) \
    APPLY(      unsigned short, any_unsigned,   __VA_ARGS__) \
    APPLY(            unsigned, any_unsigned,   __VA_ARGS__) \
    APPLY(       unsigned long, any_unsigned,   __VA_ARGS__) \
    APPLY(  unsigned long long, any_unsigned,   __VA_ARGS__) \
    APPLY(               short, any_signed,     __VA_ARGS__) \
    APPLY(                 int, any_signed,     __VA_ARGS__) \
    APPLY(                long, any_signed,     __VA_ARGS__) \
    APPLY(           long long, any_signed,     __VA_ARGS__) \
    APPLY(               float, any_float,      __VA_ARGS__) \
    APPLY(              double, any_float,      __VA_ARGS__) \
    APPLY(         long double, any_float,      __VA_ARGS__) \
    APPLY(         const char*, cstring,        __VA_ARGS__) \
    APPLY(               char*, cstring,        __VA_ARGS__) \
    APPLY(  const signed char*, cstring,        __VA_ARGS__) \
    APPLY(        signed char*, cstring,        __VA_ARGS__) \
    APPLY(const unsigned char*, cstring,        __VA_ARGS__) \
    APPLY(      unsigned char*, cstring,        __VA_ARGS__)

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
#define _NT_OP(op) _NT_CONCAT(_NT, op)
#define _NT_OP_LT_      <
#define _NT_OP_GT_      >
#define _NT_OP_LE_      <=
#define _NT_OP_GE_      >=
#define _NT_OP_EQ_      ==
#define _NT_OP_NE_      !=
#define _NT_OP_STREQ_    &&
#define _NT_OP_STRNE_    &&
#define _NT_OP_CONTAINS_ &&
#define _NT_OP_MATCHES_  &&

/* This macros transform symbolic name of the binary operation to the
 * string which represents operation on assertion text. */
#define _NT_OP_NAME(op) _NT_CONCAT(_NT_NAME, op)
#define _NT_NAME_OP_LT_         "<"
#define _NT_NAME_OP_GT_         ">"
#define _NT_NAME_OP_GE_         ">="
#define _NT_NAME_OP_LE_         "<="
#define _NT_NAME_OP_EQ_         "=="
#define _NT_NAME_OP_NE_         "!="
#define _NT_NAME_OP_STREQ_      "STREQ"
#define _NT_NAME_OP_STRNE_      "STRNE"
#define _NT_NAME_OP_CONTAINS_   "CONTAINS"
#define _NT_NAME_OP_MATCHES_    "MATCHES"

/* Following set of definitions used as symbolic names of the binary
 * operations applicable with CHECK and REQUIRE macroses. */
#define _LT_       ,_OP_LT_,
#define _GT_       ,_OP_GT_,
#define _LE_       ,_OP_LE_,
#define _GE_       ,_OP_GE_,
#define _EQ_       ,_OP_EQ_,
#define _NE_       ,_OP_NE_,
#define _STREQ_    ,_OP_STREQ_,
#define _STRNE_    ,_OP_STRNE_,
#define _CONTAINS_ ,_OP_CONTAINS_,
#define _MATCHES_  ,_OP_MATCHES_,

/* Define names of the known binary operations. */
#define _NT_BINOPS_SCALAR(APPLY, ...) \
    APPLY(_OP_LT_,  __VA_ARGS__)      \
    APPLY(_OP_GT_,  __VA_ARGS__)      \
    APPLY(_OP_LE_,  __VA_ARGS__)      \
    APPLY(_OP_GE_,  __VA_ARGS__)      \
    APPLY(_OP_EQ_,  __VA_ARGS__)      \
    APPLY(_OP_NE_,  __VA_ARGS__)

#define _NT_BINOPS_STRING(APPLY, ...)       \
    APPLY(_OP_STREQ_,    __VA_ARGS__)       \
    APPLY(_OP_STRNE_,    __VA_ARGS__)       \
    APPLY(_OP_CONTAINS_, __VA_ARGS__)       \
    APPLY(_OP_MATCHES_,  __VA_ARGS__)

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

/* Define union to represent pointer to any printed types */
#define _NT_UNION(name, type, ...) _NT_TYPENAME(name) const* name;
typedef union {
    _NT_TYPES(_NT_UNION, dummy)
} _nt_value;

/* Define type of the functions which able to print arguments from _NT_TYPES list */
typedef void _nt_print_fn(_nt_value);

/* This macros generates declaration of the function which checks boolean
 * operation, prints assertion if needed, and returns result of the operation. */
#define _NT_DECL_FUNC(op_name, type)                                                    \
    int _NT_FUNC(op_name, type)(int dummy,                                              \
        const char *name, const char *file, unsigned line,                              \
        const char *op, int neg, int narg,                                              \
        const char *left_expr,  _NT_TYPENAME(type) left_val,  _nt_print_fn left_print,  \
        const char *right_expr, _NT_TYPENAME(type) right_val, _nt_print_fn right_print)

/* Forward declaration of all check functions, for all types and binary
 * operations applicatble to scalar arguments. */
//#define _NT_CDAR(a, b, c) b
#define _NT_DECL_FUNC_BINOP(op, type)   _NT_DECL_FUNC(op, type);
#define _NT_DECL_ALL_FUNC(type, ...)    _NT_BINOPS_ALL(_NT_DECL_FUNC_BINOP, type)
_NT_TYPES(_NT_DECL_ALL_FUNC, dummy)

#if 0
/* Prepend previous declarations with functions defined for strings. */
_NT_DECL_FUNC_BINOP(_STREQ_, cstring)
_NT_DECL_FUNC_BINOP(_STRNE_, cstring)
_NT_DECL_FUNC_BINOP(_CONTAINS_, cstring)
_NT_DECL_FUNC_BINOP(_MATCHES_, cstring)
#endif

/* Define name of function which is able to print value of specified type. */
#define _NT_TYPE_PRINT_FN(type) _NT_CONCAT(_nt_print_, type)

/* Forward declaration of all "printer" functions. */
#define _NT_DECL_PRINT(type, ...) void _NT_TYPE_PRINT_FN(type)(_nt_value);
_NT_TYPES(_NT_DECL_PRINT, dummy)

/* Define mapping between all primitive types and corresponding
 * comparator functions. */
#define _NT_GENERIC(type, result, op) type: _NT_FUNC(op, result),

/* Define mapping between all primitive types and functions able to
 * print value of that type. */
#define _NT_TYPE_PRINT_SEL(type, result, ...)  type: _NT_TYPE_PRINT_FN(result),

/* Select printer function appropriate to type of the given expression. */
#define _NT_SEL_PRINT(expr) \
    _Generic((expr), _NT_TYPEMAP(_NT_TYPE_PRINT_SEL, dummy) \
                    default: _NT_TYPE_PRINT_FN(any_pointer))

/* Generic, type-depending assertyion function. */
#define _NT_ASSERT(name, left, op, right, neg, narg, continuation) (    \
    _Generic( _NT_EXPR_TYPE((left), (right)),                           \
        _NT_TYPEMAP(_NT_GENERIC, op)                                    \
        default: _NT_FUNC(op, any_pointer) )                            \
           (sizeof((left) _NT_OP(op) (right)),                          \
            name, __FILE__, __LINE__, _NT_OP_NAME(op), neg, narg,       \
            _NT_STRINGIFY(left), (left), _NT_SEL_PRINT((left)),         \
            _NT_STRINGIFY(right), (right), _NT_SEL_PRINT((right)) )     \
                ? (void)0 : continuation)

/* Functions called in case if assertion failed and must be ignored
 * or must cause test termination. */
#define _NT_IGNORE ((void)0)
#define _NT_FAIL _nt_fail

void _nt_fail(void);
struct _nt_dummy { char tag[sizeof(&_nt_fail)]; };


/* Split arguments and call generic assertion function. */
#define _NT_CHECK(...)          _NT_APPLY(_NT_CHECK_TRUE_,    __VA_ARGS__ _NE_ 0,, 1,)
#define _NT_REQUIRE(...)        _NT_APPLY(_NT_REQUIRE_,       __VA_ARGS__ _NE_ 0,, 1,)
#define _NT_CHECK_FALSE(...)    _NT_APPLY(_NT_CHECK_FALSE_,   __VA_ARGS__ _NE_ 0,, 1,)
#define _NT_REQUIRE_FALSE(...)  _NT_APPLY(_NT_REQUIRE_FALSE_, __VA_ARGS__ _NE_ 0,, 1,)

#define _NT_CHECK_TRUE_(left, op, right, op2, narg, ...) \
    _NT_ASSERT("CHECK", left, op, right, 0, narg, _NT_IGNORE)

#define _NT_REQUIRE_TRUE_(left, op, right, op2, narg, ...) \
    _NT_ASSERT("REQUIRE", left, op, right, 0, narg, _NT_FAIL)


#define _NT_CHECK_FALSE_(left, op, right, op2, narg, ...) \
    _NT_ASSERT("CHECK_FALSE", left, op, right, 1, narg, _NT_IGNORE)

#define _NT_REQUIRE_FALSE_(left, op, right, op2, narg, ...) \
    _NT_ASSERT("CHECK_FALSE", left, op, right, 1, narg, _NT_FAIL)


typedef struct { int tag; } _nt_fixture_tag;

struct _nt_suite {
    const char *const name;
    unsigned fixture_size;
    void (*setup)(_nt_fixture_tag *);
    void (*teardown)(_nt_fixture_tag *);
    struct _nt_suite *next;
};

struct _nt_test {
    const char *const file;
    unsigned line;
    const char *const name;
    void (*const func)(void);
    const struct _nt_suite *suite;
    int fail;
    struct _nt_test *next;
};

void _nt_register_test(struct _nt_test *);

const struct _nt_suite* _nt_setup_suite(struct _nt_suite *suite);


/* Combine test name from suite name and particular test name. */
#define _NT_TEST_NAME(suite, name) \
    _NT_CONCAT(_test_, _NT_CONCAT(_NT_CONCAT(_NT_CONCAT(suite, _), _), name))

/* Generate signature and name of function which registers test within system. */
#define _NT_REGISTER_SIG(prefix, name) \
    static __attribute__((constructor)) void _NT_CONCAT(prefix, name)(void)

/* Create name of the function implementing particular test. */
#define _NT_TEST_FN_NAME(suite, name) _NT_CONCAT(_nt_test_, _NT_TEST_NAME(suite, name))

/* Generate signature of the function which performs the test. */
#define _NT_SETUP_FUNC(suite)       _NT_CONCAT(_nt_setup_, suite)
#define _NT_TEARDOWN_FUNC(suite)    _NT_CONCAT(_nt_teardown_, suite)
#define _NT_SETUP_HELPER(suite)     _NT_CONCAT(_nt_setup_helper_, suite)
#define _NT_TEARDOWN_HELPER(suite)  _NT_CONCAT(_nt_teardown_helper_, suite)

/* Generate function which registers the test. */
#define _NT_FIXTURE(suite)          _NT_CONCAT(_nt_fixture_, suite)

#ifdef __cplusplus
#define _NT_NULL 0
#else
#define _NT_NULL ((void*)0)
#endif

/* Define test function. */
#define _NT_TEST(Suite, Name)                                           \
    static void _NT_TEST_FN_NAME(Suite, Name)();                        \
    _NT_REGISTER_SIG(_nt_register_test_, _NT_TEST_NAME(Suite, Name)) {  \
        static struct _nt_suite smem = {                                \
            _NT_STRINGIFY(Suite), 0, _NT_NULL, _NT_NULL, _NT_NULL       \
        };                                                              \
        const struct _nt_suite *s_ptr = _nt_setup_suite(&smem);         \
        static struct _nt_test test = {                                 \
            __FILE__, __LINE__,                                         \
            _NT_STRINGIFY(Suite) "/" _NT_STRINGIFY(Name),               \
            _NT_TEST_FN_NAME(Suite, Name),                              \
            0, 0, _NT_NULL                                              \
        };                                                              \
        test.suite = s_ptr;                                             \
        _nt_register_test(&test);                                       \
    }                                                                   \
    static void _NT_TEST_FN_NAME(Suite, Name)(void)


#define _NT_FIXTURE_TEMPLATE(suite, prefix, helper, func, setup, teardown)  \
    static void func(struct _NT_FIXTURE(suite) *);                          \
    static void helper(_nt_fixture_tag *ptr) {                              \
        func((struct _NT_FIXTURE(suite)*)ptr);                              \
    }                                                                       \
    _NT_REGISTER_SIG(prefix, suite) {                                       \
        static struct _nt_suite var = {                                     \
            _NT_STRINGIFY(suite),                                           \
            sizeof(struct _NT_FIXTURE(suite)),                              \
            setup, teardown, {}                                             \
        };                                                                  \
        _nt_setup_suite(&var);                                              \
    }                                                                       \
    static void func(struct _NT_FIXTURE(suite) *thiz)


#define _NT_TEST_SETUP(suite) \
    _NT_FIXTURE_TEMPLATE(suite, _nt_register_setup_, _NT_SETUP_HELPER(suite),    \
        _NT_SETUP_FUNC(suite), _NT_SETUP_HELPER(suite), {})

#define _NT_TEST_TEARDOWN(suite) \
    _NT_FIXTURE_TEMPLATE(suite, _nt_register_teardown_,_NT_TEARDOWN_HELPER(suite), \
        _NT_TEARDOWN_FUNC(suite), {}, _NT_TEARDOWN_HELPER(suite))


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


#define _NT_MOCK_TYPE(func) struct _NT_CONCAT(_nt_mock_type_, func)
#define _NT_MOCK(func) (_NT_CONCAT(_nt_mock_, func)())

/* Arguments must be function argument types. */
#define _NT_MOCK_ANY_FUNCTION(Real, Wrap, Result, Func, ...)        \
    _NT_MOCK_TYPE(Func) {                                           \
            unsigned long count;                                    \
            Result (*func)(__VA_ARGS__);                            \
            union {                                                 \
                int dummy;                                          \
                Result result;                                      \
            };                                                      \
    };                                                              \
    _NT_MOCK_TYPE(Func) *_NT_CONCAT(_nt_mock_, Func)(void) {        \
        static _NT_MOCK_TYPE(Func) mock = {0, _NT_NULL, {0}};       \
        return &mock;                                               \
    }                                                               \
    void _NT_CONCAT(_nt_mock_reset_, Func)(void) {                  \
        static const _NT_MOCK_TYPE(Func) init = {0, _NT_NULL, {0}}; \
        *_NT_MOCK(Func) = init;                                     \
    }                                                               \
    Result Real(__VA_ARGS__);                                       \
    Result _NT_CONCAT(_nt_fake_, Func)(_NT_MOCK_DECL(__VA_ARGS__)){ \
        return 0 ? _NT_MOCK(Func)->func(_NT_MOCK_ARGS(__VA_ARGS__)) \
                    :_NT_MOCK(Func)->result;                        \
    }                                                               \
    Result Wrap(_NT_MOCK_DECL(__VA_ARGS__))                         \
    {                                                               \
        _NT_MOCK_TYPE(Func) *mock = _NT_MOCK(Func);                 \
        mock->count++;                                              \
        return (mock->func ? mock->func : Real)                     \
                    (_NT_MOCK_ARGS(__VA_ARGS__));                   \
    }

#define _NT_MOCK_FUNCTION(result, func, ...) \
    _NT_MOCK_ANY_FUNCTION(_NT_CONCAT(_nt_fake_, func), func, result, func, __VA_ARGS__)

#define _NT_MOCK_OVERRIDE(result, func, ...) \
    _NT_MOCK_ANY_FUNCTION(_NT_CONCAT(__real_, func), _NT_CONCAT(__wrap_, func), \
                            result, func, __VA_ARGS__)

#define _NT_MOCK_RESET(func) (_NT_CONCAT(_nt_mock_reset_, func)())

#define _NT_MOCK_COUNT(func) (_NT_MOCK(func)->count)

#define _NT_MOCK_SET_RESULT(Func, val) \
    (_NT_MOCK(Func)->func = _NT_CONCAT(_nt_fake_, Func), _NT_MOCK(Func)->result = (val))

#define _NT_MOCK_SET_FUNC(func, func_ptr) (_NT_MOCK(func)->func = (func_ptr))

#endif  /* _NT_HEADER */
