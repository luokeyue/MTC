#include "json_rpc_packet.h"

/******************************************************
 * JsonPacket
******************************************************/
const char *JsonRpcPacket::get_string_ptr()
{
    return json_buffer.GetString();
}

int JsonRpcPacket::get_string_length()
{
    return strlen(json_buffer.GetString()+1)+2;
}

void JsonRpcPacket::parse(const char *json_str,char * buffer,int size)
{
    //BufferReadHandler handle(buffer,size);
    handle.setBuffer(buffer,size);
    rapidjson::StringStream ss(json_str);
    json_reader.Parse(ss, handle);
    handle.append_tail_offset();
}

void JsonRpcPacket::clear_packet()
{
    json_buffer.Clear();
    //handle.clear();
    json_writer.Reset(json_buffer);
}

void JsonRpcPacket::set_packet_header(const char *h, int size)
{
    json_writer.StartArray();
    json_writer.StartObject();

    json_writer.Key("SegmentType");
    json_writer.Int(PacketSegmentType::HEADER);
    json_writer.Key("PacketHeader");
    json_writer.String(h, size);

    json_writer.EndObject();
}

void JsonRpcPacket::set_packet_item(const char *key, const char *val, int val_size, PacketItemType type)
{
    if (type == PacketItemType::FIRST || type == PacketItemType::SINGLE)
    {
        json_writer.StartObject();
        json_writer.Key("SegmentType");
        json_writer.Int(PacketSegmentType::BODY);
    }

    json_writer.Key(key);
    json_writer.String(val, val_size);

    if (type == PacketItemType::LAST || type == PacketItemType::SINGLE)
    {
        json_writer.EndObject();
    }
}

void JsonRpcPacket::set_note_for_ptr(const char *item_key, int offset, const char *val, int val_size, PacketItemType type)
{
    if (type == PacketItemType::FIRST || type == PacketItemType::SINGLE)
    {
        json_writer.StartObject();
        json_writer.Key("SegmentType");
        json_writer.Int(PacketSegmentType::PTR_NOTE);
        json_writer.Key("note_arry");
        json_writer.StartArray();
    }

    json_writer.StartObject();
    // json_writer.Key("ptr_name");
    // json_writer.String(ptr_name);
    json_writer.Key("ptr_key");
    json_writer.String(item_key);
    json_writer.Key("ptr_offset");
    json_writer.Int(offset);
    json_writer.Key("ptr_val");
    json_writer.String(val, val_size);
    json_writer.EndObject();

    if (type == PacketItemType::LAST || type == PacketItemType::SINGLE)
    {
        json_writer.EndArray();
        json_writer.EndObject();
        json_writer.EndArray();
    }
}

char * JsonRpcPacket::get_packet_header_ptr()
{
    int index = handle.name_id_map["PacketHeader"];
    int offset = handle.object_ptrs[index];
    return handle.get_address(offset);
}

int JsonRpcPacket::get_packet_header_size()
{
    int index = handle.name_id_map["PacketHeader"];
    return handle.object_ptrs[index+1] - handle.object_ptrs[index];
}

char * JsonRpcPacket::get_packet_item_ptr(int i)
{
    int offset = handle.object_ptrs[i];
    return handle.get_address(offset);
}

int JsonRpcPacket::get_packet_item_size(int i)
{
    return handle.object_ptrs[i+1] - handle.object_ptrs[i];
}

char * JsonRpcPacket::get_packet_item_ptr(const char * key)
{
    int index = handle.name_id_map[key];
    int offset = handle.object_ptrs[index];
    return handle.get_address(offset);
}

int JsonRpcPacket::get_packet_item_size(const char * key)
{
  int index = handle.name_id_map[key];
    return handle.object_ptrs[index+1] - handle.object_ptrs[index];
}
/******************************************************
 * JsonPacket :: ReadHandler
******************************************************/
bool JsonRpcPacket::BufferReadHandler::Uint(unsigned u)
{
    #if JSON_PACKET_DEBUG
        cout << "Uint(" << u << ")" << endl;// return true;
    #endif

    ParseItem item;
    item = parse_stack.top();
    parse_stack.pop();

    //cout << "uint key : " << item.key << std::endl;
    if (item.key == "ptr_offset")
    {
      //  cout << "********* ptr offset : " << u << std::endl;
        ParseItem item_temp;
        item_temp.key = std::string((const char *)&u,sizeof(u));
        item_temp.type = ParseType::PTR_OFFSET;
        parse_stack.push(item_temp);
    }
    else if(item.key == "SegmentType")
    {

    }
    else
    {
        int offset = objectBuffer.append((const char *)&u,sizeof(u));
        object_ptrs.push_back(offset);
        name_id_map[item.key] = object_ptrs.size() - 1 ;
    }
    return true;
}

