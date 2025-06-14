#ifndef JSON_RPC_PACKET_HEADER
#define JSON_RPC_PACKET_HEADER

#ifdef __cplusplus

#include <cstddef>
#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson {
typedef ::std::size_t SizeType;
}
#endif


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"

#include "multi_buffer.h"

#include<iostream>
#include<vector>
#include<map>
#include<stack>
//#include "network_handle.h"
#define JSON_PACKET_DEBUG 0

/*
1. string -> json -> args : get args by array index or key(string)
2. args -> json -> string : store the name and val of each arg , record some special annotation
when convert json to args, handle these special annotation
*/

using std::cout;
using std::endl;
using rapidjson::SizeType;


/**
 * ----------- Json Packet Format ---------------
 * json array :
 * 1. Packet header (object) -- <SegmentType,type>,<Header,val>
 * 2. Packet Item (Object) -- <SegmentType,type>,<key,val>......
 * 3. Packet Note (Object) --
 *          a. <SegmentType,type>
 *          b. note array: note object1,note object2......
 *    ps:note object =  ptr_name , ptr_key , ptr_offset , ptr_Val
 *
 * so object_ptrs[0] is the address of header
*/

class JsonRpcPacket
{
    public:
    enum PacketItemType
    {
        FIRST,
        NORMAL,
        LAST,
        SINGLE
    };

    JsonRpcPacket():json_writer(json_buffer)
    {
    }

    const char * get_string_ptr();
    int get_string_length();
    void parse(const char * json_str,char * buffer,int size);

    void clear_packet();

    void set_packet_header(const char *,int size);
    void set_packet_item(const char * key,const char * val,int val_size,PacketItemType type = PacketItemType::NORMAL);
    void set_note_for_ptr(const char * item_key,int offset,const char * val,int size,PacketItemType type = PacketItemType::NORMAL);


    char * get_packet_header_ptr();
    int get_packet_header_size();
    char * get_packet_item_ptr(int i);
    int get_packet_item_size(int i);
    char * get_packet_item_ptr(const char * key);
    int get_packet_item_size(const char * key);

    private:

    enum PacketSegmentType
    {
        HEADER,
        BODY,
        PTR_NOTE,
    };

    enum ParseType
    {
        OBJECT,
        ARRAY,
        KEY,
        PTR_KEY,
        PTR_OFFSET
    };

    struct ParseItem
    {
        ParseType type;
        std::string key;
    };


    class BufferReadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, BufferReadHandler> {
        public:
        /* Debug point function */
        bool Null() { cout << "Null()" << endl; return true; }
        bool Bool(bool b) { cout << "Bool("  << b << ")" << endl; return true; }
        bool Int(int i) { cout << "Int(" << i << ")" << endl; return true; }

        bool Int64(int64_t i) { cout << "Int64(" << i << ")" << endl; return true; }
        bool Uint64(uint64_t u) { cout << "Uint64(" << u << ")" << endl; return true; }
        bool Double(double d) { cout << "Double(" << d << ")" << endl; return true; }

        bool Uint(unsigned u);
        bool String(const char* str, SizeType length, bool copy);
        bool StartObject();
        bool Key(const char* str, SizeType length, bool copy);
        bool EndObject(SizeType memberCount);
        bool StartArray();
        bool EndArray(SizeType elementCount);

        void clear()
        {
            objectBuffer.clear();
            object_ptrs.clear();
            name_id_map.clear();
            while(!parse_stack.empty())
                parse_stack.pop();
        }

        // void print_buffer()
        // {
        //     objectBuffer.print_buffer();
        // }

        // BufferReadHandler(char * buffer,int size) : objectBuffer(buffer,size)
        // {
        //     ptr_key = "";
        //     ptr_offset = 0;
        // }

        void setBuffer(char * buffer,int size)
        {
            objectBuffer.setBuffer(buffer,size);
        }

        char * get_address(int offset)
        {
            return objectBuffer.get_address(offset);
        }

        void append_tail_offset()
        {
            object_ptrs.push_back(objectBuffer.size() );
        }

        std::vector<int> object_ptrs;
        std::map<std::string,int> name_id_map;

        private:
       // bool start_parse_ptr_note;
        std::stack<ParseItem> parse_stack;

        std::string ptr_key;
        int ptr_offset;

        FixedBuffer objectBuffer;
    };

    rapidjson::StringBuffer json_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json_writer;
    rapidjson::Reader json_reader;
    BufferReadHandler handle;

};

#endif // !JSON_PACKET_HEADER

