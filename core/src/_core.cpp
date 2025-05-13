
# include <BytePairEncoding.hpp>

# define LOG_LEVEL 0

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
    py::str default_pattern = GPT4_PATTERN;
    py::str displayTable_default_out_file = "";


    m.doc() = "Example pybind11 plugin";
    pybind11::class_<BPE::PyBytePairEncoding>(m,"pyBytePairEncoding")
        .def(pybind11::init<py::int_,py::str>(),py::arg("vocabSize") , py::arg("pattern") = default_pattern )
        .def("_compile",&BPE::PyBytePairEncoding::compile)
        .def("_train",&BPE::PyBytePairEncoding::train)
        .def("_encode",&BPE::PyBytePairEncoding::encode)
        .def("_decode",&BPE::PyBytePairEncoding::decode)
        .def_property("_capacity",&BPE::PyBytePairEncoding::vocabCap,&BPE::PyBytePairEncoding::updateVocabCap)
        .def_property_readonly("_size",&BPE::PyBytePairEncoding::vocabSize)
        .def_property_readonly("_pattern",&BPE::PyBytePairEncoding::pattern)
        .def_static("_fromJson",&BPE::LoadFromJson)
        .def("_table",&BPE::PyBytePairEncoding::toDict);
}