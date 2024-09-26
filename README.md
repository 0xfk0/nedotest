
# Nedotest

**Nedotest** is a minimalistic (~2000 SLOC) Unit Testing framework for C and C++.


## Features

  * lightweight;
  * works with modern C (C11) and C++ (C++11);
  * Catch2 like assertion syntax;
  * No any bloat in header file -- no any unnecessary dependencies at all;
  * Supports test fixtures;
  * automatic test registration, no need to write main() function;
  * in case of error test system prints variable values (as Catch2 does this, in plain C!)
  * unlike many other unit test systems all values in assertions evaluated only once, so following code remains valid:

        CHECK(x EQ 0);
        CHECK(++x EQ 1);
        CHECK(++x EQ 2)...
        

## Future plans

  This is not complete project, future plans include:

  1. tests isolation (via spawning child processes);
  2. parallel execution on multiprocessor platforms;
  3. output to XML (xUnit);
  4. timeouts for tests;
  5. debug breakpoints in place of failure (you can set breakpoint on `_nt_trap` function);
  6. strict C11 standard compliance, no unnecessary dependencies on Unix/Windows, particular compiler version, compiler extensions, etc...
  7. must be able to run on any platform, including embedded environments.
  8. use C++ exceptions instead of longjmp.


## Installation

  Just include "nedotest.h" into each file implementing the tests, and link the result with "nedotest.c"...

## Writing tests

*Please see comments in `nedotest.h` for clarify any details that are not described sufficiently.*

In very basic form, each separate test is a C-function. This function needs to
be written in form:

        #include "nedotest.h"

        TEST(SuiteName, TestName)
        {
            ... test's code
        }

First of all, you need to include header file in the source containing the
tests itself, and compileq/link the executable with `nedotest.c` file.

Keyword **TEST** allows to define a new test and test function. There
`SuiteName` and `TestName` can be arbitrary selected alphanumeric identifiers.
It is assumed, that distinct tests are grouped in named suites. Later, you can
run only a selected test from a particular suite, or all tests from selected
suites.

Test body can contain regular C source code and special keywords used for test
assertion, for function mocking control, for diagnostic output, etc...
These keywords are described below.


### Test assertions.

In basic form, test succeeds if the test function reaches its end and returns,
and the program not crashes. And if no assertions happen during test execution.
Assertions may be written in the following form, example:

        #include "nedotest.h"

        TEST(Suite1, Test1)
        {
            int x = 2;
            int y = pow2(x, 2)
            REQUIRE(y EQ 4);
            CHECK(y NE 2);
            REQUIRE_FALSE(y EQ 2);
            CHECK_FALSE(y NE 4);
            ...
        }

Assertion checks, that given condition met or not met (for `REQUIRE_FALSE` and
`CHECK_FALSE` assertions). And if the condition is not satisfied, the test is
marked as failed, and an appropriate message will be printed, see an example:

        file.c:38: assertion failed: REQUIRE(x == y)
            where (x) = 1,
              and (y) = 2.

There is a difference between `REQUIRE` and `CHECK` assertions: first will stop
the test (by aborting test function) as soon, as the first assertion failed.
Second, `CHECK` assertion is only marks the test as failed, but execution of
the test function continues, which may cause diagnostic output from following
assertions.

Assertions can be a unary boolean expression or binary operation. Following
binary operations are supported for numbers (of any types):

        LT - less than        (<)
        GT - greater than     (>)
        LE - less or equal    (<=)
        GE - greater or equal (>=)
        EQ - equal            (==)
        NE - not equal        (!=)

You can use regular C-operators shown in parentheses on the right, but it is
preferable to use special keywords: in the latter case, the test system will be
able to print variable values if the assertion fails. In case if you use
regular C-operators, assertion turns into unary boolean expression and
diagnostic message containing actual values will not be printed.

And following binary operations is supported for strings:

        STREQ        - strings equal
        STRNE        - strings not equal
        CONTAINS     - left operand contains sequence of right operand
        NOT_CONTAINS - reverse condition
        MATCHES      - left operand matches to "glob-expression" on the right
        NOT_MATCHES  - reverse condition

