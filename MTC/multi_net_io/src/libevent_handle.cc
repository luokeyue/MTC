#include "libevent_handle.h"

bool LibeventHandle::init_handle(int port)
{
    isFree.store(false);

    main_base = event_base_new();
    if (!main_base)
    {
        fprintf(stderr, "Could not initialize libevent!\n");
        return false;
    }
    local_port = port;

    max_bev_id.store(0);
    bev_map_rw_lock_singal.store(0);
    listen_vector_rw_lock_singal.store(0);

    callback_funtion = NULL;

    init_listener();

    event_base_thread = new std::thread(event_loop_run, this);

    return true;
}

bool LibeventHandle::free_handle()
{
    if (isFree.load() == true)
        return false;

    isFree.store(true);

    event_base_loopexit(main_base, NULL);
    free(event_base_thread);
    evconnlistener_free(conn_listener);
    rw_w_lock(bev_map_rw_lock_singal);
    //std::cout <<"Free Bev Map Size ==== " << bev_map.size() << std::endl;
    while(bev_map.empty())
    {
        auto it = bev_map.begin();
        int id = it->first;
        remove_buffevent(id);
    }
    bev_map.clear();
    listen_id_vector.clear();
    rw_w_unlock(bev_map_rw_lock_singal);

    event_base_free(main_base);
    return isFree.load();
}

bool LibeventHandle::init_listener()
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(local_port);
    sin.sin_addr.s_addr = htonl(0);

    conn_listener = evconnlistener_new_bind(main_base, &connlistener_cb, this,
                                            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE | LEV_OPT_REUSEABLE,
                                            -1, (struct sockaddr *)&sin, sizeof(sin));

    if (!conn_listener)
    {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
        std::cout << "conn_listener address = " << conn_listener << std::endl;
#endif // DEBUG
        return false;
    }
    evconnlistener_set_error_cb(conn_listener, listen_error_cb);
    evconnlistener_enable(conn_listener);

#if LIBEVENT_HANDLE_DEBUG
    std::cout << "evconnlistener_get_fd = " << evconnlistener_get_fd(conn_listener) << std::endl;
#endif // DEBUG0

    return true;
}

int LibeventHandle::add_bufferevent_connect(const char *ip, const int port)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port = htons(port);

    struct bufferevent *bev = bufferevent_socket_new(main_base, -1, BEV_OPT_CLOSE_ON_FREE); //| BEV_OPT_THREADSAFE);

    if (bev == NULL)
    {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
        std::cout << "bev address = " << bev << std::endl;
#endif // DEBUG0
        return -1;
    }

    int re;
    if (re = bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        std::cout << "connect to " << ip << ":" << htons(port) << "failure !" << std::endl;
#endif // DEBUG
        bufferevent_free(bev);
        return -1;
    }
#if LIBEVENT_HANDLE_DEBUG
    std::cout << "bufferevent_socket_connect :" << re << std::endl;
    std::cout << "bufferevent_getfd :" << bufferevent_getfd(bev) << std::endl;
    std::cout << "connect to " << ip << ":" << htons(port) << " success !" << std::endl;
#endif // DEBUG

    //bufferevent_setcb();

    struct BevInfor peer;
    peer.ip = ip;
    peer.port = port;
    peer.bev = bev;
    peer.is_listen = false;
    peer.read_singal_ptr = new std::atomic<int>{0};
    peer.write_singal_ptr = new std::atomic<int>{0};

    int peer_id = max_bev_id.load();
    max_bev_id.fetch_add(1);

    rw_w_lock(bev_map_rw_lock_singal);
    bev_map[peer_id] = peer;
    rw_w_unlock(bev_map_rw_lock_singal);

    set_connection_cb(peer_id);
    return peer_id;
}

