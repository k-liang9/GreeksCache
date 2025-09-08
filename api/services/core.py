import sys
import os

project_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
pybind_path = os.path.join(project_root, 'build', 'pybind')
sys.path.append(pybind_path)

import core_pybind