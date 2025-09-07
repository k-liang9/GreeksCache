from fastapi import APIRouter, Request
from schemas.positions import *
from schemas.redis import RedisContract
from utils import *
from errors import AppError
from services.keyspace import *
from services.cache_reader import *

router = APIRouter()

def format_positions(contract_list, units_dict) -> Positions:
    positions = []
    for key, redis_contract in contract_list:
        contract = contract_key_to_contract(key)
        position_key = contract_key_to_positions_key(key)
        raw = {
            "contract": contract,
            "units": units_dict[position_key],
            "price": redis_contract.theo_price,
        }
        positions.append(Position(**raw))
    positions_raw = {
        "positions": positions,
        "stale": False
    }
    return Positions(**positions_raw)

@router.get("/positions", response_model=Positions)
async def get_positions(request: Request, contract: Contract = Contract()) -> Positions:
    key = get_key(contract)
    contract_key = "greeks:" + key
    if not hasattr(request.app.state, "redis"):
        raise AppError(500, "INTERNAL", "could not find redis client")
    contract_list = await find_greeks(request.app.state.redis, contract_key)
    if len(contract_list) == 0:
        raise AppError(400, "INVALID_ARGUMENT", "no contract with specified terms found", contract.model_dump())
    units_dict = await find_units_count(request.app.state.redis, key)
    if len(contract_list) != len(units_dict):
        raise AppError(500, 'INTERNAL', "count of found contracts does not equal count of found contract quantities")
    return format_positions(contract_list, units_dict)