from fastapi import APIRouter, Request
from schemas.positions import *
from schemas.redis import RedisContract
from schemas.orders import *
from utils import *
from typing import List, Dict
from errors import AppError
from services.keyspace import *
from services.cache_reader import *

router = APIRouter()

@router.get("/positions", response_model=Positions)
async def get_positions(request: Request, contract: Contract = Contract()) -> Positions:
    key = get_key(contract)
    contract_key = "greeks:" + key
    if not hasattr(request.app.state, "redis"):
        raise AppError(500, "INTERNAL", "could not find redis client")
    contract_list = await find_greeks(request.app.state.redis, contract_key)
    if len(contract_list) == 0:
        raise AppError(400, "INVALID_ARGUMENT", "no contract with specified terms found", contract.model_dump())
    units_dict = await get_position_sizes(request.app.state.redis, key)
    if len(contract_list) != len(units_dict):
        raise AppError(500, 'INTERNAL', "count of found contracts does not equal count of found contract quantities")
    return format_positions(contract_list, units_dict)


@router.post("/positions", response_model=ActionStatus)
async def update_positions(
    request: Request, 
    orders_batch: OrdersBatch,
) -> ActionStatus:
    if not hasattr(request.app.state, "redis"):
        raise AppError(500, "INTERNAL", "could not find redis client")
    
    raw = await update_positions_impl(request.app.state.redis, orders_batch.orders)
    
    return ActionStatus(**raw)
        
        
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

async def update_positions_impl(r : redis.Redis, orders : List[Order]) -> Dict:
    changed = 0
    failed_orders = []
    for order in orders:
        if (
            order.contract.symbol == "" or
            order.contract.expiry == "" or
            math.isclose(order.contract.strike, 0.0, abs_tol=1e-6) or
            order.contract.type == "" or
            order.units_delta == 0
        ):
            failed_orders.append(order)
        
        is_new = False
        key = get_key(order.contract)
        pos_size = await get_position_size(r, key)
        if pos_size is None:
            if order.units_delta < 0:
                failed_orders.append(order)
                continue
            else:
                is_new = True
                #TODO: flush contract updates into the cpp core
        elif pos_size + order.units_delta < 0:
            failed_orders.append(order)
            continue
                
        if await try_update_position(r, key, is_new, order.units_delta):
            changed += 1
        else:
            failed_orders.append(order)
    
    if changed == len(orders):
        message = "success"
    else:
        message = "some or all positions updates were unable to be processed"
    
    return {
        "positions_changed": changed,
        "message": message,
        "failed_orders": failed_orders
    }