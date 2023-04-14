/* vim: set et sts=4 sw=4: */

#ifdef __cplusplus
#include <cstddef>      // std::max_align_t
#include <alloca.h>
#else
#include <stddef.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <limits.h>

#ifdef __unix__
#include <signal.h>
#endif

#include "nedotest.h"

#define Size(array) sizeof(array)/sizeof(array[0])

#define container_of(ptr, type, member) \
    ( (void)sizeof(0 ? (ptr) : &((type *)0)->member), \
      (type *)((char*)(ptr) - offsetof(type, member)) )

enum { MAX_STRING_LEN = 70 };

const char *argv0;
static FILE *out;
static int verbose;

static struct _nt_suite *suites_list;
static struct _nt_test* tests_list;
static struct _nt_mock* mocks_list;

static _Thread_local struct _nt_test *current_test;

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

struct string_buf {
    const size_t capacity;
    size_t len;
    size_t max_len;
    char str[];
};

struct _nt_print;
typedef struct _nt_print* _nt_print_t;

struct _nt_print {
    struct string_buf buf;
};

static inline struct string_buf* string_buf_reset(struct string_buf *buf)
{
    buf->len = buf->max_len = 0;
    if (buf->capacity > 0)
        buf->str[0] = 0;
    return buf;
}

static inline struct string_buf* string_buf_init(struct string_buf *buf, size_t capacity)
{
    *(size_t*)(intptr_t)&buf->capacity = capacity;
    return string_buf_reset(buf);
}

#define MAKE_STRING_BUF(capacity)                               \
    (string_buf_init(&(union {                                  \
        struct string_buf buf;                                  \
        char data [offsetof(struct string_buf, str[capacity])]; \
    }){}.buf, capacity))


/* This macros allows to check, that format string corresponds to argument
 * data types given to _nt_printf function. */
#define _NT_PRINTF(buf, ...) (0 ? (void)printf(__VA_ARGS__) : _printf_buf(buf, __VA_ARGS__))

static int vprintf_buf(_nt_print_t out, const char *fmt, va_list args)
{
    struct string_buf *buf = &out->buf;
    assert(buf->capacity >= buf->len);
    int len = vsnprintf(&buf->str[buf->len], buf->capacity - buf->len, fmt, args);
    if (len < 0)
        return -1;

    buf->max_len += len;
    if (buf->capacity) {
        buf->len += len;
        if (buf->len > buf->capacity)
            buf->len = buf->capacity - 1;
    }
    return len;
}

static int _printf_buf(_nt_print_t out, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vprintf_buf(out, fmt, args);
    va_end(args);
    return len;
}

/* Define set of "printer" functions for all known types. */

static void print_char(_nt_print_t out, int val, const char *fmt)
{
    int graph = isgraph(val);
    if (graph) _NT_PRINTF(out, "'%c' (", val);
    _NT_PRINTF(out, fmt, val);
    if (graph) _NT_PRINTF(out, ")");
}

/* Note, due to integral promotion all chars actualy expanded to integers. */

void _nt_print_Char(_nt_print_t out, const _nt_value_t* val)
{
    print_char(out, (unsigned)val->Char, "%#2x");
}

void _nt_print_unsigned_char(_nt_print_t out, const _nt_value_t* val)
{
    print_char(out, val->unsigned_char, "%u");
}

void _nt_print_signed_char(_nt_print_t out, const _nt_value_t* val)
{
    print_char(out, val->signed_char, "%d");
}

void _nt_print_any_unsigned(_nt_print_t out, const _nt_value_t* val)
{
    _NT_PRINTF(out, "%llu", val->any_unsigned);
}

void _nt_print_any_signed(_nt_print_t out, const _nt_value_t* val)
{
    _NT_PRINTF(out, "%lld", val->any_signed);
}

void _nt_print_any_float(_nt_print_t out, const _nt_value_t* val)
{
    _NT_PRINTF(out, "%Lf", val->any_float);
}

void _nt_print_any_pointer(_nt_print_t out, const _nt_value_t* val)
{
    _NT_PRINTF(out, "%p", val->any_pointer);
}

