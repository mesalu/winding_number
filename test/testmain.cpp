#include <gtest/gtest.h>

int main(int argc, char **argv, char** env) {
    testing::InitGoogleTest(&argc, argv);
    auto rex = RUN_ALL_TESTS();

    return rex;
}