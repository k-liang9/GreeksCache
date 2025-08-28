#include "types.hpp"
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
double ns_to_yrs(t_ns tau);
double ns_to_ms(t_ns tau);
double ns_to_s(t_ns tau);
t_ns ms_to_ns(t_ms ms);
t_ns s_to_ns(t_s s);
t_ns yrs_to_ns(double yrs);
string ns_to_date(t_ns ns);
const string payoff_type_to_string(PayoffType type);
std::string ns_to_iso8601(t_ns ns);

#endif