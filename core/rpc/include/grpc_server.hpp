#ifndef GRPC
#define GRPC

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "core.grpc.pb.h"

#include <functional>
#include <vector>

#include "types.hpp"

using namespace std;
using namespace grpc;

void run_server(
    const function<bool()>& core_ready_impl, 
    const function<bool(Contract&)>& enqueue_contract_impl
);
bool parse_grpc_contract(const grpc_ipc::Contract& contract_msg, Contract& contract);

class PortfolioUpdatesImpl final : public grpc_ipc::PortfolioUpdates::Service {
private:
    const function<bool()>& core_ready_;
    const function<bool(Contract&)>& enqueue_contract_;

public:
    explicit PortfolioUpdatesImpl(
        const function<bool()>& core_ready_impl,
        const function<bool(Contract&)>& enqueue_contract_impl
    );

    Status core_alive(ServerContext* context, const google::protobuf::Empty* req, google::protobuf::Empty* res)override;

    Status enqueue_contracts(ServerContext* context, ServerReader<grpc_ipc::Contract>* contracts, google::protobuf::Empty* res) override;
};

#endif