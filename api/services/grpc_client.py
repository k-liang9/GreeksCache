"""
gRPC client service for communicating with the core C++ engine.

This service handles all gRPC communication with the GreeksCache core.
"""

import sys
import os
from typing import List, Dict, Any

# Add generated protobuf directory to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'generated', 'python'))

import grpc
import core_pb2
import core_pb2_grpc
from google.protobuf import empty_pb2


class CoreGrpcService:
    """Service for communicating with the GreeksCache core via gRPC."""
    
    def __init__(self, host: str = "localhost", port: int = 8000):
        self.address = f"{host}:{port}"
        self._channel = None
        self._stub = None
    
    def __enter__(self):
        """Context manager entry."""
        self.connect()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.disconnect()
    
    def connect(self):
        """Establish connection to the core service."""
        self._channel = grpc.insecure_channel(self.address)
        self._stub = core_pb2_grpc.PortfolioUpdatesStub(self._channel)
    
    def disconnect(self):
        """Close connection to the core service."""
        if self._channel:
            self._channel.close()
            self._channel = None
            self._stub = None
    
    def is_core_alive(self) -> bool:
        """Check if the core service is alive and responding."""
        try:
            self._stub.core_alive(empty_pb2.Empty())
            return True
        except grpc.RpcError:
            return False
    
    def enqueue_contracts(self, contracts: List[Dict[str, Any]]) -> bool:
        """
        Send a list of contracts to the core for processing.
        
        Args:
            contracts: List of contract dictionaries with keys:
                      - symbol (str): The underlying symbol
                      - expiry (str): Expiry date (YYYY-MM-DD format)
                      - strike (float): Strike price
                      - type (str): Contract type (CALL/PUT)
        
        Returns:
            bool: True if contracts were successfully enqueued
        """
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
            print(f"Error enqueuing contracts: {e}")
            return False


# Convenience functions for direct use
def check_core_status() -> bool:
    """Quick check if core is alive."""
    with CoreGrpcService() as service:
        return service.is_core_alive()


def send_contracts(contracts: List[Dict[str, Any]]) -> bool:
    """Quick function to send contracts to core."""
    with CoreGrpcService() as service:
        return service.enqueue_contracts(contracts)


# Example usage
if __name__ == "__main__":
    # Test the service
    print("Testing gRPC connection to core...")
    
    if check_core_status():
        print("✅ Core is alive!")
        
        # Example contracts
        test_contracts = [
            {
                "symbol": "AAPL",
                "expiry": "2025-12-19",
                "strike": 150.0,
                "type": "CALL"
            },
            {
                "symbol": "GOOGL",
                "expiry": "2025-12-19", 
                "strike": 2800.0,
                "type": "PUT"
            }
        ]
        
        if send_contracts(test_contracts):
            print(f"✅ Successfully sent {len(test_contracts)} contracts")
        else:
            print("❌ Failed to send contracts")
    else:
        print("❌ Core is not responding")
