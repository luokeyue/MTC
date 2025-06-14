source ./percona_build_env.sh

CONF_FILE_PATH=${ROOT_PATH}/percona_server_impl.conf

USER_OPTION=--user=$1

${BUILD_DIR}/scripts/mysqld_safe --defaults-file=${CONF_FILE_PATH} ${USER_OPTION} &