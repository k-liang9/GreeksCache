#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "runtime.hpp"
#include "types.hpp"

namespace py = pybind11;
using namespace std;

void contract_from_dict(const py::dict& dict, Contract& c) {
    c.symbol = py::cast<string>(dict["symbol"]);
    c.expiry = py::cast<string>(dict["expiry"]);
    c.strike = py::cast<double>(dict["strike"]);
    string type = py::cast<string>(dict["type"]);
    if (type == "VAN_CALL") {
        c.payoff_type = VAN_CALL;
    } else if (type == "VAN_PUT") {
        c.payoff_type = VAN_PUT;
    }
}

bool parse_contracts(const py::list& lst) {
    vector<Contract> vec;
    vec.resize(lst.size());
    for (size_t i = 0; i < lst.size(); i++) {
        contract_from_dict(lst[i], vec[i]);
    }
    {
        py::gil_scoped_release release;
        return enqueue_contracts(vec);
    }
}

PYBIND11_MODULE(core_pybind, m) {
    m.doc() = "Python bindings for interacting with the core (updating portfolio)";
    m.def("core_ready", &core_ready);
    m.def("enqueue_contracts", &parse_contracts);
}