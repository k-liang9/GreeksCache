from utils import now_iso8601
from fastapi import Request
from fastapi.responses import JSONResponse
from fastapi import status
import uuid
from logger import logger

class AppError(Exception):
    def __init__(self, http_status: int, code: str, message: str, details: dict | None = None):
        self.http_status = http_status
        self.code = code
        self.message = message
        self.details = details or {}
        
def format_error_payload(exc: AppError | Exception, request: Request, trace_id: str | None = None):
    if trace_id is None:
        trace_id = str(uuid.uuid4())
    if isinstance(exc, AppError):
        code = exc.code
        message = exc.message
        details = exc.details
        status_code = exc.http_status
    else:
        code = "INTERNAL"
        message = "Internal server error"
        details = {"error": str(exc)}
        status_code = status.HTTP_500_INTERNAL_SERVER_ERROR
        
    payload = {
        "error": {
            "code": code,
            "message": message,
            "details": details
        },
        "meta": {
            "trace_id": trace_id,
            "path": request.url.path,
            "ts": now_iso8601()
        }
    }
    return status_code, payload

async def error_handler(request: Request, exc: AppError | Exception):
    status_code, payload = format_error_payload(exc, request)
    logger.error(payload["error"]["message"])
    return JSONResponse(status_code=status_code, content=payload)

def register_exception_handlers(app):
    app.add_exception_handler(AppError, error_handler)
    app.add_exception_handler(Exception, error_handler)