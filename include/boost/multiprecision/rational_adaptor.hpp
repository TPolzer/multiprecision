///////////////////////////////////////////////////////////////
//  Copyright 2011 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_

#ifndef BOOST_MATH_RATIONAL_ADAPTER_HPP
#define BOOST_MATH_RATIONAL_ADAPTER_HPP

#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/cstdint.hpp>
#include <boost/multiprecision/number.hpp>

namespace boost{
namespace multiprecision{
namespace backends{

template <class IntBackend>
struct rational_adaptor
{
   typedef number<IntBackend>                   integer_type;

   typedef typename IntBackend::signed_types    signed_types;
   typedef typename IntBackend::unsigned_types  unsigned_types;
   typedef typename IntBackend::float_types     float_types;

private:

   IntBackend m_n, m_d;

   typedef typename mpl::front<unsigned_types>::type ui_type;

public:

   rational_adaptor(){}
   rational_adaptor(const rational_adaptor& o) : m_n(o.m_n), m_d(o.m_d) {}
   rational_adaptor(const IntBackend& o) : m_n(o), m_d(static_cast<ui_type>(1u)) {}

   template <class U>
   rational_adaptor(const U& u, typename enable_if_c<is_convertible<U, IntBackend>::value>::type* = 0) 
      : m_n(u), m_d(static_cast<ui_type>(1u)) {}
   template <class U>
   explicit rational_adaptor(const U& u, 
      typename enable_if_c<
         boost::multiprecision::detail::is_explicitly_convertible<U, IntBackend>::value && !is_convertible<U, IntBackend>::value
      >::type* = 0) 
      : m_n(u), m_d(static_cast<ui_type>(1u)) {}
   template <class U>
   typename enable_if_c<(boost::multiprecision::detail::is_explicitly_convertible<U, IntBackend>::value && !is_arithmetic<U>::value), rational_adaptor&>::type operator = (const U& u) 
   {
      m_n = u;
      m_d = static_cast<ui_type>(1u);
   }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
   rational_adaptor(rational_adaptor&& o) : m_n(BOOST_MP_MOVE(o.m_n)), m_d(BOOST_MP_MOVE(o.m_d)) {}
   rational_adaptor(IntBackend&& o) : m_n(o), m_d(static_cast<ui_type>(1u)) {}
   rational_adaptor& operator = (rational_adaptor&& o)
   {
      m_n = BOOST_MP_MOVE(o.m_n);
      m_d = BOOST_MP_MOVE(o.m_d);
      return *this;
   }
#endif
   rational_adaptor& operator = (const rational_adaptor& o)
   {
      m_n = o.m_n;
      m_d = o.m_d;
      return *this;
   }
   rational_adaptor& operator = (const IntBackend& o)
   {
      m_n = o;
      m_d = static_cast<ui_type>(1u);
      return *this;
   }
   template <class Int>
   typename enable_if<is_integral<Int>, rational_adaptor&>::type operator = (Int i)
   {
      m_n = i;
      m_d = static_cast<ui_type>(1u);
      return *this;
   }
   template <class Float>
   typename enable_if<is_floating_point<Float>, rational_adaptor&>::type operator = (Float i)
   {
      using default_ops::eval_left_shift;
      using default_ops::eval_right_shift;
      using default_ops::eval_divide;
      using default_ops::eval_gcd;
      using default_ops::eval_eq;

      int e;
      Float f = std::frexp(i, &e);
      f = std::ldexp(f, std::numeric_limits<Float>::digits);
      e -= std::numeric_limits<Float>::digits;
      m_n = f;
      m_d = static_cast<ui_type>(1u);
      if(e > 0)
      {
         eval_left_shift(m_n, e);
      }
      else if(e < 0)
      {
         eval_left_shift(m_d, -e);
      }
      IntBackend t1, t2;
      eval_gcd(t1, m_n, m_d);
      if(!eval_eq(t1, static_cast<ui_type>(1u)))
      {
         eval_divide(t2, m_n, t1);
         m_n = t2;
         eval_divide(t2, m_d, t1);
         m_d = BOOST_MP_MOVE(t2);
      }
      return *this;
   }
   rational_adaptor& operator = (const char* s)
   {
      std::string s1;
      char c;
      bool have_hex = false;
      const char* p = s; // saved for later

      while((0 != (c = *s)) && (c == 'x' || c == 'X' || c == '-' || c == '+' || (c >= '0' && c <= '9') || (have_hex && (c >= 'a' && c <= 'f')) || (have_hex && (c >= 'A' && c <= 'F'))))
      {
         if(c == 'x' || c == 'X')
            have_hex = true;
         s1.append(1, c);
         ++s;
      }
      m_n = s1.c_str();
      s1.erase();
      if(c == '/')
      {
         ++s;
         while((0 != (c = *s)) && (c == 'x' || c == 'X' || c == '-' || c == '+' || (c >= '0' && c <= '9') || (have_hex && (c >= 'a' && c <= 'f')) || (have_hex && (c >= 'A' && c <= 'F'))))
         {
            if(c == 'x' || c == 'X')
               have_hex = true;
            s1.append(1, c);
            ++s;
         }
         m_d = s1.c_str();
      }
      else
         m_d = static_cast<ui_type>(1u);
      if(*s)
      {
         BOOST_THROW_EXCEPTION(std::runtime_error(std::string("Could not parse the string \"") + p + std::string("\" as a valid rational number.")));
      }

      using default_ops::eval_divide;
      using default_ops::eval_gcd;
      using default_ops::eval_eq;

      IntBackend t1, t2;
      eval_gcd(t1, m_n, m_d);
      if(!eval_eq(t1, static_cast<ui_type>(1u)))
      {
         eval_divide(t2, m_n, t1);
         m_n = t2;
         eval_divide(t2, m_d, t1);
         m_d = BOOST_MP_MOVE(t2);
      }
      return *this;
   }
   void swap(rational_adaptor& o)
   {
      m_n.swap(o.m_n);
      m_d.swap(o.m_d);
   }
   std::string str(std::streamsize digits, std::ios_base::fmtflags f)const
   {
      //
      // We format the string ourselves so we can match what GMP's mpq type does:
      //
      std::string result = m_n.str(digits, f);
      std::string t = m_d.str(digits, f);
      if(t != "1")
      {
         result.append(1, '/');
         result.append(t);
      }
      return result;
   }
   void negate()
   {
      m_n.negate();
   }
   int compare(const rational_adaptor& o)const
   {
      using default_ops::eval_multiply;
      IntBackend t1, t2;
      eval_multiply(t1, m_n, o.m_d);
      eval_multiply(t2, m_d, o.m_n);
      return t1.compare(t2);
   }
   template <class Arithmatic>
   typename enable_if_c<is_arithmetic<Arithmatic>::value && !is_floating_point<Arithmatic>::value, int>::type compare(Arithmatic i)const
   {
      using default_ops::eval_multiply;
      IntBackend t1;
      eval_multiply(t1, m_d, i);
      return m_n.compare(t1);
   }
   template <class Arithmatic>
   typename enable_if_c<is_floating_point<Arithmatic>::value, int>::type compare(Arithmatic i)const
   {
      rational_adaptor r;
      r = i;
      return this->compare(r);
   }

