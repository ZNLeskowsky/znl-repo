#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>

//#define ZHAD_DEBUG

#include "zhad.h"
#include "zhad_function.h"

using namespace std;
using namespace zhad;
using namespace zhad_function;

#define NearEqualAssert(a, b) \
    assert(std::fabs((a) - (b)) < 1e-8)

double testAdd() {
    double x0 = 1.0;
    double x1 = 2.0;
    double x2 = 3.0;
    double y = x0 + x0 + x1 + x2;
    return y;
}

void testAdd(ADDepVar& y, ADVar& x0, ADVar& x1, ADVar& x2) {
    x0 = 1.0;
    x1 = 2.0;
    x2 = 3.0;
    y = x0 + x0 + x1 + x2;

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);
    double dydx2 = y.derivative1(x2);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx0x2 = y.derivative2(x0, x2);
    double dydx1x1 = y.derivative2(x1, x1);
    double dydx1x2 = y.derivative2(x1, x2);
    double dydx2x2 = y.derivative2(x2, x2);

    NearEqualAssert(dydx0, 2.0);
    NearEqualAssert(dydx1, 1.0);
    NearEqualAssert(dydx2, 1.0);

    NearEqualAssert(dydx0x0, 0.0);
    NearEqualAssert(dydx0x1, 0.0);
    NearEqualAssert(dydx0x2, 0.0);
    NearEqualAssert(dydx1x1, 0.0);
    NearEqualAssert(dydx1x2, 0.0);
    NearEqualAssert(dydx2x2, 0.0);
}

double testMinus() {
    double x0 = 1.0;
    double x1 = 2.0;
    double x2 = 3.0;
    double y = - x0 - x0 - x1 - x2;
    return y;
}

void testMinus(ADDepVar& y, ADVar& x0, ADVar& x1, ADVar& x2) {
    x0 = 1.0;
    x1 = 2.0;
    x2 = 3.0;
    y = -x0 - x0 - x1 - x2;

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);
    double dydx2 = y.derivative1(x2);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx0x2 = y.derivative2(x0, x2);
    double dydx1x1 = y.derivative2(x1, x1);
    double dydx1x2 = y.derivative2(x1, x2);
    double dydx2x2 = y.derivative2(x2, x2);

    NearEqualAssert(dydx0, -2.0);
    NearEqualAssert(dydx1, -1.0);
    NearEqualAssert(dydx2, -1.0);

    NearEqualAssert(dydx0x0, 0.0);
    NearEqualAssert(dydx0x1, 0.0);
    NearEqualAssert(dydx0x2, 0.0);
    NearEqualAssert(dydx1x1, 0.0);
    NearEqualAssert(dydx1x2, 0.0);
    NearEqualAssert(dydx2x2, 0.0);
}

double testMultiply() {
    double x0 = 1.0;
    double x1 = 2.0;
    double x2 = 3.0;
    double y = x0 * x0 * x1 * x1 * x2;
    return y;
}

void testMultiply(ADDepVar& y, ADVar& x0, ADVar& x1, ADVar& x2) {
    x0 = 1.0;
    x1 = 2.0;
    x2 = 3.0;
    y = x0 * x0 * x1 * x1 * x2;
    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);
    double dydx2 = y.derivative1(x2);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx0x2 = y.derivative2(x0, x2);
    double dydx1x1 = y.derivative2(x1, x1);
    double dydx1x2 = y.derivative2(x1, x2);
    double dydx2x2 = y.derivative2(x2, x2);

    NearEqualAssert(dydx0, 2.0 * x0.value() * x1.value() * x1.value() * x2.value());
    NearEqualAssert(dydx1, 2.0 * x0.value() * x0.value() * x1.value() * x2.value());
    NearEqualAssert(dydx2, x0.value() * x0.value() * x1.value() * x1.value());

    NearEqualAssert(dydx0x0, 2.0 * x1.value() * x1.value() * x2.value());
    NearEqualAssert(dydx0x1, 4.0 * x0.value() * x1.value() * x2.value());
    NearEqualAssert(dydx0x2, 2.0 * x0.value() * x1.value() * x1.value());
    NearEqualAssert(dydx1x1, 2.0 * x0.value() * x0.value() * x2.value());
    NearEqualAssert(dydx1x2, 2.0 * x0.value() * x0.value() * x1.value());
    NearEqualAssert(dydx2x2, 0.0);
}

