/**
 *  Test rapidjson basic function
 * 
 * **/

#include<iostream>
#include"json_rpc_packet.h"

using namespace std;

struct TestStruct
{
    char  space1[5];
    const char * ptr1;
   
    const char * ptr2;
    char  space2[5];
     char i;
     char ch;
};


void initTestBuffer(char * ch,int size,int val)
{
    for(int i =0; i<size; i++)
    {
        ch[i] = '0' + val;
    }
}


int main(void) 
{
    cout << "********** DynamicBuffer Test **********" << endl;
    DynamicBuffer dy;

    char tb1[128];
    char tb2[256];
    char tb3[512];
    char tb4[1024];
    char tb5[1524];

    initTestBuffer(tb1,128,1);
    initTestBuffer(tb2,256,2);
    initTestBuffer(tb3,512,3);
    initTestBuffer(tb4,1024,4);
    initTestBuffer(tb5,1524,5);

    dy.append(tb1,128);
     dy.append(tb2,256);
      dy.append(tb3,512);
    dy.print_buffer();

    char ch1,ch2,ch3;
    ch1 = *dy.get_address(383);
    ch2 = *dy.get_address(384);
    ch3 = *dy.get_address(385);
    

    cout << "ch1 = " << ch1 << std::endl;
    cout << "ch2 = " << ch2 << std::endl;
    cout << "ch3 = " << ch3 << std::endl;


    cout << "********** DynamicBuffer Test END **********" << endl;

    cout  << endl << "********** JSON Test **********" << endl;

    JsonRpcPacket json;
    // json.set_packet_header("from ip",7);

    // json.set_packet_item("first","hello",5,JsonPacket::PacketItemType::FIRST);
    // json.set_packet_item("second","world",5);
    // json.set_packet_item("third",tb1,10,JsonPacket::PacketItemType::LAST);

    // json.set_note_for_ptr("third",0,"575",3,JsonPacket::PacketItemType::FIRST);
    // json.set_note_for_ptr("second",23,"989",3,JsonPacket::PacketItemType::LAST);
    // cout << "JSON STRING : " << json.get_string_ptr() << endl

  //  json.parse(json.get_string_ptr());

   // json.printdy();

    cout  << endl << "********** JSON Test END **********" << endl;

    cout  << endl << "********** Struct Parse Test **********" << endl;

   // JsonPacket & json2 = json;

  // json.clear_packet();
    TestStruct testSTRUCT;
    int testInt1 = 123456796;
    int testInt2 = 45646887;
    std::string testStr = "hello world   ";
    std::string testStr1 = " xml ";
    std::string testStr2 = " java ";

    testSTRUCT.i = 'Z';
    testSTRUCT.ch = 'P';
    memset(testSTRUCT.space1,' ',5);
    memset(testSTRUCT.space2,' ',5);
    testSTRUCT.ptr1 = testStr1.c_str() + 3;
    testSTRUCT.ptr2 = testStr2.c_str() + 4;

    cout << "testSTRUCT----------" << sizeof(char *) << endl;
    for(int q = 0;q<sizeof(testSTRUCT);q++)
    {
        cout << " " <<*(((char *)&testSTRUCT)+q) <<" " ;
    }
    cout << endl;

    json.clear_packet();

   // cout << "JSON STRUCT STRING : " << json.get_string_ptr() << endl;
   
    json.set_packet_header("MY STRUCT TEST",14);
    json.set_packet_item("testInt1",testStr2.c_str(),6,JsonRpcPacket::PacketItemType::FIRST);
    json.set_packet_item("testStr",testStr.c_str(),14);
    json.set_packet_item("testSTRUCT",(const char *)&testSTRUCT,sizeof(TestStruct),JsonRpcPacket::PacketItemType::LAST);

    json.set_note_for_ptr("testSTRUCT",16,(const char *)testSTRUCT.ptr2,sizeof(char),JsonRpcPacket::PacketItemType::FIRST);
    json.set_note_for_ptr("testSTRUCT",8, (const char *)testSTRUCT.ptr1 ,sizeof(char),JsonRpcPacket::PacketItemType::LAST);

   

     cout << "JSON STRUCT STRING : " << json.get_string_ptr() << endl;
    int  temp_size =  json.get_string_length();
  //  cout << "temp_size ============== " << *(json.get_string_ptr() + 330)<< endl;
    char * temp_space = new char[temp_size];

    json.parse(json.get_string_ptr(),temp_space,temp_size);

    print_buffer(temp_space,temp_size);
    //json.printdy();

    cout  << endl << "********** Struct Parse Test END **********" << endl;

     cout  << endl << "********** Struct GET Test **********" << endl;

    cout  << "+++++ Packet Header +++++" << endl;
    cout  << json.get_packet_header_size() << endl;
    cout  << std::string(json.get_packet_header_ptr(),json.get_packet_header_size()) << endl;
    cout  << "+++++ Packet Header END +++++" << endl;

    cout << endl << "+++++ Packet ITEM INDEX +++++" << endl;
    int index = 2;
    cout  << json.get_packet_item_size(index) << endl;
    cout  << std::string(json.get_packet_item_ptr(index),json.get_packet_item_size(index)) << endl;
    cout  << "+++++ Packet ITEM INDEX END +++++" << endl;

    cout << endl << "+++++ Packet KEY INDEX +++++" << endl;
    std::string  key = "testStr";
    cout  << json.get_packet_item_size(key.c_str()) << endl;
    cout  << std::string(json.get_packet_item_ptr(key.c_str()),json.get_packet_item_size(key.c_str())) << endl;
    cout  << "+++++ Packet KEY INDEX END +++++" << endl;

    cout << endl << "+++++ Packet Struct INDEX +++++" << endl;
    std::string  key2 = "testSTRUCT";
    cout  << json.get_packet_item_size(key2.c_str()) << endl;
    cout  << std::string(json.get_packet_item_ptr(key2.c_str()),json.get_packet_item_size(key2.c_str())) << endl;

    struct TestStruct * test_ptr = ( struct TestStruct * )json.get_packet_item_ptr(key2.c_str());

    cout << test_ptr << endl;
     cout <<(int64_t) &test_ptr->ptr1 - (int64_t) test_ptr << endl;
     cout <<(int64_t) &test_ptr->ptr2  - (int64_t) test_ptr << endl;
     cout <<*test_ptr->ptr1 << endl;
     cout <<*test_ptr->ptr2 << endl;
     cout << test_ptr->ch << endl;
     cout << test_ptr->i << endl;

    cout  << "+++++ Packet Struct INDEX END +++++" << endl;

     cout  << endl << "********** Struct GET Test END **********" << endl;
    return 0;
}