#!/bin/bash

# Name of the program
NAME=standalone

# Make
${VIVADO_VERSION} make all

# Copy
# Host executable
scp ${NAME} ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/.
# Host sources for GDB
scp *.c ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/.
scp *.h ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/.
# Accelerator binary
scp ${CMAKE_PATH}/hsa_tests/*.bin ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/.