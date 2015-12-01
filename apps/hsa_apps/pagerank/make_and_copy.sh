#!/bin/bash

# Name of the program
APP=pagerank

# Make
vivado-2015.1 make all

# Copy
# Host executable
scp ${APP} ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/${APP}/.
# Test data
scp -r test ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/${APP}/.
# Host sources for GDB
scp *.c ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/${APP}/.
scp *.h ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/${APP}/.
# Accelerator binary
scp ${PULP_SW_PATH}/hsa_apps/${APP}/${APP}.bin ${SCP_TARGET_MACHINE}:${SCP_TARGET_PATH}/programs/${APP}/.