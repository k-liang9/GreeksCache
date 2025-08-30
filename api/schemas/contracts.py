from pydantic import BaseModel
from typing import List

class Contract(BaseModel):
    symbol: str = ""
    expiry: str = ""
    strike: float = 0.0
    type: str = ""
    
class ContractVector(BaseModel):
    strike: float
    time_to_expiry_yrs: float
    vol: float
    
class Surface(BaseModel):
    symbol: str
    expiry: str = ""
    surface: List[ContractVector]