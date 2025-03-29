project('BytePairEncoding', 'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++23'])


inc = include_directories('include')

# for utf-8 
icu_dep = dependency('icu-uc', required: true)

executable('BytePairEncoding',
           'main.cpp',
           include_directories : inc ,
           dependencies: [icu_dep],
           install : true)
