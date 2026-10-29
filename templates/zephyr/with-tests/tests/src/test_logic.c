#include <zephyr/ztest.h>
#include "blink_logic.h"

ZTEST(blink_logic, test_period_start) { zassert_equal(blink_period_ms(0), 2000); }
ZTEST(blink_logic, test_period_step) { zassert_equal(blink_period_ms(5), 1500); }
ZTEST(blink_logic, test_period_clamp) { zassert_equal(blink_period_ms(19), 100); zassert_equal(blink_period_ms(500), 50); }
ZTEST_SUITE(blink_logic, NULL, NULL, NULL, NULL, NULL);
