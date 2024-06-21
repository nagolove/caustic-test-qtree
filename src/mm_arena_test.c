// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_rand.h"
#include "koh_mm.h"
#include "uthash.h"
#include "munit.h"
#include "stdbool.h"
#include "stdbool.h"
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool verbose = true;

static MunitResult test_new_add_exist_free(
    const MunitParameter params[], void* data
) {

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
  {
    (char*) "/new_add_exist_free",
    test_new_add_exist_free,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "strset",
 test_suite_tests,
 NULL,
 1,
 MUNIT_SUITE_OPTION_NONE,
 .verbose = &verbose,
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
