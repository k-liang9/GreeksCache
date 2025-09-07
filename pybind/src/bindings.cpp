#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "state_orchestrator.hpp"

namespace py = pybind11;

PYBIND11_MODULE(core_pybind, m) {
    m.doc() = "Python bindings for interacting with the core (updating portfolio)";
}