int LibeventHandle::add_bufferevent_listen(const char *ip, const int port, int socket_fd)
{
    struct bufferevent *bev = bufferevent_socket_new(main_base, socket_fd, BEV_OPT_CLOSE_ON_FREE); //| BEV_OPT_THREADSAFE);
    //bufferevent_setcb(bev,default_bufferevent_read_cb,default_bufferevent_write_cb,NULL,this);
    //bufferevent_setwatermark(bev,EV_READ,4,512);

    struct BevInfor peer;
    peer.ip = ip;
    peer.port = port;
    peer.bev = bev;
    peer.is_listen = true;
    peer.read_singal_ptr = new std::atomic<int>{0};
    peer.write_singal_ptr = new std::atomic<int>{0};

    int peer_id = max_bev_id.load();
    max_bev_id.fetch_add(1);
    //  std::cout << "listen singal :" <<bev_map_rw_lock_singal.load()  <<port << std::endl;
    rw_w_lock(bev_map_rw_lock_singal);
    bev_map[peer_id] = peer;
    rw_w_unlock(bev_map_rw_lock_singal);

    set_connection_cb(peer_id);

    rw_w_lock(listen_vector_rw_lock_singal);
    listen_id_vector.push_back(peer_id);
    rw_w_unlock(listen_vector_rw_lock_singal);
    //std::cout << "listen id" << peer_id << std::endl;
    return peer_id;
}

int LibeventHandle::remove_buffevent(int id)
{
    rw_w_lock(bev_map_rw_lock_singal);
    if (bev_map.find(id) == bev_map.end())
    {
        rw_w_unlock(bev_map_rw_lock_singal);
        return -1;
    }
    BevInfor info = bev_map[id];
    bufferevent_free(info.bev);
    delete info.read_singal_ptr;
    delete info.write_singal_ptr;

    if (info.is_listen)
    {
        rw_w_lock(listen_vector_rw_lock_singal);
        listen_id_vector.erase(listen_id_vector.begin() + id);
        rw_w_unlock(listen_vector_rw_lock_singal);
    }

    bev_map.erase(id);
    rw_w_unlock(bev_map_rw_lock_singal);
    return id;
}

int LibeventHandle::try_connect(const char *ip, const int port)
{

    int connect_id = -1;
    // connect_id = get_connection_id(ip,port);
    if (connect_id >= 0)
    {
        return connect_id;
    }

    connect_id = add_bufferevent_connect(ip, port);

    return connect_id;
}

void LibeventHandle::start_event_base_loop()
{
#if LIBEVENT_HANDLE_DEBUG
    event_base_dispatch(main_base);
#else
    event_base_dispatch(main_base);
    // event_base_loop(main_base,EVLOOP_NO_EXIT_ON_EMPTY);
#endif
}

bool LibeventHandle::send(const int connect_id, const char *send_bytes, const int send_size)
{

    if (connect_id < 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        std::cout << "send error ---- connect failure" << std::endl;
#endif // DEBUG
        return false;
    }

    rw_r_lock(bev_map_rw_lock_singal);
    if (bev_map.find(connect_id) == bev_map.end())
        return false;
    struct BevInfor &info = bev_map[connect_id];
    rw_r_unlock(bev_map_rw_lock_singal);

    writeBufferOnce(info, send_bytes, send_size);
    //std::cout << "STOP!!!" <<std::endl;
    return true;
}

int LibeventHandle::wait_recive(const int connect_id, char *recive_bytes,int sleep_interval)
{
    rw_r_lock(bev_map_rw_lock_singal);
    if (bev_map.find(connect_id) == bev_map.end())
    {
        rw_r_unlock(bev_map_rw_lock_singal);
        return -1;
    }
    struct BevInfor &info = bev_map[connect_id];
    rw_r_unlock(bev_map_rw_lock_singal);

   readBufferControlBlock_Wait(info,sleep_interval);
   return readBuffer_Wait(info,recive_bytes,sleep_interval);
}

 int LibeventHandle::recive_str_Wait(const int connect_id,std::string & buffer_str,int sleep_interval)
 {
     rw_r_lock(bev_map_rw_lock_singal);
    if (bev_map.find(connect_id) == bev_map.end())
    {
        rw_r_unlock(bev_map_rw_lock_singal);
        return -1;
    }
    struct BevInfor &info = bev_map[connect_id];
    rw_r_unlock(bev_map_rw_lock_singal);

    readBufferControlBlock_Wait(info,sleep_interval);
    if(buffer_str.length() <= info.block.size)
        buffer_str.append(info.block.size,0);
    return readBuffer_Wait(info,(char *)buffer_str.c_str(),sleep_interval);
 }

