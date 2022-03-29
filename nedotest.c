/* vim: set et sts=4 sw=4: */

#ifdef __cplusplus
#include <cstddef>      // std::max_align_t
#include <alloca.h>
#else
#include <stddef.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

#ifdef __unix__
#include <signal.h>
#endif

#include "nedotest.h"

#define Size(array) sizeof(array)/sizeof(array[0])

enum { MAX_STRING_LEN = 70 };

const char *argv0;
static FILE *out;
static int verbose;

static struct _nt_suite *suites_list;
static struct _nt_test* tests_list;
static struct _nt_mock* mocks_list;

#ifdef __cplusplus
#define _Thread_local thread_local
#define _Alignas alignas
#define max_align_t std::max_align_t
#endif

// FIXME MSVC, also max_align_t, alignas, void *_alloca(size_t size), constructor

struct context {
    struct _nt_test *test;
    jmp_buf exception;
};

_Thread_local struct context *tls_context;


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
    struct _nt_test *test = tls_context->test;
    if (!test->fail) {
        tls_context->test->fail = 1;
        _NT_PRINTF("Test %s FAILED:\n", test->name);
    }

    _NT_PRINTF("%4s%s: %u: assertion failed: %s", "", file, line, name);
    if (!narg)
        _NT_PRINTF("(%s %s %s)\n", left_expr, op, right_expr);
    else
        _NT_PRINTF("(%s)\n", left_expr);

    unsigned nl = 0;
    if (!is_literal(left_expr)) {
        _NT_PRINTF("%8swhere (%s) = ", "", left_expr);
        left_print(left_val);
        ++nl;
    }

    if (!narg && !is_literal(right_expr)) {
        _NT_PRINTF("%s%8s%s (%s) = ",
            nl ? ",\n" : "",
            "",
            nl ? "  and" : "where",
            right_expr);
        right_print(right_val);
        ++nl;
    }       

    if (nl) _NT_PRINTF(".\n\n");

    fflush(out);
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

void _nt_register_mock(struct _nt_mock *mock)
{
    mock->next = mocks_list;
    mocks_list = mock;
}

static void reset_all_mocks(void)
{
    struct _nt_mock *mock = mocks_list;
    while (mock) {
        mock->func();
        mock = mock->next;
    }
}


void _nt_trap(void)
{
    volatile int x = 0;
    (void)x;
}

void _nt_fail(const char *msg, const char *file, unsigned line)
{
    if (msg) {
        _NT_PRINTF("Test %s FAILED:\n", tls_context->test->name);
        _NT_PRINTF("%s: %u: failure with a message: %s", file, line, msg);
        fflush(out);
    }
    longjmp(tls_context->exception, 1);
}


static int run_test(struct _nt_test *test)
{
    _NT_PRINTF("running %s\n", test->name);
    fflush(out);

    reset_all_mocks();

    #ifndef __cplusplus
    _Alignas(max_align_t) char fixture [test->suite->fixture_size];
    #else
    char *fixture = (char*)alloca(test->suite->fixture_size);
    #endif

    memset(fixture, 0, test->suite->fixture_size);

    if (test->suite->setup)
        test->suite->setup((_nt_fixture_tag*)fixture);

    struct context ctx;
    ctx.test = test;

    if (!setjmp(ctx.exception))
    {
        tls_context = &ctx;

        test->fail = 0;
        test->func((_nt_fixture_tag*) fixture);
    }
    else {
        test->fail = 1;
    }

    tls_context = NULL;

    if (test->suite->teardown)
        test->suite->teardown((_nt_fixture_tag*)fixture);

    return test->fail;
}


static int run_all_tests(int nfilt, const char *const* filters)
{
    unsigned ntests = 0, nfails = 0;
    struct _nt_test *test = tests_list;
    while (test)
    {
        char const *const* filt = filters;
        while (filt != &filters[nfilt]) {
            if (match(test->name, *filt))
                break;

            ++filt;
        }

        if (!nfilt || filt != &filters[nfilt])
        {
            ++ntests;
            test->fail = run_test(test);
            if (test->fail)
                ++nfails;
        }

        test = test->next;
    }

    _NT_PRINTF("%u/%u tests passed, %u tests failed.\n", ntests-nfails, ntests, nfails);
    return ntests != nfails;
}


static int cmd_list(int argc, const char *const* argv)
{
    (void)argc, (void)argv;

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

    exit(EXIT_SUCCESS);
    return 0;
}

static int cmd_verbose(int argc, const char *const* argv)
{
    (void)argc, (void)argv;
    
    verbose = 0;

    return 0;
}



static int cmd_help(int argc, const char *const* argv);

struct cmdline_opt {
    int (*handler)(int argc, const char *const* argv);
    const char *const name[4];
    const char *args;
    char const* help;

};

static const struct cmdline_opt cmdline_opts[] = {
    {cmd_help,      {"-h", "-help", "--help"},      "",     "print help"},
    {cmd_list,      {"-l", "-list", "--list"},      "",     "list available tests"},
    {cmd_verbose,   {"-v", "-verbose", "--verbose"}, "",    "print name of each executed test"}
};

    
static int cmd_help(int argc, const char *const* argv)
{
    (void)argc, (void)argv;

    printf("%s [-options] [tests-filter...]\n", argv0);
    puts("Available options are:");
    const struct cmdline_opt *opts = cmdline_opts;
    while (opts != &cmdline_opts[Size(cmdline_opts)]) {
        printf("%-10s %-16s %s\n", opts->name[0], opts->args, opts->help);
        ++opts;
    }

    exit(EXIT_SUCCESS);
    return 0;
}


static int argv_sort(const void *Left, const void *Right)
{
    const char *const* left = (const char*const*)Left, 
               *const *right = (const char *const*)Right;
    
    int res = ((*left[0] - '-') & 0xff) - ((*right[0] - '-') & 0xff);
    return res ? res : strcmp(*left, *right);
}

int main(int argc, const char *argv[])
{
    argv0 = argv[0];
    out = stdout;

    /* parse command line options */
    qsort(&argv[1], argc-1, sizeof(char*), argv_sort);

    const char *const* arg = &argv[1];
    while (arg != &argv[argc])
    {
        if (**arg != '-')
            break;

        if (!strcmp(*arg, "--")) {
            ++arg;
            break;
        }

        int unknown = 1;
        const struct cmdline_opt *opts = cmdline_opts;
        while (opts != &cmdline_opts[Size(cmdline_opts)])
        {
            for (unsigned i = 0; i < Size(opts->name); ++i)
            {
                if (!opts->name[i])
                    break;

                if (!strcmp(*arg, opts->name[i]))
                {
                    arg += opts->handler(&argv[argc] - arg, &argv[arg - &argv[0] + 1]);
                    unknown = 0;
                    break;
                }
            }

            ++opts;
        }

        if (unknown)
        {
            fprintf(stderr, "%s: '%s': unknown option\n", argv0, *arg);
            exit(EXIT_FAILURE);
        }

        ++arg;
    }

    return run_all_tests(&argv[argc] - arg, arg);
}

