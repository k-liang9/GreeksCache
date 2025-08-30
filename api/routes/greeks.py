from fastapi import APIRouter, Request
from schemas.greeks import *
from schemas.contracts import Contract
from schemas.redis import *
from utils import *
from errors import AppError
import math
from services.keyspace import *
from services.cache_reader import find_greeks
from typing import List, Tuple

router = APIRouter()

def parse_contract(contract_list: List[Tuple[str, RedisContract]]) -> SingleContractInfo:
    key, r_contract = contract_list[0]
    contract = contract_key_to_contract(key)
    market_dict = {
        "spot": r_contract.spot,
        "vol": r_contract.vol,
        "rate": r_contract.rate,
        "div_yield": r_contract.div_yield
    }
    greeks_dict = {
        "theo_price": r_contract.theo_price,
        "delta": r_contract.delta,
        "gamma": r_contract.gamma,
        "vega": r_contract.vega,
        "rho": r_contract.rho,
        "theta": r_contract.theta
    }
    market = Market(**market_dict)
    greeks = Greeks(**greeks_dict)
    contract_snapshot_dict = {
        "as_of": r_contract.as_of,
        "stale": False,
        "market": market,
        "greeks": greeks
    }
    contract_snapshot = ContractSnapshot(**contract_snapshot_dict)
    contract_info_dict = {
        "contract": contract,
        "snapshot": contract_snapshot
    }
    return SingleContractInfo(**contract_info_dict)

@router.get("/greeks", response_model=SingleContractInfo)
async def get_contracts(request: Request, contract: Contract = Contract()) -> SingleContractInfo:
    if (
        contract.symbol == "" or
        contract.expiry == "" or
        math.isclose(contract.strike, 0.0, abs_tol=1e-6) or
        contract.type == ""
    ):
        raise AppError(400, "INVALID_ARGUMENT", "contract fields incomplete", contract.model_dump())
    key = get_contract_key(contract)
    if not hasattr(request.app.state, "redis"):
        raise AppError(500, "INTERNAL", "could not find redis client")
    contract_list = await find_greeks(request.app.state.redis, key)
    if len(contract_list) == 0:
        raise AppError(400, "INVALID_ARGUMENT", "no contract with specified terms found", contract.model_dump())
    return parse_contract(contract_list)