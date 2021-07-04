#pragma once
#include <string>
#include <malloc.h>
#include <string.h>
#include <iostream>

#include <gsky/log/log.hh>
using namespace gsky::log;

namespace gsky {
namespace util {

#define VESSEL_DEFAULT_SIZE  0x18
#define VESSEL_DEFAULT_ALIGN 0x20

class vessel {
public:
    vessel() :
        size_(0),
        capacity_(VESSEL_DEFAULT_SIZE),
        data_ptr_(static_cast<char*>(malloc(VESSEL_DEFAULT_SIZE))) {
#ifdef DEBUG
        dlog << "gsky::util::vessel::vessel()\n";
#endif
        data_offset_ = 0;
    }
    ~vessel() {
#ifdef DEBUG
        dlog << "gsky::util::vessel::~vessel()\n";
#endif
        free(data_ptr_);
    };

    //调整内存容量
    void resize(size_t size) {
#ifdef DEBUG
        std::cout << std::hex;
        dlog << "gsky::util::vessel::~resize(" << size << ")\n";
#endif
        size_t mem_size = size + data_offset_;
        void* ret_ptr = realloc(data_ptr_, mem_size);
        if(ret_ptr == nullptr) {
            error() << "data realloc, can't resize\n";
            return;
        }
        if(size_ > size) {
            size_ = size;
        }
        data_ptr_ = static_cast<char *>(ret_ptr);
        capacity_ = mem_size;
    }

    void operator<<(std::string data) {
        size_t size =  data.size();
        if((capacity_ - data_offset_ - size_) < size) {
           size_t mem_size = align(capacity_ + size);
           void* ret_ptr = realloc(data_ptr_, mem_size);
           if(ret_ptr == nullptr) {
               error() << "data realloc, can't append\n";
               return;
           }
            data_ptr_ = static_cast<char *>(ret_ptr);
            capacity_ = mem_size;
            times_ ++;
        }
        memcpy(data_ptr_ + data_offset_ + size_, data.data(), size); //拷贝至数据末尾
        size_ += size;
    }

    void operator<<(const char *data) {
        size_t size =  strlen(data);
        if((capacity_ - data_offset_ - size_) < size) {
           size_t mem_size = align(capacity_ + size);
           void* ret_ptr = realloc(data_ptr_, mem_size);
           if(ret_ptr == nullptr) {
               error() << "data realloc, can't append\n";
               return;;
           }
            data_ptr_ = static_cast<char *>(ret_ptr);
            capacity_ = mem_size;
            times_ ++;
        }
        memcpy(data_ptr_ + size_, data, size);
        size_ += size;
    }
    void append(void *data, size_t length) {
        size_t size = length;
        if((capacity_ - data_offset_ - size_) < size) {
           size_t mem_size = align(capacity_ + size);
           void* ret_ptr = realloc(data_ptr_, mem_size);
           if(ret_ptr == nullptr) {
               error() << "data realloc, can't append\n";
               return;;
           }
            data_ptr_ = static_cast<char *>(ret_ptr);
            capacity_ = mem_size;
            times_ ++;
        }
        memcpy(data_ptr_ + data_offset_ + size_, data, size);
        size_ += size;
    }

    void sub(size_t size) {
#ifdef DEBUG
        dlog << "sub(" << size << ") size:" << size_ << " data_offset_: " << data_offset_ << " capacity:" << capacity_ << "\n";
#endif
        if(size_ < size) {
            error() << "sub data : out of range\n";
            size_ = 0;
            return ;
        }
        size_ -= size;
        data_offset_ += size;
    }

    int find(char c) {
        size_t i = 0;
        for(i = 0; i < size_; ++i)
            if(data_ptr_[i + data_offset_] == c)
                break;
        if(i == size_)
            return -1;
        return i;
    }

    int find(const char *ts) {
        const char *bp;
        const char *sp;
        const char *src = data_ptr_ + data_offset_;
        for(size_t i = 0; i < size_; ++i) {
            bp = src;
            sp = ts;
            do {
                if(!*sp)
                    return (int)(src - data_ptr_ - data_offset_);
            } while(*bp ++ == *sp ++);
            src ++;
        }
        return -1;
    }

    int find_to_end(const char *ts) {
        const char *bp;
        const char *sp;
        const char *src = data_ptr_ + data_offset_;
        for(size_t i = 0; i < size_; ++i) {
            bp = src;
            sp = ts;
            do {
                if(!*sp)
                    return (int)(bp - data_ptr_ - data_offset_);
            } while(*bp ++ == *sp ++);
            src ++;
        }
        return -1;
        return 0;
    }

    int find(const char *ts, int length) {
        const char *bp;
        const char *sp;
        const char *src = data_ptr_ + data_offset_;
        for(size_t i = 0; i < size_; ++i) {
            bp = src;
            sp = ts;
            int len = 0;
            do {
                if(len == length)
                    return (int)(src - data_ptr_ - data_offset_);
                len ++;
            } while(*bp ++ == *sp ++);
            src ++;
        }
        return -1;
    }

    std::string get_string(int start, int length) {
        return std::string(data_ptr_ + data_offset_ + start, length);
    }

    void clear() {
        size_ = 0;
        data_offset_ = 0;
        free(data_ptr_);
        capacity_ = VESSEL_DEFAULT_SIZE;
        data_ptr_ = (char *)malloc(VESSEL_DEFAULT_SIZE);
    }

    char *data() {
        return data_ptr_ + data_offset_;
    }

    std::string to_string() {
        return std::string(data_ptr_ + data_offset_, data_ptr_ + data_offset_ + size_);
    }

    size_t size() {
        return size_;
    }
    size_t capacity() {
        return capacity_;
    }

    size_t offset() {
        return data_offset_;
    }

private:
    size_t size_;
    size_t capacity_;
    size_t data_offset_;
    char *data_ptr_;
    char times_ = 1; // append 时开辟大小 2次方增加，进可能的避免多次 realloc。

    size_t align(size_t size) {
        size_t n = size / VESSEL_DEFAULT_ALIGN;
        if(size % VESSEL_DEFAULT_ALIGN == 0)
            return n * VESSEL_DEFAULT_ALIGN;
        return  ((n + 1) * VESSEL_DEFAULT_ALIGN) * times_ * times_;
    }
};

}
}
