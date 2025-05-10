

# Maximum optimization level 
add_project_arguments('-O2', language: 'cpp')
inc = include_directories('include')

# pybind11_dep = dependency('pybind11', required: false)

# if not pybind11_dep.found()
#   pybind11_proj = subproject('pybind11')
#   pybind11_dep = pybind11_proj.dependency('pybind11')
# endif

pybind11_inc ='../extern/pybind11/include'

# precompiled header 
pch_src = files('include/common.cpp')
pch_header = 'BPE_Common.hpp'

# Create the precompiled header
Common_PCH = static_library('common_precompiled', pch_src,
  include_directories : [inc,pybind11_inc],
  dependencies : [python.dependency()],
  c_args: ['-Winvalid-pch'], # for gcc to force precompiled header usage.
  cpp_args: ['-Winvalid-pch'], # for gcc to force precompiled header usage.
)

curr_dir = run_command('pwd').stdout().strip()

python.extension_module(
  '_core',
  'src/_core.cpp',
  include_directories: [inc,pybind11_inc],
  link_with : [Common_PCH],
  install: true,
  install_dir: curr_dir
)


