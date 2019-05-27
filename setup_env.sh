#!/bin/bash

# modify the path to your own path
export CYPRESS_CODE_ROOT_PATH=$HOME/code_base/cypress

export ARMGCC_VERSION=4.8.1

export PATH=$PATH:$CYPRESS_CODE_ROOT_PATH/arm-2013.11/bin
export FX3_INSTALL_PATH=$CYPRESS_CODE_ROOT_PATH/cyfx3sdk
export ARMGCC_INSTALL_PATH=$CYPRESS_CODE_ROOT_PATH/arm-2013.11
export CYUSB_ROOT=$CYPRESS_CODE_ROOT_PATH/cyusb_linux_1.0.5

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