int LibeventHandle::recive_str_NoWait(const int connect_id, std::string &buffer_str)
{
    rw_r_lock(bev_map_rw_lock_singal);
    if (bev_map.find(connect_id) == bev_map.end())
    {
        rw_r_unlock(bev_map_rw_lock_singal);
        return -1;
    }
    struct BevInfor &info = bev_map[connect_id];
    rw_r_unlock(bev_map_rw_lock_singal);


    if (get_recive_buffer_length(connect_id) <= sizeof(BufferControlBlock))
    {
        return 0;
    }
    if(!info.cache_block)
        readBufferControlBlock_NoWait(info);
    if(buffer_str.length() <= info.block.size)
        buffer_str.append(info.block.size,0);
    return readBuffer_NoWait(info,(char *)buffer_str.c_str());
}

int LibeventHandle::get_recive_buffer_length(const int connect_id)
{
    //std::cout << "get_recive_buffer_length listen singal :" <<bev_map_rw_lock_singal.load()  << std::endl;
    rw_r_lock(bev_map_rw_lock_singal);
    if (bev_map.find(connect_id) == bev_map.end())
    {
        rw_r_unlock(bev_map_rw_lock_singal);
        return false;
    }
    struct BevInfor &info = bev_map[connect_id];
    rw_r_unlock(bev_map_rw_lock_singal);

    rw_r_lock(*(info.read_singal_ptr));
    evbuffer *input = bufferevent_get_input(info.bev);
    int len = evbuffer_get_length(input);
    rw_r_unlock(*(info.read_singal_ptr));
    return len;
}

int LibeventHandle::get_listen_connection_count()
{
    rw_r_lock(listen_vector_rw_lock_singal);

    int len = listen_id_vector.size();

    rw_r_unlock(listen_vector_rw_lock_singal);

    return len;
}

void LibeventHandle::get_listen_connection_array(int *array)
{
    rw_r_lock(listen_vector_rw_lock_singal);
    memcpy(array, &listen_id_vector[0], listen_id_vector.size() * sizeof(int));
    rw_r_unlock(listen_vector_rw_lock_singal);
}

void LibeventHandle::set_connection_cb(int id,
                                       bufferevent_data_cb readcb, bufferevent_data_cb writecb,
                                       bufferevent_event_cb eventcb, void *cbarg)
{
    struct bufferevent *bev = bev_map[id].bev;

    bufferevent_disable(bev, EV_READ | EV_WRITE);

    if (cbarg == NULL)
        cbarg = this;
    bufferevent_setcb(bev, readcb, writecb, eventcb, cbarg);

    bufferevent_enable(bev, EV_READ | EV_WRITE);

#if LIBEVENT_HANDLE_DEBUG
    std::cout << "set default callback success " << std::endl;
#endif
}

int LibeventHandle::readBufferControlBlock_Wait(struct BevInfor & info,int sleep_interval)
{
   rw_w_lock(*(info.read_singal_ptr));
   evbuffer *input = bufferevent_get_input(info.bev);
   int len = evbuffer_get_length(input);
   //rw_w_unlock(*(info.read_singal_ptr));
   while ( len <sizeof(BufferControlBlock) )
   {
       sleep(sleep_interval);
       len = evbuffer_get_length(input);
   }

   int read_size = 0;
   info.cache_block = true;
   memset(&info.block, 0, sizeof(BufferControlBlock));

   char * block_buffer = (char *)&info.block;
   read_size = bufferevent_read(info.bev,block_buffer , sizeof(BufferControlBlock));
   if (read_size < 0)
   {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "bufferevent_read error --- "
                  << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
#endif // DEBUG
        rw_w_unlock(*(info.read_singal_ptr));
        return -1;
   }
 //  rw_w_unlock(*(info.read_singal_ptr));
   return read_size;
}

