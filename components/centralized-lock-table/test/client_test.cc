#include "remote_page_lock_client.h"
#include <iostream>

#include "server/server_config.h"

using namespace std;

void lock_reply_print( PageLockReply & reply )
{
  std::cout <<"Space id : " <<reply.space_id <<std::endl;
  std::cout <<"Page no : " <<reply.page_no << std::endl;
  std::cout <<"Reply Type : " <<reply.reply_type << std::endl;
}

int main(void)
{
    std::string client_host_config ="";
    client_host_config += MESS_CONFIG_DIR;
    client_host_config += "/test/lock_client_host_config.json";
    std::string mess_type_config = "";
    mess_type_config += MESS_CONFIG_DIR;
    mess_type_config +="/lock_server_mess_config.json";

    EventMessageHandle handle;
    handle.init_handle(client_host_config.c_str(),mess_type_config.c_str());
    RemotePageLockClient client(&handle,"lock_server");

    /* request new page lock */
    PageLockRequest lock_request;
    lock_request.space_id = 1;
    lock_request.page_no = 1;
    lock_request.version_no = 1;
    lock_request.lock_type = PageLockType::W_LOCK;

    PageLockReply lock_reply;
    int return_val = client.request_page_lock(&lock_request, &lock_reply);
    cout << "=== request new page lock ==="  << endl;
    lock_reply_print(lock_reply);

    /* r-r lock*/
    return_val = client.request_page_lock(&lock_request, &lock_reply);
    cout << "=== request r-r page lock ==="  << endl;
    lock_reply_print(lock_reply);

    /* W-r lock*/
    lock_request.space_id = 1;
    lock_request.page_no = 1;
    lock_request.version_no = 1;
    lock_request.lock_type = PageLockType::W_LOCK;
     return_val = client.request_page_lock(&lock_request, &lock_reply);
    cout << "=== request W-r page lock ==="  << endl;
    lock_reply_print(lock_reply);

    /* unlock-r lock*/
    lock_request.space_id = 1;
    lock_request.page_no = 1;
    lock_request.version_no = 1;
    lock_request.lock_type = PageLockType::UNLOCK;
     return_val = client.request_page_lock(&lock_request, &lock_reply);
    cout << "=== request unlock-r page lock ==="  << endl;
    lock_reply_print(lock_reply);

    handle.free_handle();
}