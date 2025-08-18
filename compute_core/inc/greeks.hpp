#ifndef GREEKS
#define GREEKS
#include <string>
#include <string_view>
#include <ctime>
#include <vector>

void compute_call_greeks(std::vector<double>& greeks, float S, std::string_view T, float K, float sigma, float q, float r);
void compute_put_greeks(std::vector<double>& greeks, float S, std::string_view T, float K, float sigma, float q, float r);
const double N(const double x);
const double n(const double x);
inline double phi(const double x) { return n(x); }
inline double Phi(const double x) { return N(x); }
float time_to_expiry(const std::tm& T);
void parse_time(std::string_view T, std::tm& exp_time);

#endif