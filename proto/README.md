# Protocol Buffers Organization

This directory contains `.proto` files that define the gRPC interfaces for the GreeksCache system.

## Current Structure
```
proto/
├── CMakeLists.txt          # Builds both C++ and Python code
├── core.proto              # Main service interface
└── README.md              # This file
```

## Generated Files Location

### C++ Files
Generated in `build/proto/` and linked via `proto_grpc` library:
- `core.pb.h`, `core.pb.cc` - Message definitions
- `core.grpc.pb.h`, `core.grpc.pb.cc` - Service definitions

### Python Files (Shared)
Generated in `generated/python/` for use by **all Python services**:
- `core_pb2.py` - Message definitions
- `core_pb2_grpc.py` - Service stubs

## Multi-Service Usage

### In Any Python Service
```python
import sys
import os
sys.path.append(os.path.join('generated', 'python'))

import core_pb2, core_pb2_grpc
from google.protobuf import empty_pb2
```

### Benefits of This Approach
- ✅ **Standard protobuf pattern** - no custom helpers
- ✅ **Single source of truth** for all services  
- ✅ **No code duplication** across services
- ✅ **Clean and simple** - minimal file clutter
- ✅ **Easy maintenance** and updates

## For Multiple Proto Files

When adding more services:

```
proto/
├── core.proto              # Core computation services
├── market_data.proto       # Market data services  
├── user_mgmt.proto         # User management services
└── CMakeLists.txt          # Updated to handle multiple files
```

All generated files will go to the same shared `generated/python/` directory.
