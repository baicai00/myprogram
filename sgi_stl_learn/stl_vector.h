#ifndef _STL_VECTOR_H_
#define _STL_VECTOR_H_

template <class _Tp, class _Alloc>
class _Vector_base 
{
public:
    typedef _Alloc allocator_type;
    allocator_type get_allocator() const {
        return allocator_type();
    }

    _Vector_base(const _Alloc&)
        : _M_start(0), _M_finish(0), _M_end_of_storage(0) {}
    _Vector_base(size_t n, const _Alloc&)
        : _M_start(0), _M_finish(0), _M_end_of_storage(0) {
        _M_start = _M_allocate(n);
        _M_finish = _M_start;
        _M_end_of_storage = _M_start + n;
    }
    ~_Vector_base() {
        _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    }

protected:
    _Tp* _M_start;
    _Tp* _M_finish;
    _Tp* _M_end_of_storage;

    typedef simple_alloc<_Tp, _Alloc> _M_data_allocator;
    _Tp* _M_allocate(size_t n) {
        return _M_data_allocator::allocate(n);
    }
    void _M_deallocate(_Tp* p, size_t n) {
        _M_data_allocator::deallocate(p, n);
    }
};

////////////////////////////////////////////////////////////////////////////////
template <class _Tp, class _Alloc>
class vector : protected _Vector_base<_Tp, _Alloc>
{
private:
    typedef _Vector_base<_Tp, _Alloc> _Base;
public:
    typedef _Tp value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    // todo
};

#endif