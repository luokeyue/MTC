#include "event_mess_handle.h"

const char *MessageError::getEventErrorStr(EventMessageErrorNo no)
{
    return EVENT_ERROR_STRING[no];
}

const char *MessageError::EVENT_ERROR_STRING[] = {
    "The event message has none error",
    "The message is incomplete",
    "Can't found this message group",
    "Can't found this message type",
    "No outstanding messages",
    "Invalid host",
    "None requested messages"
};

/***********************************
 * class  EventMessage
 * *******************************/
void EventMessage::copy(const EventMessage &src_mess)
{
    buffer_str = src_mess.buffer_str;
    buffer_size = src_mess.buffer_size;
    init_message_ptr();
}

 #define  MESSAGE_CURSOR_PTR  ((EventMessageCursor *)buffer_str.c_str())
void EventMessage::init_buffer_str()
{
    buffer_str = "";
    buffer_size = 0;

    buffer_str.append(sizeof(EventMessageCursor), 0);

    buffer_size += sizeof(EventMessageCursor);
    MESSAGE_CURSOR_PTR->group_name_offset = buffer_size;
    buffer_str.append( strlen(group_name)+1,0);
    memcpy((void *)buffer_str.c_str()+buffer_size,group_name,strlen(group_name));
//std::cout << "group name buffer ==== " << buffer_str.c_str()+buffer_size << std::endl;
    buffer_size += strlen(group_name) + 1;

    MESSAGE_CURSOR_PTR->mess_type_offset = buffer_size;
    buffer_str.append(strlen(mess_type)+1,0);
    memcpy((void *)buffer_str.c_str()+buffer_size,mess_type,strlen(mess_type));
//std::cout << "mess type buffer ==== " << buffer_str.c_str()+buffer_size << std::endl;
    buffer_size += strlen(mess_type) + 1;


    MESSAGE_CURSOR_PTR->send_host_name_offset = buffer_size;
    buffer_str.append(strlen(send_host_name)+1,0);
    memcpy((void *)buffer_str.c_str()+buffer_size,send_host_name,strlen(send_host_name));
    buffer_size += strlen(send_host_name) + 1;

    MESSAGE_CURSOR_PTR->recive_host_name_offset = buffer_size;
    buffer_str.append(strlen(recive_host_name)+1,0);
    memcpy((void *)buffer_str.c_str()+buffer_size,recive_host_name,strlen(recive_host_name));
    buffer_size += strlen(recive_host_name) + 1;

    MESSAGE_CURSOR_PTR->message_offset = buffer_size;
    MESSAGE_CURSOR_PTR->message_size = message_size;
    buffer_str.append(message_size,0);
    memcpy((void *)buffer_str.c_str()+buffer_size,message,message_size);
    buffer_size += message_size;

    cursor = MESSAGE_CURSOR_PTR;
   // std::cout <<"prepare send ====== "<<  buffer_str << " & " << cursor->send_host_name_offset  << std::endl;

 //   std::cout << buffer_size << " | " << buffer_str.size() << std::endl;
}

void EventMessage::init_message_ptr()
{
  //   std::cout << buffer_size  <<" | " << sizeof(EventMessageCursor)<< std::endl;
    if (buffer_size <= sizeof(EventMessageCursor))
        return;
// std::cout << "Group Name : " << (int64_t)group_name  <<" | " << (int64_t)buffer_str.c_str() << " | " << cursor->group_name_offset << std::endl;
    cursor = (EventMessageCursor *)buffer_str.c_str();
    group_name = (const char *)(buffer_str.c_str() + cursor->group_name_offset);
    mess_type = (const char *)(buffer_str.c_str() + cursor->mess_type_offset);
    send_host_name = (const char *)(buffer_str.c_str() + cursor->send_host_name_offset);
    recive_host_name = (const char *)(buffer_str.c_str() + cursor->recive_host_name_offset);
    message = (const char *)(buffer_str.c_str() + cursor->message_offset);
    message_size = cursor->message_size;
  //  std::cout <<"prepare receive ====== "<<  buffer_str << " & " << cursor->send_host_name_offset << std::endl;
}

/***********************************
 * class  EventMessageHandle
 * *******************************/

