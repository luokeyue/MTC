#include "multi_buffer.h"

/******************************************************
 * DynamicBuffer Defintion
******************************************************/
DynamicBuffer::DynamicBuffer()
{
    blocks_list.clear();
    tail_offset_in_block = 0;
}

DynamicBuffer::~DynamicBuffer()
{
    destory();
}

void DynamicBuffer::clear()
{
    for (auto it = blocks_list.begin(); it != blocks_list.end(); it++)
    {
        memset(*it, 0, BLOCK_SIZE);
    }
    tail_offset_in_block = 0;
}

void DynamicBuffer::destory()
{
    for (auto it = blocks_list.begin(); it != blocks_list.end(); it++)
    {
        delete[](*it);
    }
    blocks_list.clear();
    tail_offset_in_block = 0;
}

bool DynamicBuffer::put(const char *data, int offset, int size)
{
    check_space(offset, size, true);
    bool append_byte = false;
    if(offset + size > this->size())
    {
        append_byte = true;
    }

    int index = offset / BLOCK_SIZE;

    offset = offset % BLOCK_SIZE;
    auto block_it = blocks_list.begin() + index;

    while (size != 0)
    {
        if (size > BLOCK_SIZE - offset)
        {
            memcpy((*block_it) + offset, data, BLOCK_SIZE - offset);
            size = size - (BLOCK_SIZE - offset);
            offset = 0;
            block_it++;
        }
        else
        {
            memcpy((*block_it) + offset, data, size);
        #if JSON_PACKET_DEBUG
        cout << "----------------- before put tail: " << tail_offset_in_block << " " << offset << " " << size << endl;
        #endif // 0
            if(append_byte)
                tail_offset_in_block = offset + size;
        #if JSON_PACKET_DEBUG
        cout << "----------------- after put tail: " << tail_offset_in_block<< " " << offset << " " << size << endl;
        #endif // 0
            size = 0;
        }
    }
    return true;
}

char * DynamicBuffer::get_address(int offset)
{
    if(offset > size())
    {
        return NULL;
    }

    int index = offset / BLOCK_SIZE;
    offset = offset % BLOCK_SIZE;
    auto block_it = blocks_list.begin() + index;

    return (*block_it) + offset;
}

bool DynamicBuffer::get(char *data, int offset, int size)
{
    if (!check_space(offset, size, false))
    {
        return false;
    }
    int index = offset / BLOCK_SIZE;
    offset = offset % BLOCK_SIZE;
    auto block_it = blocks_list.begin() + index;

    while (size != 0)
    {
        if (size > BLOCK_SIZE - offset)
        {
            memcpy(data, (*block_it) + offset, BLOCK_SIZE - offset);
            size = size - (BLOCK_SIZE - offset);
            data = data + (BLOCK_SIZE - offset);
            offset = 0;
            block_it++;
        }
        else
        {
            memcpy(data, (*block_it) + offset, size);
            size = 0;
        }
    }
    return true;
}

int DynamicBuffer::append(const char *data, int data_size)
{
    int offset = size();
    #if JSON_PACKET_DEBUG
        cout << "************ before append_offset: " << " "<<offset << endl;
    #endif // 0
    put(data, offset, data_size);

    #if JSON_PACKET_DEBUG
        cout << "************ after append_offset: "<< size() << endl;
    #endif // 0
    return offset;
}

bool DynamicBuffer::check_space(int offset, int size, bool expand)
{
    if (offset + size > capacity())
    {
        if (!expand)
        {
            return false;
        }
        else
        {
            int require = offset + size - capacity();
            if (require % BLOCK_SIZE == 0)
            {
                require = require / BLOCK_SIZE;
            }
            else
            {
                require = require / BLOCK_SIZE + 1;
            }
            add_blocks(require);
            return true;
        }
    }
    else
    {
        return true;
    }
}

void DynamicBuffer::add_blocks(int i)
{
    // cout << "add" << i << " blocks" << endl; 
    for (int j = 0; j < i; j++)
    {
        blocks_list.push_back(new char[BLOCK_SIZE]{0});
    }
}

int DynamicBuffer::size()
{
    int count = blocks_list.size();
    if (count > 0)
    {
        count--;
    }
    return count * BLOCK_SIZE + tail_offset_in_block;
}

int DynamicBuffer::capacity()
{
    return blocks_list.size() * BLOCK_SIZE;
}

void DynamicBuffer::print_buffer()
{
    int i = 0;
    for (auto it = blocks_list.begin(); it != blocks_list.end(); it++)
    {
        std::cout << "Block NO." << i << " : " << std::endl;
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            std::cout << blocks_list[i][j];
        }
        i++;
        std::cout << std::endl;
    }
    std::cout << "Dynamic Buffer Size : " << size() << std::endl;
}

/******************************************************
 * FixedBuffer Defintion
******************************************************/
bool FixedBuffer::put(const char * data,int offset,int size)
{
 //   std::cout << "*********** put( " << data <<"           ," << offset <<"," <<size << ") "<< std::endl ;
    if(offset + size > length)
    {
        return false;
    }
    if(offset + size > tail_offset)
    {
        tail_offset =  offset + size;
    }
    memcpy(bytes+offset,data,size);
    return true;
}

bool FixedBuffer::get(char * data,int offset,int size)
{
    memcpy(data, bytes + offset, size);
}

int FixedBuffer::append(const char * data,int size)
{
    int before_append = tail_offset;
    put(data,tail_offset,size);
    return before_append;
}

char * FixedBuffer::get_address(int offset)
{
    return bytes + offset;
}

int FixedBuffer::size()
{
    return tail_offset;
}

int FixedBuffer::capacity()
{
    return length;
}

void FixedBuffer::destory()
{
    delete [] bytes;
}

void FixedBuffer::clear()
{
    memset(bytes,0,length);
}

void print_buffer(char * bytes,int size)
{
    int i = 0;
     std::cout << "Fix Buffer : " <<std::endl;
    for (int  j = 0; j < size; j++)
    {
        std::cout << bytes[j];

    }
    std::cout << std::endl;
    std::cout << "Fixed Buffer Size : " << size << std::endl;
}