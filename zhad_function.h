#ifndef ZHAD_FUNCTION_H__
#define ZHAD_FUNCTION_H__

#include "zhad.h"

//#define ZHAD_FUNCTION_COMPARISON

namespace zhad_function {

using namespace zhad;

#ifdef M_PI
static constexpr double PI = M_PI;
#else
//static constexpr double PI = std::acos(-1)
static constexpr double PI = 3.14159265358979323846;
#endif
#ifdef M_E
static constexpr double E = M_E;
#else
//static constexpr double E = <sum for k=0 to infinity of 1/k!>
static constexpr double E = 2.718281828459045534885;
#endif

////////////////////// Addition ///////////////////////////

inline ADValue operator+(const ADValue& adValL, const ADValue& adValR) {
    return ADValue(adValL.value() + adValR.value(), adValL, 1.0, adValR, 1.0, 0.0);
}
inline ADValue operator+(const ADValue& adValL, double valR) {
    return ADValue(adValL.value() + valR, adValL, 1.0, 0.0);
}
inline ADValue operator+(double valL, const ADValue& adValR) {
    return adValR + valL;
}

////////////////// Subtraction ////////////////////////////

inline ADValue operator-(const ADValue& adValL, const ADValue& adValR) {
    return ADValue(adValL.value() - adValR.value(), adValL, 1.0, adValR, -1.0, 0.0);
}
inline ADValue operator-(const ADValue& adValL, double valR) {
    return ADValue(adValL.value() - valR, adValL, 1.0, 0.0);
}
inline ADValue operator-(double valL, const ADValue& adValR) {
    return ADValue(valL - adValR.value(), adValR, -1.0, 0.0);
}
inline ADValue operator-(const ADValue& adVal) {
    return ADValue(-adVal.value(), adVal, -1.0, 0.0);
}

////////////////// Multiplication /////////////////////////

inline ADValue operator*(const ADValue& adValL, const ADValue& adValR) {
    return ADValue(adValL.value() * adValR.value(), adValL, adValR.value(), adValR, adValL.value(), 1.0);
}
inline ADValue operator*(const ADValue& adValL, double valR) {
    return ADValue(adValL.value() * valR, adValL, valR, 0.0);
}
inline ADValue operator*(double valL, const ADValue& adValR) {
    return adValR * valL;
}

////////////////// Inversion //////////////////////////////

inline ADValue inv(const ADValue& adVal) {
    double invVal = 1.0 / adVal.value();
    double invValSq = invVal * invVal;
    return ADValue(invVal, adVal, -invValSq, 2.0 * invValSq * invVal);
}
inline double inv(double val) {
    return 1.0 / val;
}

////////////////// Division ///////////////////////////////

inline ADValue operator/(const ADValue& adValL, const ADValue& adValR) {
    return adValL * inv(adValR);
}
inline ADValue operator/(const ADValue& adValL, double valR) {
    return adValL * (1.0 / valR);
}
inline ADValue operator/(double valL, const ADValue& adValR) {
    return valL * inv(adValR);
}

//////////////// Misc functions ///////////////////////////

inline ADValue square(const ADValue& adVal) {
    return ADValue(adVal.value() * adVal.value(), adVal, 2.0 * adVal.value(), 0.0);
}
inline double square(double val) {
    return val * val;
}

inline ADValue sqrt(const ADValue& adVal) {
    double sqrtVal = std::sqrt(adVal.value());
    double invSqrtVal = 1.0 / sqrtVal;
    return ADValue(sqrtVal, adVal, 0.5 * invSqrtVal, -0.25 * invSqrtVal / adVal.value());
}

inline ADValue pow(const ADValue& adVal, double expVal) {
    double powVal0 = std::pow(adVal.value(), expVal);
    double powVal1 = powVal0 / adVal.value(); //std::pow(adVal.value(), (expVal - 1.0));
    return ADValue(powVal0, adVal, expVal * powVal1, expVal * (expVal - 1.0) * powVal1 / adVal.value());
}

inline ADValue exp(const ADValue& adVal) {
    double expVal = std::exp(adVal.value());
    return ADValue(expVal, adVal, expVal, expVal);
}

inline ADValue log(const ADValue& adVal) {
    double invVal = 1.0 / adVal.value();
    return ADValue(std::log(adVal.value()), adVal, invVal, -invVal * invVal);
}

inline ADValue sin(const ADValue& adVal) {
    double sinVal = std::sin(adVal.value());
    return ADValue(sinVal, adVal, std::cos(adVal.value()), -sinVal);
}

inline ADValue cos(const ADValue& adVal) {
    double cosVal = std::cos(adVal.value());
    return ADValue(cosVal, adVal, -std::sin(adVal.value()), -cosVal);
}

inline ADValue tan(const ADValue& adVal) {
    double tanVal = std::tan(adVal.value());
    double secVal = 1.0 / std::cos(adVal.value());
    double secValSqr = secVal * secVal;
    return ADValue(tanVal, adVal, secValSqr, 2.0 * tanVal * secValSqr);
}

inline ADValue asin(const ADValue& adVal) {
    double tmp = 1.0 / (1.0 - adVal.value() * adVal.value());
    double sqrtTmp = std::sqrt(tmp);
    return ADValue(std::asin(adVal.value()), adVal, sqrtTmp, adVal.value() * sqrtTmp * tmp);
}

inline ADValue acos(const ADValue& adVal) {
    double tmp = 1.0 / (1.0 - adVal.value() * adVal.value());
    double negSqrtTmp = -std::sqrt(tmp);
    return ADValue(std::acos(adVal.value()), adVal, negSqrtTmp, adVal.value() * negSqrtTmp * tmp);
}

// TODO: Add more functions

////////////////// Comparisons ////////////////////////////

#ifdef ZHAD_FUNCTION_COMPARISON

inline bool operator<(const ADValue& adValL, const ADValue& adValR) {
    return adValL.value() < adValR.value();
}

inline bool operator<=(const ADValue& adValL, const ADValue& adValR) {
    return adValL.value() <= adValR.value();
}

inline bool operator>(const ADValue& adValL, const ADValue& adValR) {
    return adValL.value() > adValR.value();
}

inline bool operator>=(const ADValue& adValL, const ADValue& adValR) {
    return adValL.value() >= adValR.value();
}

inline bool operator==(const ADValue& adValL, const ADValue& adValR) {
    return adValL.value() == adValR.value();
}

#endif //ZHAD_FUNCTION_COMPARISON

} //namespace zhad_function

#endif //ZHAD_FUNCTION_H__
