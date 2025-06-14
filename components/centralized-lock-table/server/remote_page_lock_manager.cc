#include "remote_page_lock_manager.h"
#ifdef REMOTE_PAGE_LOCK_MANAGER_DEBUG
#include <iostream>
#endif


const bool RemotePageLockManager::lock_compatibility_matrix[2][2] = {
          /*s , x*/   //v:request ; h:status
    /*s*/ {true,false},
    /*x*/ {false,false}
};

/********************************
 * Static Functions
*********************************/
ulint RemotePageLockManager::cal_fold(uint32_t m_space,uint32_t m_page_no)
{
    ulint m_fold = (m_space << 20) + m_space + m_page_no;
    return m_fold;
}

void RemotePageLockManager::lock_request_callback(EventMessageHandle * mess_handle,EventMessage * mess,void * arg)
{
    #ifdef REMOTE_PAGE_LOCK_MANAGER_DEBUG
    std::cout << "Lock Request Callback invoked " <<std::endl;
    #endif
    RemotePageLockManager * lock_manager = (RemotePageLockManager * )arg;
    PageLockReply reply;
    lock_manager->check_lock_request(mess->send_host_name,(struct PageLockRequest *)mess->message,&reply);

    EventMessage replyMsg;
    replyMsg.prepare_send(mess->group_name,"PageLockReply",mess->send_host_name,(const char *)&reply,sizeof(PageLockReply));

    if(mess_handle->sendMessage(&replyMsg)<0)
    {
        //std::cout << mess->send_host_name << std::endl;
        std::cout << MessageError::getEventErrorStr( replyMsg.error_no) << std::endl;
        assert(false);
    }
}

/********************************
 * Public Member Functions
*********************************/
void RemotePageLockManager::lock_manager_init(const char * host_config_path,const char * mess_config_path)
{
    msg_handle.init_handle(host_config_path, mess_config_path);

    page_lock_hash = NULL; //init hash table

    /**
     * Register callbacks for lock requests
     *
     * todo:
     *      1. recive r lock request
     *      2. recive w lock request
     *      3. recive unlock request
    */
    msg_handle.register_recive_handler( "centralized_lock", "PageLockRequest",lock_request_callback,this);
}

void RemotePageLockManager::lock_manager_free()
{
    //todo : free page_lock_t
    msg_handle.free_handle();
}

void RemotePageLockManager::lock_manager_listen()
{
    while(1)
    {
        // Keep Run
    }
}

 /*
    1. Check lock request, construct page lock reply
        Return ---- 0 : no page_lock_t
        Return ---- 1 : lock succeeded
        Reurn ----- 2 : unlock succeeded
        Return ---- -1 : lock failed
    2. if 0 ,insert page_lock_t
*/
int RemotePageLockManager::check_lock_request(const char * asker,PageLockRequest * request,PageLockReply * reply)
{
//std::cout <<"page lock hash address :" << std::endl;// (int64_t)page_lock_hash << std::endl;
    ulint fold = cal_fold(request->space_id,request->page_no);

    struct PageLock_t * page_lock_infor = NULL;

    if(page_lock_hash != NULL)
        HASH_FIND_INT(page_lock_hash,&fold,page_lock_infor);
    else
        page_lock_infor = NULL;
//std::cout << asker << std::endl;
    reply->space_id = request->space_id;
    reply->page_no = request->page_no;

    if(page_lock_infor == NULL)
    {
        insert_page_lock_t(asker,request);
//        reply->page_holder = NULL;
        reply->reply_type =  PageReplyType::LOCK_SUCCESS;
        return 0;
    }

    if(request->lock_type == PageLockType::UNLOCK)
    {
        update_unlock_page_lock_t(asker,request,page_lock_infor);
        reply->reply_type = PageReplyType::UNLOCK_SUCCESS;
        return 2;
    }

    if( page_lock_infor->lock_type == PageLockType::UNLOCK)
    {
        update_success_page_lock_t(asker,request,page_lock_infor);
        reply->reply_type = PageReplyType::LOCK_SUCCESS;
        return 1;
    }

    if(lock_compatibility_matrix[request->lock_type][page_lock_infor->lock_type])
    {
        update_success_page_lock_t(asker,request,page_lock_infor);
        reply->reply_type = PageReplyType::LOCK_SUCCESS;
        return 1;
    }
    else
    {
        update_fail_page_lock_t(asker,request,page_lock_infor);
        reply->reply_type = PageReplyType::LOCK_FAIL;
        return -1;
    }
}

/********************************
 * Private Member Functions
*********************************/


int RemotePageLockManager::insert_page_lock_t(const char * asker,PageLockRequest * request)
{
    PageLock_t * page_lock = new PageLock_t;
    page_lock->space_id = request->space_id;
    page_lock->page_no = request->page_no;
    page_lock->m_fold = cal_fold(page_lock->space_id,page_lock->page_no);
    page_lock->version_no = 0;
    page_lock->lock_type = request->lock_type;
    page_lock->lock_holder.push_back(asker);

    HASH_ADD_INT(page_lock_hash,m_fold,page_lock);
    return 1;
}

int RemotePageLockManager::update_unlock_page_lock_t(const char * asker,PageLockRequest * request,PageLock_t * page_lock)
{
    page_lock->lock_type = PageLockType::UNLOCK;
    page_lock->lock_holder.remove(asker);
    if(page_lock->lock_holder.empty())
    {
       // std::cout << page_lock->wait_list.size() <<  std::endl;
        // first waiter get lock
        if(!page_lock->wait_list.empty())
        {
              auto iter = page_lock->wait_list.begin();
            page_lock->lock_holder.push_back(iter->name);
            page_lock->lock_type = iter->type;
            page_lock->wait_list.erase(iter);
        }

    }
    return 1;
}

int  RemotePageLockManager::update_success_page_lock_t(const char * asker,PageLockRequest * request,PageLock_t * page_lock)
{
    if(request->lock_type == PageLockType::W_LOCK)
    {
        page_lock->version_no++;
    }
    page_lock->lock_type = request->lock_type;
    page_lock->lock_holder.push_back(asker);

    return 1;
}

int  RemotePageLockManager::update_fail_page_lock_t(const char * asker,PageLockRequest * request,PageLock_t * page_lock)
{
    bool found = false;
    for(auto iter = page_lock->wait_list.begin();iter != page_lock->wait_list.end();iter++)
    {
        if(iter->name == asker)
        {
            if(request->lock_type > iter->type)
            {
                iter->type = request->lock_type;
            }
            found = true;
        }
    }
    if(!found)
    {
        struct WaitInfor infor;
        infor.name = asker;
        infor.type = request->lock_type;
    }
    return 1;
}