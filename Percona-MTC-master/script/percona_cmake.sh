###
 # @Author: wei
 # @Date: 2020-06-17 19:25:53
 # @LastEditors: Do not edit
 # @LastEditTime: 2020-06-29 09:07:06
 # @Description: file content
 # @FilePath: /script/percona_cmake.sh
###
# !/bin/bash
#echo 'export PERCONA_BUILD_PATH="$HOME/percona_build"' >> ~/.bashrc


source ./percona_build_env.sh


echo ${BUILD_DIR}
if [ ! -d "$BUILD_DIR" ]; then
  mkdir $BUILD_DIR
else
  echo "${ROOT_PATH}/build has created"
fi


cd ${BUILD_DIR}

cmake ${ROOT_PATH}/percona-server/  -DDOWNLOAD_BOOST=1  -DWITH_BOOST=${ROOT_PATH}/boost_1_70_0/ -DWITHOUT_TOKUDB=1  -DWITHOUT_ROCKSDB=1 -DENABLE_DOWNLOADS=1  -DWITH_MYSQLX=OFF  -DWITH_GROUP_REPLICATION=OFF -DCMAKE_INSTALL_PREFIX=${BUILD_DIR}/../build-install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cd ${PWD_PATH}

echo "**************************"
echo "Percona-Server Cmake Finish"
echo "**************************"