   IntBackend& n() { return m_n; }
   IntBackend& d() { return m_d; }
   const IntBackend& n()const { return m_n; }
   const IntBackend& d()const { return m_d; }

   template <class Archive>
   void serialize(Archive& ar, const unsigned int /*version*/)
   {
      ar & m_n;
      ar & m_d;
   }
};

namespace detail{

template <class IntBackend, class T>
void rational_add_subtract_scalar(rational_adaptor<IntBackend>& result, const T& val, bool is_add)
{
   typedef typename mpl::front<typename IntBackend::unsigned_types>::type ui_type;

   using default_ops::eval_multiply;
   using default_ops::eval_add;
   using default_ops::eval_subtract;
   using default_ops::eval_gcd;
   using default_ops::eval_divide;
   using default_ops::eval_eq;

   IntBackend t;
   eval_multiply(t, result.d(), val);
   if(is_add)
      eval_add(result.n(), t);
   else
      eval_subtract(result.n(), t);
   eval_gcd(t, result.n(), result.d());
   if(!eval_eq(t, static_cast<ui_type>(1u)))
   {
      IntBackend t2;
      eval_divide(t2, result.n(), t);
      result.n() = t2;
      eval_divide(t2, result.d(), t);
      result.d() = BOOST_MP_MOVE(t2);
   }
}

template <class IntBackend, class T>
void rational_add_subtract_scalar(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const T& b, bool is_add)
{
   typedef typename mpl::front<typename IntBackend::unsigned_types>::type ui_type;

   using default_ops::eval_multiply;
   using default_ops::eval_add;
   using default_ops::eval_subtract;
   using default_ops::eval_gcd;
   using default_ops::eval_divide;
   using default_ops::eval_eq;

   IntBackend t;
   eval_multiply(result.n(), a.d(), b);
   if(is_add)
      eval_add(result.n(), a.n());
   else
   {
      eval_subtract(result.n(), a.n());
      result.n().negate();
   }
   eval_gcd(t, result.n(), a.d());
   if(!eval_eq(t, static_cast<ui_type>(1u)))
   {
      eval_divide(result.d(), result.n(), t);
      result.n() = result.d();
      eval_divide(result.d(), a.d(), t);
   }
   else
   {
      result.d() = a.d();
   }
}

template <class IntBackend>
void rational_add_subtract(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const rational_adaptor<IntBackend>& b, bool is_add)
{
   typedef typename mpl::front<typename IntBackend::unsigned_types>::type ui_type;

   using default_ops::eval_multiply;
   using default_ops::eval_add;
   using default_ops::eval_subtract;
   using default_ops::eval_gcd;
   using default_ops::eval_divide;
   using default_ops::eval_eq;

   IntBackend t1, t2;
   eval_multiply(t1, a.n(), b.d());
   eval_multiply(t2, a.d(), b.n());
   if(is_add)
      eval_add(result.n(), t1, t2);
   else
      eval_subtract(result.n(), t1, t2);
   eval_multiply(result.d(), a.d(), b.d());
   eval_gcd(t1, result.n(), result.d());
   if(!eval_eq(t1, static_cast<ui_type>(1u)))
   {
      eval_divide(t2, result.n(), t1);
      result.n() = t2;
      eval_divide(t2, result.d(), t1);
      result.d() = BOOST_MP_MOVE(t2);
   }
}

template <class IntBackend>
void rational_multiply_divide(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const rational_adaptor<IntBackend>& b, bool is_mult)
{
   typedef typename mpl::front<typename IntBackend::unsigned_types>::type ui_type;

   using default_ops::eval_multiply;
   using default_ops::eval_gcd;
   using default_ops::eval_divide;
   using default_ops::eval_eq;

   IntBackend t1, t2;
   if(is_mult)
   {
      eval_multiply(t1, a.n(), b.n());
      eval_multiply(t2, a.d(), b.d());
   }
   else
   {
      if(eval_is_zero(b.n()))
         BOOST_THROW_EXCEPTION(std::overflow_error("Division by zero"));
      eval_multiply(t1, a.n(), b.d());
      eval_multiply(t2, a.d(), b.n());
   }
   eval_gcd(result.d(), t1, t2);
   if(!eval_eq(result.d(), static_cast<ui_type>(1u)))
   {
      eval_divide(result.n(), t1, result.d());
      eval_divide(t1, t2, result.d());
      result.d() = BOOST_MP_MOVE(t1);
   }
   else
   {
      result.n() = BOOST_MP_MOVE(t1);
      result.d() = BOOST_MP_MOVE(t2);
   }
   if(eval_get_sign(result.d()) < 0)
   {
      result.n().negate();
      result.d().negate();
   }
}

template <class IntBackend, class T>
void rational_multiply_scalar(rational_adaptor<IntBackend>& result, const T& o)
{
   typedef typename mpl::front<typename IntBackend::unsigned_types>::type ui_type;

   using default_ops::eval_multiply;
   using default_ops::eval_gcd;
   using default_ops::eval_divide;
   using default_ops::eval_eq;

   IntBackend t1, t2;
   eval_multiply(t1, result.n(), o);
   eval_gcd(t2, t1, result.d());
   if(!eval_eq(t2, static_cast<ui_type>(1u)))
   {
      eval_divide(result.n(), t1, t2);
      eval_divide(t1, result.d(), t2);
      result.d() = BOOST_MP_MOVE(t1);
   }
   else
   {
      result.n() = BOOST_MP_MOVE(t1);
   }
}
template <class IntBackend, class T>
void rational_divide_scalar(rational_adaptor<IntBackend>& result, const T& o)
{
   typedef typename mpl::front<typename IntBackend::unsigned_types>::type ui_type;

   using default_ops::eval_multiply;
   using default_ops::eval_gcd;
   using default_ops::eval_divide;
   using default_ops::eval_eq;
   using default_ops::eval_is_zero;

   IntBackend t1, t2;
   eval_multiply(t1, result.d(), o);

   if(eval_is_zero(t1))
      BOOST_THROW_EXCEPTION(std::overflow_error("Division by zero"));

   eval_gcd(t2, t1, result.n());
   if(!eval_eq(t2, static_cast<ui_type>(1u)))
   {
      eval_divide(result.d(), t1, t2);
      eval_divide(t1, result.n(), t2);
      result.n() = BOOST_MP_MOVE(t1);
   }
   else
   {
      result.d() = BOOST_MP_MOVE(t1);
   }
   if(eval_get_sign(result.d()) < 0)
   {
      result.n().negate();
      result.d().negate();
   }
}

} // namespace detail


template <class IntBackend>
inline void eval_add(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& o)
{
   rational_adaptor<IntBackend> t;
   detail::rational_add_subtract(t, result, o, true);
   result = BOOST_MP_MOVE(t);
}
template <class IntBackend>
inline void eval_subtract(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& o)
{
   rational_adaptor<IntBackend> t;
   detail::rational_add_subtract(t, result, o, false);
   result = BOOST_MP_MOVE(t);
}
template <class IntBackend>
inline void eval_add(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const rational_adaptor<IntBackend>& b)
{
   detail::rational_add_subtract(result, a, b, true);
}
template <class IntBackend>
inline void eval_subtract(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const rational_adaptor<IntBackend>& b)
{
   detail::rational_add_subtract(result, a, b, false);
}

template <class IntBackend, class T>
inline typename enable_if_c<number_category<T>::value != number_kind_floating_point>::type eval_add(rational_adaptor<IntBackend>& result, const T& o)
{
   detail::rational_add_subtract_scalar(result, o, true);
}
template <class IntBackend, class T>
inline typename enable_if_c<number_category<T>::value != number_kind_floating_point>::type eval_subtract(rational_adaptor<IntBackend>& result, const T& o)
{
   detail::rational_add_subtract_scalar(result, o, false);
}
template <class IntBackend, class T>
inline typename enable_if_c<number_category<T>::value != number_kind_floating_point>::type eval_add(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const T& o)
{
   detail::rational_add_subtract_scalar(result, a, o, true);
}
template <class IntBackend, class T>
inline typename enable_if_c<number_category<T>::value != number_kind_floating_point>::type eval_subtract(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const T& o)
{
   detail::rational_add_subtract_scalar(result, a, o, false);
}

template <class IntBackend>
inline void eval_multiply(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& o)
{
   rational_adaptor<IntBackend> t;
   detail::rational_multiply_divide(t, result, o, true);
   result = BOOST_MP_MOVE(t);
}
template <class IntBackend>
inline void eval_divide(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& o)
{
   rational_adaptor<IntBackend> t;
   detail::rational_multiply_divide(t, result, o, false);
   result = BOOST_MP_MOVE(t);
}
template <class IntBackend>
inline void eval_multiply(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const rational_adaptor<IntBackend>& o)
{
   detail::rational_multiply_divide(result, a, o, true);
}
template <class IntBackend>
inline void eval_divide(rational_adaptor<IntBackend>& result, const rational_adaptor<IntBackend>& a, const rational_adaptor<IntBackend>& o)
{
   detail::rational_multiply_divide(result, a, o, false);
}

template <class IntBackend, class T>
inline typename enable_if_c<number_category<T>::value != number_kind_floating_point>::type eval_multiply(rational_adaptor<IntBackend>& result, const T& o)
{
   detail::rational_multiply_scalar(result, o);
}
template <class IntBackend, class T>
inline typename enable_if_c<number_category<T>::value != number_kind_floating_point>::type eval_divide(rational_adaptor<IntBackend>& result, const T& o)
{
   detail::rational_divide_scalar(result, o);
}

template <class R, class IntBackend>
inline typename enable_if_c<number_category<R>::value == number_kind_floating_point>::type eval_convert_to(R* result, const rational_adaptor<IntBackend>& backend)
{
   //
   // The generic conversion is as good as anything we can write here:
   //
   ::boost::multiprecision::detail::generic_convert_rational_to_float(*result, backend);
}

template <class R, class IntBackend>
inline typename enable_if_c<
   (number_category<R>::value != number_kind_integer) && (number_category<R>::value != number_kind_floating_point)>::type 
   eval_convert_to(R* result, const rational_adaptor<IntBackend>& arg)
{
   using default_ops::eval_convert_to;
   R t;
   eval_convert_to(result, arg.n());
   eval_convert_to(result, arg.d());
   *result /= t;
}

template <class R, class IntBackend>
inline typename enable_if_c<number_category<R>::value == number_kind_integer>::type 
   eval_convert_to(R* result, const rational_adaptor<IntBackend>& arg)
{
   using default_ops::eval_convert_to;
   using default_ops::eval_divide;
   IntBackend t;
   eval_divide(t, arg.n(), arg.d());
   eval_convert_to(result, t);
}

template <class IntBackend>
inline bool eval_is_zero(const rational_adaptor<IntBackend>& val)
{
   return eval_is_zero(val.n());
}
template <class IntBackend>
inline int eval_get_sign(const rational_adaptor<IntBackend>& val)
{
   return eval_get_sign(val.n());
}

template<class IntBackend, class V>
inline void assign_components(rational_adaptor<IntBackend>& result, const V& v1, const V& v2)
{
   result.n() = v1;
   result.d() = v2;
}

} // namespace backends

template<class IntBackend>
struct expression_template_default<backends::rational_adaptor<IntBackend> > : public expression_template_default<IntBackend> {};
   
template<class IntBackend>
struct number_category<backends::rational_adaptor<IntBackend> > : public mpl::int_<number_kind_rational>{};

using boost::multiprecision::backends::rational_adaptor;

template <class T>
struct component_type<rational_adaptor<T> >
{
   typedef number<T> type;
};

template <class IntBackend, expression_template_option ET>
inline number<IntBackend, ET> numerator(const number<rational_adaptor<IntBackend>, ET>& val)
{
   return val.backend().n();
}
template <class IntBackend, expression_template_option ET>
inline number<IntBackend, ET> denominator(const number<rational_adaptor<IntBackend>, ET>& val)
{
   return val.backend().d();
}

#ifdef BOOST_NO_SFINAE_EXPR

namespace detail{

template<class U, class IntBackend>
struct is_explicitly_convertible<U, rational_adaptor<IntBackend> > : public is_explicitly_convertible<U, IntBackend> {};

}

#endif

}} // namespaces


