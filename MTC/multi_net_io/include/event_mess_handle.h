#ifndef EVENT_MESS_HANDLE_HEADER
#define EVENT_MESS_HANDLE_HEADER

#include "libevent_handle.h"

#ifdef __cplusplus

#include <cstddef>
#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson {
typedef ::std::size_t SizeType;
}
#endif

//#include <rapidjson/rapidjson.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <list>

//#define EVENT_MESS_HANDLE_DEBUG 1

namespace MessageError
{
    extern const char *EVENT_ERROR_STRING[];
    enum EventMessageErrorNo
    {
        SUCCESS = 0,
        INCOMPLETE_MESSAGE = 1,
        GROUP_NOT_FOUND=2,
        MESS_TYPE_NOT_FOUND = 3,
        NONE_UNPROCESSED_MESSAGE=4,
        INVALID_HOST=5,
        NONE_REQUESTED_MESSAGE=6
    };

    const char *getEventErrorStr(EventMessageErrorNo no);
} // namespace MessageError

struct EventMessageCursor
{
    uint32_t group_name_offset;
    uint32_t mess_type_offset;
    uint32_t send_host_name_offset;
    uint32_t recive_host_name_offset;
    uint32_t message_offset;
    uint32_t message_size;

    void clear()
    {
        group_name_offset=0;
        mess_type_offset=0;
        send_host_name_offset=0;
        recive_host_name_offset=0;
        message_offset=0;
    }
};

class EventMessage
{
    public:
    // Public API
    //-------------------------------------------------------------------------------------------------------------------------
    void  prepare_send(const char * g_name,const char * m_type,
        const char * r_host,const char * mess,uint32_t mess_size)
         {
            group_name = g_name;
            mess_type = m_type;
//            send_host_name = s_host;
            recive_host_name = r_host;
            message = mess,
            message_size = mess_size;
            error_no = MessageError::EventMessageErrorNo::SUCCESS;
            will_send = true;
            will_recive = false;
        }

    void  prepare_recive(const char * g_name,const char * msg_type)
    {
        group_name = g_name;
        mess_type = msg_type;
        will_send = false;
        will_recive = true;
    }

    void clear()
    {
        group_name = mess_type = send_host_name = recive_host_name = 0;
        message = 0;
        message_size = 0;
        cursor=0;
        error_no=MessageError::EventMessageErrorNo::SUCCESS;
        buffer_str = "";
        buffer_size = 0;
        will_send = false;
        will_recive = false;
    }

    const char  * group_name;
    const char  * mess_type;
    const char  * send_host_name;
    const char  * recive_host_name;
    const char  * message;
    uint32_t message_size;

    // read only
    MessageError::EventMessageErrorNo error_no;
    bool will_send;
    bool will_recive;


    //-------------------------------------------------------------------------------------------------------------------------

    friend class EventMessageHandle;
    EventMessage()
    {
        clear();
    }

    void copy(const EventMessage & src_mess);

    EventMessage(const EventMessage & other)
    {
        this->copy(other);
    }

    const EventMessage & operator=(const EventMessage & other)
    {
        this->copy(other);
    }

    private:

    struct EventMessageCursor * cursor;
    std::string buffer_str;
    uint32_t buffer_size;


    void init_buffer_str(); //when you want to send
    void init_message_ptr(); //when you recive a message string
};

class EventMessageFilter
{
    public:
    virtual bool operator()(EventMessage &)=0;
};

/**
 * 1. Read ip and host name from json file.  ------  host_config.json
 * 2. Declare Message Group and Message Type in json file. ------ message_type.json
 * **/
class EventMessageHandle //:public MessageHandle
{
public:
    typedef void (*EventMessageHandle_RECIVE_CB)(EventMessageHandle * mess_handle,EventMessage * mess,void * arg);

    struct CallbackInfor
    {
        EventMessageHandle_RECIVE_CB cb_function;
        void * cb_arg;
        std::list<EventMessage> unprocessed_mess_list;
    };

    struct GroupInfor
    {
       // int index;
        LibeventHandle handle;
        std::map<std::string,int> host_port_map;  //host_name <-> port
        std::map<std::string,struct CallbackInfor>  mess_callback_map;  // mess_type <-> CallbackInfor
        //std::list<EventMessage>  unprocessed_mess_list;
    };

    EventMessageHandle()
    {
        this_free = true;
    }

    ~EventMessageHandle()
    {
        free_handle();
    }

    // Public API
    //-------------------------------------------------------------------------------------------------------------------------
    /*init or free handle object*/
    bool init_handle(const char *host_config_path, const char *mess_config_path);
    bool free_handle();
    bool is_free();
    bool is_init();

    //register handle function about message
    bool register_recive_handler(const char * group_name, const char * mess_type,EventMessageHandle_RECIVE_CB cb,void * arg);

    //recive message
    int readMessage(EventMessage *mess); //message size
    int readMessage(EventMessage *mess,EventMessageFilter & filter); //message filter

    //get unprocessed message count
    uint32_t get_unprocessed_message_count(const char * group_name,const char * mess_type);

    //send message
    int sendMessage(EventMessage *mess);

    //-------------------------------------------------------------------------------------------------------------------------

    private:
    void read_config(const char *host_config_path, const char *mess_config_path);

    void init_group_infor_map(rapidjson::Document & d);
   // bool add_group_infor(rapidjson::Document & d,int group_index, int port);


    const char * get_ip(const char * hostname);
    int get_port(const char * hostname,const char * group_name);
    LibeventHandle * get_libeventhandle(const char * group_name);

    bool check_mess_type(EventMessage * mess_ptr);

    //try callback for message
    bool try_run_callback(EventMessage *);

    //read from unprocessed message list
    //bool read_from_unprocessed_list(GroupInfor * infor,EventMessage * mess);

    //read Messgae from libevent
    bool read_callback_message_from_libevent(int connect_id,LibeventHandle * handle);

    const int FILE_IO_BUFFER_SIZE = 8 * 1024;

    bool this_free;

    //host map
    std::string local_host_name;
    std::map<std::string, std::string> host_config_map; // <host_name,ip>

    //group map
    //rapidjson::Document mess_config_doc;
    std::map<std::string, GroupInfor *> mess_group_map; // <group_name,group_infor>

    friend void libevent_callback(NET_EVENT event_id,NetworkHandle * handle,int connect_id,void * arg);
};


void libevent_callback(NET_EVENT event_id,NetworkHandle * handle,int connect_id,void * arg);
#endif // !