int LibeventHandle::readBuffer_Wait(struct BevInfor & info, char *data,int sleep_interval)
{
    if(!info.cache_block) return -1;
    int max_size = info.block.size;
    int read_size = 0;
#if LIBEVENT_HANDLE_DEBUG
    std::cout << "Buffer control block Size ===== " <<max_size << std::endl;
#endif //
   // rw_w_lock(*(info.read_singal_ptr));
    int len = 0;
    evbuffer *input = bufferevent_get_input(info.bev);
    do
    {
#if LIBEVENT_HANDLE_DEBUG
        std::cout << "============ wait recive evbuffer len :" <<len << std::endl;
#endif // DEBUG
        sleep(sleep_interval);
        len = evbuffer_get_length(input);
    }while(len < max_size);

    read_size = bufferevent_read(info.bev, data, max_size);
    if (read_size < 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "bufferevent_read error --- "
                    << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
#endif // DEBUG
        rw_w_unlock(*(info.read_singal_ptr));
        return -1;
    }
    rw_w_unlock(*(info.read_singal_ptr));
    info.cache_block = false;
    return read_size;
}

int LibeventHandle::readBufferControlBlock_NoWait(struct BevInfor & info )
{
    rw_w_lock(*(info.read_singal_ptr));

    evbuffer *input = bufferevent_get_input(info.bev);
    int len = evbuffer_get_length(input);
    if( len <sizeof(BufferControlBlock) )
    {
    #if LIBEVENT_HANDLE_DEBUG
        std::cout<< "no enough evbuffer space for read BufferControlBlock" << std::endl;
    #endif // DEBUG
        rw_w_unlock(*(info.read_singal_ptr));
        return 0;
    }

    int read_size = 0;
    info.cache_block = true;
    memset(&info.block, 0, sizeof(BufferControlBlock));
    char * block_buffer = (char *)&info.block;
    read_size = bufferevent_read(info.bev,block_buffer , sizeof(BufferControlBlock));
    if (read_size < 0)
    {
    #if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "bufferevent_read error --- "
                    << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
    #endif // DEBUG
        rw_w_unlock(*(info.read_singal_ptr));
        return -1;
    }
    //rw_w_unlock(*(info.read_singal_ptr));
    return read_size;
}

int LibeventHandle::readBuffer_NoWait(struct BevInfor & info, char *data)
{
    if(!info.cache_block) return -1;
    int max_size = info.block.size;
    int read_size = 0;
    #if LIBEVENT_HANDLE_DEBUG
    std::cout << "Buffer control block Size ===== " <<max_size << std::endl;
    #endif //
    // rw_r_lock(*(info.read_singal_ptr));
    evbuffer *input = bufferevent_get_input(info.bev);
    int len = evbuffer_get_length(input);
    // rw_r_unlock(*(info.read_singal_ptr));
    if(len<max_size)
    {
        #if LIBEVENT_HANDLE_DEBUG
        std::cout << "============ NoWait recive evbuffer len :" <<len << std::endl;
         #endif //
        rw_w_unlock(*(info.read_singal_ptr));
       // std::cout << "============ NoWait !!!" <<len << std::endl;
       bufferevent_setwatermark(info.bev, EV_READ ,max_size, 0);

        return 0;
    }
    bufferevent_setwatermark(info.bev, EV_READ ,0, 0);
    // rw_w_lock(*(info.read_singal_ptr));
 //std::cout << "============ NoWait !!!" <<len << std::endl;
    read_size = bufferevent_read(info.bev, data, max_size);
    if (read_size < 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "bufferevent_read error --- "
                    << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
#endif // DEBUG
        rw_w_unlock(*(info.read_singal_ptr));
        return -1;
    }
    info.cache_block = false;

    rw_w_unlock(*(info.read_singal_ptr));
    return read_size;
}

