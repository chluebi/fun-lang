#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>

// ANSI escape codes for colors
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

namespace SimpleTestFramework {
    class TestRunner {
    private:
        std::vector<std::function<void()>> tests;
        int tests_run = 0;
        int tests_failed = 0;

    public:
        void addTest(std::function<void()> test_func) {
            tests.push_back(test_func);
        }

        void runAll() {
            for (const auto& test : tests) {
                tests_run++;
                test();
            }
            std::cout << "\n--- Test Summary ---" << std::endl;
            std::cout << "Tests run: " << tests_run << std::endl;
            if (tests_failed > 0) {
                std::cout << RED << "Tests failed: " << tests_failed << RESET << std::endl;
            } else {
                std::cout << GREEN << "Tests failed: " << tests_failed << RESET << std::endl;
                std::cout << GREEN << "All tests passed! 🎉" << RESET << std::endl;
            }
        }

        void failTest() {
            tests_failed++;
        }
    };

    static TestRunner globalTestRunner;

    #define TEST_CASE(name) \
        void test_##name(); \
        struct Test_##name { \
            Test_##name() { \
                SimpleTestFramework::globalTestRunner.addTest([](){ \
                    std::cout << "\nRunning test case: " << #name << std::endl; \
                    test_##name(); \
                }); \
            } \
        }; \
        static Test_##name test_instance_##name; \
        void test_##name()

    #define ASSERT_EQ(expected, actual) \
        if (expected != actual) { \
            std::cout << RED << "Assertion Failed: " << #expected << " (" << expected << ") != " << #actual << " (" << actual << ")" << RESET << std::endl; \
            SimpleTestFramework::globalTestRunner.failTest(); \
        } else { \
            std::cout << GREEN << "Assertion Passed: " << #expected << " == " << #actual << RESET << std::endl; \
        }

    #define RUN_ALL_TESTS() \
        SimpleTestFramework::globalTestRunner.runAll();
} // namespace SimpleTestFramework