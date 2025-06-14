
source ./percona_build_env.sh

SOCK_FILE=$(cat ${ROOT_PATH}/percona_server_impl.conf | grep -m 1 ^socket=.*$)

SOCK_FILE=${SOCK_FILE#*=}

${BUILD_DIR}/runtime_output_directory/mysqladmin  -u $1 -p"$2" -S ${SOCK_FILE} shutdown