#include <cassert>
#include <ctql.hpp>
#include <tests/test.hpp>

using namespace ctql;

int main() {
    Test::initialize();
    
    Test::test("basic match", []() {
        Test::assert_that($match(12,
            []<is<int> T>(T i) { return i + 1; },
            []<is<double> T>(T d) { return d + 2.0; },
            []<is<std::string> T>(T& s) { return s + "test"; }
        ) == 13);
    });

    return 0;
}