import asyncio
import time
from fastapi import APIRouter, Request
from utils import *
from errors import AppError

router = APIRouter()

@router.get("/health")
async def get_health(request: Request):
    return {
        "status": "ok",
        "service": "greeks-read-api",
        "redis": getattr(request.app.state, "redis_heartbeat", None),
        "last_update_ts": now_iso8601(),
    }
    
@router.get("/health/ready")
async def get_redis_health(request: Request):
    try:
        start = time.monotonic()
        await asyncio.wait_for(request.app.state.redis.ping(), timeout=0.5)
        rtt_ms = 1000 * (time.monotonic() - start)
        
        request.app.state.redis_heartbeat = {
            "ok": True,
            "last_ok_ts": now_iso8601(),
            "rtt_ms": rtt_ms,
            "last_error": ""
        }
        
    except asyncio.TimeoutError:
        raise AppError(504, "TIMEOUT", "redis ping timeout", {"service": "redis"})
    except Exception as e:
        raise AppError(503, "SERVICE_UNAVAILABLE", "redis ping failed", {'service': 'redis', 'error': short(e)})
        
    return {
        "redis_ok": True,
        "core_ok": True,
        "rtt_ms": rtt_ms
    }