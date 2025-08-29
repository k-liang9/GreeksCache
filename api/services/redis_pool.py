# create one async pooled client on startup; close on shutdown
import redis.asyncio as redis


async def create_redis_pool():
    pool = redis.ConnectionPool(
        host="localhost", port=6379,
        decode_responses=True,
    )
    r = redis.Redis(connection_pool=pool)
    await r.ping()
    return r

async def close_redis_pool(r):
    await r.close()
    await r.wait_closed()