bool EventMessageHandle::init_handle(const char *host_config_path, const char *mess_config_path)
{
    if (this_free == true)
        this_free = false;
    else
        return false;

    read_config(host_config_path, mess_config_path);

#ifdef EVENT_MESS_HANDLE_DEBUG
    std::cout <<"LOCAL HOST NAME" << " : " << local_host_name << std::endl;
#endif // DEBUG
    //init_group_infor_map();
}

bool EventMessageHandle::free_handle()
{
    if (this_free == false)
        this_free = true;
    else
        return false;
    //mess_config_doc.Clear();

    for (auto it = mess_group_map.begin(); it != mess_group_map.end(); it++)
    {
        delete it->second;
    }
}

bool EventMessageHandle::is_free()
{
    return this_free;
}

bool EventMessageHandle::is_init()
{
    return !this_free;
}


bool EventMessageHandle::register_recive_handler(const char * group_name, const char * mess_type,EventMessageHandle_RECIVE_CB cb,void * arg)
{
    EventMessage mess;
    mess.group_name = group_name;
    mess.mess_type = mess_type;
    if(!check_mess_type(&mess))
    {
        return false;
    }

    CallbackInfor * cb_infor_ptr = &mess_group_map[group_name]->mess_callback_map[mess_type];

    cb_infor_ptr->cb_function = cb;
    cb_infor_ptr->cb_arg = arg;

    return true;
}

int EventMessageHandle::readMessage(EventMessage *mess_ptr)
{
    if(!mess_ptr->will_recive)
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::INCOMPLETE_MESSAGE;
        return -1;
    }

    if(!check_mess_type(mess_ptr))
    {
        //mess_ptr->error_no =  MessageError::EventMessageErrorNo::
        return -1;
    }

    auto & mess_type_map = mess_group_map[mess_ptr->group_name]->mess_callback_map;
    if(mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.empty())
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::NONE_UNPROCESSED_MESSAGE;
        return 0;
    }

    mess_ptr->copy(mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.front());
    mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.erase(mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.begin());
    return 1;
}

int EventMessageHandle::readMessage(EventMessage *mess_ptr,EventMessageFilter & filter)
{
    if(!mess_ptr->will_recive)
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::INCOMPLETE_MESSAGE;
        return -1;
    }

    if(!check_mess_type(mess_ptr))
    {
        //mess_ptr->error_no =  MessageError::EventMessageErrorNo::
        return -1;
    }

    auto & mess_type_map = mess_group_map[mess_ptr->group_name]->mess_callback_map;
    if(mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.empty())
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::NONE_UNPROCESSED_MESSAGE;
        return 0;
    }

    auto iter = mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.begin();
    for(;iter != mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.end();iter++)
    {
        if(filter(*iter))
        {
            mess_ptr->copy(*iter);
            mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.erase(iter);
            return 1;
        }
    }
    mess_ptr->error_no =  MessageError::EventMessageErrorNo::NONE_REQUESTED_MESSAGE;
    return 0;
}

uint32_t EventMessageHandle::get_unprocessed_message_count(const char * group_name,const char * mess_type)
{
    EventMessage mess;
    mess.group_name = group_name;
    mess.mess_type = mess_type;
    if(!check_mess_type(&mess))
    {
        return 0;
    }

    return mess_group_map[group_name]->mess_callback_map[mess_type].unprocessed_mess_list.size();
}

int EventMessageHandle::sendMessage(EventMessage *mess_ptr)
{
    if(!mess_ptr->will_send)
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::INCOMPLETE_MESSAGE;
        return -1;
    }

    if(!check_mess_type(mess_ptr))
    {
        return -1;
    }

    LibeventHandle & event_handle = mess_group_map[mess_ptr->group_name]->handle;

    const char *  ip = get_ip(mess_ptr->recive_host_name);
    int port = get_port(mess_ptr->recive_host_name,mess_ptr->group_name);

    if(ip == NULL || port == 0)
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::INVALID_HOST;
        return -1;
    }

#ifdef EVENT_MESS_HANDLE_DEBUG
    std::cout << ip << " : " << port << std::endl;
#endif // DEBUG

    mess_ptr->send_host_name = local_host_name.c_str();
    mess_ptr->init_buffer_str();

    int connect_id = event_handle.get_connection_id(ip,port,true);

    bool return_bool = event_handle.send(connect_id,mess_ptr->buffer_str.c_str(),mess_ptr->buffer_size);

