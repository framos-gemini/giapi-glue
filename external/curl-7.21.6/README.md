# Overview
This is the stepst that GEMINI uses to compile this library on CENTOS 7 and ROCKY 8. Execute the following actions to compile the library. 

# Configure and Build
Execute configure to set the environment.
```
./configure  --without-ssl  --prefix=$GIAPI_ROOT/external/libcurl
```
Compile and install the library. 

```
make && make install
```
