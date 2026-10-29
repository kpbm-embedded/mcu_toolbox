#include "unity.h"
#include "calc.h"

void setUp(void) {}
void tearDown(void) {}

static void test_clamp(void)
{
    TEST_ASSERT_EQUAL_INT(0, calc_clamp(-5, 0, 10));
    TEST_ASSERT_EQUAL_INT(10, calc_clamp(50, 0, 10));
    TEST_ASSERT_EQUAL_INT(7, calc_clamp(7, 0, 10));
}

static void test_sum(void)
{
    int a[] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_INT(10, calc_sum(a, 4));
    TEST_ASSERT_EQUAL_INT(0, calc_sum(a, 0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_clamp);
    RUN_TEST(test_sum);
    return UNITY_END();
}