void _nt_print_cstring(_nt_print_t out, const _nt_value_t* val)
{
    const char *const str = val->cstring;
    const char *p = str;
    
    if (!str) {
        _NT_PRINTF(out, "(null)");
        return;
    }

    _NT_PRINTF(out, "\"");

    while (*p && p - str < MAX_STRING_LEN)
    {
        if (isprint(*p) && *p != '\"') {
            _NT_PRINTF(out, "%c", *p);
        }
        else {
            _NT_PRINTF(out, "\\x%02x", (unsigned)*p & 0xff);
        }
        ++p;
    }

    _NT_PRINTF(out, "\"%s", (p - str >= MAX_STRING_LEN) ? ".." : "");
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

        count = 0;
        if (sscanf(expr, " ( ( void * ) 0 ) %n", &count) >= 0 && count > 0 && !expr[count])
            return 1;

        count = 0;
        if (sscanf(expr, " ( void * ) 0 %n", &count) >= 0 && count > 0 && !expr[count])
            return 1;

        if (sscanf(expr, "%lli%n", &sval, &count) == 1 && len == (size_t)count) 
            return 1;

        if (sscanf(expr, "%llu%n", &uval, &count) == 1 && len == (size_t)count) 
            return 1;

        if (sscanf(expr, "%Lf%n", &fval, &count) == 1 && len == (size_t)count) 
            return 1;

        if (expr[0] == '"' || expr[0] == '\'')
            return 1;

        return 0;
}


struct scope_msg {
    const _nt_uniq_t ident;
    struct scope_msg *next_order, *prev_order;
    struct scope_msg *next_hash;
    const char text[];
};

struct _nt_scope_msg {
    const size_t hash_size;
    struct scope_msg *first_order;
    struct scope_msg *last_order;
    struct scope_msg* hash[];
};

static unsigned ident_hash(_nt_uniq_t ident)
{
    unsigned long long in = (unsigned long long)ident;
    unsigned hash = 2166136261;
    for (unsigned n = 0; n < sizeof(ident); n++, in >>= CHAR_BIT)
        hash = (hash ^ (char)in) * 16777619;

    return hash;
}

static struct _nt_scope_msg* scope_msg_create(size_t hash_size)
{
    struct _nt_scope_msg* ptr = malloc(offsetof(struct _nt_scope_msg, hash[hash_size]));
    if (!ptr) abort();

    memcpy(ptr, &(struct _nt_scope_msg){hash_size, NULL, NULL}, sizeof(struct _nt_scope_msg));
    memset(ptr->hash, 0, hash_size * sizeof(ptr->hash[0]));
    return ptr;
}

static void scope_msg_add(struct _nt_scope_msg *thiz, _nt_uniq_t ident, const char *str)
{
    unsigned idx = ident_hash(ident) % thiz->hash_size;

    /* find already existing message with same identifier or insert new at end of hash bucket */
    struct scope_msg **pmsg = &thiz->hash[idx];
    while (*pmsg) {
        if ((*pmsg)->ident == ident)
            break;

        pmsg = &(*pmsg)->next_hash;
    }

    size_t len = strlen(str) + 1;
    struct scope_msg* msg = malloc(offsetof(struct scope_msg, text[len]));
    if (!msg) abort();

    memcpy(msg, &(struct scope_msg){ident, NULL, NULL, NULL}, sizeof(struct scope_msg));
    memcpy((void*)(intptr_t)msg->text, str, len);

    if (*pmsg)
    {
        msg->next_hash = (*pmsg)->next_hash;

        if ((*pmsg)->prev_order)
            (*pmsg)->prev_order->next_order = (*pmsg)->next_order;

        if ((*pmsg)->next_order)
            (*pmsg)->next_order->prev_order = (*pmsg)->prev_order;

        if (thiz->last_order == *pmsg)
            thiz->last_order = (*pmsg)->prev_order;

        if (thiz->first_order == *pmsg)
            thiz->first_order = (*pmsg)->next_order;

        free(*pmsg);
    }

    *pmsg = msg;

    msg->prev_order = thiz->last_order;

    if (thiz->last_order)
        thiz->last_order->next_order = msg;

    thiz->last_order = msg;

    if (!thiz->first_order)
        thiz->first_order = msg;
}

