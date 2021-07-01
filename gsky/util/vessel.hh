#pragma once
#include <string>
#include <malloc.h>
#include <string.h>
#include <iostream>

#define VESSEL_DEFAULT_SIZE  0x100
#define VESSEL_DEFAULT_ALIGN 0x100
#include "../gsky.hh"


class gsky::util::vessel {
public:
    vessel() :
        sub_(false),
        size_(0),
        capacity_(align(VESSEL_DEFAULT_SIZE)),
        data_ptr_(static_cast<char*>(malloc(align(VESSEL_DEFAULT_SIZE)))) {
#ifdef DEBUG
        dbg_log("gsky::util::vessel::vessel()");
#endif
        data_start_ptr_ = data_ptr_;
    }
    ~vessel() {
#ifdef DEBUG
        d_cout << "gsky::util::vessel::~vessel()";
#endif
        free(data_ptr_);
    };

    void resize(size_t size) {
        if(sub_) {
            std::cout << "data subed can't resize\n";
            return;
        }else if(size == 0) {
            return;
        }

        size_t align_size = align(size);
        void* ret_ptr = realloc(data_ptr_, align_size);
        if(ret_ptr == nullptr) {
            std::cout << "data realloc, can't append\n";
            return;
        }

        data_ptr_ = static_cast<char *>(ret_ptr);
        data_start_ptr_ = data_ptr_;
        capacity_ = align_size;
    }

    void operator<<(std::string data) {
        size_t size =  data.size();
        if(sub_ || size == 0) {
            std::cout << "data subed, can't append\n";
            return;
        }
        if((capacity_ - size_) < size) {
            std::cout << "realloc\n";
           size_t align_size = align(capacity_ + size);
           void* ret_ptr = realloc(data_ptr_, align_size);
           if(ret_ptr == nullptr) {
               std::cout << "data realloc, can't append\n";
               return;
           }
           data_ptr_ = static_cast<char *>(ret_ptr);
           data_start_ptr_ = data_ptr_;
           capacity_ = align_size;
        }
        memcpy(data_start_ptr_ + size_, data.data(), size);
        size_ += size;
    }

    void operator<<(const char *data) {
        size_t size =  strlen(data);
        if(sub_ || size == 0) {
            std::cout << "data subed, can't append\n";
            return;
        }
        if((capacity_ - size_) < size) {
           size_t align_size = align(capacity_ + size);
           void* ret_ptr = realloc(data_ptr_, align_size);
           if(ret_ptr == nullptr) {
               std::cout << "data realloc, can't append\n";
               return;;
           }
           data_ptr_ = static_cast<char *>(ret_ptr);
           data_start_ptr_ = data_ptr_;
           capacity_ = align_size;
        }
        memcpy(data_start_ptr_ + size_, data, size);
        size_ += size;
    }
    void append(void *data, size_t length) {
        size_t size = length;
        if(sub_ || size == 0) {
            std::cout << "data subed, can't append\n";
            return;
        }
        if((capacity_ - size_) < size) {
           size_t align_size = align(capacity_ + size);
           void* ret_ptr = realloc(data_ptr_, align_size);
           if(ret_ptr == nullptr) {
               std::cout << "data realloc, can't append\n";
               return;;
           }
           data_ptr_ = static_cast<char *>(ret_ptr);
           data_start_ptr_ = data_ptr_;
           capacity_ = align_size;
        }
        memcpy(data_start_ptr_ + size_, data, size);
        size_ += size;
    }

    void sub(size_t size) {
        sub_ = true;
        if(size_ < size) {
            std::cout << "sub data : out of range\n";
            size_ = 0;
        }else
            size_ -= size;
        data_start_ptr_ += size;
    }
    int find(char c) {
        size_t i = 0;
        for(i = 0; i < size_; ++i)
            if(data_start_ptr_[i] == c)
                break;
        if(i == size_)
            return -1;
        return i;
    }

    int find(const char *ts) {
        const char *bp;
        const char *sp;
        const char *src = data_start_ptr_;
        for(size_t i = 0; i < size_; ++i) {
            bp = src;
            sp = ts;
            do {
                if(!*sp)
                    return (int)(src - data_start_ptr_);
            } while(*bp ++ == *sp ++);
            src ++;
        }
        return -1;
    }

    int find_to_end(const char *ts) {
        const char *bp;
        const char *sp;
        const char *src = data_start_ptr_;
        for(size_t i = 0; i < size_; ++i) {
            bp = src;
            sp = ts;
            do {
                if(!*sp)
                    return (int)(bp - data_start_ptr_);
            } while(*bp ++ == *sp ++);
            src ++;
        }
        return -1;
        return 0;
    }

    int find(const char *ts, int length) {
        const char *bp;
        const char *sp;
        const char *src = data_start_ptr_;
        for(size_t i = 0; i < size_; ++i) {
            bp = src;
            sp = ts;
            int len = 0;
            do {
                if(len == length)
                    return (int)(src - data_start_ptr_);
                len ++;
            } while(*bp ++ == *sp ++);
            src ++;
        }
        return -1;
    }

    std::string get_string(int start, int length) {
        return std::string(data_start_ptr_ + start, length);
    }

    void clear() {
        sub_ = false;
        size_ = 0;
        data_start_ptr_ = data_ptr_;
    }
    char *data() {
        return data_start_ptr_;
    }
    std::string to_string() {
        return std::string(data_start_ptr_, data_start_ptr_ + size_);
    }

    size_t size() {
        return size_;
    }
    size_t capacity() {
        return capacity_;
    }
private:
    bool sub_;
    size_t size_;
    size_t capacity_;
    char *data_ptr_;
    char *data_start_ptr_;

    size_t align(size_t size) {
        size_t n = size / VESSEL_DEFAULT_ALIGN;
        if(size % VESSEL_DEFAULT_ALIGN == 0)
            return n * VESSEL_DEFAULT_ALIGN;
        return  (n + 1) * VESSEL_DEFAULT_ALIGN;
    }
};
