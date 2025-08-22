#ifndef CONTRACTS
#define CONTRACTS

#include <vector>
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>

using namespace std;

enum OptionType {
    EUR_CALL,
    EUR_PUT,
    NUM_OPTION_TYPES
};

struct Contract {
    string symbol;
    string expiry;
    float strike;
    OptionType type;
};

class ContractsBatch {
private:
    array<vector<double>, NUM_OPTION_TYPES> strikes_;
    array<vector<tm>, NUM_OPTION_TYPES> expiries_;
    string symbol_;
public:
    array<vector<double>, NUM_OPTION_TYPES> prices_;
    array<vector<double>, NUM_OPTION_TYPES> deltas_;
    array<vector<double>, NUM_OPTION_TYPES> gammas_;
    array<vector<double>, NUM_OPTION_TYPES> vegas_;
    array<vector<double>, NUM_OPTION_TYPES> rhos_;
    array<vector<double>, NUM_OPTION_TYPES> thetas_;
    const array<vector<double>, NUM_OPTION_TYPES>& strikes() {return strikes_;}
    const array<vector<tm>, NUM_OPTION_TYPES>& expiries() {return expiries_;}
    string_view symbol() {return symbol_;}
    void set_symbol(string symbol) {symbol_ = symbol;}

    ContractsBatch() : symbol_("") {}
    ContractsBatch(string symbol);
    ~ContractsBatch() {}

    void add_contract(Contract& contract);
};

class ContractsBook {
private:
    unordered_map<string, ContractsBatch> contracts_book_;

public:
    void add_contract(Contract& contract);
    unordered_map<string, ContractsBatch>& contracts_book() {return contracts_book_;}

    ContractsBook() {}
    ~ContractsBook() {}
};

#endif