struct msg_cons {
    int (*func)(struct msg_cons *, const char *);
};

static void scope_msg_get(struct _nt_scope_msg *thiz, struct msg_cons *cons)
{
    struct scope_msg *msg = thiz->first_order;
    if (msg) {
        while (msg) {
            if (cons->func(cons, msg->text) < 0)
                break;
            msg = msg->next_order;
        }
    }
}

static void scope_msg_reset(struct _nt_scope_msg *thiz)
{
    for (unsigned n = 0; n < thiz->hash_size; n++)
    {
        struct scope_msg *msg = thiz->hash[n];
        while (msg) {
            void *mem = msg;
            msg = msg->next_hash;
            free(mem);
        }
        thiz->hash[n] = NULL;
    }

    thiz->first_order = thiz->last_order = NULL;
}

static void scope_msg_destroy(struct _nt_scope_msg *thiz)
{
    scope_msg_reset(thiz);
    free(thiz);
}




#define _NT_GET_FUNC(val_type, res_type) _NT_CONCAT(_NT_CONCAT(_NT_CONCAT(_nt_get_, val_type), _as_), res_type)
#define _NT_GETTER(name) _NT_CONCAT(_nt_get_as_, name)
#define _NT_DECL_CONV(name, type, ...) _NT_TYPENAME(name) (*_NT_GETTER(name))(const _nt_value_t *);

typedef void _nt_print_fn(_nt_print_t, const _nt_value_t *);

struct _nt_typeinfo {
    _nt_print_fn* print;
    _NT_TYPES(_NT_DECL_CONV, dummy)
};


/* Define getters. */

#define NT_TYPES(APPLY, ...) \
    APPLY(Char,         __VA_ARGS__) \
    APPLY(signed_char,  __VA_ARGS__) \
    APPLY(unsigned_char, __VA_ARGS__) \
    APPLY(any_signed,   __VA_ARGS__) \
    APPLY(any_unsigned, __VA_ARGS__) \
    APPLY(any_float,    __VA_ARGS__) \
    APPLY(any_pointer,  __VA_ARGS__) \
    APPLY(cstring,      __VA_ARGS__)

// TODO #define CHECK_TYPES(type, ...)

#define NT_DEF_CONV1(type, ...) _NT_TYPES(NT_DEF_CONV2, type, __VA_ARGS__)

#define NT_DEF_CONV2(res_type, unused, val_type, ...) \
    static _NT_TYPENAME(res_type) _NT_GET_FUNC(val_type, res_type)(const _nt_value_t *val) {\
        return (_NT_TYPENAME(res_type))_Generic((_NT_TYPENAME(res_type))0,                  \
            _NT_TYPENAME(any_pointer): NT_CONV_PTR(res_type, val_type, val),                \
            _NT_TYPENAME(cstring): NT_CONV_PTR(res_type, val_type, val),                    \
            default: NT_CONV_INT(res_type, val_type, val));                                 \
    }

#define NT_CONV_PTR(res_type, val_type, val)        \
    _Generic((_NT_TYPENAME(val_type))0,             \
        _NT_TYPENAME(any_pointer): val->val_type,   \
        _NT_TYPENAME(cstring): val->val_type,       \
        default: 0)

#define NT_CONV_INT(res_type, val_type, val)        \
    _Generic((_NT_TYPENAME(val_type))0,             \
        _NT_TYPENAME(any_pointer): 0,               \
        _NT_TYPENAME(cstring): 0,                   \
        default: val->val_type)

NT_TYPES(NT_DEF_CONV1, dummy)

/* Definition of all typeinfo structures (TODO apply MAP macro). */
#define NT_DEF_CONV3(res_type, unused, val_type, ...) \
    ._NT_GETTER(res_type) = _NT_GET_FUNC(val_type, res_type),

#define NT_DEF_TYPEINFO(val_type, ...)                      \
    const struct _nt_typeinfo _NT_TYPEINFO(val_type) = {    \
        .print = _NT_CONCAT(_nt_print_, val_type),          \
        _NT_TYPES(NT_DEF_CONV3, val_type)                   \
    };

NT_TYPES(NT_DEF_TYPEINFO, dummy)

