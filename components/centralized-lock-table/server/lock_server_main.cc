#include "remote_page_lock_manager.h"
#include "server/server_config.h"
#include <iostream>
#include <string>

using namespace std;

int main(void)
{
    cout << "Message Config Dir : " << MESS_CONFIG_DIR << endl;
    RemotePageLockManager lock_manager;

    string host_config = string(MESS_CONFIG_DIR) + "/lock_server_host_config.json";
    string mess_config = string(MESS_CONFIG_DIR) + "/lock_server_mess_config.json";

    lock_manager.lock_manager_init(host_config.c_str(), mess_config.c_str());

    lock_manager.lock_manager_listen();

    lock_manager.lock_manager_free();
}