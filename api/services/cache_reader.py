import redis.asyncio as redis
from typing import List
from typing import Tuple
from schemas.redis import *

async def find_greeks(r : redis.Redis, key : str) -> List[Tuple[str, RedisContract]]:
    cursor = 0
    all_greeks = []
    while True:
        cursor, keys = await r.scan(cursor, match=key, count=100)
        if keys:
            pipe = r.pipeline()
            for k in keys:
                pipe.hgetall(k)
            results = await pipe.execute()
            all_greeks.extend([(keys[i], RedisContract(**results[i])) for i in range(len(keys))])
        if cursor == 0:
            break
    return all_greeks

async def find_contracts(r : redis.Redis, key : str) -> List[str]:
    cursor = 0
    all_contracts = []
    while True:
        cursor, keys = await r.scan(cursor, match=key, count=100)
        if keys:
            all_contracts.extend(keys)
        if cursor == 0:
            break
    return all_contracts