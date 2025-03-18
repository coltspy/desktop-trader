# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-src"
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-build"
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix"
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix/tmp"
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix/src/curl-populate-stamp"
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix/src"
  "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix/src/curl-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix/src/curl-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/dev/vsrepos/trading-platform/out/build/x64-debug/_deps/curl-subbuild/curl-populate-prefix/src/curl-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
