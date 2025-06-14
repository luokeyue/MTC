#include "event_mess_handle.h"
#include "test_config.h"

using namespace std;

 void client_recive_cb(EventMessageHandle * mess_handle,EventMessage * mess,void * arg)
 {
    cout << "success get reply from Group "<<mess->group_name<<"["<<mess->mess_type<<"] ---- Message : " << mess->message << endl;
 }


int main(void)
{
    EventMessageHandle handler;

    string host_config_path = std::string(TEST_SOURCE_DIR) + "host_config1.json";
    string mess_config_path = std::string(TEST_SOURCE_DIR) + "mess_config.json";
    cout << "host config file path = " << host_config_path << endl << "message config file path = " << mess_config_path << endl;
    handler.init_handle(host_config_path.c_str(),mess_config_path.c_str());

    handler.register_recive_handler("admin","reply",client_recive_cb,NULL);

    cout << "Message Client Test" << endl;

 ///   cout << "---------- Message Callback Recive Test ------------" << endl;

    int count = 0;
    int return_code =0;

    cout << "---------- Large Message Send Test ------------" << endl;
     EventMessage mess;
     string  mess_str = "";
     for(int i=0; i<1024*1024;i++)
     {
         mess_str += "w";
     }
     mess.prepare_send("admin","heart","host2",mess_str.c_str(),mess_str.size());
     handler.sendMessage(&mess);
int c = 0;
    while(c<10){
        cout << "---------- Message Constantly Send Test ------------" << endl;
        EventMessage while_mess;
        EventMessage reply_mess;
        string while_str = "send count : " + std::to_string(count);
        string reply_str = "try to get reply --- count : " + std::to_string(count);
        while_mess.prepare_send("admin","heart","host2",while_str.c_str(),while_str.size());
        reply_mess.prepare_send("admin","reply","host2",reply_str.c_str(),reply_str.size());
        return_code = handler.sendMessage(&while_mess);
        handler.sendMessage(&reply_mess);
        cout << "sendMessage return_code : " << return_code << endl;
        if(return_code < 0)
        {
             cout << "send error" << std::endl;
             cout << MessageError::getEventErrorStr(while_mess.error_no) << endl;
        }
        else
        {
            cout << while_mess.message << std::endl;
            count++;
        }
c++;
cout << "client Test [ "<< c << "]"<< std::endl;
        //sleep(5);
    }
    while(1);
    //handler.free_handle();
cout << " END client Test" << std::endl;


    return 0;
}