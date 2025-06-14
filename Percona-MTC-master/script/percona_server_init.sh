#!/bin/sh

source ./percona_build_env.sh

rm ${ROOT_PATH}/percona_server_impl.conf
cp ${ROOT_PATH}/script/temlate/percona_server_template.conf ${ROOT_PATH}/percona_server_impl.conf

CONF_FILE_PATH=${ROOT_PATH}/percona_server_impl.conf


OUTPUT_DIR=$1
PORT=$2

if [ ! $OUTPUT_DIR ]
  then
    OUTPUT_DIR=/tmp/percona_output
    mkdir -p ${OUTPUT_DIR}
fi

if [ ! $PORT ]
  then
    PORT=9898
fi

PORT=$(echo $PORT)

sed -i "s|port=.*$|port=${PORT}|g" ${CONF_FILE_PATH}
sed -i "s|socket=.*$|socket=${OUTPUT_DIR}/data/percona.sock|g" ${CONF_FILE_PATH} 
sed -i "s|basedir=.*$|basedir=${BUILD_DIR}/|g" ${CONF_FILE_PATH}
sed -i "s|datadir=.*$|datadir=${OUTPUT_DIR}/data/|g" ${CONF_FILE_PATH}
sed -i "s|log-error=.*$|log-error=${OUTPUT_DIR}/percona_error.log|g" ${CONF_FILE_PATH}
sed -i "s|pid-file=.*$|pid-file=${OUTPUT_DIR}/data/percona.pid|g" ${CONF_FILE_PATH}


${BUILD_DIR}/bin/mysqld --defaults-file=${CONF_FILE_PATH}  --initialize  


