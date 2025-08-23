#ifndef UTILS
#define UTILS
#include <string>
#include <string_view>
#include <ctime>
#include <vector>

using namespace std;

const double N(const double x);
const double n(const double x);
inline double phi(const double x) { return n(x); }
inline double Phi(const double x) { return N(x); }
void parse_time(std::string_view T, std::tm& exp_time);
void time_to_expiry(const vector<tm>& Ts, const tm& now, vector<double>& tau);
void get_cur_time(tm& time);


#endif