from pydantic import BaseModel
from .contracts import Contract

class Greeks(BaseModel):
    theo_price: float
    delta: float
    gamma: float
    vega: float
    rho: float
    theta: float

class Market(BaseModel):
    spot: float
    vol: float
    rate: float
    div_yield: float
    
class ContractSnapshot(BaseModel):
    as_of: str
    time_to_expiry_yrs: float
    stale: bool
    market: Market
    greeks: Greeks
    
class SingleContractInfo(BaseModel):
    contract: Contract
    snapshot: ContractSnapshot
    