# create one async pooled client on startup; close on shutdown
import redis.asyncio as redis
from logger import logger

async def create_redis_pool():
    pool = redis.ConnectionPool(
        host="localhost", port=6379,
        decode_responses=True,
    )
    r = redis.Redis(connection_pool=pool)
    await r.ping()
    return r

async def close_redis_pool(r):
    try:
        await r.close()
    except Exception:
        pass

    try:
        await r.wait_closed()
    except Exception:
        pass
    logger.info("shut down redis")