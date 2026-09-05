#pragma once

// This is really weird, but we should use this https://github.com/LiveMirror/stlport/blob/master/stlport/vector
// But I don't like to move STLPort directly without any reason. So I will use this 'creepy' impl for some time

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <new>


namespace Glacier
{
    template <typename T>
    struct zallocator 
    {
        T* allocate(size_t n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
        void deallocate(T* p, size_t n) { ::operator delete(p); }
    };

    template <typename Ptr_Type, typename Value_Type, typename Alloc>
    struct _STLP_alloc_proxy : public Alloc 
    {
        Ptr_Type _M_data;

        _STLP_alloc_proxy() : _M_data(nullptr) {}
        _STLP_alloc_proxy(Ptr_Type p) : _M_data(p) {}
    };

    template <typename T, typename Alloc = zallocator<T>>
    struct _Vector_base 
    {
        T* m_start;
        T* m_finish;
        _STLP_alloc_proxy<T*, T, Alloc> m_end_of_storage; // +0x08

        _Vector_base() : m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr) {}
    };

    template <typename T, typename Alloc = zallocator<T>>
    class zvector : public _Vector_base<T, Alloc> 
    {
    private:
        void _Construct(T* p, const T& value) 
        {
            ::new (static_cast<void*>(p)) T(value);
        }

        void _Destroy(T* first, T* last) {
            for (; first != last; ++first) first->~T();
        }

        void _M_clear() 
        {
            if (this->m_start) 
            {
                _Destroy(this->m_start, this->m_finish);
                this->m_end_of_storage.deallocate(this->m_start, this->m_end_of_storage._M_data - this->m_start);
            }
        }

        void _M_set(T* start, T* finish, T* end_of_storage) 
        {
            this->m_start = start;
            this->m_finish = finish;
            this->m_end_of_storage._M_data = end_of_storage;
        }

        void _M_insert_overflow(T* position, const T& value) 
        {
            const size_t old_size = size();
            const size_t new_capacity = old_size + std::max(old_size, static_cast<size_t>(1));

            T* new_start = this->m_end_of_storage.allocate(new_capacity);
            T* new_finish = new_start;

            for (T* p = this->m_start; p != position; ++p, ++new_finish) 
            {
                _Construct(new_finish, *p);
            }

            _Construct(new_finish, value);
            new_finish++;

            _M_clear();
            _M_set(new_start, new_finish, new_start + new_capacity);
        }

    public:
        zvector() : _Vector_base<T, Alloc>() {}
        ~zvector() { _M_clear(); }

        T* begin() { return this->m_start; }
        T* end() { return this->m_finish; }
        size_t size() const { return static_cast<size_t>(this->m_finish - this->m_start); }

        void push_back(const T& value) 
        {
            if (this->m_finish == this->m_end_of_storage._M_data) 
            {
                _M_insert_overflow(this->m_finish, value);
            } 
            else 
            {
                _Construct(this->m_finish, value);
                this->m_finish++;
            }
        }

        T& operator[](size_t index) 
        { 
            return this->m_start[index]; 
        }
    };
}