# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-src"
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-build"
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix"
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix/tmp"
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix/src/implot-populate-stamp"
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix/src"
  "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix/src/implot-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix/src/implot-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/colts/Desktop/desktop-trader/out/build/x64-debug/_deps/implot-subbuild/implot-populate-prefix/src/implot-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