`MATCHES`  and  `NOT_MATCHES`  operations   are  case-insensitive,  but  STREQ`,
``STRNE`, `CONTAINS` and `NOT_CONTAINS` are case-sensitive.

In the following examples, all the assertions doesn't cause test failure:

        int x = 1, y = 2;
        CHECK(x LT 2);
        CHECK(x LE 2);
        CHECK(x EQ 1);
        CHECK(x NE 2);
        CHECK(y GT 1);
        CHECK(y GE 2);

        const char *s = "test";
        CHECK(s STREQ "test");
        CHECK(s STRNE "abcd");
        CHECK(s MATCH "*ES*");
        CHECK(s CONTAINS "es");
        CHECK(s NOT_MATCHES "1*");
        CHECK(s NOT_CONTAINS "a");


### Logging messages from the test functions.

In case of the test failure, it will be convenient to see a variable values at
lines preceding the line causing test failure. Variable values can be captured
in the following way:

        int x = 1, y = 2;
        char s[] = "text";
        ...
        CAPTURE(x, y, s);
        ...
        REQUIRE(x EQ y);

Keyword `CAPTURE` records variable values, which will be printed only if the
test case fails in current test-function. In this way, debugging of the test
case can be simplified. Output example:

        file.c:38: assertion failed: REQUIRE(x == y)
            where (x) = 1,
              and (y) = 2.

        file.c:33: x = 1, y = 2, s = "text".


Keyword `INFO` has similar role and  allows writing arbitrary message in case
if following assertion fails. The difference  between `INFO` and `CAPTURE`
keywords is that `CAPTURE` prints variable names  and their values, but `INFO`
only print values (which is similar to `printf` functions but not relies on the
presence of format string: variables of all basic types printed automatically).
Example:

        int x = 3;
        int y = pow(x, 2);
        INFO("x is ", x, ", y is ", y);
        ...
        REQUIRE(x EQ sqrt(y));


In case if `REQUIRE` assertion fails, additional following message will be
printed:

        file.c:20: assertion failed: REQUIRE(x == sqrt(y))
            where (x) = 3,
              and (sqrt(y)) = 0.0001.

        file.c:16: x is 3, y is 9


Keyword `WARN` allows to print a warning message, which will be printed to
stderr in any case, if the test failed or not. It accepts argument in the same
way as `INFO` function. Example:

        int x = 42;
        char *s = "test";
        WARN("message x=", x, ", s=", s, ".");

This will print:

        file.c:32: message x=42, s=test.


Keywords `CHECKED_IF` and `CHECKED_ELSE` allow to print a diagnostic message in
a case, if the condition given in argument evaluates to false. These keywords
can be used in place of `if` and `if..else` operators in source code. Examples:

        FILE *file = fopen("file.txt", "r");
        CHECKED_IF(file NE NULL) {
                // Do test on file, if the file have not been
                // opened, this branch will never be executed and
                // appropriate diagnostic message will be printed.
                fread(buffer, size, 1, file);
        }

        int y = f(x);
        CHECKED_ELSE(y GT x) {
                // At this point x >= y
                // and diagnostic message will be printed.
        }

Keyword `CHECK_NOFAIL` allows to print a diagnostic message in a case, if the
condition given in argument is evaluated to false (same as `REQUIRE` or `CHECK`
keywords do). The difference is that the test will not be marked as failed in
this case, and function execution continues.


### Controlling test execution.

Test function can be stopped at any moment with a diagnostic message. This can
be done with use of `FAIL` keyword, example:

        const char *filename = ...;
        FILE *file = fopen(filename, "r");
        if (!file) {
            FAIL("Unable to open file: '", filename, "'");
            // at this point execution stopped
        }

The test can be only marked as failed with a diagnostic message, but execution
of the test may be continued. This can be done with `FAIL_CHECK` keyword, which
differs from `FAIL` in that it doesn't interrupt test function execution.
Example:

        if (unlink(filename) < 0)
            FAIL_CHECK("unable to remove file '", filename, "'");
        ...
        // test continues

