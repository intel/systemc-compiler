# This file is part of the ParaNut project.

#   Copyright (C) 2022 Marco Milenkovic <marco.milenkovic@hs-augsburg.de>
#       Hochschule Augsburg, University of Applied Sciences

#   Redistribution and use in source and binary forms, with or without modification,
#   are permitted provided that the following conditions are met:

#   1. Redistributions of source code must retain the above copyright notice, this 
#      list of conditions and the following disclaimer.

#   2. Redistributions in binary form must reproduce the above copyright notice,
#      this list of conditions and the following disclaimer in the documentation and/or
#      other materials provided with the distribution.

#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
#   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
#   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
#   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Example Paranut DebugModule
project(dm_paranut)

# Add alle the nessesery files here
add_executable(dm_paranut dm_tb.cpp dm.cpp base.cpp)
target_include_directories(dm_paranut PUBLIC "${PROJECT_SOURCE_DIR}")

# use cmake -DSYN=ON to enable the debug flag or OFF to disable it
OPTION(SYN "set the __SYNTHESIS__ Flag" ON) # Enabled by default

# Add compilation options
# target_compile_options(${PROJECT_NAME} PUBLIC -Wall)
IF(SYN)
    target_compile_definitions(dm_paranut PUBLIC -D__SYNTHESIS__)
ENDIF(SYN)

# svc_target will create ${PROJECT_NAME}_sctool executable that runs code generation 
# and ${PROJECT_NAME} that runs general SystemC simulation
# ELAB_TOP parameter accepts hierarchical name of design  
# (that is SystemC name, returned by sc_object::name() method)
svc_target(dm_paranut INIT_LOCAL_VARS ELAB_TOP tb.dm)

