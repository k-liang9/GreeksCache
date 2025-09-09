#include "types.hpp"
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <ctime>
#include <string>
#include <stdexcept>
#include <chrono>
#include <string_view>
#include <cstdio>

using namespace std;

t_ns now() {
    return static_cast<t_ns>(
        chrono::duration_cast<chrono::nanoseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count()
    );
}

std::string ns_to_iso8601_ny(t_ns ns, bool show_time) {
    // Thread safety: use a mutex to protect TZ manipulation
    static std::mutex tz_mutex;
    std::lock_guard<std::mutex> lock(tz_mutex);
    
    std::time_t t = static_cast<std::time_t>(ns / 1000000000ULL);
    int ms = static_cast<int>((ns / 1000000ULL) % 1000ULL);

    // Save original TZ environment variable
    const char* old_tz = getenv("TZ");
    std::string old_tz_val;
    bool had_tz = (old_tz != nullptr);
    if (had_tz) {
        old_tz_val = old_tz;
    }

    // Set to New York timezone
    if (setenv("TZ", "America/New_York", 1) != 0) {
        // Handle error - fallback to original TZ
        if (had_tz) {
            setenv("TZ", old_tz_val.c_str(), 1);
        } else {
            unsetenv("TZ");
        }
        tzset();
        // Could throw exception or return error indicator here
        // For now, continue with whatever timezone we have
    }
    tzset();

    std::tm tm_buf;
    if (localtime_r(&t, &tm_buf) == nullptr) {
        // Handle error in time conversion
        // Restore TZ and return empty or error string
        if (had_tz) {
            setenv("TZ", old_tz_val.c_str(), 1);
        } else {
            unsetenv("TZ");
        }
        tzset();
        return ""; // or throw exception
    }

    // Restore original TZ environment variable
    if (had_tz) {
        setenv("TZ", old_tz_val.c_str(), 1);
    } else {
        unsetenv("TZ");
    }
    tzset();

    // Format the output
    if (show_time) {
        char time_buf[32];
        if (std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%S", &tm_buf) == 0) {
            return ""; // strftime failed
        }
        
        char result[48];
        if (snprintf(result, sizeof(result), "%s.%03d", time_buf, ms) < 0) {
            return ""; // snprintf failed
        }
        return std::string(result);
    } else {
        char date_buf[16];
        if (std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &tm_buf) == 0) {
            return ""; // strftime failed
        }
        return std::string(date_buf);
    }
}