Test execution may be aborted with a diagnostic message and test marked as
skipped. This can be done with `SKIP_TEST` keyword. Example:

        const char fname[] = "/proc/self/fd";
        FILE *file = fopen(fname, "r");
        if (!file) {
                SKIP_TEST("test skipped: ", fname, " unavailable: ", strerror(errno))
                // at this point execution stopped
        }
        ...

Test execution may be aborted with the message and test marked as passed. This
can be done with `SUCCEED` keyword, see an example:

        int y = pow(x, 2);
        if (x * x == y) {
            SUCCEED("test passed: ", x * x,  " == ", y);
            // will not reach this line
        }


As it was said before, some conditions, such as `CHECK`, or `CHECK_FALSE` may
result in test failure, but test execution continues. You may check, that
currently running test is already in failed state with `TEST_FAILED()`
function, which returns `true` if test previously failed. This mostly needed to
interrupt test execution if test running in a loop. For example, following test
runs in a loop multiple times and has multiple checks in a loop body. But if
previous loop iteration caused a failure, test execution terminated:

        for (unsigned n = 0; n < 1000; n++) 
        {
            REQUIRE(!TEST_FAILED());
            ...
            CHECK(condition1);
            ...
            CHECK(condition2);
            ...
        }


### Static assertions.

Sometimes it is useful to check the conditions, which should be satisfied at
the compile time. `_Static_assert` operator can be employed for this. But at
the same time, compilation error in unit tests might be very undesirable, as it
will break the whole testing process. It can be preferable to turn compile time
check into run time check. For this, `STATIC_REQUIRE` and
`STATIC_REQUIRE_FALSE` keywords exist. Both are similar to `REQUIRE` and
`REQUIRE_FALSE`, but additionally, both require, that argument is a
compile-time constant. Example:

        struct data {
            int x;
            int y;
            ...
        };

        struct holder {
            char _Alignof(max_align_t) bytes[64];
        };

        STATIC_REQUIRE(sizeof(struct data) LE sizeof(struct holder));
        ...
        STATIC_REQUIRE_FALSE(sizeof(struct data) GT sizeof(struct holder));


### Fixtures, setup and teardown functions.

Also, each group of tests belonging to a particular suite might have "setup"
and "teardown" functions. First one is run each time before starting the test,
and second one after finishing the test, even in a case, if the test fails.
Setup and teardown functions are optional: you may not define no one, or define
only one of them. Also with the notion of setup/teardown functions, the notion
of "fixture" is closely coupled. "Fixture" is a structure, which may hold user
provided context. This context is created in "setup" function, then may be
accessed in "test" function, and then may be accessed and destroyed in
"teardown" function. See an example:

        struct FIXTURE(Suite1)
        {
             FILE *file;
             int count;
        }

        TEST_SETUP(Suite1)
        {
            struct FIXTURE(Suite1) *fix = GET_FIXTURE();
            fix->file = fopen("/dev/zero", ...);
            fix->count = 0;
        }

        TEST_TEARDOWN(Suite1)
        {
            fclose(GET_FIXTURE()->file);
        }

        TEST(Suite1, Test1)
        {
            char buf[512];
            fread(buf, 64, 1, GET_FIXTURE()->file);
            ...
            GET_FIXTURE()->count++;
            ...
        }


### Mocking functions.

Test system allows mocking function calls. This possibility might be useful in
a case, for example, when some component under  the test uses some function and
no implementation  of this  function exists,  or in  a case  when calls  to a
real implementation of this function under the test is undesirable and instead
a fake function must  be called instead, and  this fake function must  return
specially prepared result, or in a case when real function must be called, but
the fact of function call must be detected or the number of function calls must
be counted.

Mock function can  be created with `MOCK_FUNCTION` keyword if  the real
function doesn't exist, or  with `MOCK_OVERRIDE` keyword if mocking of  the
existing real function is needed.

Both of these macros accepts following arguments list:

  * the result of the function (its type);
  * the function name;
  * rest of arguments (optional) is the types of function arguments.

