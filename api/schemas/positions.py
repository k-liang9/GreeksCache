from pydantic import BaseModel
from .contracts import Contract
from typing import List

class Position(BaseModel):
    contract: Contract
    units: int
    price: float

class Positions(BaseModel):
    positions: List[Position]
    stale: bool