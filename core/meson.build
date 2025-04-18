

# Maximum optimization level 
add_project_arguments('-O2', language: 'cpp')
inc = include_directories('include')

pybind11_dep = dependency('pybind11', required: false)

if not pybind11_dep.found()
  pybind11_proj = subproject('pybind11')
  pybind11_dep = pybind11_proj.dependency('pybind11')
endif

# for utf-8 
icu_dep = dependency('icu-uc', required: true)

# boost lib
boost_dep = dependency('boost', modules: ['regex','container','system'] , required: true)

pybind11_inc ='../extern/pybind11/include'
arena_inc ='../extern/arena'

# precompiled header 
pch_src = files('include/common.cpp')
pch_header = 'BPE_Common.hpp'

# Create the precompiled header
Common_PCH = static_library('common_precompiled', pch_src,
  include_directories : [inc,pybind11_inc,arena_inc],
  dependencies : [pybind11_dep,python.dependency()],
  c_args: ['-Winvalid-pch'], # for gcc to force precompiled header usage.
  cpp_args: ['-Winvalid-pch'], # for gcc to force precompiled header usage.
)

curr_dir = run_command('pwd').stdout().strip()

python.extension_module(
  '_core',
  '_core.cpp',
  include_directories: [inc,pybind11_inc,arena_inc],
  dependencies: [pybind11_dep,icu_dep,boost_dep],
  link_with : [Common_PCH],
  install: true,
  install_dir: curr_dir
)


