#ifndef REMOTE_PAGE_LOCK_CLIENT_HEADER
#define REMOTE_PAGE_LOCK_CLIENT_HEADER

#include"event_mess_handle.h"
#include "lock_common.h"
#include <list>

//centralized_lock
class RemotePageLockClient
{
    public:
    RemotePageLockClient(EventMessageHandle * h,const char * server_name)
    {
        network_handle = h;
        lock_server_host = server_name;
    }

    int request_page_lock(PageLockRequest * request,PageLockReply * reply,int sleep_interval = 2); // send and wait for reply ?

    private:
    int wait_for_lock_reply(PageLockReply * reply,int sleep_interval);

    EventMessageHandle * network_handle;
    std::string lock_server_host;
};

class PageLockReplyFilter : public EventMessageFilter
{
    public:
    bool operator()(EventMessage & mess)
    {
        struct PageLockReply * reply_ptr = (struct PageLockReply *)mess.message;
        if(reply_ptr->space_id == this->space_id && reply_ptr->page_no == this->page_no)
        {
            return true;
        }
        return false;
    }

    uint32_t space_id;
    uint32_t page_no;
};

#endif // !REMOTE_PAGE_LOCK_CLIENT_HEADER