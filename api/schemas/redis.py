from pydantic import BaseModel

class RedisContract(BaseModel):
    as_of: int
    spot: float
    vol: float
    rate: float
    div_yield: float
    theo_price: float
    delta: float
    gamma: float
    vega: float
    rho: float
    theta: float
    calibration_version: int
    seqno: int
    #TODO: add time_to_expiry_yrs