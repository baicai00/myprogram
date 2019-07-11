#ifndef _STL_ALLOC_H_
#define _STL_ALLOC_H_

template<class _Tp, class _Alloc>
class simple_alloc 
{
public:
    static _Tp* allocate(size_t n) {
        return 0 == n ? 0 : (_Tp*)_Alloc::allocate(n*sizeof(_Tp));
    }
    static _Tp* allocate(void) {
        return (_Tp*)_Alloc::allocate(sizeof(_Tp));
    }
    static void deallocate(_Tp* p, size_t n) {
        if (n != 0) {
            _Alloc::deallocate(p, n*sizeof(_Tp));
        }
    }
    static void deallocate(_Tp* p) {
        _Alloc::deallocate(p, sizeof(_Tp));
    }
};

//////////////////////////////////////////////////////////////////////////
template<bool threads, int inst>
class __default_alloc_template
{
    // 下次从这里开始
};

#endif