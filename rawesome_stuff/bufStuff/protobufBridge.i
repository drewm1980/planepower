%module protobufBridge

%include "std_vector.i"
%feature("autodoc", "1");
%template(DVector)             std::vector<double>;

%include "protobufBridge.hpp"
%{
#include "protobufBridge.hpp"
%}
