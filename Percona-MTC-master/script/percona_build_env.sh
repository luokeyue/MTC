# !/bin/bash
#echo 'export PERCONA_BUILD_PATH="$HOME/percona_build"' >> ~/.bashrc 


PWD_PATH=$(pwd)
ROOT_PATH=$(cd $(dirname ${BASH_SOURCE[0]}); pwd )/..

BUILD_DIR=${ROOT_PATH}/build

if [ $PERCONA_BUILD_PATH ]
  then
    echo "use PERCONA_BUILD_PATH = $PERCONA_BUILD_PATH"
    BUILD_DIR=$PERCONA_BUILD_PATH/build
fi