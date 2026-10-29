#include "unity.h"
#include "logic.h"

void setUp(void) {}
void tearDown(void) {}

static void test_clamp(void)
{
  TEST_ASSERT_EQUAL_UINT32(5, clamp_u32(1, 5, 9));
  TEST_ASSERT_EQUAL_UINT32(9, clamp_u32(20, 5, 9));
  TEST_ASSERT_EQUAL_UINT32(7, clamp_u32(7, 5, 9));
}
static void test_triangle(void)
{
  TEST_ASSERT_EQUAL_UINT8(0, triangle_u8(0));
  TEST_ASSERT_EQUAL_UINT8(255, triangle_u8(255));
  TEST_ASSERT_EQUAL_UINT8(1, triangle_u8(509));
  TEST_ASSERT_EQUAL_UINT8(0, triangle_u8(510));
}
static void test_sos(void)
{
  TEST_ASSERT_EQUAL_UINT(18, sos_len());
  TEST_ASSERT_TRUE(sos_led_on(0));
  TEST_ASSERT_FALSE(sos_led_on(1));
  TEST_ASSERT_EQUAL_UINT32(150, sos_step_ms(0));
  TEST_ASSERT_EQUAL_UINT32(450, sos_step_ms(6));        /* first long flash */
  TEST_ASSERT_EQUAL_UINT32(1050, sos_step_ms(17));      /* word gap */
  TEST_ASSERT_TRUE(sos_led_on(18));                     /* wraps */
}
int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_clamp);
  RUN_TEST(test_triangle);
  RUN_TEST(test_sos);
  return UNITY_END();
}
