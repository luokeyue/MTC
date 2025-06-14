#ifndef LIBEVENT_HANDLE_HEADER

#define LIBEVENT_HANDLE_HEADER


#include "network_handle.h"
#include <map>
#include <vector>

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>

#include <thread>
#include <atomic>
#include <condition_variable>

#define LIBEVENT_HANDLE_DEBUG  0


 /** callback function  **/
void connlistener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void * arg);
void listen_error_cb(struct evconnlistener *, void *);

void default_bufferevent_read_cb(struct bufferevent *bev, void *ctx);
void default_bufferevent_write_cb(struct bufferevent *bev, void *ctx);

/* Packet information */
struct BufferControlBlock
{
    int size;
};


/*bufferevent  and connection information*/
struct BevInfor
{
    std::string ip;
    int port;
    struct bufferevent * bev;
    bool is_listen;
    BufferControlBlock  block;
    bool cache_block;
    std::atomic<int> * read_singal_ptr;
    std::atomic<int> * write_singal_ptr;
};


class LibeventHandle:public NetworkHandle
{
    public:

    bool init_handle(int port);

    virtual bool init_handle()
    {
        return false;
    }

    virtual bool free_handle();

    bool send(const int id,const char * send_bytes,const int send_size);
    int wait_recive(const int id,char * recive_bytes,int sleep_interval=0);

    int recive_str_Wait(const int id,std::string & buffer_str,int sleep_interval=0);
    int recive_str_NoWait(const int id,std::string & buffer_str);

    int get_recive_buffer_length(const int id);
    void set_event_callback(NetworkHandle_CB cb,void * arg)
    {
        callback_funtion = cb;
        callback_args = arg;
    }

    int get_listen_connection_count();
    void get_listen_connection_array(int * array);

    int get_connection_id()
    {
        return -1;
    }

    int get_connection_id(const char * ip,const int port,bool tryConnect);

    void get_connection_ip(const int id,char * ip);

    int get_connection_port(const int id);

    int get_connection_count()
    {
        return bev_map.size();
    }

    LibeventHandle()
    {
        isFree.store(true);
    }

    ~LibeventHandle()
    {
        if(isFree.load() == false)
        {
           // std::cout << "Free Handle in Destructor Function"<<std::endl;
            free_handle();
        }
    }

    bool is_free()
    {
        return isFree;
    }

    bool is_init()
    {
        return !isFree;
    }

    private:
    bool init_listener();
    bool init_bufferevent();

    int try_connect(const char* ip,const int port);

    void start_event_base_loop();

    void set_connection_cb(int id,
    bufferevent_data_cb readcb = default_bufferevent_read_cb , bufferevent_data_cb writecb = default_bufferevent_write_cb,//default_bufferevent_write_cb,
    bufferevent_event_cb eventcb = NULL , void *cbarg = NULL);

    int add_bufferevent_connect(const char* ip,const int port);
    int add_bufferevent_listen(const char* ip,const int port,int socket_fd);
    int remove_buffevent(int id);

    int get_connection_id(struct bufferevent * bev);

    int readBufferControlBlock_Wait(struct BevInfor &,int sleep_interval);  // start -- lock
    int readBuffer_Wait(struct BevInfor &, char *data,int sleep_interval);  // end -- unlock
    int readBufferControlBlock_NoWait(struct BevInfor &);  // start -- lock
    int readBuffer_NoWait(struct BevInfor &, char *data); // end -- unlock
    bool writeBufferOnce(struct BevInfor &,const char * data,const int data_size);

    struct event_base *main_base;
    struct evconnlistener *conn_listener;

    std::atomic<bool> isFree;   //atomic

    int local_port;  // only modify in  inithandle()

    std::atomic<int> max_bev_id;  //atomic var

    std::atomic<int> bev_map_rw_lock_singal;  // read/write singal
    std::map<int,BevInfor> bev_map;

    std::atomic<int> listen_vector_rw_lock_singal;
    std::vector<int> listen_id_vector;

    NetworkHandle_CB callback_funtion;
    void * callback_args;

    std::thread * event_base_thread;


    /****************  friend function  **********************/
   // friend class EventMessageHandle;
    friend void event_loop_run(LibeventHandle * lib);

    friend void connlistener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * sock, int socklen, void * arg);

    friend void default_bufferevent_read_cb(struct bufferevent *bev, void *ctx);
    friend void default_bufferevent_write_cb(struct bufferevent *bev, void *ctx);

};


 /** thread task **/
void event_loop_run(LibeventHandle * lib);

/** rw lock **/
void rw_r_lock(std::atomic<int> & signal_int);
void rw_w_lock(std::atomic<int> & signal_int);
void rw_r_unlock(std::atomic<int> & signal_int);
void rw_w_unlock(std::atomic<int> & signal_int);

#endif // !LIBEVENT_HANDLE_HEADER