double testDivide() {
    double x0 = 1.0;
    double x1 = 2.0;
    double x2 = 3.0;
    double y = (x0 * x0) / (x1 * x2);
    return y;
}

void testDivide(ADDepVar& y, ADVar& x0, ADVar& x1, ADVar& x2) {
    x0 = 1.0;
    x1 = 2.0;
    x2 = 3.0;
    y = (x0 * x0) / (x1 * x2);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);
    double dydx2 = y.derivative1(x2);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx0x2 = y.derivative2(x0, x2);
    double dydx1x1 = y.derivative2(x1, x1);
    double dydx1x2 = y.derivative2(x1, x2);
    double dydx2x2 = y.derivative2(x2, x2);

    NearEqualAssert(dydx0, 2.0 * x0.value() / (x1.value() * x2.value()));
    NearEqualAssert(dydx1, - (x0.value() * x0.value()) / (x1.value() * x1.value() * x2.value()));
    NearEqualAssert(dydx2, - (x0.value() * x0.value()) / (x1.value() * x2.value() * x2.value()));

    NearEqualAssert(dydx0x0, 2.0 / (x1.value() * x2.value()));
    NearEqualAssert(dydx0x1, -2.0 * x0.value() / (x1.value() * x1.value() * x2.value()));
    NearEqualAssert(dydx0x2, -2.0 * x0.value() / (x1.value() * x2.value() * x2.value()));
    NearEqualAssert(dydx1x1, (2.0 * x0.value() * x0.value()) / (x1.value() * x1.value() * x1.value() * x2.value()));
    NearEqualAssert(dydx1x2, x0.value() * x0.value() / (x1.value() * x1.value() * x2.value() * x2.value()));
    NearEqualAssert(dydx2x2, (2.0 * x0.value() * x0.value()) / (x1.value() * x2.value() * x2.value() * x2.value()));
}

double testSqrt() {
    double x0 = 1.0;
    double x1 = 2.0;
    double x2 = 3.0;
    double y = sqrt(x0 * x1 + x2);
    return y;
}

void testSqrt(ADDepVar& y, ADVar& x0, ADVar& x1, ADVar& x2) {
    x0 = 1.0;
    x1 = 2.0;
    x2 = 3.0;
    y = sqrt(x0 * x1 + x2);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);
    double dydx2 = y.derivative1(x2);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx0x2 = y.derivative2(x0, x2);
    double dydx1x1 = y.derivative2(x1, x1);
    double dydx1x2 = y.derivative2(x1, x2);
    double dydx2x2 = y.derivative2(x2, x2);


    NearEqualAssert(dydx0, x1.value() / (2.0 * sqrt(x0.value() * x1.value() + x2.value())));
    NearEqualAssert(dydx1, x0.value() / (2.0 * sqrt(x0.value() * x1.value() + x2.value())));
    NearEqualAssert(dydx2, 1.0 / (2.0 * sqrt(x0.value() * x1.value() + x2.value())));

    NearEqualAssert(dydx0x0, - (x1.value() * x1.value()) / (4.0 * pow(x0.value() * x1.value() + x2.value(), 3.0/2.0)));
    NearEqualAssert(dydx0x1, (x0.value() * x1.value() + 2.0 * x2.value()) / (4.0 * pow(x0.value() * x1.value() + x2.value(), 3.0/2.0)));
    NearEqualAssert(dydx0x2, - (x1.value()) / (4.0 * pow(x0.value() * x1.value() + x2.value(), 3.0/2.0)));
    NearEqualAssert(dydx1x1, - (x0.value() * x0.value()) / (4.0 * pow(x0.value() * x1.value() + x2.value(), 3.0/2.0)));
    NearEqualAssert(dydx1x2, - (x0.value()) / (4.0 * pow(x0.value() * x1.value() + x2.value(), 3.0/2.0)));
    NearEqualAssert(dydx2x2, -1.0 / (4.0 * pow(x0.value() * x1.value() + x2.value(), 3.0/2.0)));
}

double testPow() {
    double x0 = 1.0;
    double x1 = 2.0;
    double y = pow(x0 + x1, 5.3);
    return y;
}