namespace std{

template <class IntBackend, boost::multiprecision::expression_template_option ExpressionTemplates>
class numeric_limits<boost::multiprecision::number<boost::multiprecision::rational_adaptor<IntBackend>, ExpressionTemplates> > : public std::numeric_limits<boost::multiprecision::number<IntBackend, ExpressionTemplates> >
{
   typedef std::numeric_limits<boost::multiprecision::number<IntBackend> > base_type;
   typedef boost::multiprecision::number<boost::multiprecision::rational_adaptor<IntBackend> > number_type;
public:
   BOOST_STATIC_CONSTEXPR bool is_integer = false;
   BOOST_STATIC_CONSTEXPR bool is_exact = true;
   BOOST_STATIC_CONSTEXPR number_type (min)() { return (base_type::min)(); }
   BOOST_STATIC_CONSTEXPR number_type (max)() { return (base_type::max)(); }
   BOOST_STATIC_CONSTEXPR number_type lowest() { return -(max)(); }
   BOOST_STATIC_CONSTEXPR number_type epsilon() { return base_type::epsilon(); }
   BOOST_STATIC_CONSTEXPR number_type round_error() { return epsilon() / 2; }
   BOOST_STATIC_CONSTEXPR number_type infinity() { return base_type::infinity(); }
   BOOST_STATIC_CONSTEXPR number_type quiet_NaN() { return base_type::quiet_NaN(); }
   BOOST_STATIC_CONSTEXPR number_type signaling_NaN() { return base_type::signaling_NaN(); }
   BOOST_STATIC_CONSTEXPR number_type denorm_min() { return base_type::denorm_min(); }
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION

template <class IntBackend, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::rational_adaptor<IntBackend>, ExpressionTemplates> >::is_integer;
template <class IntBackend, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::rational_adaptor<IntBackend>, ExpressionTemplates> >::is_exact;

#endif


}

#endif
