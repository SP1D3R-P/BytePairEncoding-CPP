
# include <BytePairEncoding.hpp>

# define LOG_LEVEL 0

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
    py::str default_pattern = GPT4_PATTERN;
    py::str displayTable_default_out_file = "";


    m.doc() = "Example pybind11 plugin";
    pybind11::class_<BPE::PyBytePairEncoding>(m,"pyBytePairEncoding")
        .def(pybind11::init<py::int_,py::str>(),py::arg("vocabSize") , py::arg("pattern") = default_pattern )
        .def("compile",&BPE::PyBytePairEncoding::compile)
        .def("train",&BPE::PyBytePairEncoding::train)
        .def("encode",&BPE::PyBytePairEncoding::encode)
        .def("decode",&BPE::PyBytePairEncoding::decode)
        .def("displayTable",&BPE::PyBytePairEncoding::displayTable , py::arg("file_name") = displayTable_default_out_file);
}