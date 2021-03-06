#!/bin/bash

# Compile script for Buildroot

# Make sure required variables are set
if [ -z "${KERNEL_ARCH}" ] || [ -z "${KERNEL_CROSS_COMPILE}" ] ; then
  echo "ERROR: Missing required environment variables KERNEL_ARCH and/or KERNEL_CROSS_COMPILE, aborting now."
  exit 1;
fi

# Info
echo "-----------------------------------------"
echo "-  Executing Buildroot compile script   -"
echo "-----------------------------------------"

# Sync BusyBox busybox-config to .config
./sync_busybox.sh 0

# Build the root file system
echo "Generating the root file system"
make ARCH=${KERNEL_ARCH} CROSS_COMPILE=${KERNEL_CROSS_COMPILE} $1

BUILD_STATUS=$?

# Copy
echo "Copying the root file system image to ../root_file_system/rootfs.cpio.gz"
cp output/images/rootfs.cpio.gz ../root_filesystem/.

# Sync BusyBox .config to busybox-config
./sync_busybox.sh 1

exit ${BUILD_STATUS}
