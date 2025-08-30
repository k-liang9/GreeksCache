from fastapi import APIRouter, Request
from schemas.contracts import *
from schemas.redis import *
from errors import AppError
from utils import *
from services.keyspace import *
from services.cache_reader import find_greeks
from typing import List, Tuple

router = APIRouter()

def parse_contract(contract_list : List[Tuple[str, RedisContract]]) -> Surface:
    contract_vectors = []
    contract = contract_key_to_contract(contract_list[0][0])
    for key, r_contract in contract_list:
        strike = contract_key_to_contract(key).strike
        vector_dict = {
            "strike": strike,
            "time_to_expiry_yrs": 0.0, #TODO
            "vol": r_contract.vol
        }
        contract_vectors.append(ContractVector(**vector_dict))
    surface_dict = {
        "symbol": contract.symbol,
        "expiry": contract.expiry,
        "surface": contract_vectors
    }
    return Surface(**surface_dict)

@router.get("/surface", response_model=Surface)
async def get_surface(request: Request, contract: Contract = Contract()) -> Surface:
    if contract.symbol == "":
        raise AppError(400, "INVALID_ARGUMENT", "symbol field cannot be empty", contract.model_dump())
    key = get_contract_key(contract)
    if not hasattr(request.app.state, "redis"):
        raise AppError(500, "INTERNAL", "could not find redis client")
    contract_list = await find_greeks(request.app.state.redis, key)
    if len(contract_list) == 0:
        raise AppError(400, "INVALID_ARGUMENT", "no contract with specified terms found", contract.model_dump())
    return parse_contract(contract_list)
    