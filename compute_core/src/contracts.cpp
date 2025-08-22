#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include "contracts.hpp"
#include "utils.hpp"

using namespace std;

ContractsBatch::ContractsBatch(string symbol) : symbol_{symbol} {}

void ContractsBatch::add_contract(Contract& contract) {
    strikes_[contract.type].push_back(contract.strike);
    tm expiry;
    parse_time(contract.expiry, expiry);
    expiries_[contract.type].push_back(expiry);
}

void ContractsBook::add_contract(Contract& contract) {
    if (contracts_book_.contains(contract.symbol)) {
        contracts_book_[contract.symbol].add_contract(contract);
    } else {
        contracts_book_[contract.symbol] = ContractsBatch(contract.symbol);
        contracts_book_[contract.symbol].add_contract(contract);
    }
}