# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/meshSiplification/build/_deps/libigl-src"
  "E:/meshSiplification/build/_deps/libigl-build"
  "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix"
  "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix/tmp"
  "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix/src/libigl-populate-stamp"
  "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix/src"
  "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix/src/libigl-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix/src/libigl-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/meshSiplification/build/_deps/libigl-subbuild/libigl-populate-prefix/src/libigl-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
