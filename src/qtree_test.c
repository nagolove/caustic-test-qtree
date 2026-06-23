// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_qtree.h"
#include "munit.h"
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

// qtree_fill создаёт root и сохраняет value
static MunitResult test_fill_basic(
    const MunitParameter params[], void* data
) {
    QTree qt;
    qtree_init(&qt);
    munit_assert_null(qt.root);
    munit_assert(qt.size == 0);

    int marker = 42;
    qtree_fill(&qt, (Rectangle){ 0, 0, 1, 1 }, &marker);
    munit_assert_not_null(qt.root);
    munit_assert(qt.size > 0);
    munit_assert(qt.root->value == &marker);

    qtree_shutdown(&qt);
    return MUNIT_OK;
}

static int query_calls;

static bool _query_count_cb(
    QTreeNode *node, Vector2 point, float size, void *data
) {
    query_calls++;
    return true;
}

// qtree_query — callback вызывается при попадании в область fill
static MunitResult test_fill_query(
    const MunitParameter params[], void* data
) {
    QTree qt;
    qtree_init(&qt);

    int marker = 42;
    qtree_fill(&qt, (Rectangle){ 0, 0, 10, 10 }, &marker);

    query_calls = 0;
    qtree_query(&qt, (struct QTreeQuery){
        .r = { 0, 0, 10, 10 },
        .min_size = 0,
        .func = _query_count_cb,
        .data = NULL,
    });
    munit_assert(query_calls > 0);

    qtree_shutdown(&qt);
    return MUNIT_OK;
}

// qtree_query — callback не вызывается при запросе вне области
static MunitResult test_query_no_intersect(
    const MunitParameter params[], void* data
) {
    QTree qt;
    qtree_init(&qt);

    int marker = 42;
    qtree_fill(&qt, (Rectangle){ 0, 0, 10, 10 }, &marker);

    query_calls = 0;
    qtree_query(&qt, (struct QTreeQuery){
        .r = { 100, 100, 10, 10 },
        .min_size = 0,
        .func = _query_count_cb,
        .data = NULL,
    });
    munit_assert(query_calls == 0);

    qtree_shutdown(&qt);
    return MUNIT_OK;
}

static void *found_value;

static bool _query_value_cb(
    QTreeNode *node, Vector2 point, float size, void *data
) {
    if (node->value) {
        found_value = node->value;
        return false;
    }
    return true;
}

// qtree_query — callback видит правильный value
static MunitResult test_query_value(
    const MunitParameter params[], void* data
) {
    QTree qt;
    qtree_init(&qt);

    int marker = 99;
    qtree_fill(&qt, (Rectangle){ 0, 0, 100, 100 }, &marker);

    found_value = NULL;
    qtree_query(&qt, (struct QTreeQuery){
        .r = { 30, 30, 1, 1 },
        .min_size = 0,
        .func = _query_value_cb,
        .data = NULL,
    });
    munit_assert(found_value == &marker);

    qtree_shutdown(&qt);
    return MUNIT_OK;
}

// частичный fill + query: прямоугольник меньше области ноды
static MunitResult test_fill_partial(
    const MunitParameter params[], void* data
) {
    QTree qt;
    qtree_init(&qt);

    int marker_a = 1, marker_b = 2;
    // заполняем два непересекающихся прямоугольника разными значениями
    qtree_fill(&qt, (Rectangle){ 0, 0, 5, 5 }, &marker_a);
    qtree_fill(&qt, (Rectangle){ 10, 10, 5, 5 }, &marker_b);

    // запрос в области первого — должен найти marker_a
    found_value = NULL;
    qtree_query(&qt, (struct QTreeQuery){
        .r = { 2, 2, 1, 1 },
        .min_size = 0,
        .func = _query_value_cb,
        .data = NULL,
    });
    munit_assert(found_value == &marker_a);

    // запрос в области второго — должен найти marker_b
    found_value = NULL;
    qtree_query(&qt, (struct QTreeQuery){
        .r = { 12, 12, 1, 1 },
        .min_size = 0,
        .func = _query_value_cb,
        .data = NULL,
    });
    munit_assert(found_value == &marker_b);
    munit_assert(found_value == &marker_b);

    // запрос в пустой области — callback не должен вызваться
    query_calls = 0;
    qtree_query(&qt, (struct QTreeQuery){
        .r = { 50, 50, 1, 1 },
        .min_size = 0,
        .func = _query_count_cb,
        .data = NULL,
    });
    munit_assert(query_calls == 0);

    qtree_shutdown(&qt);
    return MUNIT_OK;
}

// qtree_node_num — подсчёт ненулевых детей
static MunitResult test_node_num(
    const MunitParameter params[], void* data
) {
    QTreeNode node = {};
    munit_assert(qtree_node_num(&node) == 0);

    node.nodes[0] = calloc(1, sizeof(QTreeNode));
    munit_assert(qtree_node_num(&node) == 1);

    node.nodes[1] = calloc(1, sizeof(QTreeNode));
    munit_assert(qtree_node_num(&node) == 2);

    free(node.nodes[0]);
    free(node.nodes[1]);
    return MUNIT_OK;
}

// qtree_node_clear — очистка обнуляет value
static MunitResult test_node_clear(
    const MunitParameter params[], void* data
) {
    QTreeNode *node = calloc(1, sizeof(*node));
    int marker = 42;
    node->value = &marker;
    munit_assert(node->value == &marker);

    qtree_node_clear(node);
    munit_assert_null(node->value);

    free(node);
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

  {
    (char*) "/fill_basic",
    test_fill_basic,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/fill_query",
    test_fill_query,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/query_no_intersect",
    test_query_no_intersect,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/query_value",
    test_query_value,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/fill_partial",
    test_fill_partial,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/node_num",
    test_node_num,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },

  {
    (char*) "/node_clear",
    test_node_clear,
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
