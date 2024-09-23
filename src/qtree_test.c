// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_qtree.h"
#include "munit.h"
#include "stdbool.h"
#include "stdbool.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
Загрузить файл Quadtree.lua в Луа систему. Тестирование производить на
сравнении с оригиналом. То ест вызывать методы оригинального кода.

Будет задана некоторая последовальность точек и будет известен результат
который я получаю от оригинального дерева.

Вообще весь оригинальный результат я могу получить последовательность 
записывая последовательность внутренний состояний из main.lua
*/

static bool verbose = true;

static MunitResult test_init_shutdown(
    const MunitParameter params[], void* data
) {
    QTree q;
    qtree_init(&q);
    qtree_shutdown(&q);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
  {
    (char*) "/init_shutdown",
    test_init_shutdown,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char*) "quad tree",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE,
    .verbose = &verbose,
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