void testPow(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 1.0;
    x1 = 2.0;
    y = pow(x0 + x1, 5.3);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, 5.3 * pow(x0.value() + x1.value(), 4.3));
    NearEqualAssert(dydx1, 5.3 * pow(x0.value() + x1.value(), 4.3));

    NearEqualAssert(dydx0x0, 22.79 * pow(x0.value() + x1.value(), 3.3));
    NearEqualAssert(dydx0x1, 22.79 * pow(x0.value() + x1.value(), 3.3));
    NearEqualAssert(dydx1x1, 22.79 * pow(x0.value() + x1.value(), 3.3));
}

double testExp() {
    double x0 = 1.0;
    double x1 = 2.0;
    double y = exp(x0 * x0 + x1);
    return y;
}

void testExp(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 1.0;
    x1 = 2.0;
    y = exp(x0 * x0 + x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, 2.0 * x0.value() * exp(x0.value() * x0.value() + x1.value()));
    NearEqualAssert(dydx1, exp(x0.value() * x0.value() + x1.value()));

    NearEqualAssert(dydx0x0, 2.0 * (2.0 * x0.value() * x0.value() + 1.0) * exp(x0.value() * x0.value() + x1.value()));
    NearEqualAssert(dydx0x1, 2.0 * x0.value() * exp(x0.value() * x0.value() + x1.value()));
    NearEqualAssert(dydx1x1, exp(x0.value() * x0.value() + x1.value()));
}

double testLog() {
    double x0 = 1.0;
    double x1 = 2.0;
    double y = log(x0 * x0 + x1);
    return y;
}

void testLog(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 1.0;
    x1 = 2.0;
    y = log(x0 * x0 + x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, 2.0 * x0.value() / (x0.value() * x0.value() + x1.value()));
    NearEqualAssert(dydx1, 1.0 / (x0.value() * x0.value() + x1.value()));

    NearEqualAssert(dydx0x0, -2.0 * (x0.value() * x0.value() - x1.value()) / ((x0.value() * x0.value() + x1.value()) * (x0.value() * x0.value() + x1.value())));
    NearEqualAssert(dydx0x1, -2.0 * x0.value() / ((x0.value() * x0.value() + x1.value()) * (x0.value() * x0.value() + x1.value())));
    NearEqualAssert(dydx1x1, -1.0 / ((x0.value() * x0.value() + x1.value()) * (x0.value() * x0.value() + x1.value())));
}

double testSin() {
    double x0 = 1.0;
    double x1 = 2.0;
    double y = sin(x0) * sin(x1);
    return y;
}

void testSin(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 1.0;
    x1 = 2.0;
    y = sin(x0) * sin(x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, cos(x0.value()) * sin(x1.value()));
    NearEqualAssert(dydx1, sin(x0.value()) * cos(x1.value()));

    NearEqualAssert(dydx0x0, - sin(x0.value()) * sin(x1.value()));
    NearEqualAssert(dydx0x1, cos(x0.value()) * cos(x1.value()));
    NearEqualAssert(dydx1x1, - sin(x0.value()) * sin(x1.value()));
}

double testCos() {
    double x0 = 1.0;
    double x1 = 2.0;
    double y = cos(x0) * cos(x1);
    return y;
}

void testCos(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 1.0;
    x1 = 2.0;
    y = cos(x0) * cos(x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, - sin(x0.value()) * cos(x1.value()));
    NearEqualAssert(dydx1, - cos(x0.value()) * sin(x1.value()));

    NearEqualAssert(dydx0x0, - cos(x0.value()) * cos(x1.value()));
    NearEqualAssert(dydx0x1,   sin(x0.value()) * sin(x1.value()));
    NearEqualAssert(dydx1x1, - cos(x0.value()) * cos(x1.value()));
}

double testTan() {
    double x0 = 1.0;
    double x1 = 2.0;
    double y = tan(x0) * tan(x1);
    return y;
}

void testTan(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 1.0;
    x1 = 2.0;
    y = tan(x0) * tan(x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, tan(x1.value()) / (cos(x0.value()) * cos(x0.value())));
    NearEqualAssert(dydx1, tan(x0.value()) / (cos(x1.value()) * cos(x1.value())));

    NearEqualAssert(dydx0x0, 2.0 * tan(x0.value()) * tan(x1.value()) / (cos(x0.value()) * cos(x0.value())));
    NearEqualAssert(dydx0x1, 1.0 / (cos(x0.value()) * cos(x0.value()) * cos(x1.value()) * cos(x1.value())));
    NearEqualAssert(dydx1x1, 2.0 * tan(x0.value()) * tan(x1.value()) / (cos(x1.value()) * cos(x1.value())));
}