/* Function which is called when particular assertion is failed. */
static void assertion(
    const char *name, const char *file, unsigned line, int flags, const char *op, int narg,
    const char *left_expr, const _nt_value_t* left_val, _nt_print_fn left_print, 
    const char *right_expr, const _nt_value_t* right_val, _nt_print_fn right_print)
{
    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(20 * LINE_MAX);

    _NT_PRINTF(msg, "%s:%u: assertion failed: %s", file, line, name);
    if (!narg)
        _NT_PRINTF(msg, "(%s %s %s)", left_expr, op, right_expr);
    else
        _NT_PRINTF(msg, "(%s)", left_expr);

    assert(current_test != NULL);

    unsigned offs = 0;

    if (!(flags & _NT_NOASSERT) && !current_test->first.fail_line)
        fprintf(out, "Test %s FAILED:\n", current_test->name), fflush(out);

    unsigned nl = 0;
    if (!is_literal(left_expr)) {
	_NT_PRINTF(msg, "\n%*swhere (%s) = ", offs + 4, "", left_expr);
	left_print(msg, left_val);
	++nl;
    }

    if (!narg && !is_literal(right_expr)) {
	_NT_PRINTF(msg, "%s%*s%s (%s) = ",
	    nl ? ",\n" : "",
	    offs + 4, "",
	    nl ? "  and" : "where",
	    right_expr);
	right_print(msg, right_val);
	++nl;
    }       

    if (nl) _NT_PRINTF(msg, ".\n");

    fprintf(out, "%*s%s\n", offs, "", msg->buf.str);
    fflush(out);

    current_test->last.fail_file = file;
    current_test->last.fail_line = line;
    free(current_test->last.fail_msg);
    current_test->last.fail_msg = strdup(msg->buf.str);

    if (!current_test->first.fail_line && !(flags & _NT_NOASSERT)) {
        current_test->first.fail_file = file;
        current_test->first.fail_line = line;
        free(current_test->first.fail_msg);
        current_test->first.fail_msg = strdup(msg->buf.str);
    }
}

struct msg_collect {
    struct msg_cons cons;
    char *text;
};

int msg_collect(struct msg_cons *cons, const char *msg)
{
    struct msg_collect *col = container_of(cons, struct msg_collect, cons);
    
    size_t prev_len = col->text ? strlen(col->text) : 0;
    size_t next_len = strlen(msg);
    size_t len = prev_len + next_len + 2;
    char *text = realloc(col->text, len);
    if (!text) return -1;

    snprintf(&text[prev_len], len, "\n%s", msg);
    col->text = text;
    return next_len;
}

void _nt_abort(void)
{
    assert(current_test != NULL);

    struct msg_collect col = {.cons.func = msg_collect, .text = strdup(current_test->first.fail_msg)};
    scope_msg_get(current_test->scope_msg, &col.cons);

    fprintf(out, "%s: %u: failure with a message: %s\n",
        current_test->first.fail_file, current_test->first.fail_line, col.text);
    fflush(out);

    free(col.text);

    longjmp(*(jmp_buf*)current_test->jmpbuf, 1);
}

void _nt_assert(void)
{
    assert(current_test != NULL);

    struct msg_collect col = {.cons.func = msg_collect, .text = strdup(current_test->last.fail_msg)};
    scope_msg_get(current_test->scope_msg, &col.cons);

    scope_msg_reset(current_test->scope_msg);

    fprintf(out, "%s: %u: failure with a message: %s\n",
        current_test->first.fail_file, current_test->first.fail_line, col.text);
    fflush(out);

    free(col.text);
}


/* Define implementation of named (LT, GT, etc...) binary operations */
#define IMPL_FUNC(op) _NT_CONCAT(IMPL, op)

#define IMPL_OP_LT_(a, b) (a < b)
#define IMPL_OP_GT_(a, b) (a > b)
#define IMPL_OP_LE_(a, b) (a <= b)
#define IMPL_OP_GE_(a, b) (a >= b)
#define IMPL_OP_EQ_(a, b) (a == b)
#define IMPL_OP_NE_(a, b) (a != b)