bool JsonRpcPacket::BufferReadHandler::String(const char *str, SizeType length, bool copy)
{
    #if JSON_PACKET_DEBUG
       cout << "String(" << str << ", " << length << ", "  << copy << ")" << endl;
    #endif

    ParseItem item;
    item  = parse_stack.top();
    parse_stack.pop();

    if(item.key == "ptr_key")
    {
        ParseItem item_temp;
        item_temp.key = std::string(str,length);
        item_temp.type = ParseType::PTR_KEY;
        parse_stack.push(item_temp);
        return true;
    }

    if(item.key == "ptr_val")
    {
        ParseItem item_key,item_offset;
        item_offset = parse_stack.top();
        parse_stack.pop();
        item_key = parse_stack.top();
        parse_stack.pop();
        unsigned ptr_offset = *((unsigned *)item_offset.key.c_str());

        //append value which ptr point to
        int val_offset = objectBuffer.append(str,length);
        const char * address = objectBuffer.get_address(val_offset);

        int index = name_id_map[item_key.key];
        int start_offset = object_ptrs[index];
        #if JSON_PACKET_DEBUG
        cout << "************* adderss_str  = " << (int)*address << " " << *address << endl;
        cout << "************* adderss = " <<  (int64_t)address <<  endl;
        cout << "************* adderss & = " << *((int64_t *)std::string((const char *)&address,sizeof(address)).c_str()) <<  endl;
        cout << "--------- val_offset: "<<  val_offset << endl;
        cout << "start : " << start_offset << " ptr_offset : " << ptr_offset << ";"<< std::endl;
        #endif // 0

        objectBuffer.put((const char *)&address,start_offset+ptr_offset,sizeof(address)); // modify address

        #if JSON_PACKET_DEBUG
        cout << "************* put adderss = " <<  *((int64_t*)objectBuffer.get_address(start_offset+ptr_offset)) <<  endl;
        #endif // 0

        return true;
    }

    int offset = objectBuffer.append(str,length);
    object_ptrs.push_back(offset);
    name_id_map[item.key] = object_ptrs.size() - 1 ;
    return true;
}

bool JsonRpcPacket::BufferReadHandler::StartObject()
{
     #if JSON_PACKET_DEBUG
       cout << "StartObject()" << endl;// return true;
    #endif

    ParseItem item;
    item.key = "";
    item.type = ParseType::OBJECT;
    parse_stack.push(item);
    return true;
}

bool JsonRpcPacket::BufferReadHandler::Key(const char *str, SizeType length, bool copy)
{
    #if JSON_PACKET_DEBUG
       cout << "Key(" << str << ", " << length << ", "  << copy << ")" << endl;
    #endif

    ParseItem item;
    item.key = std::string(str,length);
    item.type = ParseType::KEY;
    parse_stack.push(item);
    return true;
}

bool JsonRpcPacket::BufferReadHandler::EndObject(SizeType memberCount)
{
    #if JSON_PACKET_DEBUG
      cout << "EndObject(" << memberCount << ")" << endl; //return true;
    #endif

    ParseItem item;
    item = parse_stack.top();
    parse_stack.pop();
    if(item.type != ParseType::OBJECT)
    {
        std::cout << item.type << ";" << item.key << " ERROR: Object can not find start point" << std::endl;
        return false;
    }
    return true;
}

bool JsonRpcPacket::BufferReadHandler::StartArray()
{
    #if JSON_PACKET_DEBUG
     cout << "StartArray()" << endl; //return true;
    #endif

    ParseItem item;
    if(!parse_stack.empty())
        item = parse_stack.top();
    if(item.type == ParseType::KEY)
    {
        parse_stack.pop();
    }

    item.key = "";
    item.type = ParseType::ARRAY;
    parse_stack.push(item);
    return true;
}

bool JsonRpcPacket::BufferReadHandler::EndArray(SizeType elementCount)
{
     #if JSON_PACKET_DEBUG
      cout << "EndArray(" << elementCount << ")" << endl;// return true;
    #endif

    ParseItem item;
    item = parse_stack.top();
    parse_stack.pop();
    if(item.type != ParseType::ARRAY)
    {
        std::cout << "ERROR: Array is not empty after handing" << std::endl;
        return false;
    }
    return true;
}