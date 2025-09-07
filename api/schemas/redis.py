from pydantic import BaseModel
from typing import Dict

class RedisContract(BaseModel):
    as_of: str
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
    time_to_expiry_yrs: float
    
class ContractCount(BaseModel):
    units: Dict