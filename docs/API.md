# Real-Time Greeks Cache -- API Contract (v0.2)

Owner: Kevin Liang  
Date: Aug 23, 2025  
Status: Draft  
Audience: Client developers, backend developers, QA  

## 1: Overview  

HTTP API that serves the latest option greeks from individual contracts from a hot in-memory cache (Redis). It also serves Option Greeks of entire portfolios as well as historical positions. UPdates are produced by an ingest+compute worker. Reads must be **low-latency**. Historical data written asynchronously to memory adn is **not** on the hot path. 
- Primary use: fetch current Greeks for a contract, portfolio, or small surface
- SLA (initial): p50 <= 3ms, p99 <= 10ms for reads under nominal load
- Staleness: responses include a `stale` flag when last update age > `STALE_TTL_SEC`    

## 2: Versioning & Base URL
- Base URL (dev): http://localhost:6969/api/v0
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

Codes: INVALID_ARGUMENT, NOT_FOUND, STALE_DATA, INTERNAL, SERVICE_UNAVAILABLE
```json
{
    "error": {
        "code": "INVALID_ARGUMENT",
        "message": "strike must be positive",
        "details": {"field": "strike"}
    }
}
```

---

### 5.1 Health
Checks API and cache health status.

GET /health

200 OK
```json
{
    "status": "ok",
    "redis": "ok",
    "last_update_ts": "2025-08-16T13:45:00Z"
}
```
4XX
- INVALID_ARGUMENT if symbol format invalid
5xx
- SERVICE_UNAVAILABLE if cache or database is unreachable

---

### 5.2 greeks (single contract)
Fetches greeks for a single European option contract.

GET /price?symbol=SYMBOL%expiry=DATE&strike=K&type=C|P

200 OK  
```json
{
    "symbol": "AAPL",
    "expiry": "2025-09-19",
    "strike": "150.95",
    "type": "C",
    "snapshot": {
        "as_of": "2025-08-17T13:45:01Z",
        "stale": false,
        "inputs": {
            "spot": 196.42,
            "vol": 0.24,
            "rate": 0.03,
            "div_yield": 0.00,
            "time_to_expiry_yrs": 0.84,
            "source": "sim"
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

4xx: INVALID_ARGUMENT, NOT_FOUND    
5xx: SERVICE_UNAVAILABLE, INTERNAL

---

### 5.3 List Tracked Contracts
Lists all tracked option contracts for a given symbol.

GET /contracts?symbol=SYMBOL

200 OK  
```json
{
  "contracts": [
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
}
```
4xx: INVALID_ARGUMENT  
5xx: INTERNAL

---

### 5.4 Option Surface
Returns a 3D vector surface of (strike, time_to_expiry, vol) for a symbol and expiry.

GET /surface?symbol=SYMBOL&expiry=DATE

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
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

---

### 5.5 Portfolio Summary
Fetches all positions and summary greeks for a portfolio.

GET /portfolios/{portfolio_id}

200 OK  
```json
{
  "portfolio_id": "abc123",
  "positions": [
    {
      "position_id": "pos1",
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
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

---

### 5.6 List Portfolio Positions
Lists all positions in a portfolio.

GET /portfolios/{portfolio_id}/positions

200 OK  
```json
{
  "positions": [
    {
      "position_id": "pos1",
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
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

---

### 5.7 Add Portfolio Position
Adds a new position to a portfolio.

POST /portfolios/{portfolio_id}/positions

Request body:
```json
{
  "symbol": "AAPL",
  "expiry": "2025-09-19",
  "strike": 150.0,
  "type": "C",
  "units": 10,
  "price": 12.34 // optional
}
```
200 OK  
```json
{
  "position_id": "pos1",
  "status": "added"
}
```
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

---

### 5.8 Adjust Portfolio Position
Adjusts the units of an existing portfolio position.

PATCH /portfolios/{portfolio_id}/positions/{position_id}

Request body:
```json
{
  "units_delta": -5
}
```
200 OK  
```json
{
  "position_id": "pos1",
  "units": 5,
  "status": "updated"
}
```
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

---

### 5.9 Delete Portfolio
Deletes a portfolio and all its positions.

DELETE /portfolios/{portfolio_id}

200 OK  
```json
{
  "portfolio_id": "abc123",
  "status": "deleted"
}
```
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

---

### 5.10 Revalue Portfolio
Recomputes and returns summary greeks for a portfolio.

POST /portfolios/{portfolio_id}/revalue

200 OK  
```json
{
  "portfolio_id": "abc123",
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
4xx: INVALID_ARGUMENT, NOT_FOUND  
5xx: INTERNAL

## 6: Redis Key Schema

### Current Greeks (Latest Data)
- `greeks:symbol:expiry:strike:type` →  
  `{ as_of, spot, vol, rate, div_yield, theo_price, delta, gamma, vega, rho, theta, calibration_version, seqno }`
  
  Example: `greeks:AAPL:2025-09-19:150.95:C`

### Historical Greeks (Optional - for analytics)
- `greeks_hist:symbol:expiry:strike:type:timestamp` →  
  Same structure as current Greeks, for historical analysis

### Portfolio Data
- `portfolio:id:positions` →  
  Set of `position_id`s

- `portfolio:(id):position:(position_id)` →  
  `{ symbol, expiry, strike, type, units, avg_entry_price, open_ts, last_update_ts }`