t_ns parse_time(std::string_view T) {
    // Thread safety: use a mutex to protect TZ manipulation
    static std::mutex tz_mutex;
    std::lock_guard<std::mutex> lock(tz_mutex);
    
    if (T.length() < 10) {
        throw std::invalid_argument("Invalid date format - too short");
    }
    
    tm exp_time = {};
    
    auto to_int = [&](size_t pos, size_t len) -> int {
        if (pos + len > T.length()) {
            throw std::invalid_argument("Invalid date format - insufficient length");
        }
        int v = 0;
        for (size_t i = 0; i < len; ++i) {
            char c = T[pos + i];
            if (c < '0' || c > '9') {
                throw std::invalid_argument("Invalid date format - non-digit character");
            }
            v = v * 10 + (c - '0');
        }
        return v;
    };

    // Check basic format requirements
    if (T[4] != '-' || T[7] != '-') {
        throw std::invalid_argument("Invalid date format - missing dashes");
    }

    // Parse date part
    int year = to_int(0, 4);
    int month = to_int(5, 2);
    int day = to_int(8, 2);
    
    // Validate date values
    if (year < 1900 || year > 3000) {
        throw std::invalid_argument("Invalid year");
    }
    if (month < 1 || month > 12) {
        throw std::invalid_argument("Invalid month");
    }
    if (day < 1 || day > 31) {
        throw std::invalid_argument("Invalid day");
    }

    // Default time to 4 PM (market close)
    int hour = 16, min = 0, sec = 0;
    int nanoseconds = 0;
    
    // Check if time component is present
    if (T.length() >= 19 && T[10] == 'T') {
        if (T[13] != ':' || T[16] != ':') {
            throw std::invalid_argument("Invalid time format - missing colons");
        }
        
        hour = to_int(11, 2);
        min = to_int(14, 2);
        sec = to_int(17, 2);
        
        // Validate time values
        if (hour < 0 || hour > 23) {
            throw std::invalid_argument("Invalid hour");
        }
        if (min < 0 || min > 59) {
            throw std::invalid_argument("Invalid minute");
        }
        if (sec < 0 || sec > 60) { // 60 for leap seconds
            throw std::invalid_argument("Invalid second");
        }
        
        // Look for fractional seconds
        if (T.length() > 19 && T[19] == '.') {
            size_t frac_start = 20;
            size_t frac_end = frac_start;
            
            // Find end of fractional part
            while (frac_end < T.length() && T[frac_end] >= '0' && T[frac_end] <= '9') {
                frac_end++;
            }
            
            if (frac_end > frac_start) {
                std::string frac_str(T.substr(frac_start, frac_end - frac_start));
                
                // Convert to nanoseconds (pad or truncate to 9 digits)
                if (frac_str.length() > 9) {
                    frac_str = frac_str.substr(0, 9);
                } else {
                    while (frac_str.length() < 9) {
                        frac_str += '0';
                    }
                }
                nanoseconds = std::stoi(frac_str);
            }
        }
    }

    // Set up tm structure
    exp_time.tm_year = year - 1900;
    exp_time.tm_mon = month - 1; // 0-11
    exp_time.tm_mday = day;
    exp_time.tm_hour = hour;
    exp_time.tm_min = min;
    exp_time.tm_sec = sec;
    exp_time.tm_isdst = -1; // Let mktime determine DST

    // Save original TZ
    const char* old_tz = getenv("TZ");
    std::string old_tz_val;
    bool had_tz = (old_tz != nullptr);
    if (had_tz) {
        old_tz_val = old_tz;
    }

    // Set to New York timezone since we're parsing NY time
    if (setenv("TZ", "America/New_York", 1) != 0) {
        throw std::runtime_error("Failed to set timezone");
    }
    tzset();

    time_t tt = mktime(&exp_time);

    // Restore original TZ
    if (had_tz) {
        setenv("TZ", old_tz_val.c_str(), 1);
    } else {
        unsetenv("TZ");
    }
    tzset();

    if (tt == -1) {
        throw std::invalid_argument("Invalid date/time - mktime failed");
    }

    t_ns result = static_cast<t_ns>(tt) * 1000000000ULL + nanoseconds;
    return result;
}

double ns_to_yrs(t_ns tau) {
    return static_cast<double>(tau) / (365.25 * 24 * 60 * 60 * 1e9);
}

double ns_to_ms(t_ns tau) {
    return static_cast<double>(tau) / 1e6;
}

double ns_to_s(t_ns tau) {
    return static_cast<double>(tau) / 1e9;
}

t_ns ms_to_ns(t_ms ms) {
    return static_cast<t_ns>(ms * 1e6);
}

t_ns s_to_ns(t_s s) {
    return static_cast<t_ns>(s * 1e9);
}

t_ns yrs_to_ns(double yrs) {
    return static_cast<t_ns>(yrs * 365.25 * 24 * 60 * 60 * 1e9);
}

const double N(const double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

const double n(const double x) {
    return 1/sqrt(2 * M_PI) * exp(- pow(x, 2) / 2);
}

const string payoff_type_to_string(PayoffType type) {
    switch (type) {
        case VAN_CALL: return "VAN_CALL";
        case VAN_PUT:  return "VAN_PUT";
        default:       return "UNKNOWN";
    }
}

const PayoffType string_to_payoff_type(string_view type) {
    if (type == "VAN_CALL") {
        return VAN_CALL;
    } else if (type == "VAN_PUT") {
        return VAN_PUT;
    } else {
        return PAYOFFTYPE_ERROR;
    }
}

int get_payoff_group_id(PayoffType payoff) {
    switch(payoff) {
        case VAN_CALL:
        case VAN_PUT:
            return 0; // Group 0: vanilla options
        default:
            return -1; // No group
    }
}

bool are_payoffs_in_same_group(PayoffType a, PayoffType b) {
    return (a == VAN_CALL || a == VAN_PUT) && (b == VAN_CALL || b == VAN_PUT);
}