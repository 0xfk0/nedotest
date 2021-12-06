/* vim: set et sts=4 sw=4: */

#ifdef __cplusplus
#include <cstddef>      // std::max_align_t
#include <alloca.h>
#else
#include <stddef.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include "nedotest.h"

enum { MAX_STRING_LEN = 70 };

static FILE *out;

static struct _nt_suite *suites_list;
static struct _nt_test *tests_list;

#ifdef __cplusplus
#define _Thread_local thread_local
#define _Alignas alignas
#define max_align_t std::max_align_t
#endif

// FIXME MSVC, also max_align_t, alignas, void *_alloca(size_t size), constructor
_Thread_local jmp_buf exception;

/* This macros allows to check, that format string corresponds to argument
 * data types given to _nt_printf function. */
#define _NT_PRINTF(...) (0 ? (void)printf(__VA_ARGS__) : _nt_printf(__VA_ARGS__))
void _nt_printf(const char *fmt, ...);

void _nt_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
}


/* Define set of "printer" functions for all known types. */

static void print_char(int val, const char *fmt)
{
    int graph = isgraph(val);
    if (graph) _NT_PRINTF("'%c' (", val);
    _NT_PRINTF(fmt, val);
    if (graph) _NT_PRINTF(")");
}

/* Note, due to integral promotion all chars actualy expanded to integers. */

void _nt_print_char(_nt_value val)
{
    print_char((unsigned)*val.any_signed, "%#2x");
}

void _nt_print_unsigned_char(_nt_value val)
{
    print_char((unsigned)*val.any_unsigned, "%u");
}

void _nt_print_signed_char(_nt_value val)
{
    print_char((int)*val.any_signed, "%d");
}

void _nt_print_any_unsigned(_nt_value val)
{
    _NT_PRINTF("%llu", *val.any_unsigned);
}

void _nt_print_any_signed(_nt_value val)
{
    _NT_PRINTF("%lld", *val.any_signed);
}

void _nt_print_any_float(_nt_value val)
{
    _NT_PRINTF("%Lf", *val.any_float);
}

void _nt_print_any_pointer(_nt_value val)
{
    _NT_PRINTF("%p", *val.any_pointer);
}

void _nt_print_cstring(_nt_value val)
{
    _NT_PRINTF("\"");

    const char *const str = *val.cstring;
    const char *p = str;
    while (*p && p - str < MAX_STRING_LEN)
    {
        if (isprint(*p) && *p != '\"') {
            _NT_PRINTF("%c", *p);
        }
        else {
            _NT_PRINTF("\\x%02x", (unsigned)*p);
        }
        ++p;
    }

    _NT_PRINTF("\"");
}


/* Function matched string `str` against pattern `pattern`.
 * Characters compared in case-insensetive way (only ASCII),
 * asterisk (*) substitutes any amount of characters.
 * Function returns 1 if string matches to given pattern. */
static int match(const char *str, const char *pattern)
{
    while (1) 
    {
        if (!*pattern)
            return !*str;

        if (*pattern == '*')
        {
            ++pattern;
            while (1)
            {
                if (match(str, pattern))
                    return 1;

                if (!*str)
                    return 0;

                ++str;
            }
        }
            
        if (!*str || tolower(*pattern) != tolower(*str))
            return 0;

        ++str, ++pattern;
    }
}


/* This function dtermines if particular expression (given in assertions)
 * is the literal (so it's value must not be printed separately) */
static int is_literal(const char *expr)
{
        long long sval;
        unsigned long long uval;
        long double fval;
        int count;
        size_t const len = strlen(expr);

        if (sscanf(expr, "%lli%n", &sval, &count) == 1 && len == (size_t)count) 
            return 1;

        if (sscanf(expr, "%llu%n", &uval, &count) == 1 && len == (size_t)count) 
            return 1;

        if (sscanf(expr, "%Lf%n", &fval, &count) == 1 && len == (size_t)count) 
            return 1;

        if (expr[0] == '"')
            return 1;

        return 0;
}


/* Function which is called when particular assertion is failed. */
static void assertion(
    const char *name, const char *file, unsigned line, const char *op, int narg,
    const char *left_expr, _nt_value left_val, _nt_print_fn left_print, 
    const char *right_expr, _nt_value right_val, _nt_print_fn right_print)
{
    _NT_PRINTF("%s: %u: assertion failed: %s", file, line, name);
    if (!narg)
        _NT_PRINTF("(%s %s %s)\n", left_expr, op, right_expr);
    else
        _NT_PRINTF("(%s)\n", left_expr);

    unsigned nl = 0;
    if (!is_literal(left_expr)) {
        _NT_PRINTF("where (%s) = ", left_expr);
        left_print(left_val);
        ++nl;
    }

    if (!narg && !is_literal(right_expr)) {
        _NT_PRINTF("%s%s (%s) = ",
            nl ? ",\n" : "",
            nl ? "  and" : "where",
            right_expr);
        right_print(right_val);
        ++nl;
    }       

    if (nl) _NT_PRINTF(".\n\n");
}

