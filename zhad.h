/**
    ZHAD is a C++ reverse-mode automatic differentiation library using operator overloading,
    producing first- and second-order derivatives (Hessian).
    It implements the edge-pushing algorithm presented in "Hessian Matrices via Automatic Differentiation"
    (Gower and Mello, 2010) to efficiently compute the first and second derivatives.

    It is based on HAD, an earlier implementation by Tzu-Mao Li. Its logic is unchanged from the original,
    but it has been completely redesigned, with the goal of improving its usability.

    Author: Zoltan N. Leskowsky

    The MIT License (MIT)

    Copyright (c) 2019 Zoltan N. Leskowsky
**/

/**
    HAD is a single header C++ reverse-mode automatic differentiation library using operator overloading, with focus on 
    second-order derivatives (Hessian).  
    It implements the edge_pushing algorithm (see "Hessian Matrices via Automatic Differentiation", 
    Gower and Mello 2010) to efficiently compute the second derivatives.

    See https://github.com/BachiLi/had for more details.

    Author: Tzu-Mao Li

    The MIT License (MIT)

    Copyright (c) 2015 Tzu-Mao Li

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
**/

#ifndef ZHAD_H__
#define ZHAD_H__

#include <cmath>
#include <functional>
#include <iosfwd>
#include <limits>
#include <unordered_map>
#include <vector>

//#define ZHAD_SINGLE_EDGE_PROPAGATE //TODO?

namespace zhad {

template <typename T>
constexpr T Nil() { return std::numeric_limits<T>::max(); }

typedef std::vector<std::size_t>::size_type VertexId;
static constexpr VertexId NOID = Nil<VertexId>();

class ADGraph;

class ADBaseValue {
public:
    double value() const { return m_value; }
    VertexId id() const { return m_id; }

    friend std::ostream& operator<<(std::ostream& os, const ADBaseValue& adVal);

protected:
    friend class ADGraph;
    ADBaseValue() : m_value(0.0), m_id(NOID) {}
    ADBaseValue(double val, VertexId id = NOID) : m_value(val), m_id(id) {}
    ADBaseValue& operator=(const ADBaseValue& val) = default;
    ADBaseValue& operator=(double val) { m_value = val; return *this; }
    void setValueue(double val) { m_value = val; }
    void setId(VertexId id) { m_id = id; }

private:
    double m_value;
    mutable VertexId m_id;
};

class ADValue;
class ADDepVar;

class ADGraph {
public:
    double getAdjointWeight(VertexId id) const {
        return m_vertices[id].m_weight;
    }

    double getAdjointWeight(VertexId id1, VertexId id2) const {
        return (id1 == id2) ? m_vertices[id1].m_selfOrd2Weight : getEdgeWeight(id1, id2);
    }

protected:
    ADGraph() : m_numIndepVars(0), m_once(false) {}

    void setAdjointWeight(VertexId id, double weight) {
        m_vertices[id].m_weight = weight;
    }

    void propagateAdjoint(VertexId depVarId, double depVarWeight = 1.0);

    friend struct ADGraphFriend;

    void clear() {
        m_vertices.clear();
        m_numIndepVars = 0;
        m_once = false;
    }

private:
    friend class ADValue;
    friend class ADDepVar;

    VertexId createVertex();
    VertexId createVertex(const ADValue& adVal, double weight, double ord2Weight);
    VertexId createVertex(const ADValue& adValL, double weightL, const ADValue& adValR, double weightR, double ord2Weight);

#ifdef ZHAD_SINGLE_EDGE_PROPAGATE
    VertexId singleEdgePropagate(VertexId id, double& result);
#endif

    struct Edge {
        Edge() : m_id(NOID), m_weight(0.0) {}
        Edge(VertexId id, double weight) : m_id(id), m_weight(weight) {}
        VertexId id() const { return m_id; }
        double weight() const { return m_weight; }
        VertexId m_id;
        double m_weight;
    };

    void insertEdgeWeight(VertexId idFrom, VertexId idTo, double weight);
    double getEdgeWeight(VertexId idFrom, VertexId idTo) const;

    friend std::ostream& operator<<(std::ostream& os, const ADGraph& graph);

private:
    template <typename T>
    struct SubVector : public std::vector<T> {
        SubVector() : m_subSize(0) {}
        void clear() { m_subSize = 0; std::vector<T>::clear(); }
        typedef typename std::vector<T>::size_type size_type;
        size_type m_subSize;
    };

    struct Vertex {
        Vertex() : m_weight(0.0), m_ord2Weight(0.0) {}
        double weight() const { return m_weight; }
        double ord2Weight() const { return m_ord2Weight; }
        const Edge* edges() const { return m_edges; }
        const SubVector<Edge>& ord2Edges() const { return m_ord2Edges; }
        double selfOrd2Weight() const { return m_selfOrd2Weight; }

