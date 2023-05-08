# Introduction

The GIAPI C++ Language glue API  is one of the Language binding options used by
instrument code to integrate with Gemini.  The C++ API is implemented as a
shared library in C++. The API sits between the instrument code and the
Gemini Master Process (GMP).

The GIAPI C++ API library is included with any process that the builder creates
that must communicate with Gemini. Which processes in the instrument must link
with and use the C++ API is an instrument design decision.

The communication protocol between the C++ API and the GMP is encapsulated
within the GIAPI implementation.

Details about the usage of the API can be found in the Gemini Observatory
ICD50 - GIAPI C++ Language Glue API.

# Dependencies

The GIAPI C++ API supports the C++11 standard. The C++11 is a major upgrade 
over C++98/03, with performance and convenience features that make it feel 
like a new language. The GIAPI library can be compiled from any version 
of the gcc 4.8.1 compiler, which was the first to implement the C++11 language.
Currently, Gemini is using the GIAPI on centos7 (gcc 4.8.5) and 
rocky 8 (gcc 8.5.0). 


The following are tools used to build the GIAPI C++ API:
|             | Centos 7 | Rocky 8|
|    :---:    |  :---:   | :---:  |
|g++ (GCC)    | 4.8.5    | 8.5.0  |
|GNU Make     | 3.82     | 4.2.1  |
|GNU automake | 1.16.1   | 1.16.1 |
|cmake        | 2.8.12.2 | 3.20.2 |

# Binary distribution


# External libraries
The external libraries required by the GIAPI are:
<ol>
   <li> Apache Active MQ CMS (C++ Messaging system) version 3.4.1. [link]()</li>
   <li> Apache Log4cxx version 0.11.0. [link](external/apache-log4cxx-0.11.0/README.md) </li>
   <li> Apache Portable Runtime Library version 1.3.12. [link](external) </li>
   <li> libCurl version 7.21.6. [link](external/curl-7.21.6) </li>
   <li> curlpp version 0.8.1 [link](https://github.com/framos-gemini/giapi-glue/blob/c%2B%2B11_Release/external/curlpp-0.8.1) </li>
</ol>

## Compile libraries.
### RPM distribution
Compiled versions of these libraries are available in the distribution
package of the GIAPI (under the 'lib' directory) or as RPM packages
following the Gemini RPM framework.
### Manual compilation
It is possible compile each of the GIAPI external libraries (described in 
the External libraries point) from their source code.  However, each of 
the commands used by Gemini on its Centos 7 and 
Rocky 8 will be listed bellow.



# History
The first version of the giapi-glue dates from October 24, 2008. It was written by AN (Arturo Nunez). 
During these years it has been maintained mainly by Carlos Quiroz and Ignacio Arriagada. 

