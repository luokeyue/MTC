
#include "network_handle.h"
#include <iostream>
#include <string>
#include "libevent_handle.h"

using namespace std;

int main(int argc,char * argv[])
{
    // string mess = "Hello World";
    // SimplePacket<string> strPacket(mess);
    // std::cout << *(string *)strPacket.get_val_ptr() << endl;
    //  std::cout << (mess == "123") << endl;
    //  std::cout << (mess == "Hello World") << endl;
    // // std::cout << testINT() << endl;



    LibeventHandle lib;
    cout << "arg = "<< argv[1] <<" "<<argv[2] << std::endl;
    int port = std::stoi(argv[1]);
    int port2 = std::stoi(argv[2]);
    cout << "Local Port = " << port << std::endl;
    cout << "Connect Port = " << port2 << std::endl;


//10.11.6.120

    if(!lib.init_handle(port))
    {
        cout << "init error" << std::endl;
    }
    if(port2 != 0)
    {
        cout << "send ---- " << std::endl;
        int id = lib.get_connection_id("127.0.0.1",port2,true);
        cout <<  "connection id  :"<< id << std::endl;
       std::string mess_str = "";
        for (int i=0 ; i< 1024*1024;i++)
        {
            mess_str += "w";
        }

         int re = lib.send(id,mess_str.c_str(),1024*1024);

        lib.send(id,"HELLO",6);

        lib.send(id,"APPLE",6);

        lib.send(id,"WORLD",6);


        cout <<  "send return :"<< re << std::endl;

    }

    //LibeventHandle::event_loop_run(&lib);
    //int last_count =
    while(1)
    {
        // sleep(10);
        // cout <<  "connection_count return :"<< lib.get_connection_count() << std::endl;
        //       cout <<  "error :"<< errno << std::endl;
    };
    // return 0;
}
