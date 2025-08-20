#ifndef TESTING_VARS
#define TESTING_VARS

#include <string>
#include <vector>
#include <string_view>

using namespace std;

namespace test {
    extern vector<double> vol;
    extern vector<string_view> T;
    extern vector<double> K;
    extern vector<double> spot;
    extern double risk_free_rate;
    extern vector<double> dividend;
}

#endif