/* Define implementation of named (LT, GT, etc...) binary operations */
#define IMPL_FUNC(op) _NT_CONCAT(IMPL, op)

#define IMPL_OP_LT_(a, b) (a < b)
#define IMPL_OP_GT_(a, b) (a > b)
#define IMPL_OP_LE_(a, b) (a <= b)
#define IMPL_OP_GE_(a, b) (a >= b)
#define IMPL_OP_EQ_(a, b) (a == b)
#define IMPL_OP_NE_(a, b) (a != b)

#define IMPL_OP_STREQ_(a, b)                (!strcmp(a, b))
#define IMPL_OP_STRNE_(a, b)                (!strcmp(a, b))
#define IMPL_OP_CONTAINS_(haystack, needle) (!strstr(haystack, needle))
#define IMPL_OP_MATCHES_(string, pattern)   match(string, pattern)

/* Function template for _nt_OP_TYPE functions declared by _NT_DECL_FUNC macros. */
#define COMP_TEMPLATE(op_name, type)                                \
    _NT_DECL_FUNC(op_name, type) {                                  \
        (void)dummy;                                                \
        _nt_value left;  left.type = &left_val;                     \
        _nt_value right; right.type = &right_val;                   \
        return !!IMPL_FUNC(op_name)(left_val, right_val) ^ neg ? 1  \
         : (assertion(name, file, line, op, narg,                   \
                left_expr, left, left_print,                        \
                right_expr, right, right_print), 0);                \
    }

/* Instantiate template listed above for all types and all binary operations
 * applicable to scalar types. */
#define INST_TMPL_OP(op, type) COMP_TEMPLATE(op, type)
#define INST_TMPL_ALL(type, ...) _NT_BINOPS_SCALAR(INST_TMPL_OP, type)
_NT_TYPES(INST_TMPL_ALL, dummy)

/* Instantiate template for string-related operations. */
COMP_TEMPLATE(_OP_STREQ_, cstring)
COMP_TEMPLATE(_OP_STRNE_, cstring)
COMP_TEMPLATE(_OP_CONTAINS_, cstring)
COMP_TEMPLATE(_OP_MATCHES_, cstring)

static struct _nt_suite* find_suite(const char *name)
{
    struct _nt_suite *suite = suites_list;
    while (suite) 
    {
        if (!strcmp(suite->name, name))
            return suite;

        suite = suite->next;
    }
    return NULL;
}

const struct _nt_suite* _nt_setup_suite(struct _nt_suite *suite)
{
    struct _nt_suite *other = find_suite(suite->name);
    if (!other) 
    {
        suite->next = suites_list;
        suites_list = suite;
    }
    else {
        if (!other->setup)
            other->setup = suite->setup;

        if (!other->teardown)
            other->teardown = suite->teardown;

        suite = other;
    }

    return suite;
}

void _nt_register_test(struct _nt_test *test)
{
    /* sort tests first by file name, second by line number */
    struct _nt_test **next = &tests_list;
    while (*next)
    {
        int nf = strcmp(test->file, (*next)->file);
        if (nf < 0)
            break;

        if (!nf && test->file < (*next)->file)
            break;

        next = &(*next)->next;
    }
   
    test->next = *next;
    *next = test;
}


void _nt_fail(void)
{
    longjmp(exception, 1);
}


static int run_test(struct _nt_test *test)
{
    int result;

    #ifndef __cplusplus
    _Alignas(max_align_t) char fixture [test->suite->fixture_size];
    #else
    char *fixture = (char*)alloca(test->suite->fixture_size);
    #endif

    memset(fixture, 0, test->suite->fixture_size);

    if (test->suite->setup)
        test->suite->setup((_nt_fixture_tag*)fixture);

    if (!setjmp(exception))
    {
        test->func();
        result = 0;
    }
    else {
        result = 1;
    }

    if (test->suite->teardown)
        test->suite->teardown((_nt_fixture_tag*)fixture);

    return result;
}


static void run_all_tests(void)
{
    struct _nt_test *test = tests_list;
    while (test)
    {
        test->fail = run_test(test);
        if (test->fail)
            _NT_PRINTF("Test %s FAILED!\n", test->name);
        test = test->next;
    }
}


int main()
{
    puts("Suites:");
    struct _nt_suite *suite = suites_list;
    while (suite) {
        printf("\t%s\n", suite->name);
        suite = suite->next;
    }
    puts("");

    puts("Tests:");
    struct _nt_test *test = tests_list;
    while (test) {
        printf("\t%s %s\n", test->suite->name, test->name);
        test = test->next;
    }
    puts("");

    out = stdout;
    run_all_tests();

    return 0;
}

