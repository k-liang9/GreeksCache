import time
from datetime import datetime, timezone

def now_iso8601(ts=None):
    if ts is None:
        ts = time.time()
    dt = datetime.fromtimestamp(ts, tz=timezone.utc)
    return dt.strftime('%Y-%m-%dT%H:%M:%SZ')


def short(e : Exception):
    """
    Return a short, human-readable summary of an exception or error object.
    """
    return str(e).splitlines()[0][:80]