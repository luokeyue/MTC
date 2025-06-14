#ifndef LOCK_COMMON_HEADER
#define LOCK_COMMON_HEADER

#include <string>
#include<vector>
#include<list>
#include<map>
#include "uthash.h"

typedef unsigned long int ulint;

enum PageLockType
{
    R_LOCK = 0,
    W_LOCK = 1,
    UNLOCK
}; // unlock must be last; others will sort by lock level

enum PageReplyType
{
    LOCK_SUCCESS,
    LOCK_FAIL,
    UNLOCK_SUCCESS,
    UNLOCK_FAIL
};

/**
 * Version Update rule
 * 1. Version_no start from 0.It will increase but not reduce.
 * 2. When lock manager recive a write request about a page, it will add version_no of the page.
 * 3. Version_no is independent of any mysql node.
 *
 * Node will require a page from lock manager before requiring from storage.
 * **/

struct WaitInfor
{
    std::string name;
    PageLockType type;
};

/* Lock Table Item Information */
struct PageLock_t
{
    ulint m_fold;  /* hash key */
    uint32_t space_id;
    uint32_t page_no;
    uint64_t version_no;
    PageLockType lock_type;
   // const char * page_holder; //page location
    std::list<std::string> lock_holder;
    std::list<WaitInfor> wait_list;
    UT_hash_handle hh;
};

/* Lock Request
1. Write Page : send write lock request;
2. Read Page : if the page is in local buffer pool,mysql node will not send lock request and read local page; (Log sync mechanism will update page later)
*/
struct PageLockRequest
{
    uint32_t space_id;
    uint32_t page_no;
    uint64_t version_no; // if not execute writing operation on latest page
    PageLockType lock_type;
};

/* Lock reply*/
struct PageLockReply
{
    uint32_t space_id;
    uint32_t page_no;
//    const char * page_holder;
    PageReplyType reply_type;
};


#define REMOTE_LOCK_GROUP_NAME "centralized_lock"
#define REMOTE_LOCK_REQUEST_MESS_TYPE "PageLockRequest"
#define REMOTE_LOCK_REPLY_MESS_TYPE "PageLockReply"
#endif
