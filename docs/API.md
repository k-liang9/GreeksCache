# Real-Time Greeks Cache -- API Contract (v0.2)

Owner: Kevin Liang  
Date: Sep 6, 2025  
Status: Draft  
Audience: Client developers, backend developers, QA  

## 1: Overview  

HTTP API that serves the latest option greeks from individual contracts from a hot in-memory cache (Redis). It also serves Option Greeks of entire portfolios as well as historical positions. UPdates are produced by an ingest+compute worker. Reads must be **low-latency**. Historical data written asynchronously to memory adn is **not** on the hot path. 
- Primary use: fetch current Greeks for a contract, portfolio, or small surface
- SLA (initial): p50 <= 3ms, p99 <= 10ms for reads under nominal load
- Staleness: responses include a `stale` flag when last update age > `STALE_TTL_SEC`    

## 2: Versioning & Base URL
- Base URL (dev): http://localhost:8000/
- Semantic version: v0 and localhost while iterating, promote to v1 when stable
- Change policy: Additive fields are ok

## 3: Authentication
- None  

## 4: Common Types & Conventions
- symbol: uppercase string (e.g. AAPL)
- expiry: date only, ISO 8601 format (e.g. 2025-08-17)
- date: ISO 8601 UTC (e.g. 2025-08-17T11:14:00Z)
- strike: floating point number
- type: "C" for call, "P" for put
- currency: USD 

## 5: Endpoints

### 5.0 Error
Returns a standard error object for failed requests.

Common HTTP status mappings:
- 400 Bad Request — INVALID_ARGUMENT
- 404 Not Found — NOT_FOUND
- 409 Conflict — STALE_DATA
- 500 Internal Server Error — INTERNAL
- 503 Service Unavailable — SERVICE_UNAVAILABLE
- 504 Timeout - TIMEOUT

Example error payload (JSON):
```json
{
  "error": {
    "code": "INVALID_ARGUMENT",
    "message": "strike must be positive",
    "details": {"field": "strike"}
  }
}
```

Meta block
- `meta.trace_id` (string): correlation id for requests (generated if not provided). Use this id when opening support tickets.
- `meta.path` (string): the request path that caused the error.
- `meta.ts` (string): ISO8601 timestamp when the error was produced.

Example payload with `meta`:
```json
{
  "error": {
    "code": "INVALID_ARGUMENT",
    "message": "strike must be positive",
    "details": {"field": "strike"}
  },
  "meta": {
    "trace_id": "f47ac10b-58cc-4372-a567-0e02b2c3d479",
    "path": "/price",
    "ts": "2025-08-29T13:01:03Z"
  }
}
```

---

### 5.1 Health
Checks API and service health status. Returns service name, redis heartbeat status, and last update timestamp.

GET /health

200
```json
{
  "status": "ok",
  "service": "greeks-read-api",
  "redis": {
    "ok": true,
    "last_ok_ts": "2025-08-16T13:45:00Z",
    "rtt_ms": 25,
    "last_error": ""
  },
  "last_update_ts": "2025-08-16T13:45:00Z"
}
```
400: INVALID_ARGUMENT if symbol format invalid
500: SERVICE_UNAVAILABLE if cache or database is unreachable

---

### 5.1.1 Redis Readiness
Checks if Redis and core threads is reachable and responsive. Returns RTT and status.

GET /health/ready

200
```json
{
  "redis_ok": true,
  "core_ok" : true,
  "rtt_ms": 2.1
}
```
503: Redis or core not reachable
504: Redis or core timeout

---

### 5.2 greeks (single contract)
Fetches greeks for a single European option contract.

POST /greeks

Request body (Pydantic model):
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.95,
  "type": "C"
}
```

200 OK  
```json
{
  "contract": {
    "symbol": "AAPL",
    "expiry": "2025-09-19",
    "strike": "150.95",
    "type": "C"
  },
  "snapshot": {
    "as_of": "2025-08-17T13:45:01Z",
    "time_to_expiry_yrs": 0.84,
    "stale": false,
    "inputs": {
      "spot": 196.42,
      "vol": 0.24,
      "rate": 0.03,
      "div_yield": 0.00
    },
    "outputs": {
      "theo_price": 12.3456,
      "delta": 0.53,
      "gamma": 0.0081,
      "vega": 0.145,
      "rho": 0.456,
      "theta": -0.0123
    }
  }
}
```

400: INVALID_ARGUMENT, NOT_FOUND
500: SERVICE_UNAVAILABLE, INTERNAL

---


### 5.3 List Tracked Contracts
Lists all tracked option contracts for a given symbol.

POST /contracts

Request body (Pydantic model):
```json
// optional
{
  "symbol": "AAPL", //optional
  "expiry": "2025-09-19", //optional
  "strike": 150.95, //optional
  "type": "C" //optional
}
```

200 OK
```json
[
  {
    "symbol": "AAPL",
    "expiry": "2025-09-19",
    "strike": 150.0,
    "type": "C"
  },
  {
    "symbol": "AAPL",
    "expiry": "2025-09-19",
    "strike": 155.0,
    "type": "P"
  }
  // ...more contracts...
]
```
400: INVALID_ARGUMENT
500: INTERNAL

---


### 5.4 Option Surface
Returns a 3D vector surface of (strike, time_to_expiry, vol) for a symbol and expiry.

POST /surface

Request body (Pydantic model):
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19" //optional
}
```

