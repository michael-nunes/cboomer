#ifndef __SUGAR_H__
#define __SUGAR_H__

#define ignore (void)(0)
#define ignore_var (void)

#define addr_of(_var) &(_var)

#define bit_or |
#define bit_and &
#define bit_not ~

#define or  |
#define not !
#define and &&


//////
// FUN WITH MACROS
//
//#define macro_nth_arg(_1,_2,_3,_4,_5,_7,_8,_9,_10, N, ...) N
#define macro_nth_arg(_1,_2,_3,_4,_5, N, ...) N

#define _m_1(a, ...) a
#define _m_2(a, ...) _m_1(__VA_ARGS__)
#define _m_3(a, ...) _m_2(__VA_ARGS__)
#define _m_4(a, ...) _m_3(__VA_ARGS__)
#define _m_5(a, ...) _m_4(__VA_ARGS__)

#define macro_comma ,
#define macro_paren(a) (a)
#define macro_nop(v)


#define _flatten0(p,o,a,...) p((o)a)
#define _flatten1(p,o,a,...) _flatten0(p,o,a), _flatten0(p,o, __VA_ARGS__)
#define _flatten2(p,o,a,...) _flatten0(p,o,a), _flatten1(p,o, __VA_ARGS__)
#define _flatten3(p,o,a,...) _flatten0(p,o,a), _flatten2(p,o, __VA_ARGS__)
#define _flatten4(p,o,a,...) _flatten0(p,o,a), _flatten3(p,o, __VA_ARGS__)

#define flatten_ex(_exp, _obj, ...) macro_nth_arg(__VA_ARGS__,\
		_flatten4(_exp,_obj,__VA_ARGS__),\
		_flatten3(_exp,_obj,__VA_ARGS__),\
		_flatten2(_exp,_obj,__VA_ARGS__),\
		_flatten1(_exp,_obj,__VA_ARGS__),\
		_flatten0(_exp,_obj,__VA_ARGS__))

/// 
/// Flattening object by props:
/// example: flatten(MyRect,.x,.y,.w) will be, MyRect.x, MyRect.y, MyRect.w
///
#define flatten(_obj, ...) flatten_ex(macro_paren,_obj,__VA_ARGS__)

#define _case_of0(_n, ...) case _n: 
#define _case_of1(_n, ...) case _n: _case_of0(__VA_ARGS__)
#define _case_of2(_n, ...) case _n: _case_of1(__VA_ARGS__)
#define _case_of3(_n, ...) case _n: _case_of2(__VA_ARGS__)
#define _case_of4(_n, ...) case _n: _case_of3(__VA_ARGS__)

#define _case_of_dispatch(...) macro_nth_arg(__VA_ARGS__,\
		_case_of4(__VA_ARGS__),\
		_case_of3(__VA_ARGS__),\
		_case_of2(__VA_ARGS__),\
		_case_of1(__VA_ARGS__),\
		_case_of0(__VA_ARGS__))

#define case_end        } break;
#define case_of(...)    case_end _case_of_dispatch(__VA_ARGS__) {
#define case_begin(...) _case_of_dispatch(__VA_ARGS__) {
#define case_else       case_end default: {

//
//////

#endif
