from pydantic import BaseModel
from schemas.contracts import Contract
from typing import List
    
class Order(BaseModel):
    contract: Contract
    units_delta: int

class OrdersBatch(BaseModel):
    orders: List[Order]
    
class ActionStatus(BaseModel):
    message: str
    positions_changed: int
    failed_orders: List[Order]