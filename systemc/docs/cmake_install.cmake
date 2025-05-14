# Install script for directory: /nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18/icsc/systemc/docs

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdocx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/systemc" TYPE FILE OPTIONAL FILES "/nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18/icsc/systemc/docs/DEVELOPMENT.md")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdocx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/systemc" TYPE FILE OPTIONAL FILES "/nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18/icsc/systemc/docs/git_branches.svg")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdocx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/systemc/sysc" TYPE FILE OPTIONAL FILES "/nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18/icsc/systemc/docs/sysc/README")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdocx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/systemc/sysc" TYPE FILE OPTIONAL FILES "/nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18/icsc/systemc/docs/sysc/SIGN_EXTENSION.md")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdocx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/systemc/tlm" TYPE FILE OPTIONAL FILES "/nfs/pdx/disks/or_ip_lab_disk001/users/mmoiseev/projects/sc_tools_clang18/icsc/systemc/docs/tlm/tlm_README.md")
endif()

