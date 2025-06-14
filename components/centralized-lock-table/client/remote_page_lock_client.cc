#include "remote_page_lock_client.h"


int RemotePageLockClient::request_page_lock(PageLockRequest * request,PageLockReply * reply,int sleep_interval)
{
    EventMessage request_msg;

    request_msg.prepare_send(REMOTE_LOCK_GROUP_NAME,REMOTE_LOCK_REQUEST_MESS_TYPE,lock_server_host.c_str(),(const char *)request,sizeof(PageLockRequest));
    //reply_msg.prepare_recive(REMOTE_LOCK_GROUP_NAME,REMOTE_LOCK_REPLY_MESS_TYPE);
    if(network_handle->sendMessage(&request_msg) < 0)
    {
        return -1;
    }
    reply->space_id = request->space_id;
    reply->page_no = request->page_no;
    return wait_for_lock_reply(reply,sleep_interval);
}

 int RemotePageLockClient::wait_for_lock_reply(PageLockReply * reply,int sleep_interval)
 {
    EventMessage reply_msg;
    PageLockReplyFilter filter;
    filter.space_id = reply->space_id;
    filter.page_no = reply->page_no;

    while(1)
    {
        reply_msg.clear();
        reply_msg.prepare_recive(REMOTE_LOCK_GROUP_NAME,REMOTE_LOCK_REPLY_MESS_TYPE);
        int return_val = network_handle->readMessage(&reply_msg,filter);
        if(return_val == -1)
        {
            return -1;
        }
        if(return_val == 0 )
        {
            sleep(sleep_interval);
        }
        if(return_val == 1)
        {
           memcpy(reply,reply_msg.message,sizeof(PageLockReply));
            return 1;
        }
    }
 }