    private:
        friend class ADGraph;
        // We assume there are at most 2 outgoing edges from any vertex
        double m_weight; // first-order weight
        double m_ord2Weight; // second-order weight
        Edge m_edges[2]; // If an edge id is NOID, then the edge does not exist
        SubVector<Edge> m_ord2Edges;
        double m_selfOrd2Weight;
        // For a vertex with a single outgoing edge, ord2Weight represents
        // the second-order weight of the connecting vertex (d^2f/dx^2).
        // For a vertex with two outgoing edges, ord2Weight represents
        // the second-order weight between the conntecting vertices (d^2f/dxdy).
        // In the case of two outgoing edges, d^2f/dx^2 & d^2f/dy^2 are both assumed to be zero to save memory.
    };

    SubVector<Vertex> m_vertices;
    SubVector<Vertex>::size_type m_numIndepVars;
    bool m_once;
};

namespace detail {

template <int Ord>
double getAdjointWeight(const ADGraph& graph, VertexId id);
template <>
inline double getAdjointWeight<1>(const ADGraph& graph, VertexId id) {
    return graph.getAdjointWeight(id);
}
template <>
inline double getAdjointWeight<2>(const ADGraph& graph, VertexId id) {
    return graph.getAdjointWeight(id, id);
}

template <int Ord>
double getAdjointWeight(const ADGraph& graph, VertexId id1, VertexId id2);
template <>
inline double getAdjointWeight<2>(const ADGraph& graph, VertexId id1, VertexId id2) {
    return graph.getAdjointWeight(id1, id2);
}

}; //namespace detail

class ADValue : public ADBaseValue {
public:
    ADValue(const ADValue& adVal) = default;
    ADValue(double val, const ADValue& adVal, double weight, double ord2Weight)
        : ADBaseValue(val, adVal.m_graph->createVertex(adVal, weight, ord2Weight)),
          m_graph(adVal.m_graph) {}
    ADValue(double val, const ADValue& adValL, double weightL,
          const ADValue& adValR, double weightR, double ord2Weight)
        : ADBaseValue(val, adValL.m_graph->createVertex(adValL, weightL, adValR, weightR, ord2Weight)),
          m_graph(adValL.m_graph) {}

    friend std::ostream& operator<<(std::ostream& os, const ADValue& adVal);

protected:
    ADValue() : m_graph(nullptr) {}
    ADValue& operator=(ADValue&& adVal) = default;
    ADValue& operator=(double val) { return static_cast<ADValue&>(ADBaseValue::operator=(val)); }

private:
    friend class ADGraph;
    friend class ADDepVar;
    ADGraph& graph() { return *m_graph; }
    const ADGraph& graph() const { return *m_graph; }
    void setGraph(ADGraph& graph) { m_graph = &graph; }

private:
    ADGraph* m_graph;
};

class ADVar : public ADValue {
public:
    ADVar() = default;
    ADVar(const ADVar& adVar) = default;
    ADVar(const ADValue& adVal) : ADValue(adVal) {}
    ADVar& operator=(ADVar&& adVar) = default;
    ADVar& operator=(double val) { return static_cast<ADVar&>(ADValue::operator=(val)); }

    friend std::ostream& operator<<(std::ostream& os, const ADVar& var);
};

class ADDepVar : public ADBaseValue, public ADGraph {
public:
    ADDepVar() = default;
    ADDepVar(const ADDepVar& var) = delete;

    ADDepVar& operator=(const ADDepVar& var) = delete;
    ADDepVar& operator=(double val) { return static_cast<ADDepVar&>(ADBaseValue::operator=(val)); }

    inline ADDepVar& operator=(const ADValue& adVal) {
        ADBaseValue::operator=(adVal);
        propagateAdjoint(id(), 1.0);
        return *this;
    }

    inline ADDepVar& operator=(ADValue&& adVal) {
        ADBaseValue::operator=(adVal);
        propagateAdjoint(id(), 1.0);
        return *this;
    }

    template <typename... Args>
    void dependOn(ADVar& var, Args&&... args) {
        dependOn(std::forward<Args>(args)...);
        var.setGraph(*this);
        var.setId(createVertex());
        ++m_numIndepVars;
    }

    double derivative1(const ADValue& adVal) const { return ADGraph::getAdjointWeight(adVal.id()); }
    double derivative2(const ADValue& adVal1, const ADValue& adVal2) const { return ADGraph::getAdjointWeight(adVal1.id(), adVal2.id()); }
    double derivative2(const ADValue& adVal) const { return ADGraph::getAdjointWeight(adVal.id(), adVal.id()); }

    template <int Ord> double derivative(const ADValue& adVal) const {
        return detail::getAdjointWeight<Ord>(*this, adVal.id());
    }
    template <int Ord> double derivative(const ADValue& adVal1, const ADValue& adVal2) const {
        return detail::getAdjointWeight<Ord>(*this, adVal1.id(), adVal2.id());
    }

    friend std::ostream& operator<<(std::ostream& os, const ADDepVar& var);

private:
    void dependOn() { ADGraph::clear(); }
};

} //namespace zhad

#endif //ZHAD_H__
