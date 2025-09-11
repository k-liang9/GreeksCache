"""
gRPC client service for communicating with the core C++ engine.

This service handles all gRPC communication with the GreeksCache core.
"""

import sys
import os
from typing import List, Dict, Any, Callable, Iterator

sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from errors import AppError
from schemas.orders import Order

# Add generated protobuf directory to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'generated', 'python'))

import grpc
import core_pb2
import core_pb2_grpc
from google.protobuf import empty_pb2


class CoreGrpcService:
    """Service for communicating with the GreeksCache core via gRPC."""
    
    def __init__(self, host: str = "localhost", port: int = 8080):
        self.address = f"{host}:{port}"
        self._channel = grpc.insecure_channel(self.address)
        self._stub = core_pb2_grpc.PortfolioUpdatesStub(self._channel)   
    
    def disconnect(self):
        """Close connection to the core service."""
        if self._channel:
            self._channel.close()
            self._channel = None
            self._stub = None
    
    def core_ready(self) -> AppError | None:
        if not self._stub:
            return AppError(500, "INTERNAL", "gRPC client not connected", {})
            
        try:
            self._stub.core_alive(empty_pb2.Empty())
            return None
        except grpc.RpcError as e:
            status_code = e.code()
            details = e.details()
            return AppError(503, "SERVICE_UNAVAILABLE", "gRPC client not connected", {"status_code": status_code.name, "grpc_details": details})
    
    def enqueue_contracts(self, contracts: List[Dict[str, Any]]) -> bool:
        """Send a list of contracts to the core for processing."""
        if not self._stub:
            return False               
        try:
            def contract_generator():
                for contract_dict in contracts:
                    contract_msg = core_pb2.Contract(
                        symbol=contract_dict["symbol"],
                        expiry=contract_dict["expiry"],
                        strike=float(contract_dict["strike"]),
                        type=contract_dict["type"]
                    )
                    yield contract_msg
            
            self._stub.enqueue_contracts(contract_generator())
            return True
        except grpc.RpcError as e:
            return False
        
    def get_enqueue_contracts_stub(self) -> Callable[[Iterator[core_pb2.Contract]], empty_pb2.Empty] | None:
        """Get the raw gRPC stub method for advanced usage."""
        if not self._stub:
            return None
        return self._stub.enqueue_contracts


# Example usage
if __name__ == "__main__":
    # Test the service
    print("Testing gRPC connection to core...")
    
    grpc_service = CoreGrpcService()  # Auto-connects on init
    
    core_error = grpc_service.core_ready()
    if core_error is None:
        print("✅ Core is alive!")
        
        # Example contracts
        test_contracts = [
            {
                "symbol": "AAPL",
                "expiry": "2025-12-19",
                "strike": 150.0,
                "type": "VAN_CALL"
            },
            {
                "symbol": "GOOGL",
                "expiry": "2025-12-19", 
                "strike": 2800.0,
                "type": "VAN_PUT"
            }
        ]
        
        res = grpc_service.enqueue_contracts(test_contracts)
        if res:
            print(f"✅ Successfully sent {len(test_contracts)} contracts")
        else:
            print(f"❌ Failed to send contracts")
    else:
        print(f"❌ Core is not responding: {core_error.message}")
    
    # Cleanup when done
    grpc_service.disconnect()
