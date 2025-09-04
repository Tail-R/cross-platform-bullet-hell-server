#include <gtest/gtest.h>

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

TEST(MathTest, Add) {
    EXPECT_EQ(add(2, 3), 5);
}

TEST(MathTest, Subtract) {
    EXPECT_EQ(sub(5, 3), 2);
}
