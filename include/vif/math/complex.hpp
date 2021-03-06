#ifndef VIF_MATH_COMPLEX_HPP
#define VIF_MATH_COMPLEX_HPP

#include "vif/core/vec.hpp"

// This file allows you to choose between the standard C++ implementation of
// complex numbers (the default choice), and a simple backup implementation
// from vif. This backup implementation was provided because of compatibility
// issues between the clang compiler and the gcc standard library implementation
// when enabling the C++14 mode, regarding the use of the 'constexpr' keyword.
// This issue has been fixed with more recent versionf of both compilers, so
// the workaround is not used by default anymore, but is still available for
// users running on older platforms.

#ifndef USE_VIF_COMPLEX

    #include <complex>

    namespace vif {
        // Just bring the standard complex number class into the vif namespace
        using std::complex;
    }

#else // USE_VIF_COMPLEX

    namespace vif {
        // Define our own complex number class
        template<typename T>
        struct complex {
            T re, im;

            template<typename U>
            void operator *= (U&& c) {
                *this = (*this)*std::forward<U>(c);
            }

            template<typename U>
            void operator /= (U&& c) {
                *this = (*this)/std::forward<U>(c);
            }

            template<typename U>
            void operator += (U&& c) {
                *this = (*this) + std::forward<U>(c);
            }

            template<typename U>
            void operator -= (U&& c) {
                *this = (*this) - std::forward<U>(c);
            }
        };

        T& real() {
            return re;
        }

        const T& real() const {
            return re;
        }

        T& imag() {
            return im;
        }

        const T& imag() const {
            return im;
        }

        template<typename T>
        T norm_sqr(complex<T> c) {
            return c.re*c.re + c.im*c.im;
        }

        template<typename T>
        T norm(complex<T> c) {
            return sqrt(norm_sqr(c));
        }

        template<typename T, typename U>
        auto operator* (complex<T> c1, complex<U> c2) -> complex<decltype(c1.re*c2.re)> {
            return {c1.re*c2.re - c1.im*c2.im, c1.re*c2.im + c1.im*c2.re};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator* (complex<T> c, U a) -> complex<decltype(c.re*a)> {
            return {c.re*a, c.im*a};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator* (U a, complex<T> c) -> complex<decltype(a*c.re)> {
            return {a*c.re, a*c.im};
        }

        template<typename T, typename U>
        auto operator/ (complex<T> c1, complex<U> c2) -> complex<decltype(c1.re*c2.re)> {
            auto tmp = 1.0/norm_sqr(c2);
            return {c1.re*(c2.re*tmp) + c1.im*(c2.im*tmp), -c1.re*(c2.im*tmp) + c1.im*(c2.re*tmp)};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator/ (complex<T> c, U a) -> complex<decltype(c.re/a)> {
            return {c.re/a, c.im/a};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator/ (U a, complex<T> c) -> complex<decltype(a/c.re)> {
            auto tmp = 1.0/norm_sqr(c);
            return {a*(c.re*tmp), -a*(c.im*tmp)};
        }

        template<typename T, typename U>
        auto operator+ (complex<T> c1, complex<U> c2) -> complex<decltype(c1.re + c2.re)> {
            return {c1.re + c2.re, c1.im + c2.im};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator+ (complex<T> c, U a) -> complex<decltype(c.re + a)> {
            return {c.re + a, c.im};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator+ (U a, complex<T> c) -> complex<decltype(a + c.re)> {
            return {c.re + a, c.im};
        }

        template<typename T, typename U>
        auto operator- (complex<T> c1, complex<U> c2) -> complex<decltype(c1.re - c2.re)> {
            return {c1.re - c2.re, c1.im - c2.im};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator- (complex<T> c, U a) -> complex<decltype(c.re - a)> {
            return {c.re - a, c.im};
        }

        template<typename T, typename U, typename enable =
            typename std::enable_if<std::is_arithmetic<U>::value>::type>
        auto operator- (U a, complex<T> c) -> complex<decltype(a - c.re)> {
            return {a - c.re, -c.im};
        }

        template<typename T>
        complex<T> operator+ (complex<T> c) {
            return c;
        }

        template<typename T>
        complex<T> operator- (complex<T> c) {
            return {-c.re, -c.im};
        }
    }

#endif // USE_VIF_COMPLEX

namespace vif {

    // Define new type aliases for complex vectors

    #define MAKE_TYPEDEFS(N) \
        using vec##N##cf  = vec<N, vif::complex<float>>; \
        using vec##N##cd  = vec<N, vif::complex<double>>;

    MAKE_TYPEDEFS(1)
    MAKE_TYPEDEFS(2)
    MAKE_TYPEDEFS(3)
    MAKE_TYPEDEFS(4)
    MAKE_TYPEDEFS(5)
    MAKE_TYPEDEFS(6)

    #undef MAKE_TYPEDEFS

    template<typename T>
    T real_part(const vif::complex<T>& c) {
        return c.real();
    }

    template<typename T>
    T imag_part(const vif::complex<T>& c) {
        return c.imag();
    }

    VIF_VECTORIZE(real_part);
    VIF_VECTORIZE(imag_part);
}

#endif
