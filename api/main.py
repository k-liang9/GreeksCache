from typing import Union
from fastapi import FastAPI
from contextlib import asynccontextmanager
import time
import asyncio
from utils import *
from services.redis_pool import create_redis_pool, close_redis_pool
from errors import register_exception_handlers
from logger import logger

from routes.health import router as health_router
from routes.contracts import router as contracts_router
from routes.greeks import router as greeks_router
from routes.surface import router as surface_router
from routes.positions import router as positions_router

async def redis_heartbeat(app, redis, interval=2):
    logger.info("created redis heartbeat task")
    while True:
        try:
            await asyncio.wait_for(redis.ping(), timeout=0.5)
            start = time.monotonic()
            rtt_ms = 1000 * (time.monotonic() - start)
            app.state.redis_heartbeat = {
                "ok": True,
                "last_ok_ts": now_iso8601(),
                "rtt_ms": rtt_ms,
                "last_error": ""
            }
        except TimeoutError:
            app.state.redis_heartbeat = {
                "ok": False,
                "last_ok_ts": getattr(app.state, "redis_heartbeat", {}).get("last_ok_ts", None),
                "rtt_ms": "",
                "last_error": "redis_timeout",
            }
        except Exception as e:
            app.state.redis_heartbeat = {
                "ok": False,
                "last_ok_ts": getattr(app.state, "redis_heartbeat", {}).get("last_ok_ts", None),
                "rtt_ms": "",
                "last_error": short(e),
            }
        await asyncio.sleep(interval)


@asynccontextmanager
async def lifespan(app: FastAPI):
    app.state.redis = await create_redis_pool()
    logger.info("created redis pool")
    app.state.heartbeat_task = asyncio.create_task(redis_heartbeat(app, app.state.redis))
    # - db connection pool
    # - initialize metrics/observability
    # - set service metadata
    # - optional warmup (preload config)
    yield
    app.state.heartbeat_task.cancel()
    try:
        await app.state.heartbeat_task
    except asyncio.CancelledError:
        pass
    await close_redis_pool(app.state.redis)
    # - close db pool
    # - flush metrics/logs (last summary line)
    # - cancel background tasks

app = FastAPI(lifespan=lifespan)
app.include_router(health_router)
app.include_router(contracts_router)
app.include_router(greeks_router)
app.include_router(surface_router)
app.include_router(positions_router)
register_exception_handlers(app)

@app.get("/")
async def read_root():
    return {"Hello": "World"}

# day 8:
#     - create redis pool, ping redis, log version
#     - close redis pool