#define IMPL_OP_STREQ_(a, b)                    !strcmp(a, b)
#define IMPL_OP_STRNE_(a, b)                    strcmp(a, b)
#define IMPL_OP_CONTAINS_(haystack, needle)     strstr(haystack, needle)
#define IMPL_OP_NOT_CONTAINS_(haystack, needle) !strstr(haystack, needle)
#define IMPL_OP_MATCHES_(string, pattern)       match(string, pattern)
#define IMPL_OP_NOT_MATCHES_(string, pattern)   !match(string, pattern)

/* Function template for _nt_OP_TYPE functions declared by _NT_DECL_FUNC macros. */
#define COMP_TEMPLATE(op_name, type)                                            \
    _NT_DECL_FUNC(op_name, type) {                                              \
        (void)dummy;                                                            \
        return !!IMPL_FUNC(op_name)(                                            \
            left_type->_NT_GETTER(type)(left_val),                              \
            right_type->_NT_GETTER(type)(right_val)) ^ (flags & _NT_NEG_OP) ? 1 \
         : (assertion(name, file, line, flags, op, narg,                        \
                left_expr, left_val, left_type->print,                          \
                right_expr, right_val, right_type->print), 0);                  \
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
COMP_TEMPLATE(_OP_NOT_CONTAINS_, cstring)
COMP_TEMPLATE(_OP_MATCHES_, cstring)
COMP_TEMPLATE(_OP_NOT_MATCHES_, cstring)

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

        if (!other->fixture_size)
            other->fixture_size = suite->fixture_size;

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

static int run_test(struct _nt_test *test)
{
    fprintf(out, "running %s\n", test->name);
    fflush(out);

    #ifndef __cplusplus
    _Alignas(max_align_t) char fixture [test->suite->fixture_size];
    #else
    char *fixture = (char*)alloca(test->suite->fixture_size);
    #endif

    memset(fixture, 0, test->suite->fixture_size);
    test->fixture = (_nt_fixture_tag*)fixture;
    test->scope_msg = scope_msg_create(1024);
    test->first.fail_line = 0, test->first.fail_file = test->first.fail_msg = NULL;
    test->last.fail_line = 0, test->last.fail_file = test->last.fail_msg = NULL;

    current_test = test;
    reset_all_mocks();

    if (test->suite->setup)
        test->suite->setup((_nt_fixture_tag*)fixture);

    struct context ctx;
    ctx.test = test;
    test->jmpbuf = &ctx.exception;
    if (!setjmp(ctx.exception))
    {
        // tls_context = &ctx;
        test->func((_nt_fixture_tag*) fixture);
    }

    int result = !!current_test->first.fail_line;

    // tls_context = NULL;

    if (test->suite->teardown)
        test->suite->teardown((_nt_fixture_tag*)fixture);

    free(test->first.fail_msg), free(test->last.fail_msg);
    test->first.fail_msg = test->last.fail_msg = NULL;

    scope_msg_destroy(test->scope_msg);
    test->scope_msg = NULL;

    current_test = NULL;

    return result;
}

int _nt_is_fail(void)
{
    int result = !!current_test->first.fail_line;
    if (!result) _nt_scope_reset();
    return result;
}

static void _nt_message(_nt_print_t msg, unsigned nargs, va_list args)
{
    for (unsigned n = 0; n < nargs; n++)
    {
        const _nt_value_t *val = va_arg(args, _nt_value_t*);
        _nt_typeinfo_t type = va_arg(args, _nt_typeinfo_t);

        if (type->print == _nt_print_cstring)
            _NT_PRINTF(msg, "%s", val->cstring);
        else
            type->print(msg, val);
    }
}

void _nt_fail(const char *file, unsigned line, unsigned nargs, ...)
{
    va_list args;
    va_start(args, nargs);
    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
    _NT_PRINTF(msg, "%s:%u: ", file, line);
    _nt_message(msg, nargs, args);

    struct msg_collect col = {.cons.func = msg_collect, .text = strdup(msg->buf.str)};
    scope_msg_get(current_test->scope_msg, &col.cons);

    if (!current_test->first.fail_line) {
        fprintf(out, "Test %s FAILED:\n", current_test->name);
        current_test->first.fail_file = file;
        current_test->first.fail_line = line;
        free(current_test->first.fail_msg);
        current_test->first.fail_msg = strdup(col.text);
    }

    fprintf(out, "%s: %u: failure with a message: %s\n", file, line, col.text);
    fflush(out);

    free(col.text);

    longjmp(*(jmp_buf*)current_test->jmpbuf, 1);
}

void _nt_fail_check(const char *file, unsigned line, unsigned nargs, ...)
{
    va_list args;
    va_start(args, nargs);

    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
    _NT_PRINTF(msg, "%s:%u: ", file, line);
    _nt_message(msg, nargs, args);

    struct msg_collect col = {.cons.func = msg_collect, .text = strdup(msg->buf.str)};
    scope_msg_get(current_test->scope_msg, &col.cons);

    scope_msg_reset(current_test->scope_msg);

    if (!current_test->first.fail_line) {
        fprintf(out, "Test %s FAILED:\n", current_test->name);
        current_test->first.fail_file = file;
        current_test->first.fail_line = line;
        free(current_test->first.fail_msg);
        current_test->first.fail_msg = strdup(col.text);
    }

    fprintf(out, "%s: %u: failure with a message: %s\n", file, line, col.text);
    fflush(out);

    free(col.text);
    va_end(args);
}

void _nt_skip(const char *file, unsigned line, unsigned nargs, ...)
{
    va_list args;
    va_start(args, nargs);
    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
    _NT_PRINTF(msg, "%s:%u: ", file, line);
    _nt_message(msg, nargs, args);
    fprintf(out, "Test %s is skipped at %s:%u: %s\n", current_test->name, file, line, msg->buf.str);
    fflush(out);
    longjmp(*(jmp_buf*)current_test->jmpbuf, 1);
}

void _nt_success(const char *file, unsigned line, unsigned nargs, ...)
{
    va_list args;
    va_start(args, nargs);
    if (nargs) {
        _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
        _NT_PRINTF(msg, "%s:%u: ", file, line);
        _nt_message(msg, nargs, args);
        fprintf(out, "%s\n", msg->buf.str);
        fflush(out);
    }
    assert(current_test);
    longjmp(*(jmp_buf*)current_test->jmpbuf, 1);
}

void _nt_warn(const char *file, unsigned line, unsigned nargs, ...)
{
    va_list args;
    va_start(args, nargs);
    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
    _NT_PRINTF(msg, "%s:%u: ", file, line);
    _nt_message(msg, nargs, args);
    fprintf(out, "WARN: %s\n", msg->buf.str);
    fflush(out);
    va_end(args);
}

void _nt_info(_nt_uniq_t ident, const char *file, unsigned line, unsigned nargs, ...)
{
    va_list args;
    va_start(args, nargs);

    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
    _NT_PRINTF(msg, "%s:%u: ", file, line);
    _nt_message(msg, nargs, args);

    assert(current_test);
    scope_msg_add(current_test->scope_msg, ident, msg->buf.str);
    
    va_end(args);
}

void _nt_capture(_nt_uniq_t ident, const char *file, unsigned line, unsigned nargs, ...)
{
    if (!nargs)
        return;

    _nt_print_t msg = (_nt_print_t)MAKE_STRING_BUF(10 * LINE_MAX);
    _NT_PRINTF(msg, "%s:%u: ", file, line);

    va_list args;
    va_start(args, nargs);

    const char *dlm = "";
    for (unsigned n = 0; n < nargs; n++)
    {
        const char *name = va_arg(args, const char*);
        const _nt_value_t *val = va_arg(args, _nt_value_t*);
        _nt_typeinfo_t type = va_arg(args, _nt_typeinfo_t);

        if (!is_literal(name))
            _NT_PRINTF(msg, "%s%s = ", dlm, name);
        else
            _NT_PRINTF(msg, "%s", dlm);

        type->print(msg, val);
        dlm = ", ";
    }

    va_end(args);

    assert(current_test);
    scope_msg_add(current_test->scope_msg, ident, msg->buf.str);
}

void _nt_scope_reset(void)
{
    scope_msg_reset(current_test->scope_msg);
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
            if (run_test(test))
                ++nfails;
        }

        test = test->next;
    }

    fprintf(out, "%u/%u tests passed, %u tests failed.\n", ntests - nfails, ntests, nfails);
    fflush(out);
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

