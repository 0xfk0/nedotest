

# Nedotest

**Nedotest** is a minimalistic (1KLOC) Unit Testing framework for C and C++.


## Features

  * lightweight;
  * works with modern C (C11) and C++ (C++11);
  * Catch2 like assertion syntax;
  * No any bloat in header file -- no any unnecessary dependencies at all!
  * Supports test fixtures;
  * automatic test registration, no need to write main() function;
  * in case of error test system prints variable values (as Catch2 does this! in plain C!)
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
  5. ability to run and debug one particular test, filtering tests to be executed;
  6. debug breakpoints in place of failure;
  7. strict C11 standard compliance, no unnecessary dependencies on Unix/Windows, particular compiler version, compiler extensions, etc...
  8. must be able to run on any platform, including embedded environments.
  9. support for threading (get rid of global variables).
  10. use C++ exceptions instead of longjmp.


## Installation

  Just include "nedotest.h" into each file implementing the tests, and link the result with "nedotest.c"...

## Writing tests

See more details in nedotest.h file...


1. include header file:

    #include "nedotest.h"


2. write the test

    TEST(SuiteName, TestName)
    {
        ...
        CATCH(x EQ y);
        REQUIRE(x LT q);
        ...
        CATCH(x == y && y < z);
        ...
        int error = func();
        REUIRE_FALSE(error);
        ...
    }


3. You can add Fixture for all tests in particular suite:

    struct FIXTURE(SuiteName)
    {
        char *data;
        ... some data you need for a test...
    };

    /* This function will be called before each test in a suite */
    TEST_SETUP(SuiteName)
    {
        thiz->data = malloc(100500);
        ...
    }

    /* This will be called at end of test, even if test failed. */
    TEST_FIXTURE(SuiteName)
    {
        free(thiz->data);
    }

    /* TODO curently it isn't specified how test can access fixture... */


4. You can mock functions:

    /* mock for the function which not exists, not implemented */
    MOCK_FUNCTION(char*, func1, float)

    /* mock existing function */
    MOCK_OVERRIDE(int, func2, int, char)

    char* func1_my_impl(float x)
    {
        ...
    }

    TEST(suite1, test1)
    {
            ...
            MOCK_SET_RESULT(func1, "42");
            ...
            CHECK(MOCK_COUNT(func2) EQ 2);
            ...
            MOCK_SET_FUNC(func1, func1_my_impl);
            ...
            MOCK_RESET(func1);

    }


## Links:

Project was inspired by:

  1. https://github.com/jasmcaus/tau
  2. https://github.com/ztbrown/Simulacrum
  3. https://github.com/catchorg/Catch2/
  4. https://www.criterion.com/
  5. https://cmocka.org/
  6. https://github.com/Snaipe/Mimick
  7. https://libcheck.github.io/check/
  8. https://github.com/meekrosoft/fff

