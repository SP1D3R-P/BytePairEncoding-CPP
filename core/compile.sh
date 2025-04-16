# !/bin/sh

# This is Not Required [ this is Here for just reference ]

CC=g++

# python using as arg
PYTHON=$1

INC="-I./include"

# Make it platform independent 
PY_INC=`$PYTHON-config --includes`
PY_LIB=`$PYTHON-config --ldflags`

# Pybind11 include path (adjust if needed)
PY_BIND11_INC='-I../extern/pybind11/include'

CPP_FILE=python_entry_point.cpp
OUPUT_SHARED_FILE=_core`python3-config --extension-suffix`

# Compilation
echo "Compiling the Shared File $OUPUT_SHARED_FILE"
echo $CC $PY_BIND11_INC  $PY_INC $PY_LIB $2 -fPIC -shared $CPP_FILE -o $OUPUT_SHARED_FILE
$CC $PY_BIND11_INC $INC  $PY_INC $PY_LIB $2 -fPIC -shared $CPP_FILE -o $OUPUT_SHARED_FILE