200 OK
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "surface": [
    {
      "strike": 150.0,
      "time_to_expiry_yrs": 0.84,
      "vol": 0.24
    },
    {
      "strike": 155.0,
      "time_to_expiry_yrs": 0.84,
      "vol": 0.26
    }
    // ...more points...
  ]
}
```
400: INVALID_ARGUMENT, NOT_FOUND
500: INTERNAL

---

### 5.5 Summary
Fetches all positions and summary greeks.

GET /summary

200 OK  
```json
{
  "positions": [
    {
      "symbol": "AAPL",
      "expiry": "2025-09-19",
      "strike": 150.0,
      "type": "C",
      "units": 10,
      "price": 12.34
    }
    // ...more positions...
  ],
  "summary_greeks": {
    "delta": 1.23,
    "gamma": 0.045,
    "vega": 0.56,
    "rho": 0.453,
    "theta": -0.12
  },
  "stale": false
}
```
400: INVALID_ARGUMENT, NOT_FOUND    
500: INTERNAL

---


### 5.6 Positions
Lists all positions that match a contract.

POST /positions/query

Request body (Pydantic model):
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.0,
  "type": "C"
}
```

200 OK
```json
{
  "positions": [
    {
      "symbol": "AAPL",
      "expiry": "2025-09-19",
      "strike": 150.0,
      "type": "C",
      "units": 10,
      "price": 12.34
    }
    // ...more positions...
  ],
  "stale": false
}
```
400: INVALID_ARGUMENT, NOT_FOUND
500: INTERNAL

---

### 5.7 Add Position
Adds a new position The API currently does not track portfolios; the position will be recorded globally and can later be associated with a position_id when that feature is added.

POST /positions

Request body:
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.0,
  "type": "C",
  "units": 10,
  "price": 12.34 //optional
}
```
200 OK  
```json
{
  "status": "added"
}
```
400: INVALID_ARGUMENT, NOT_FOUND    
500: INTERNAL

---


### 5.8 Adjust Portfolio Positions
Adjusts units for positions that match a contract.

PATCH /positions/adjust

Request body (Pydantic model):
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.0,
  "type": "C",
  "units_delta": -5
}
```

200 OK
```json
{
  "updated": 1,
  "status": "updated"
}
```
400: INVALID_ARGUMENT, NOT_FOUND
500: INTERNAL

---


### 5.9 Close Positions
Closes positions that match a contract across all portfolios.

DELETE /positions/close

Request body (Pydantic model):
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.0,
  "type": "C"
}
```

200 OK
```json
{
  "deleted": 3,
  "status": "deleted"
}
```
400: INVALID_ARGUMENT, NOT_FOUND
500: INTERNAL

---


### 5.10 Revalue Positions
Recomputes and returns aggregated summary greeks for positions that match a contract across all portfolios.

POST /positions/revalue

Request body (Pydantic model):
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.0,
  "type": "C"
}
```

200 OK
```json
{
  "summary_greeks": {
    "delta": 1.23,
    "gamma": 0.045,
    "vega": 0.56,
    "rho": 0.345,
    "theta": -0.12
  },
  "as_of": "2025-08-17T13:45:01Z",
  "stale": false
}
```
400: INVALID_ARGUMENT, NOT_FOUND
500: INTERNAL

## 6: Redis Key Schema

### Current Greeks (Latest Data)
- `greeks:symbol:expiry:strike:type` →  
  `{ as_of, spot, vol, rate, div_yield, theo_price, delta, gamma, vega, rho, theta, calibration_version, seqno }`
  
  Example: `greeks:AAPL:2025-09-19:150.95:C`

### Number of Contracts of each position
- `positions` →
  `{ symbol:expiry:strike:type, symbol:expiry:strike:type, ... }`

### Portfolio Summary
- `summary` →  
  `{ delta, gamma, vega, rho, theta }`
