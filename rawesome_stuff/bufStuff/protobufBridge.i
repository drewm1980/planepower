%module protobufBridge

%include "std_vector.i"
%include "std_string.i"
%feature("autodoc", "1");
%template(DVector)             std::vector<double>;

%include "protobufBridge.hpp"
%{
#include "protobufBridge.hpp"
%}
