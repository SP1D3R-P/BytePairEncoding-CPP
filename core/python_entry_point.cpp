
# include <iostream>

# include <common.hpp>

namespace py = pybind11;

int add(int i, int j) {
    return i + j;
}

int sum_array(py::array_t<int> input)
{
    input.request(true);
    return 3;
}

// GPT4 pattern 
#define GPT4_REGEX_PATTERN_1 "[^\\r\\n\\p{L}\\p{N}]?[\\p{Lu}\\p{Lt}\\p{Lm}\\p{Lo}\\p{M}]*[\\p{Ll}\\p{Lm}\\p{Lo}\\p{M}]+(?i:'s|'t|'re|'ve|'m|'ll|'d)?"
#define GPT4_REGEX_PATTERN_2 "[^\\r\\n\\p{L}\\p{N}]?[\\p{Lu}\\p{Lt}\\p{Lm}\\p{Lo}\\p{M}]+[\\p{Ll}\\p{Lm}\\p{Lo}\\p{M}]*(?i:'s|'t|'re|'ve|'m|'ll|'d)?"
#define GPT4_REGEX_PATTERN_3 "[0-9]{1,3}"
#define GPT4_REGEX_PATTERN_4 " ?[^\\s\\p{L}\\p{N}]+[\\r\\n/]*"
#define GPT4_REGEX_PATTERN_5 "\\s*[\\r\\n]+"
#define GPT4_REGEX_PATTERN_6 "\\s+(?!\\S)"
#define GPT4_REGEX_PATTERN_7 "\\s+"
#define GPT4_PATTERN { GPT4_REGEX_PATTERN_1"|" GPT4_REGEX_PATTERN_2"|" GPT4_REGEX_PATTERN_3"|" \
    GPT4_REGEX_PATTERN_4"|" GPT4_REGEX_PATTERN_5"|" GPT4_REGEX_PATTERN_6"|" GPT4_REGEX_PATTERN_7 }


py::object parse()
{
    try
    {

        py::module_ regex_moulde = py::module_::import("regex");

        std::string GPT4 = GPT4_PATTERN;

        py::object compiled_regex  = regex_moulde.attr("compile")(GPT4.c_str()); 

        py::list res_list = compiled_regex.attr("findall")("Hello World");

        // py::list res;
        for (auto &i : res_list)
        {
            std::cout << "*" <<py::cast<std::string_view>(i) << "\n";
        }
        

        return res_list;
        

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return py::none();
}

PYBIND11_MODULE(_core, m) {
    m.doc() = "Example pybind11 plugin";
    m.def("add", &add, "A function that adds two numbers");
    m.def("sum_array",&sum_array,"");
    m.def("parse",&parse,"");
}