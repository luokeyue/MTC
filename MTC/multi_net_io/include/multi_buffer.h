#ifndef MULTI_BUFFER_HEADER
#define MULTI_BUFFER_HEADER

#include<iostream>
#include<vector>
#include<cstring>

class BufferAPI
{
    public:
    virtual bool put(const char * data,int offset,int size)=0;
    virtual bool get(char * data,int offset,int size)=0;
    virtual int append(const char * data,int size)=0;
    virtual char * get_address(int offset)=0;
    virtual int size()=0;
    virtual int capacity()=0;
    virtual void destory()=0;
    virtual void clear()=0 ;

    private:

};

class DynamicBuffer : public BufferAPI
{
    public:
    const int BLOCK_SIZE = 1024;

    DynamicBuffer();
    ~DynamicBuffer();
    bool put(const char * data,int offset,int size);
    bool get(char * data,int offset,int size);
    int append(const char * data,int size);
    char * get_address(int offset);

    int size();
    int capacity();

    void destory();
    void clear();

    void print_buffer();

    private:
    bool check_space(int offset,int size,bool expand);
    void add_blocks(int i);
    
    std::vector<char * > blocks_list;
    int tail_offset_in_block;
};

class FixedBuffer : public BufferAPI
{
    public:
    bool put(const char * data,int offset,int size);
    bool get(char * data,int offset,int size);
    int append(const char * data,int size);
    char * get_address(int offset);
    int size();
    int capacity();
    void destory();
    void clear();
    //void set_bytes(char * buffer);

   void setBuffer(char * buffer,int len)
   {
        bytes = buffer;
        length = len;
        tail_offset = 0;
        clear();
   }

    FixedBuffer()
    {
        bytes = 0;
        length = 0;
        tail_offset = 0;
        clear();
    }

    private:
    int length;
    int tail_offset;
    char * bytes;
};

 void print_buffer(char * bytes,int size);

#endif // !MULTI_BUFFER_HEADER