double testASin() {
    double x0 = 0.3;
    double x1 = 0.6;
    double y = asin(x0) * asin(x1);
    return y;
}

void testASin(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 0.3;
    x1 = 0.6;
    y = asin(x0) * asin(x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, asin(x1.value()) / sqrt(1.0 - x0.value() * x0.value()));
    NearEqualAssert(dydx1, asin(x0.value()) / sqrt(1.0 - x1.value() * x1.value()));

    NearEqualAssert(dydx0x0, x0.value() * asin(x1.value()) / pow(1.0 - x0.value() * x0.value(), 3.0/2.0));
    NearEqualAssert(dydx0x1, 1.0 / (sqrt(1.0 - x0.value() * x0.value()) * sqrt(1.0 - x1.value() * x1.value())));
    NearEqualAssert(dydx1x1, x1.value() * asin(x0.value()) / pow(1.0 - x1.value() * x1.value(), 3.0/2.0));
}

double testACos() {
    double x0 = 0.3;
    double x1 = 0.6;
    double y = acos(x0) * acos(x1);
    return y;
}

void testACos(ADDepVar& y, ADVar& x0, ADVar& x1) {
    x0 = 0.3;
    x1 = 0.6;
    y = acos(x0) * acos(x1);

    double dydx0 = y.derivative1(x0);
    double dydx1 = y.derivative1(x1);

    double dydx0x0 = y.derivative2(x0, x0);
    double dydx0x1 = y.derivative2(x0, x1);
    double dydx1x1 = y.derivative2(x1, x1);

    NearEqualAssert(dydx0, - acos(x1.value()) / sqrt(1.0 - x0.value() * x0.value()));
    NearEqualAssert(dydx1, - acos(x0.value()) / sqrt(1.0 - x1.value() * x1.value()));

    NearEqualAssert(dydx0x0, - x0.value() * acos(x1.value()) / pow(1.0 - x0.value() * x0.value(), 3.0/2.0));
    NearEqualAssert(dydx0x1, 1.0 / (sqrt(1.0 - x0.value() * x0.value()) * sqrt(1.0 - x1.value() * x1.value())));
    NearEqualAssert(dydx1x1, - x1.value() * acos(x0.value()) / pow(1.0 - x1.value() * x1.value(), 3.0/2.0));
}

void testClone(ADDepVar& y, ADVar& x) {
    ADVar xClone;
    x = 0.3;
    std::memcpy(&xClone, &x, sizeof(ADVar)); // can't use copy constructor because it doesn't copy id
    y = x * xClone; // y = x^2

    double dydx = y.derivative1(x);

    double dydxx = y.derivative2(x, x);

    NearEqualAssert(dydx, 2.0 * x.value());

    NearEqualAssert(dydxx, 2.0);
}

void testVectorReserveAndClear() {
    std::vector<double> vec;
    vec.reserve(10);
    for (size_t i = 0; i < 10; ++i)
        vec.push_back(i);
    double* pElems = &vec[0];
    vec.clear();
    for (size_t i = 0; i < 10; ++i)
        vec.push_back(i);
    double* pElems2 = &vec[0];
    assert(pElems == pElems2);
    std::vector<double> vec2(10); //cause memory allocation
    for (size_t i = 0; i < 1000; ++i)
        vec.push_back(i);
    double* pElems3 = &vec[0];
    assert(pElems != pElems3);
}

