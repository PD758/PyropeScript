/*
* Copyright 2025 github.com/PD758
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*      http://www.apache.org/licenses/LICENSE-2.0
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#pragma once

#include "memory.h"
#include <memory>
#include <vector>
#include <unordered_set>

#include <iostream>
#include <stdexcept>

using namespace std;

struct RawMemory {
	void* data  = nullptr;
	size_t size = 0;
	size_t ref_count = 0;

	RawMemory() {}
    RawMemory(size_t size) : size(size) {
        this->realloc_(size);
    }
	RawMemory(const RawMemory& other) : size(other.size) {
        this->realloc_(size);
        if (this->data != nullptr)
		    memcpy(this->data, other.data, size);
	}
    ~RawMemory() {
        if (data != nullptr && size > 0)
            free(data);
    }
    void realloc_(size_t new_size) {
       if (this->data != nullptr) {
           void* temp = realloc(data, new_size);
           if (temp == nullptr) {
               throw std::bad_alloc();
           }
           data = temp;
       }
       else {
           data = malloc(new_size);
           if (data == nullptr) {
               throw std::bad_alloc();
           }
       }
       size = new_size;
    }
    const void* cdata() const {
        return data;
    }
    void fill(uint8_t value) {
		if (data != nullptr && size > 0) {
			memset(data, value, size);
		}
    }
    void hex(ostream& os) const {
        if (this->data != nullptr && this->size > 0) {
            for (size_t it = 0; it < this->size; it++) {
                uint8_t i = ((uint8_t*)(this->data))[it];
                if ((i >> 4) > 9)
                    os << (char)('A' + (char)((i >> 4) - 10));
                else
                    os << (char)('0' + (char)(i >> 4));
                if ((i & 0xF) > 9)
                    os << (char)('A' + (char)((i & 0xF) - 10));
                else
                    os << (char)('0' + (char)(i & 0xF));
            }
        }
    }
    void chars(ostream& os) const {
        if (this->data != nullptr && this->size > 0) {
            for (size_t i = 0; i < this->size; i++)
                os << (((char*)(this->data))[i]);
        }
    }
    inline bool operator==(const RawMemory& other) const {
        return (this->size == other.size) && (this->data == other.data);
    }
	inline bool operator!=(const RawMemory& other) const {
		return (this->size != other.size) || (this->data != other.data);
	}
    bool same_as(const RawMemory& other) const {
        if (*this == other)
            return true;
        if (this->size != other.size)
            return false;
		return memcmp(this->data, other.data, this->size) == 0;
    }
    void free_() {
        if (data != nullptr && size > 0) {
			free(data);
			data = nullptr;
			size = 0;
		}
    }
    void operator++() {
        ref_count++;
    }
    void operator--() {
        if (ref_count > 0)
            ref_count--;
        if (ref_count < 1)
            free_();
    }
};

struct Allocator {
    vector<RawMemory> memory_pool;
    unordered_set<size_t> free_pool;

    Allocator() {}
    ~Allocator() {
        for (size_t i=0; i < memory_pool.size(); i++)
			memory_pool[i].free_();
    }
    
    void reserve(size_t size) {
        size_t prev_size = memory_pool.size();
        memory_pool.reserve(prev_size + size);
        for (size_t i = 0; i < size; i++) {
            memory_pool.push_back(RawMemory());
            free_pool.insert(prev_size + i);
        }
    }
    size_t alloc(size_t size) {
        if (free_pool.empty()) {
            memory_pool.push_back(RawMemory(size));
            return memory_pool.size() - 1;
        }
        else {
            size_t index = *free_pool.begin();
            free_pool.erase(free_pool.begin());
			memory_pool[index].realloc_(size);
			return index;
        }
    }
    RawMemory& ialloc(size_t size) {
		size_t index = alloc(size);
		return memory_pool[index];
    }
    void gc() {
		for (size_t i = 0; i < memory_pool.size(); i++) {
			if (memory_pool[i].ref_count == 0) {
				memory_pool[i].free_();
				free_pool.insert(i);
			}
		}
    }
};