bool LibeventHandle::writeBufferOnce(struct BevInfor &info, const char *data, const int data_size)
{
    rw_w_lock(*(info.write_singal_ptr));

    struct BufferControlBlock block;
    block.size = data_size;

    if (bufferevent_write(info.bev, &block, sizeof(block)) < 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "bufferevent_write error --- "
                  << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
#endif // DEBUG
        rw_w_unlock(*(info.write_singal_ptr));
        return false;
    }
//std::cout << "STOP!!!" <<std::endl;
    if (bufferevent_write(info.bev, data, data_size) < 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        int err = EVUTIL_SOCKET_ERROR();
        std::cout << "bufferevent_write error --- "
                  << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
#endif // DEBUG
        rw_w_unlock(*(info.write_singal_ptr));
        return false;
    }
    rw_w_unlock(*(info.write_singal_ptr));
    return true;
}

int LibeventHandle::get_connection_id(const char *ip, const int port, bool tryConnect)
{
    rw_r_lock(bev_map_rw_lock_singal);
    for (auto it = bev_map.begin(); it != bev_map.end(); it++)
    {
        struct BevInfor &info = it->second;
        if (info.ip == ip && port == info.port)
        {
            rw_r_unlock(bev_map_rw_lock_singal);
            return it->first;
        }
    }
    rw_r_unlock(bev_map_rw_lock_singal);

    int connect_id = -1;

    if (tryConnect)
    {
        connect_id = try_connect(ip, port);
    }

    return connect_id;
}

int LibeventHandle::get_connection_id(struct bufferevent *bev)
{
    //std::cout << "read STOP"<< std::endl;
    rw_r_lock(bev_map_rw_lock_singal);

    for (auto it = bev_map.begin(); it != bev_map.end(); it++)
    {
        struct BevInfor &info = it->second;
        if (info.bev == bev)
        {
            rw_r_unlock(bev_map_rw_lock_singal);
            return it->first;
        }
    }

    rw_r_unlock(bev_map_rw_lock_singal);
}

void LibeventHandle::get_connection_ip(const int id, char *ip)
{
    rw_r_lock(bev_map_rw_lock_singal);
    const BevInfor &info = bev_map[id];
    memcpy(ip, info.ip.c_str(), info.ip.length() + 1);
    rw_r_unlock(bev_map_rw_lock_singal);
}

int LibeventHandle::get_connection_port(const int id)
{
    rw_r_lock(bev_map_rw_lock_singal);
    const BevInfor &info = bev_map[id];
    rw_r_unlock(bev_map_rw_lock_singal);
    return info.port;
}

/****************  thread function  **********************/

void event_loop_run(LibeventHandle *lib)
{
#if LIBEVENT_HANDLE_DEBUG
    std::cout << "event base loop start" << std::endl;
#endif // DEBUG

    lib->start_event_base_loop();

#if LIBEVENT_HANDLE_DEBUG
    std::cout << "event base loop stop" << std::endl;
#endif // DEBUG
}

void rw_r_lock(std::atomic<int> &signal_int)
{
    //  std::cout << "read STOP"<< std::endl;
    // #if LIBEVENT_HANDLE_DEBUG
    //     std::cout << "r lock = " << signal_int.load() <<std::endl;
    // #endif
    //  std::cout << "read STOP"<< std::endl;
    // < 0  == writer
    while (signal_int.load() < 0)
    {
    }
    signal_int.fetch_add(1);
}

void rw_w_lock(std::atomic<int> &signal_int)
{
    // > 0  == reader
    //  #if LIBEVENT_HANDLE_DEBUG
    //    std::cout << "w lock = " <<signal_int.load() <<std::endl;
    //    #endif
    while (signal_int.load() > 0)
    {
    }
    signal_int.fetch_sub(1);
}

