

# Maximum optimization level 
add_project_arguments('-O2', language: 'cpp')
inc = include_directories('include')

py = import('python').find_installation()
pybind11_inc ='../extern/pybind11/include'

py_dep = py.dependency()

# precompiled header 
pch_src = files('include/common.cpp')
pch_header = 'common.hpp'

# Create the precompiled header
pch_target = static_library('common_precompile', pch_src,
  include_directories : [inc,pybind11_inc],
  dependencies : py_dep,
  c_args: ['-Winvalid-pch'], # for gcc to force precompiled header usage.
  cpp_args: ['-Winvalid-pch'], # for gcc to force precompiled header usage.
)


# pybind11_dep = dependency('pybind11', required: true)

# include dirs 


# for utf-8 
icu_dep = dependency('icu-uc', required: true)

# boost lib
boost_dep = dependency('boost', modules: ['regex','container','system'] , required: true)

# executable('BytePairEncoding',
#            'main.cpp',
#            include_directories : inc ,
#            dependencies: [icu_dep,boost_dep],
#            link_with : [pch_target],
#            cpp_args: ['-include', 'common.hpp']
# )


# compile using bash 
run_command('sh','compile.sh',py)

