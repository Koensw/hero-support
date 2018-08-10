#!/bin/bash

# Compile script for the Linux kernel

# Default variables
if [[ -z "KERNEL_ARCH" ]]; then
  export KERNEL_ARCH="arm"
fi
if [[ -z "KERNEL_CROSS_COMPILE" ]]; then
  export KERNEL_CROSS_COMPILE="arm-xilinx-linux-gnueabi-"
fi

# Info
echo "-----------------------------------------"
echo "- Executing Linux kernel compile script -"
echo "-----------------------------------------"

# Add U-Boot tools to PATH - mkimage required for building uImage
Tools="u-boot-xlnx/tools"
if [ -d "../"${Tools} ]; then
  echo "Adding U-Boot tools to PATH"
  CurrentPath=`pwd`
  cd "../"${Tools}
  export PATH=`pwd`:${PATH}
  cd ${CurrentPath}
else
  echo "ERROR: U-Boot tools not found"
  echo "Check for the folder ../"${tools}
  exit 1
fi

# Compile the Linux kernel
echo "Comiling the Linux kernel"
make ARCH=${KERNEL_ARCH} CROSS_COMPILE=${KERNEL_CROSS_COMPILE} $1 UIMAGE_LOADADDR=0x8000 uImage

# Copy
echo "Copying uImage to ../sd_image/uImage"
cp arch/arm/boot/uImage ../sd_image/uImage
