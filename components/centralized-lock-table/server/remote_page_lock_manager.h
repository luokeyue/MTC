#ifndef REMOTE_PAGE_LOCK_MANAGER_HEADER
#define REMOTE_PAGE_LOCK_MANAGER_HEADER

#include <list>
#include <vector>
#include "event_mess_handle.h"

#include "lock_common.h"

#define REMOTE_PAGE_LOCK_MANAGER_DEBUG

class RemotePageLockManager
{
    public:

    static const bool lock_compatibility_matrix[2][2];
    static ulint cal_fold(uint32_t m_space,uint32_t m_page_no);

    static void lock_request_callback(EventMessageHandle * mess_handle,EventMessage * mess,void * arg);

    // public API
    void lock_manager_init(const char * host_config_path,const char * mess_config_path);
    void lock_manager_free();
    void lock_manager_listen(); // listen lock request

    //callback API

    int check_lock_request(const char * asker,PageLockRequest * request,PageLockReply * reply);

    private:

    int insert_page_lock_t(const char * asker,PageLockRequest * request);
    int update_unlock_page_lock_t(const char * asker,PageLockRequest * request,PageLock_t * page_lock);
    int update_success_page_lock_t(const char * asker,PageLockRequest * request,PageLock_t * page_lock);
    int update_fail_page_lock_t(const char * asker,PageLockRequest * request,PageLock_t * page_lock);

    struct PageLock_t * page_lock_hash;
    EventMessageHandle msg_handle;
};


#endif // !REMOTE_PAGE_LOCK_MANAGER.h