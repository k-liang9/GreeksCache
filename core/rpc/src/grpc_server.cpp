#include <vector>
#include <memory>
#include <iostream>

#include "grpc_server.hpp"
#include "utils.hpp"

using namespace std;

PortfolioUpdatesImpl::PortfolioUpdatesImpl(
    const function<bool()>& core_ready_impl,
    const function<bool(vector<Contract>&)>& enqueue_contracts_impl
) :
core_ready_(core_ready_impl),
enqueue_contracts_(enqueue_contracts_impl) {}

Status PortfolioUpdatesImpl::core_alive(
    ServerContext* context, 
    const google::protobuf::Empty* req,
    google::protobuf::Empty* res)
{
    return core_ready_() ? Status::OK : Status::CANCELLED;
}

Status PortfolioUpdatesImpl::enqueue_contracts(ServerContext* context, ServerReader<GrpcContract>* reader, google::protobuf::Empty* res) {
    GrpcContract contract_msg;
    Contract contract;

    bool ok = true;
    int len = 0;
    vector<Contract> contracts;
    while (reader->Read(&contract_msg)) {
        if (parse_grpc_contract(contract_msg, contract)) {
            contracts.push_back(contract);
            ++len;
        } else {
            ok = false;
        }
    }

    return enqueue_contracts_(contracts) && ok ? Status::OK : Status::CANCELLED;
}

bool parse_grpc_contract(const GrpcContract& contract_msg, Contract& contract) {
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
    const function<bool(vector<Contract>&)>& enqueue_contracts_impl
) {
    string server_address("localhost:8000");
    PortfolioUpdatesImpl service(core_ready_impl, enqueue_contracts_impl);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(&service);
    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << server_address << '\n';
    server->Wait();
}