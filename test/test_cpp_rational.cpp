///////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_

//
// Compare arithmetic results using fixed_int to GMP results.
//

#ifdef _MSC_VER
#  define _SCL_SECURE_NO_WARNINGS
#endif

#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/timer.hpp>
#include "test.hpp"

template <class T>
T generate_random(unsigned bits_wanted)
{
   static boost::random::mt19937 gen;
   typedef boost::random::mt19937::result_type random_type;

   T max_val;
   unsigned digits;
   if(std::numeric_limits<T>::is_bounded && (bits_wanted == (unsigned)std::numeric_limits<T>::digits))
   {
      max_val = (std::numeric_limits<T>::max)();
      digits = std::numeric_limits<T>::digits;
   }
   else
   {
      max_val = T(1) << bits_wanted;
      digits = bits_wanted;
   }

   unsigned bits_per_r_val = std::numeric_limits<random_type>::digits - 1;
   while((random_type(1) << bits_per_r_val) > (gen.max)()) --bits_per_r_val;

   unsigned terms_needed = digits / bits_per_r_val + 1;

   T val = 0;
   for(unsigned i = 0; i < terms_needed; ++i)
   {
      val *= (gen.max)();
      val += gen();
   }
   val %= max_val;
   return val;
}

template <class T>
struct is_checked_cpp_int : public boost::mpl::false_ {};
template <unsigned MinBits, unsigned MaxBits, boost::multiprecision::cpp_integer_type SignType, class Allocator, boost::multiprecision::expression_template_option ET>
struct is_checked_cpp_int<boost::multiprecision::number<boost::multiprecision::cpp_int_backend<MinBits, MaxBits, SignType, boost::multiprecision::checked, Allocator>, ET> > : public boost::mpl::true_ {};

template <class Number>
struct tester
{
   typedef Number test_type;
   typedef typename boost::multiprecision::component_type<test_type>::type test_int_type;
   typedef typename test_int_type::backend_type::checked_type checked;

   unsigned last_error_count;
   boost::timer tim;

   boost::multiprecision::mpq_rational a, b, c, d;
   int si;
   unsigned ui;
   test_type a1, b1, c1, d1;


