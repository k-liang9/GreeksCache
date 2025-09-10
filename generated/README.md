# Generated Protocol Buffer Files

This directory contains generated Python protobuf and gRPC files that are shared across all Python services in the GreeksCache system.

## Structure
```
generated/
├── python/              # Shared Python protobuf modules
│   ├── .gitignore      # Ignore generated files
│   ├── core_pb2.py     # Generated message types
│   └── core_pb2_grpc.py # Generated service stubs
└── README.md           # This file
```

## Usage in Services

Any Python service can import the shared protobuf modules:

```python
import sys
import os
sys.path.append(os.path.join('generated', 'python'))

import core_pb2, core_pb2_grpc
from google.protobuf import empty_pb2

# Use the modules
stub = core_pb2_grpc.PortfolioUpdatesStub(channel)
contract = core_pb2.Contract(symbol="AAPL", strike=150.0)
```

## Benefits of Shared Location

- **Single source of truth** for protobuf definitions
- **No code duplication** across services
- **Consistent versions** across all services
- **Standard protobuf approach** - no custom helpers needed
