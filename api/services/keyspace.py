from schemas.contracts import Contract
from typing import List
import math

def get_contract_key(contract: Contract) -> str:
    symbol = '*' if contract.symbol == "" else contract.symbol
    expiry = '*' if contract.expiry == "" else contract.expiry
    strike = '*' if math.isclose(contract.strike, 0.0, abs_tol=1e-6) else f"{contract.strike:.4f}"
    payoff_type = '*' if contract.type == "" else contract.type
    return f"greeks:{symbol}:{expiry}:{strike}:{payoff_type}"

def contract_key_to_contract(contract_key: str) -> Contract:
    parts = contract_key.split(':')
    if len(parts) != 5:
        raise ValueError(f"invalid contract_key format: {contract_key}")
    raw = {
        "symbol": "" if parts[1] == '*' else parts[1],
        "expiry": "" if parts[2] == '*' else parts[2],
        "strike": "" if parts[3] == '*' else float(parts[3]),
        "type": "" if parts[4] == '*' else parts[4]
    }
    return Contract(**raw)