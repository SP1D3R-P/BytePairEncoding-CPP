project('BytePairEncoding', 'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++23'])

# add_project_arguments('-std=c++23', language: 'cpp')
inc = include_directories('include')

# for utf-8 
icu_dep = dependency('icu-uc', required: true)

executable('BytePairEncoding',
           'main.cpp',
           include_directories : inc ,
           dependencies: [icu_dep],
           install : true)
