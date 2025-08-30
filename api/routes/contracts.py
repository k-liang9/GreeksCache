from fastapi import APIRouter, Request
from schemas.contracts import *
from schemas.redis import RedisContract
from errors import AppError
from utils import *
from typing import List
from services.keyspace import *
from services.cache_reader import *

router = APIRouter()

@router.get("/contracts", response_model=List[Contract])
async def get_contracts(request: Request, contract: Contract = Contract()) -> List[Contract]:
    key = get_contract_key(contract)
    if not hasattr(request.app.state, "redis"):
        raise AppError(500, "INTERNAL", "could not find redis client")
    contract_keys = await find_contracts(request.app.state.redis, key)
    if len(contract_keys) == 0:
        raise AppError(400, "INVALID_ARGUMENT", "no contract with specified terms found", contract.model_dump())
    return [contract_key_to_contract(key) for key in contract_keys]