For example, lets mock `malloc` function to imitate "out of memory" error:

        MOCK_OVERRIDE(void*, malloc, size_t);

        TEST(Suite1, Test1)
        {
            MOCK_SET_RESULT(malloc, NULL);
            void *result = use_malloc(...);
            CHECK(result == NULL);
            CHECK(MOCK_COUNT(malloc) EQ 1);
        }

In this example, `malloc` function is mocked and with `MOCK_SET_RESULT` keyword
the result of all following calls to `malloc` will be changed to `NULL` (real
`malloc` function will not be called in this case). Then we are checking, that
result is actually `NULL`, and with `MOCK_COUNT` keyword we can check, that
during call to `use_malloc`, the `malloc` function has been called exactly one
time.

For a compiler, to deal with real and mock version of the function, a special
options is needed in case if `MOCK_OVERRIDE` keyword used to override real
function:

  * `-Wl,--wrap=func_name` to  "wrap" a  function with `__real_`  and `__wrap`
    prefixes and to allow coexistence of real function and mock object;

  * `-fvisibility=hidden`  to  avoid  exposing  of mock  functions  to  global
    namespace and linking it from outer code.

These options should be added to a build system for the particular test, if the
test uses function mocking.

Another scenario: suppose, we have a component which uses function `do_the_job`
which actually doesn't exist in test project, and we don't want to implement
it. Then we can mock with function with `MOCK_FUNCTION` keyword, as shown on
the example below:

        int do_the_job(int x, int y);   // declaration of non-existent function

        MOCK_FUNCTION(int, do_the_job, int, int);

        static struct {
            int x, y;
        } do_the_job_args;

        static int do_the_job_fake(int x, int y)
        {
            do_the_job_args.x = x;
            do_the_job_args.y = y;
            return -1;
        }

        TEST(Suite1, Test2)
        {
            MOCK_SET_FUNC(do_the_job, do_the_job_fake);
            int result = use_do_the_job(...);
            CHECK(result EQ -1);
            CHECK(do_the_job_args.x EQ 1);
            CHECK(do_the_job_args.y EQ 2);
            CHECK(MOCK_COUNT(malloc) EQ 1);

            MOCK_RESET(do_the_job);
            result = use_do_the_job_again(...);
            CHECK(MOCK_COUNT(malloc) EQ 1);
        }

In this example, `do_the_job` function doesn't exist. Instead, only mock object
created for it. And the code under the test then can call `do_the_job` function
as if it is existing. With `MOCK_SET_FUNC` keyword, a fake version of
`do_the_job` function is substituted. This fake version saves the argument
passed to `do_the_job` function and returns a fake result. Then test may check
correctness of argument and the result. Also, this example demonstrates, that
mocked function calls counter can be reset with `MOCK_RESET` keyword.


## Command line options:

`--help` or `-h`    shows all available options...

`--list` or `-l`    list tests list which was compiled within executable.

`--verbose` or `-v`  prints name of each test before executing it.

All other arguments interpreted as test filters: if at least one test filter
specified, the test system will run only the tests matching to specified
filters. Test filters is somethat similar to shell's glob patterns: each
character must be matched as is, but asterisk (`*`) might replace any number of
any characters. Filters must match to the names of the tests. You can get list
of test names with option `--list`.

Test names are composed from test suite name and particular test name divided
by slash. For example, expression in source code like `TEST(SuiteName,
TestName)` will create test named `SuiteName/TestName`. So you can easily run
only particular test, or only tests from particular suite (by filter like
`SuiteName/*`).


## Links:

Project was inspired by:

  1. https://github.com/jasmcaus/tau
  2. https://github.com/ztbrown/Simulacrum
  3. https://github.com/eerimoq/nala
  4. https://github.com/catchorg/Catch2/
  5. https://www.criterion.com/
  6. https://cmocka.org/
  7. https://github.com/Snaipe/Mimick
  8. https://libcheck.github.io/check/
  9. https://github.com/meekrosoft/fff

