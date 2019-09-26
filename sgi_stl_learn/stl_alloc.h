#ifndef _STL_ALLOC_H_
#define _STL_ALLOC_H_

template <int __inst>
class __malloc_alloc_template
{
private:
    static void* _S_oom_malloc(size_t);
    static void* _S_oom_realloc(void*, size_t);

public:
    static void* allocate(size_t __n)
    {
        void* __result = malloc(__n);
        if (0 == result) {
            __result = _S_oom_alloc(__n);
        }
        return __result;
    }

    static void deallocate(void* __p, size_t /* __n */)
    {
        free(__p);
    }

    static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
    {
        void* __result = realloc(__p, __new_sz);
        if (0 == __result) {
            __result = _S_oom_realloc(__p, __new_sz);
        }
        return __result;
    }
};

typedef __malloc_alloc_template<0> malloc_alloc;
//////////////////////////////////////////////////////////////////////////////
template<class _Tp, class _Alloc>
class simple_alloc 
{
public:
    static _Tp* allocate(size_t n) 
    {
        return 0 == n ? 0 : (_Tp*)_Alloc::allocate(n*sizeof(_Tp));
    }
    static _Tp* allocate(void) 
    {
        return (_Tp*)_Alloc::allocate(sizeof(_Tp));
    }
    static void deallocate(_Tp* p, size_t n) 
    {
        if (n != 0) {
            _Alloc::deallocate(p, n*sizeof(_Tp));
        }
    }
    static void deallocate(_Tp* p) 
    {
        _Alloc::deallocate(p, sizeof(_Tp));
    }
};

//////////////////////////////////////////////////////////////////////////
template<bool __threads, int __inst>
class __default_alloc_template
{
private:
    enum {_ALIGN = 8};
    enum {_MAX_BYTES = 128};
    enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN

    static size_t _S_round_up(size_t __bytes) 
    {
        return ((__bytes + (size_t)_ALIGN - 1) & ~((size_t)_ALIGN - 1));
    }

private:
    union _Obj {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];
    };

    static _Obj* volatile _S_free_list[_NFREELISTS];

    static size_t _S_freelist_index(size_t __bytes) 
    {
        return (((__bytes) + (size_t)_ALIGN - 1) / (size_t)_ALIGN - 1);
    }

    static void* _S_refill(size_t __n);
    static char* _S_chunk_alloc(size_t __size, int& __nobjs);

    static char* _S_start_free;
    static char* _S_end_free;
    static size_t _S_heap_size;

public:
    static void* allocate(size_t __n) 
    {
        void* __ret = 0;

        if (__n > (size_t)_MAX_BYTES) {
            __ret = malloc_alloc::allocate(__n);
        } else {
            _Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__n);
            _Obj* __result = *__my_free_list;
            if (__result == 0) {
                __ret = _S_refill(_S_round_up(__n));
            } else {
                *__my_free_list = __result->_M_free_list_link;
                __ret = __result;
            }
        }

        return __ret;
    }

    static void deallocate(void* __p, size_t __n) 
    {
        if (__n > (size_t)_MAX_BYTES) {
            malloc_alloc::deallocate(__p, __n);
        } else {
            _Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__n);
            _Obj* __q = (_Obj*)__p;

            __q->_M_free_list_link = *__my_free_list;
            *__my_free_list = __q;
        }
    }

    static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz);
};

template <bool __threads, int __inst>
void* __default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
    int __nobjs = 20;
    char* __chunk = _S_chunk_alloc(__n, __nobjs);
    _Obj* volatile* __my_free_list;
    _Obj* __result;
    _Obj* __current_obj;
    _Obj* __next_obj;
    int __i;

    if (1 == __nobjs) {
        return __chunk;
    }

    __result = (_Obj*)__chunk;
    *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
    for (__i = 1; ; __i++) {
        __current_obj = __next_obj;
        __next_obj = (_Obj*)((char*)__next_obj + __n);
        if (__nobjs - 1 == __i) {
            __current_obj->__M_free_list_link = 0;
            break;
        } else {
            __current_obj->_M_free_list_link = __next_obj;
        }
    }
    return __result;
}

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size, int& __nobjs)
{
    char *__result;
    size_t __total_bytes = __size * __nobjs;
    size_t __bytes_left = _S_end_free - _S_start_free;

    if (__bytes_left >= __total_bytes) {
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return __result;
    } else if (__bytes_left >= __size) {
        __nobjs = (int)(__bytes_left / __size);
        __total_bytes = __size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return __result;
    } else {
        size_t __bytes_to_get = 2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
        if (__bytes_left > 0) {
            _Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__bytes_left);
            ((_Obj*)_S_start_free)->_M_free_list_link = *__my_free_list;
            *__my_free_list = (_Obj*)_S_start_free;
        }
        _S_start_free = (char*)malloc(__bytes_to_get);
        if (0 == _S_start_free) {
            size_t __i;
            _Obj* volatile* __my_free_list;
            _Obj* __p;
            for (__i = __size; __i <= (size_t)_MAX_BYTES; __i += (size_t)_ALIGN) {
                __my_free_list = _S_free_list + _S_freelist_index(__i);
                __p = *__my_free_list;
                if (__p != 0) {
                    *__my_free_list = __p->_M_free_list_link;
                    _S_start_free = (char*)__p;
                    _S_end_free = _S_start_free + __i;
                    return (_S_chunk_alloc(__size, __nobjs));
                }
            }
            _S_end_free = 0;
            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);//malloc_alloc还要再看看--add by dengkai
        }
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
        return (_S_chunk_alloc(__size, __nobjs));
    }
}

template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::reallocate(void* __p, size_t __old_sz, size_t __new_sz)
{
    void* __result;
    size_t __copy_sz;

    if (__old_sz > (size_t)_MAX_BYTES && __new_sz > (size_t)_MAX_BYTES) {
        return (realloc(__p, __new_sz));
    }
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) {
        return __p;
    }
    __result = allocate(__new_sz);
    __copy_sz = __new_sz > __old_sz ? __old_sz : __new_sz;
    memcpy(__result, __p, __copy_sz);
    deallocate(__p, __old_sz);
    return __result;
}

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj* volatile
__default_alloc_template<__threads, __inst> ::_S_free_list[_NFREELISTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

#endif