#ifdef EVENT_MESS_HANDLE_DEBUG
    std::cout << "Libevent Send Return "<< return_bool << std::endl;
    std::cout << "Send Buffer : "<< mess_ptr->buffer_str << std::endl;
    std::cout << "Send Size : "<< mess_ptr->buffer_str.size() << std::endl;
#endif // DEBUG
    return 1;
}

void EventMessageHandle::read_config(const char *host_config_path, const char *mess_config_path)
{
    char readbuffer[FILE_IO_BUFFER_SIZE];

    //read host config
    rapidjson::Document host_document;
    FILE *host_file = fopen(host_config_path, "r");
    assert(host_file != NULL);
    memset(readbuffer, 0, FILE_IO_BUFFER_SIZE);
    rapidjson::FileReadStream read_stream1(host_file, readbuffer, FILE_IO_BUFFER_SIZE);
    host_document.ParseStream(read_stream1);
    fclose(host_file);
    // init host map
    for (auto &host : host_document.GetArray())
    {
        const char *host_name = host["name"].GetString();
        const char *host_ip = host["ip"].GetString();
        bool is_local = host["is_local"].GetBool();
        if (is_local)
        {
            local_host_name = host_name;
        }
        host_config_map[host_name] = host_ip;
    }

    // read message config
    rapidjson::Document mess_config_doc;
    FILE *config_file = fopen(mess_config_path, "r");
    assert(config_file != NULL);
    memset(readbuffer, 0, FILE_IO_BUFFER_SIZE);
    rapidjson::FileReadStream read_stream2(config_file, readbuffer, FILE_IO_BUFFER_SIZE);
    mess_config_doc.ParseStream(read_stream2);
    fclose(config_file);

    init_group_infor_map(mess_config_doc);
}

void EventMessageHandle::init_group_infor_map(rapidjson::Document &mess_config_doc)
{
    int len = mess_config_doc.Size();
    for (int i = 0; i < len; i++)
    {
        const char *group_name = mess_config_doc[i]["group_name"].GetString();
        int local_port = 0; //mess_config_doc[i]["hosts"][local_host_name.c_str()]["port"].GetInt();
        GroupInfor * group_ptr = new GroupInfor;
        mess_group_map[group_name] = group_ptr;

        for (auto &host_item : mess_config_doc[i]["hosts"].GetArray())
        {
            group_ptr->host_port_map[host_item["name"].GetString()] = host_item["port"].GetInt();
            // std::cout  << host_item["name"].GetString() << " : " << host_item["port"].GetInt()<< std::endl;
        }

        local_port = group_ptr->host_port_map[local_host_name];
        group_ptr->handle.init_handle(local_port);
        group_ptr->handle.set_event_callback(libevent_callback,this);

#ifdef EVENT_MESS_HANDLE_DEBUG
    std::cout << "Listen Port ："<<local_port << std::endl;
#endif // DEBUG

        for (auto &mess_type : mess_config_doc[i]["mess_type"].GetArray())
        {
            CallbackInfor cb_infor;
            group_ptr->mess_callback_map[mess_type["name"].GetString()] = cb_infor;

            CallbackInfor *cb_infor_ptr = &group_ptr->mess_callback_map[mess_type["name"].GetString()];

            cb_infor_ptr->cb_function = NULL;

            cb_infor_ptr->cb_arg = NULL;

            cb_infor_ptr->unprocessed_mess_list.clear();

            // std::cout << (group_ptr->mess_callback_map.find(mess_type["name"].GetString()) != group_ptr->mess_callback_map.end() )<< std::endl;
        }

        // #ifdef  EVENT_MESS_HANDLE_DEBUG
        // std::cout << "----------------- Message Callback Map Debug ------------------- " << std::endl;
        // for(auto it = group_ptr->mess_callback_map.begin(); it!=group_ptr->mess_callback_map.end(); it++)
        // {
        // }
        // #endif //
    }
}

const char *EventMessageHandle::get_ip(const char *hostname)
{
    auto iterator = host_config_map.find(hostname);
    if (iterator == host_config_map.end())
    {
        return NULL;
    }
    else
    {
        return iterator->second.c_str();
    }
}

int EventMessageHandle::get_port(const char *hostname, const char *group_name)
{
    auto iterator = mess_group_map.find(group_name);
    if (iterator == mess_group_map.end())
    {
        return 0;
    }
    else
    {
        return mess_group_map[group_name]->host_port_map[hostname];
    }
}

