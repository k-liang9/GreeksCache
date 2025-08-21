#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include "contracts.hpp"

using namespace std;

ContractsBatch::ContractsBatch(string& symbol) : symbol_{symbol} {}

void ContractsBatch::add_contract(Contract& contract) {
    strikes_[contract.type].emplace_back(move(contract.strike));
    expiries_[contract.type].emplace_back(move(contract.expiry));
}

void ContractsBook::add_contract(Contract& contract) {
    if (contracts_book_.contains(contract.symbol)) {
        contracts_book_[contract.symbol].add_contract(contract);
    } else {
        contracts_book_[contract.symbol] = ContractsBatch(contract.symbol);
        contracts_book_[contract.symbol].add_contract(contract);
    }
}