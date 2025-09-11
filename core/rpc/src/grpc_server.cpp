#include <vector>
#include <memory>
#include <iostream>

#include "grpc_server.hpp"
#include "utils.hpp"

using namespace std;

PortfolioUpdatesImpl::PortfolioUpdatesImpl(
    const function<bool()>& core_ready_impl,
    const function<bool(Contract&)>& enqueue_contract_impl
) :
core_ready_(core_ready_impl),
enqueue_contract_(enqueue_contract_impl) {}

Status PortfolioUpdatesImpl::core_alive(
    ServerContext* context, 
    const google::protobuf::Empty* req,
    google::protobuf::Empty* res)
{
    return core_ready_() ? Status::OK : Status(StatusCode::UNAVAILABLE, "Core is not ready");
}

Status PortfolioUpdatesImpl::enqueue_contracts(ServerContext* context, ServerReader<grpc_ipc::Contract>* reader, google::protobuf::Empty* res) {
    grpc_ipc::Contract contract_msg;

    bool ok = true;
    int len = 0;
    Contract contract;
    while (reader->Read(&contract_msg)) {
        if (parse_grpc_contract(contract_msg, contract)) {
            if (!enqueue_contract_(contract)) {
                ok = false;
            } else {
                ++len;
            }
        } else {
            ok = false;
        }
    }

    return ok ? Status::OK : Status(StatusCode::INVALID_ARGUMENT, "Failed to process one or more contracts");
}

bool parse_grpc_contract(const grpc_ipc::Contract& contract_msg, Contract& contract) {
    contract.symbol = contract_msg.symbol();
    contract.expiry = contract_msg.expiry();
    contract.strike = contract_msg.strike();
    contract.payoff_type = string_to_payoff_type(contract_msg.type());
    if (contract.payoff_type == PAYOFFTYPE_ERROR) {
        return false;
    }
    return true;
}

void run_server(
    const function<bool()>& core_ready_impl,
    const function<bool(Contract&)>& enqueue_contract_impl
) {
    string server_address("localhost:8080");
    PortfolioUpdatesImpl service(core_ready_impl, enqueue_contract_impl);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(&service);
    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << server_address << '\n';
    server->Wait();
}