LibeventHandle *EventMessageHandle::get_libeventhandle(const char *group_name)
{
    auto iterator = mess_group_map.find(group_name);
    if (iterator == mess_group_map.end())
    {
        return NULL;
    }
    else
    {
        return &iterator->second->handle;
    }
}

bool EventMessageHandle::check_mess_type(EventMessage * mess_ptr)
{
   // std::cout << "STOP & "<<(int64_t)mess_ptr->mess_type << std::endl;
   #ifdef EVENT_MESS_HANDLE_DEBUG
   //std::cout << "Check Mess Type : " << "group name = "<<mess_ptr->group_name << " & " << "message type = " << mess_ptr->mess_type << std::endl;
   #endif // DEBUG
    if(mess_group_map.find(mess_ptr->group_name) == mess_group_map.end())
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::GROUP_NOT_FOUND;
        return false;
    }
    auto & mess_type_map = mess_group_map[mess_ptr->group_name]->mess_callback_map;
    if(mess_type_map.find(mess_ptr->mess_type) == mess_type_map.end())
    {
        mess_ptr->error_no =  MessageError::EventMessageErrorNo::MESS_TYPE_NOT_FOUND;
        return false;
    }
    return true;
}

bool EventMessageHandle::try_run_callback(EventMessage * mess_ptr)
{
    //std::cout << "************* try callback" << std::endl;
    if(!check_mess_type(mess_ptr))
    {
        return false;
    }

    CallbackInfor * cb_infor_ptr = & mess_group_map[mess_ptr->group_name]->mess_callback_map[mess_ptr->mess_type];
  //  std::cout << "************* cb_function" << (int64_t)cb_infor_ptr->cb_function <<" ; " << cb_infor_ptr->unprocessed_mess_list.size() <<std::endl;
    if(cb_infor_ptr->cb_function != NULL)
    {
        (*cb_infor_ptr->cb_function)(this,mess_ptr,cb_infor_ptr->cb_arg);
    }
    else
    {
        cb_infor_ptr->unprocessed_mess_list.push_back(*mess_ptr);
    }
    return true;
}

bool EventMessageHandle::read_callback_message_from_libevent(int connect_id,LibeventHandle *handle_ptr)
{
    int count =0;
     int recive_size =0;
    while(handle_ptr->get_recive_buffer_length(connect_id) > 0){
        //std:: cout << "while Start Recive Buffer Length: "  << handle_ptr->get_recive_buffer_length(connect_id) << std::endl;
        // read one message from libevent
        EventMessage mess;
        #ifdef EVENT_MESS_HANDLE_DEBUG
        std::cout << "Recive String " << std::endl;
        #endif
        recive_size = handle_ptr->recive_str_NoWait(connect_id,mess.buffer_str); // run in callback function,not wait
        #ifdef EVENT_MESS_HANDLE_DEBUG
        std::cout << "Recive Size "<< recive_size << std::endl;
        #endif
        std::cout << "Recive Size "<< recive_size << std::endl;
        if(recive_size <= 0)
        {
            return false;
        }
        mess.buffer_size = recive_size;
        #ifdef EVENT_MESS_HANDLE_DEBUG
        std::cout << "Recive Buffer : " << mess.buffer_str << std::endl;
        std::cout << "Recive Buffer SIZE : " << mess.buffer_str.size() << std::endl;
        #endif // DEBUG

        mess.init_message_ptr();

        // try run some callback
        if(!try_run_callback(&mess))
        {
            return false;
        }
       // std:: cout << "while count = " << count++ << std::endl;
    }
     //std::cout << "++++++++++ Recive Buffer Length: " << handle_ptr->get_recive_buffer_length(connect_id) << std::endl;
    return true;
}

void libevent_callback(NET_EVENT event_id, NetworkHandle *handle, int connect_id, void *arg)
{
    LibeventHandle *event_handle = (LibeventHandle *)handle;
    EventMessageHandle *mess_handle = (EventMessageHandle *)arg;
    //read message from libevent
    //get mess_type in message_type
    //call mess_handle->try_run_callback
    //std::cout << "libevent callback in event message" << std::endl;

    mess_handle->read_callback_message_from_libevent(connect_id,event_handle);

    //std::cout << event_handle->get_recive_buffer_length(connect_id) << std::endl;
}