   void t1()
   {
      using namespace boost::multiprecision;
      BOOST_CHECK_EQUAL(a.str(), a1.str());
      BOOST_CHECK_EQUAL(b.str(), b1.str());
      BOOST_CHECK_EQUAL(c.str(), c1.str());
      BOOST_CHECK_EQUAL(d.str(), d1.str());
      BOOST_CHECK_EQUAL(mpq_rational(a+b).str(), test_type(a1 + b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)+=b).str(), (test_type(a1) += b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(b)+=a).str(), (test_type(b1) += a1).str());
      BOOST_CHECK_EQUAL(mpq_rational(a-b).str(), test_type(a1 - b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)-=b).str(), (test_type(a1) -= b1).str());
      BOOST_CHECK_EQUAL(mpq_rational(mpq_rational(-a)+b).str(), test_type(test_type(-a1) + b1).str());
      BOOST_CHECK_EQUAL(mpq_rational(mpq_rational(-a)-b).str(), test_type(test_type(-a1) - b1).str());
      BOOST_CHECK_EQUAL(mpq_rational(c * d).str(), test_type(c1 * d1).str());
      BOOST_CHECK_EQUAL((mpq_rational(c)*=d).str(), (test_type(c1) *= d1).str());
      BOOST_CHECK_EQUAL((mpq_rational(d)*=c).str(), (test_type(d1) *= c1).str());
      BOOST_CHECK_EQUAL(mpq_rational(c * -d).str(), test_type(c1 * -d1).str());
      BOOST_CHECK_EQUAL(mpq_rational(-c * d).str(), test_type(-c1 * d1).str());
      BOOST_CHECK_EQUAL((mpq_rational(c)*=-d).str(), (test_type(c1) *= -d1).str());
      BOOST_CHECK_EQUAL((mpq_rational(-d)*=c).str(), (test_type(-d1) *= c1).str());
      BOOST_CHECK_EQUAL(mpq_rational(b * c).str(), test_type(b1 * c1).str());
      BOOST_CHECK_EQUAL(mpq_rational(a / b).str(), test_type(a1 / b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)/=b).str(), (test_type(a1) /= b1).str());
      BOOST_CHECK_EQUAL(mpq_rational(a / -b).str(), test_type(a1 / -b1).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a / b).str(), test_type(-a1 / b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)/=-b).str(), (test_type(a1) /= -b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)/=b).str(), (test_type(-a1) /= b1).str());
      BOOST_CHECK_EQUAL(mpq_rational(a / d).str(), test_type(a1 / d1).str());

      if(!std::numeric_limits<test_type>::is_bounded)
      {
         test_type p = a1 * b1;
         p /= b1;
         BOOST_CHECK_EQUAL(p, a1);

         p = a1 * d1;
         p /= d1;
         BOOST_CHECK_EQUAL(p, a1);

         p /= test_type(1);
         BOOST_CHECK_EQUAL(p, a1);
      }
   }

   void t3()
   {
      using namespace boost::multiprecision;
      // Now check operations involving signed integers:
      BOOST_CHECK_EQUAL(mpq_rational(a + si).str(), test_type(a1 + si).str());
      BOOST_CHECK_EQUAL(mpq_rational(a + -si).str(), test_type(a1 + -si).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a + si).str(), test_type(-a1 + si).str());
      BOOST_CHECK_EQUAL(mpq_rational(si + a).str(), test_type(si + a1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)+=si).str(), (test_type(a1) += si).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)+=-si).str(), (test_type(a1) += -si).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)+=si).str(), (test_type(-a1) += si).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)+=-si).str(), (test_type(-a1) += -si).str());
      BOOST_CHECK_EQUAL(mpq_rational(a - si).str(), test_type(a1 - si).str());
      BOOST_CHECK_EQUAL(mpq_rational(a - -si).str(), test_type(a1 - -si).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a - si).str(), test_type(-a1 - si).str());
      BOOST_CHECK_EQUAL(mpq_rational(si - a).str(), test_type(si - a1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)-=si).str(), (test_type(a1) -= si).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)-=-si).str(), (test_type(a1) -= -si).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)-=si).str(), (test_type(-a1) -= si).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)-=-si).str(), (test_type(-a1) -= -si).str());
      BOOST_CHECK_EQUAL(mpq_rational(b * si).str(), test_type(b1 * si).str());
      BOOST_CHECK_EQUAL(mpq_rational(b * -si).str(), test_type(b1 * -si).str());
      BOOST_CHECK_EQUAL(mpq_rational(-b * si).str(), test_type(-b1 * si).str());
      BOOST_CHECK_EQUAL(mpq_rational(si * b).str(), test_type(si * b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)*=si).str(), (test_type(a1) *= si).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)*=-si).str(), (test_type(a1) *= -si).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)*=si).str(), (test_type(-a1) *= si).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)*=-si).str(), (test_type(-a1) *= -si).str());
      if(si)
      {
         BOOST_CHECK_EQUAL(mpq_rational(a / si).str(), test_type(a1 / si).str());
         BOOST_CHECK_EQUAL(mpq_rational(a / -si).str(), test_type(a1 / -si).str());
         BOOST_CHECK_EQUAL(mpq_rational(-a / si).str(), test_type(-a1 / si).str());
         BOOST_CHECK_EQUAL((mpq_rational(a) /= si).str(), (test_type(a1) /= si).str());
         BOOST_CHECK_EQUAL((mpq_rational(a) /= -si).str(), (test_type(a1) /= -si).str());
         BOOST_CHECK_EQUAL((mpq_rational(-a) /= si).str(), (test_type(-a1) /= si).str());
         BOOST_CHECK_EQUAL((mpq_rational(-a) /= -si).str(), (test_type(-a1) /= -si).str());
      }
   }

   void t4()
   {
      using namespace boost::multiprecision;
      // Now check operations involving unsigned integers:
      BOOST_CHECK_EQUAL(mpq_rational(a + ui).str(), test_type(a1 + ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a + ui).str(), test_type(-a1 + ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(ui + a).str(), test_type(ui + a1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)+=ui).str(), (test_type(a1) += ui).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)+=ui).str(), (test_type(-a1) += ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(a - ui).str(), test_type(a1 - ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a - ui).str(), test_type(-a1 - ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(ui - a).str(), test_type(ui - a1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)-=ui).str(), (test_type(a1) -= ui).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)-=ui).str(), (test_type(-a1) -= ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(b * ui).str(), test_type(b1 * ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(-b * ui).str(), test_type(-b1 * ui).str());
      BOOST_CHECK_EQUAL(mpq_rational(ui * b).str(), test_type(ui * b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a)*=ui).str(), (test_type(a1) *= ui).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a)*=ui).str(), (test_type(-a1) *= ui).str());
      if(ui)
      {
         BOOST_CHECK_EQUAL(mpq_rational(a / ui).str(), test_type(a1 / ui).str());
         BOOST_CHECK_EQUAL(mpq_rational(-a / ui).str(), test_type(-a1 / ui).str());
         BOOST_CHECK_EQUAL((mpq_rational(a) /= ui).str(), (test_type(a1) /= ui).str());
         BOOST_CHECK_EQUAL((mpq_rational(-a) /= ui).str(), (test_type(-a1) /= ui).str());
      }
   }
   void t5()
   {
      using namespace boost::multiprecision;
      // Now check operations involving signed integers:
      BOOST_CHECK_EQUAL(mpq_rational(a + si).str(), test_type(a1 + test_int_type(si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(a + -si).str(), test_type(a1 + test_int_type(-si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a + si).str(), test_type(-a1 + test_int_type(si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(si + a).str(), test_type(test_int_type(si) + a1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a) += si).str(), (test_type(a1) += test_int_type(si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(a) += -si).str(), (test_type(a1) += test_int_type(-si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a) += si).str(), (test_type(-a1) += test_int_type(si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a) += -si).str(), (test_type(-a1) += test_int_type(-si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(a - si).str(), test_type(a1 - test_int_type(si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(a - -si).str(), test_type(a1 - test_int_type(-si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(-a - si).str(), test_type(-a1 - test_int_type(si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(si - a).str(), test_type(test_int_type(si) - a1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a) -= si).str(), (test_type(a1) -= test_int_type(si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(a) -= -si).str(), (test_type(a1) -= test_int_type(-si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a) -= si).str(), (test_type(-a1) -= test_int_type(si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a) -= -si).str(), (test_type(-a1) -= test_int_type(-si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(b * si).str(), test_type(b1 * test_int_type(si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(b * -si).str(), test_type(b1 * test_int_type(-si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(-b * si).str(), test_type(-b1 * test_int_type(si)).str());
      BOOST_CHECK_EQUAL(mpq_rational(si * b).str(), test_type(test_int_type(si) * b1).str());
      BOOST_CHECK_EQUAL((mpq_rational(a) *= si).str(), (test_type(a1) *= test_int_type(si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(a) *= -si).str(), (test_type(a1) *= test_int_type(-si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a) *= si).str(), (test_type(-a1) *= test_int_type(si)).str());
      BOOST_CHECK_EQUAL((mpq_rational(-a) *= -si).str(), (test_type(-a1) *= test_int_type(-si)).str());
      if(si)
      {
         BOOST_CHECK_EQUAL(mpq_rational(a / si).str(), test_type(a1 / test_int_type(si)).str());
         BOOST_CHECK_EQUAL(mpq_rational(a / -si).str(), test_type(a1 / test_int_type(-si)).str());
         BOOST_CHECK_EQUAL(mpq_rational(-a / si).str(), test_type(-a1 / test_int_type(si)).str());
         BOOST_CHECK_EQUAL((mpq_rational(a) /= si).str(), (test_type(a1) /= test_int_type(si)).str());
         BOOST_CHECK_EQUAL((mpq_rational(a) /= -si).str(), (test_type(a1) /= test_int_type(-si)).str());
         BOOST_CHECK_EQUAL((mpq_rational(-a) /= si).str(), (test_type(-a1) /= test_int_type(si)).str());
         BOOST_CHECK_EQUAL((mpq_rational(-a) /= -si).str(), (test_type(-a1) /= test_int_type(-si)).str());
      }
   }



   void test()
   {
      using namespace boost::multiprecision;

      last_error_count = 0;

      BOOST_CHECK_EQUAL(Number(), 0);

      for(int i = 0; i < 10000; ++i)
      {
         a = generate_random<mpz_int>(1000);
         a /= generate_random<mpz_int>(1000);
         b = generate_random<mpz_int>(512);
         b /= generate_random<mpz_int>(512);
         c = generate_random<mpz_int>(256);
         c /= generate_random<mpz_int>(256);
         d = generate_random<mpz_int>(32);
         d /= generate_random<mpz_int>(32);

         si = d.convert_to<int>();
         ui = si;

         a1 = static_cast<test_type>(a.str());
         b1 = static_cast<test_type>(b.str());
         c1 = static_cast<test_type>(c.str());
         d1 = static_cast<test_type>(d.str());

         t1();
#ifndef SLOW_COMPILER
         t3();
         t4();
         t5();
#endif

         if(last_error_count != (unsigned)boost::detail::test_errors())
         {
            last_error_count = boost::detail::test_errors();
            std::cout << std::hex << std::showbase;

            std::cout << "a    = " << a << std::endl;
            std::cout << "a1   = " << a1 << std::endl;
            std::cout << "b    = " << b << std::endl;
            std::cout << "b1   = " << b1 << std::endl;
            std::cout << "c    = " << c << std::endl;
            std::cout << "c1   = " << c1 << std::endl;
            std::cout << "d    = " << d << std::endl;
            std::cout << "d1   = " << d1 << std::endl;
            std::cout << "a + b   = " << a+b << std::endl;
            std::cout << "a1 + b1 = " << a1+b1 << std::endl;
            std::cout << std::dec;
            std::cout << "a - b   = " << a-b << std::endl;
            std::cout << "a1 - b1 = " << a1-b1 << std::endl;
            std::cout << "-a + b   = " << mpq_rational(-a)+b << std::endl;
            std::cout << "-a1 + b1 = " << test_type(-a1)+b1 << std::endl;
            std::cout << "-a - b   = " << mpq_rational(-a) - b << std::endl;
            std::cout << "-a1 - b1 = " << test_type(-a1)-b1 << std::endl;
            std::cout << "c*d    = " << c*d << std::endl;
            std::cout << "c1*d1  = " << c1*d1 << std::endl;
            std::cout << "b*c    = " << b*c << std::endl;
            std::cout << "b1*c1  = " << b1*c1 << std::endl;
            std::cout << "a/b    = " << a/b << std::endl;
            std::cout << "a1/b1  = " << a1/b1 << std::endl;
            std::cout << "a/d    = " << a/d << std::endl;
            std::cout << "a1/d1  = " << a1/d1 << std::endl;
         }

         //
         // Check to see if test is taking too long.
         // Tests run on the compiler farm time out after 300 seconds,
         // so don't get too close to that:
         //
         if(tim.elapsed() > 200)
         {
            std::cout << "Timeout reached, aborting tests now....\n";
            break;
         }

      }
   }
};

#if !defined(TEST1) && !defined(TEST2) && !defined(TEST3)
#define TEST1
#define TEST2
#define TEST3
#endif

int main()
{
   using namespace boost::multiprecision;
#ifdef TEST1
   tester<cpp_rational> t1;
   t1.test();
#endif
#ifdef TEST2
   tester<checked_cpp_rational> t2;
   t2.test();
#endif
#ifdef TEST3
   tester<number<rational_adaptor<cpp_int_backend<2048, 2048, signed_magnitude, checked> > > > t3;
   t3.test();
#endif
   return boost::report_errors();
}