void rw_r_unlock(std::atomic<int> &signal_int)
{
    // #if LIBEVENT_HANDLE_DEBUG
    //   std::cout << "r unlock = " <<signal_int.load() <<std::endl;
    //   #endif
    if (signal_int.load() > 0)
        signal_int.fetch_sub(1);
}

void rw_w_unlock(std::atomic<int> &signal_int)
{
    //  #if LIBEVENT_HANDLE_DEBUG
    //  std::cout << "w unlock = " <<signal_int.load() <<std::endl;
    //  #endif
    if (signal_int.load() < 0)
        signal_int.fetch_add(1);
}

/****************  friend function  **********************/

void connlistener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *arg)
{
    LibeventHandle *handle_ptr = (LibeventHandle *)arg;

    struct sockaddr_in *addr = (struct sockaddr_in *)sock;
// std::cout << "first view for ip : " << inet_ntoa(addr->sin_addr) << " and port : " << addr->sin_port << std::endl;
#if LIBEVENT_HANDLE_DEBUG
    std::cout << "Recive Connect!" << std::endl;
#endif // DEBUG

    if (handle_ptr->get_connection_id(inet_ntoa(addr->sin_addr), ntohl(addr->sin_port), false) >= 0)
    {
#if LIBEVENT_HANDLE_DEBUG
        std::cout << "we has connect to ip : " << inet_ntoa(addr->sin_addr) << " and port : " << addr->sin_port << std::endl;
#endif // DEBUG
    }
    else
    {
#if LIBEVENT_HANDLE_DEBUG
        std::cout << "first view for ip : " << inet_ntoa(addr->sin_addr) << " and port : " << addr->sin_port << std::endl;
        // std::cout <<"p1: "<< handle_ptr->local_port << fd << std::endl;
#endif // DEBUG
        handle_ptr->add_bufferevent_listen(inet_ntoa(addr->sin_addr), addr->sin_port, fd);
        //std::cout << "p2: " <<handle_ptr->local_port << fd << std::endl;
    }
}

void listen_error_cb(struct evconnlistener *, void *)
{

#if LIBEVENT_HANDLE_DEBUG
    int err = EVUTIL_SOCKET_ERROR();
    std::cout << "[ERROR " << err << "] : " << evutil_socket_error_to_string(err) << std::endl;
    std::cout << "Listener ERROR" << std::endl;
#endif // DEBUG0
}

void default_bufferevent_read_cb(struct bufferevent *bev, void *ctx)
{
#if LIBEVENT_HANDLE_DEBUG
    std::cout << "read from evbuffer" << std::endl;
#endif // DEBUG

    LibeventHandle *lib = (LibeventHandle *)ctx;

    int id = lib->get_connection_id(bev);

#if LIBEVENT_HANDLE_DEBUG
    std::cout << "In callback : recive from " << lib->bev_map[id].ip << " : " << lib->bev_map[id].port << std::endl;
    evbuffer *buf = bufferevent_get_input(bev);
    std::cout << "evbuffer space : " << evbuffer_get_contiguous_space(buf) << std::endl;
    std::cout << "evbuffer  1 length : " << evbuffer_get_length(buf) << std::endl;
#endif // DEBUG

   // std::lock_guard<std::mutex> lk(*(lib->bev_map[id].mut_ptr));
    //(lib->bev_map[id].recive_cond_ptr)->notify_all();

    if (lib->callback_funtion != NULL)
    {
#if LIBEVENT_HANDLE_DEBUG
     std::cout << "Invoke libevent callback function "<< std::endl;
#endif // DEBUG
        lib->callback_funtion(NET_EVENT::RECIVE, lib,id,lib->callback_args);
    }
}

void default_bufferevent_write_cb(struct bufferevent *bev, void *ctx)
{
#if LIBEVENT_HANDLE_DEBUG
    // std::cout << "write into evbuffer" << std::endl;
    // evbuffer *buf = bufferevent_get_output(bev);
    // std::cout << "evbuffer length : " << evbuffer_get_length(buf) << std::endl;
#endif // DEBUG

   // LibeventHandle *lib = (LibeventHandle *)ctx;
}
