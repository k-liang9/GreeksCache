#ifndef UTILS
#define UTILS
#include <string>
#include <string_view>
#include <ctime>
#include <vector>
#include <chrono>
#include "types.hpp"

using namespace std;

const double N(const double x);
const double n(const double x);
inline double phi(const double x) { return n(x); }
inline double Phi(const double x) { return N(x); }
t_ns parse_time(std::string_view T);
t_ns now();

#endif