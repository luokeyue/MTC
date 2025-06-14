# !/bin/bash

source ./percona_build_env.sh

readJSON()
{
    READ_VAL=$(cat  ${ROOT_PATH}/percona_connect_impl.json | grep "\"$1\"":.*)
    READ_VAL=${READ_VAL#*:}
    READ_VAL=${READ_VAL#*\"}
    READ_VAL=${READ_VAL%\"*}
    echo "read from json for $1 : $READ_VAL"
}

updateJSON()
{
    sed -i "s%\"$1\":.*$%\"$1\":\"$2\"%" ${ROOT_PATH}/percona_connect_impl.json 
}


MYSQL_USER=$1

MYSQL_PORT=$2

MYSQL_IP=$3

if [ ! -f  ${ROOT_PATH}/percona_connect_impl.json ]
then
    echo "creating impl file"
    cp ${ROOT_PATH}/script/temlate/percona_connect_template.json ${ROOT_PATH}/percona_connect_impl.json
fi

if [ ! $1 ]
then 
    readJSON MYSQL_USER
    MYSQL_USER=$READ_VAL

    readJSON MYSQL_PORT
    MYSQL_PORT=$READ_VAL

    readJSON MYSQL_IP
    MYSQL_IP=$READ_VAL
else
    updateJSON MYSQL_USER $MYSQL_USER
    updateJSON MYSQL_IP $MYSQL_IP
    updateJSON MYSQL_PORT $MYSQL_PORT
fi


echo ${BUILD_DIR}/bin/mysql -h${MYSQL_IP} -u${MYSQL_USER}  -P${MYSQL_PORT}  -p
${BUILD_DIR}/bin/mysql -h${MYSQL_IP} -u${MYSQL_USER}  -P${MYSQL_PORT}  -p #\'${MYSQL_PASSWD}\' 

