#ifndef EUR_GREEKS
#define EUR_GREEKS

#include <vector>
#include <string_view>
#include <array>

using namespace std;

class EurContractBatch {
private:
protected:
    struct Constants {
        vector<double> tau_vec;
        vector<double> d1_vec, d2_vec;
        vector<double> q_disc_vec, r_disc_vec;
        vector<double> nd1_vec, nd2_vec;
        vector<double> Gamma, Vega;
    };
public:
    EurContractBatch() {}
    ~EurContractBatch() {}
    void compute_greeks(
        vector<array<double, 6>>& greeks,
        const vector<double>& S_vec,
        const vector<double>& K_vec,
        const vector<string_view>& T_vec,
        const vector<double>& sigma_vec,
        const vector<double>& q_vec,
        const double r
    );

protected:
    virtual void compute_greeks_impl(
        vector<array<double, 6>>& greeks,
        const vector<double>& S_vec,
        const vector<double>& K_vec,
        const vector<double>& sigma_vec,
        const vector<double>& q_vec,
        const double r,
        const Constants& constants
    ) {}
};

class EurCallContractBatch : public EurContractBatch {
private:
protected:
    void compute_greeks_impl(
        vector<array<double, 6>>& greeks,
        const vector<double>& S_vec,
        const vector<double>& K_vec,
        const vector<double>& sigma_vec,
        const vector<double>& q_vec,
        const double r,
        const Constants& constants
    );
public:
    EurCallContractBatch() {}
    ~EurCallContractBatch() {}
};

class EurPutContractBatch : public EurContractBatch {
private:
protected:
    void compute_greeks_impl(
        vector<array<double, 6>>& greeks,
        const vector<double>& S_vec,
        const vector<double>& K_vec,
        const vector<double>& sigma_vec,
        const vector<double>& q_vec,
        const double r,
        const Constants& constants
    );
public:
    EurPutContractBatch() {}
    ~EurPutContractBatch() {}
};

#endif