int main(int argc, char* argv[]) {
    using namespace std::chrono;

#ifdef ZHAD_DEBUG
    //std::cerr << "sizeof(VertexId) = " << sizeof(VertexId) << std::endl;
    //std::cerr << "sizeof(size_t) = " << sizeof(size_t) << std::endl;
    //std::cerr << "sizeof(double) = " << sizeof(double) << std::endl;
    //std::cerr << "sizeof(ADGraph::Vertex) = " << sizeof(ADGraph::Vertex) << std::endl;
    //std::cerr << "sizeof(ADGraph::Edge) = " << sizeof(ADGraph::Edge) << std::endl;
    //std::cerr << "sizeof(ADGraph::Ord2Edge) = " << sizeof(ADGraph::Ord2Edge) << std::endl;
#endif

    size_t count = 1;
    if (argc > 1)
        count = static_cast<size_t>(std::strtoul(argv[1], nullptr, 0));
    std::cout << "running " << count << " times" << std::endl << std::flush;
    ADVar x0, x1, x2;
    size_t i;
    high_resolution_clock::time_point begin(high_resolution_clock::now());
#ifdef ZHAD_DEBUG
    std::cerr << "testAdd ..." << std::endl;
#endif
    ADDepVar yAdd;
    yAdd.dependOn(x0, x1, x2);
    for (i = 0; i != count; ++i)
        testAdd(yAdd, x0, x1, x2);
#ifdef ZHAD_DEBUG
    std::cerr << "testMinus ..." << std::endl;
#endif
    ADDepVar yMinus;
    yMinus.dependOn(x0, x1, x2);
    for (i = 0; i != count; ++i)
        testMinus(yMinus, x0, x1, x2);
#ifdef ZHAD_DEBUG
    std::cerr << "testMultiply ..." << std::endl;
#endif
    ADDepVar yMultiply;
    yMultiply.dependOn(x0, x1, x2);
    for (i = 0; i != count; ++i)
        testMultiply(yMultiply, x0, x1, x2);
#ifdef ZHAD_DEBUG
    std::cerr << "testDivide ..." << std::endl;
#endif
    ADDepVar yDivide;
    yDivide.dependOn(x0, x1, x2);
    for (i = 0; i != count; ++i)
        testDivide(yDivide, x0, x1, x2);
#ifdef ZHAD_DEBUG
    std::cerr << "testSqrt ..." << std::endl;
#endif
    ADDepVar ySqrt;
    ySqrt.dependOn(x0, x1, x2);
    for (i = 0; i != count; ++i)
        testSqrt(ySqrt, x0, x1, x2);
#ifdef ZHAD_DEBUG
    std::cerr << "testPow ..." << std::endl;
#endif
    ADDepVar yPow;
    yPow.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testPow(yPow, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testExp ..." << std::endl;
#endif
    ADDepVar yExp;
    yExp.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testExp(yExp, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testLog ..." << std::endl;
#endif
    ADDepVar yLog;
    yLog.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testLog(yLog, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testSin ..." << std::endl;
#endif
    ADDepVar ySin;
    ySin.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testSin(ySin, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testCos ..." << std::endl;
#endif
    ADDepVar yCos;
    yCos.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testCos(yCos, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testTan ..." << std::endl;
#endif
    ADDepVar yTan;
    yTan.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testTan(yTan, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testASin ..." << std::endl;
#endif
    ADDepVar yASin;
    yASin.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testASin(yASin, x0, x1);
#ifdef ZHAD_DEBUG
    std::cerr << "testACos ..." << std::endl;
#endif
    ADDepVar yACos;
    yACos.dependOn(x0, x1);
    for (i = 0; i != count; ++i)
        testACos(yACos, x0, x1);
    high_resolution_clock::time_point end(high_resolution_clock::now());
    auto runtime = duration_cast<duration<double>>(end - begin);
    std::cout << "running time " << runtime.count() << " seconds" << std::endl << std::flush;

#ifdef ZHAD_DEBUG
    std::cerr << "testClone ..." << std::endl;
#endif
    ADDepVar yClone;
    yClone.dependOn(x0);
    testClone(yClone, x0);
    testVectorReserveAndClear();

    begin = high_resolution_clock::now();
    ADVar xr0, xr1, xr2, xr3, xr4;
    ADDepVar yr;
    yr.dependOn(xr0, xr1, xr2, xr3, xr4);
    double inc = 1.0 / count;
    double randVal = -(count * inc / 2.0);
    for (i = 0; i != count; ++i) {
        xr0 = 1.0 + randVal;
        xr1 = 2.0 + randVal;
        xr2 = 3.0 + randVal;
        xr3 = 4.0 + randVal;
        xr4 = 5.0 + randVal;
        yr = sqrt(xr0 + exp(xr1) * xr2 - xr3) / xr4;

        double dydx0 = yr.derivative1(xr0);
        double dydx1 = yr.derivative1(xr1);
        double dydx2 = yr.derivative1(xr2);
        double dydx3 = yr.derivative1(xr3);
        double dydx4 = yr.derivative1(xr4);

        double dydx0x0 = yr.derivative2(xr0, xr0);
        double dydx0x1 = yr.derivative2(xr0, xr1);
        double dydx0x2 = yr.derivative2(xr0, xr2);
        double dydx0x3 = yr.derivative2(xr0, xr3);
        double dydx0x4 = yr.derivative2(xr0, xr4);
        double dydx1x1 = yr.derivative2(xr1, xr1);
        double dydx1x2 = yr.derivative2(xr1, xr2);
        double dydx1x3 = yr.derivative2(xr1, xr3);
        double dydx1x4 = yr.derivative2(xr1, xr4);
        double dydx2x2 = yr.derivative2(xr2, xr2);
        double dydx2x3 = yr.derivative2(xr2, xr3);
        double dydx2x4 = yr.derivative2(xr2, xr4);
        double dydx3x3 = yr.derivative2(xr3, xr3);
        double dydx3x4 = yr.derivative2(xr3, xr4);
        double dydx4x4 = yr.derivative2(xr4, xr4);

        if (i <= 3) {
            std::cout << "sqrt(" << xr0.value() << " + exp(" << xr1.value() << ") * " << xr2.value() << " - " << xr3.value()
                << ") / " << xr4.value() << " --> " << yr.value() << std::endl;
            std::cout << "dydx0 = " << dydx0 << ", dydx1 = " << dydx1 << ", dydx2 = " << dydx2 << ", dydx3 = " << dydx3
                << ", dydx4 = " << dydx4 << std::endl;
            std::cout << "dydx0x0 = " << dydx0x0 << ", dydx0x1 = " << dydx0x1 << ", dydx0x2 = " << dydx0x2 << ", dydx0x3 = " << dydx0x3
                << ", dydx0x4 = " << dydx0x4 << std::endl;
            std::cout << "dydx1x1 = " << dydx1x1 << ", dydx1x2 = " << dydx1x2 << ", dydx1x3 = " << dydx1x3
                << ", dydx1x4 = " << dydx1x4 << std::endl;
            std::cout << "dydx2x2 = " << dydx2x2 << ", dydx2x3 = " << dydx2x3 << ", dydx2x4 = " << dydx2x4 << std::endl;
            std::cout << "dydx3x3 = " << dydx3x3 << "dydx3x4 = " << dydx3x4 << std::endl;
            std::cout << "dydx4x4 = " << dydx4x4 << std::endl;
#ifdef ZHAD_DEBUG
            std::cerr << yr << std::endl;
#endif
        }

        randVal += inc;
    }
    std::cout << "final sqrt(" << xr0.value() << " + exp(" << xr1.value() << ") * " << xr2.value() << " - " << xr3.value()
        << ") / " << xr4.value() << " --> " << yr.value() << std::endl;
    end = high_resolution_clock::now();
    runtime = duration_cast<duration<double>>(end - begin);
    std::cout << "sqrt running time " << runtime.count() << " seconds" << std::endl << std::flush;

    std::cout << "simple test ..." << std::endl << std::flush;
    i = count;
    begin = high_resolution_clock::now();
    double sum;
    for (i = 0; i != count; ++i)
        sum = testAdd() + testMinus() + testMultiply() + testDivide()
            + testSqrt() + testPow() + testExp() + testLog()
            + testSin() + testCos() + testTan()
            + testASin() + testACos();
    end = high_resolution_clock::now();
    runtime = duration_cast<duration<double>>(end - begin);
    std::cout << "simple running time " << runtime.count() << " seconds" << "[sum = " << sum << "]" << std::endl << std::flush;

    ADVar xv;
    ADDepVar yv;
    yv.dependOn(xv);
    xv = 4.0;
    yv = 2 * sqrt(xv);
    double dydx = yv.derivative1(xv);
    double dydxx = yv.derivative2(xv, xv);
    std::cout << "yv = 2 * sqrt(xv) where xv = " << xv.value() << " --> yv = " << yv.value() << std::endl;
    std::cout << "dydx = " << dydx << ", dydxx = " << dydxx << std::endl;

    ADVar xw;
    ADDepVar yw;
    yw.dependOn(xw);
    xw = 4.0;
    yw = 2 * xw;
    dydx = yw.derivative1(xw);
    dydxx = yw.derivative2(xw, xw);
    std::cout << "yw = 2 * xw where xw = " << xw.value() << " --> yw = " << yw.value() << std::endl;
    std::cout << "dydx = " << dydx << ", dydxx = " << dydxx << std::endl;

    std::exit(0);
    return 0;
}
