
#include "zhad.h"

#include <algorithm>
#include <cassert>
#include <iostream>

//#define ZHAD_DEBUG

namespace zhad {

VertexId ADGraph::createVertex() {
    VertexId id = m_vertices.m_subSize++;
    if (!m_once)
        m_vertices.resize(m_vertices.m_subSize);
    Vertex& vertex = m_vertices[id];
    vertex.m_weight = vertex.m_ord2Weight = 0.0;
#ifdef ZHAD_DEBUG
    std::cerr << "createVertex() -> id=" << id << std::endl;
#endif
    return id;
}

VertexId ADGraph::createVertex(const ADValue& adVal, double weight, double ord2Weight) {
    VertexId id = m_vertices.m_subSize++;
    if (!m_once)
        m_vertices.resize(m_vertices.m_subSize);
    Vertex& vertex = m_vertices[id];
    Edge& edge = vertex.m_edges[0];
    edge.m_id = adVal.id();
    edge.m_weight = weight;
    vertex.m_weight = 0.0;
    vertex.m_ord2Weight = ord2Weight;
#ifdef ZHAD_DEBUG
    std::cerr << "createVertex(adVal=" << adVal.id() << ", weight=" << weight
        << ", ord2Weight=" << ord2Weight << ") -> id=" << id << std::endl;
#endif
    return id;
}

VertexId ADGraph::createVertex(const ADValue& adValL, double weightL, const ADValue& adValR, double weightR, double ord2Weight) {
    VertexId id = m_vertices.m_subSize++;
    if (!m_once)
        m_vertices.resize(m_vertices.m_subSize);
    Vertex& vertex = m_vertices[id];
    Edge* edges = &(vertex.m_edges[0]);
    edges[0].m_id = adValL.id();
    edges[0].m_weight = weightL;
    edges[1].m_id = adValR.id();
    edges[1].m_weight = weightR;
    vertex.m_weight = 0.0;
    vertex.m_ord2Weight = ord2Weight;
#ifdef ZHAD_DEBUG
    std::cerr << "createVertex(adValL=" << adValL.id() << ", weightL=" << weightL << ", adValR=" << adValR.id()
        << ", weightR=" << weightR << ", ord2Weight=" << ord2Weight << ") -> id=" << id << std::endl;
#endif
    return id;
}

void ADGraph::insertEdgeWeight(VertexId idFrom, VertexId idTo, double weight) {
    if (idFrom == idTo)
        m_vertices[idFrom].m_selfOrd2Weight += 2.0 * weight;
    else {
        if (idFrom < idTo) {
            VertexId idTemp = idTo;
            idTo = idFrom;
            idFrom = idTemp;
        }
        //assert(m_vertices.size() > idFrom);
        auto& edges = m_vertices[idFrom].m_ord2Edges;
        auto iEnd = edges.m_subSize;
        auto pEdge = edges.begin();
        auto pEnd = pEdge + iEnd;
        for (; pEdge != pEnd; ++pEdge)
            if (idTo == pEdge->id()) {
                pEdge->m_weight += weight;
                return;
            }
        ++(edges.m_subSize);
        if (iEnd == edges.size()) {
            edges.resize(edges.m_subSize);
            edges[iEnd].m_id = idTo;
        }
        edges[iEnd].m_weight = weight;
    }
}

double ADGraph::getEdgeWeight(VertexId idFrom, VertexId idTo) const {
    if (idFrom < idTo) {
        VertexId idTemp = idTo;
        idTo = idFrom;
        idFrom = idTemp;
    }
    //assert(m_vertices.size() > idFrom);
    auto& edges = m_vertices[idFrom].m_ord2Edges;
    auto iEnd = edges.m_subSize;
    auto pEdge = edges.cbegin();
    auto pEnd = pEdge + iEnd;
    for (; pEdge != pEnd; ++pEdge) {
        if (idTo == pEdge->id())
            return pEdge->weight();
    }
    return 0.0;
}

void ADGraph::propagateAdjoint(VertexId depVarId, double depVarWeight) {
    //assert((m_numIndepVars != 0) && (m_vertices.m_subSize != 0) && (m_vertices.m_subSize == m_vertices.size()));
#ifdef ZHAD_DEBUG
    std::cerr << "propagateAdjoint(depVarId=" << depVarId << ", depVarWeight=" << depVarWeight
        << "): #independent vars: " << m_numIndepVars << ", #vertices: " << m_vertices.m_subSize << std::endl;
#endif
    Vertex* vertices = &(m_vertices[0]);
    Vertex* lastVertex = vertices + m_vertices.m_subSize - 1;
    Vertex* endIndepVertices = vertices + m_numIndepVars;
    for (Vertex* vertex = lastVertex;; --vertex) {
        vertex->m_ord2Edges.m_subSize = 0;
        vertex->m_selfOrd2Weight = 0.0;
        if (vertex < endIndepVertices) {
            vertex->m_weight = vertex->m_ord2Weight = 0.0;
            if (vertex == vertices)
                break;
        }
    }
    setAdjointWeight(depVarId, depVarWeight);
    m_vertices.m_subSize = m_numIndepVars;
    m_once = true;

    // Any chance for SSE/AVX parallism?
    for (Vertex* vertex = lastVertex;; --vertex) {
        const Edge* edgeL = &(vertex->edges()[0]);
        VertexId idL = edgeL->id();
        double weightL = edgeL->weight();
        //assert((idL == NOID) == (id < m_numIndepVars));
        Vertex& vertexL = vertices[idL];
        const Edge* edgeR = edgeL + 1;
        VertexId idR = edgeR->id();
        double weightR = edgeR->weight();
        bool has2Edges = (idR != NOID);

        // Pushing
        for (const auto& ord2Edge : vertex->m_ord2Edges) {
            insertEdgeWeight(idL, ord2Edge.id(), weightL * ord2Edge.weight());
            if (has2Edges)
                insertEdgeWeight(idR, ord2Edge.id(), weightR * ord2Edge.weight());
        }

        double selfOrd2Weight = vertex->m_selfOrd2Weight;
        if (selfOrd2Weight != 0.0) {
            vertexL.m_selfOrd2Weight += weightL * weightL * selfOrd2Weight;
            if (has2Edges) {
                double multWeightR = weightR * selfOrd2Weight;
                vertices[idR].m_selfOrd2Weight += weightR * multWeightR;
                insertEdgeWeight(idL, idR, weightL * multWeightR);
            }
        }

        double weight = vertex->weight();
        if (weight != 0.0) {
            // Creating
            double ord2Weight = vertex->ord2Weight();
            if (ord2Weight != 0.0) {
                if (has2Edges)
                    insertEdgeWeight(idL, idR, weight * ord2Weight);
                else
                    vertexL.m_selfOrd2Weight += weight * ord2Weight;
            }

            // Adjoint
            vertex->m_weight = 0.0;
            vertexL.m_weight += weight * weightL;
            if (has2Edges)
                vertices[idR].m_weight += weight * weightR;
        }
        if (vertex == endIndepVertices)
            break;
    }
}

#ifdef ZHAD_SINGLE_EDGE_PROPAGATE
VertexId ADGraph::singleEdgePropagate(VertexId id, double& result) {
    VertexId idL;
    while (((idL = m_vertices[id].edges()[0].id()) != NOID) &&
           (m_vertices[id].edges()[1].id() == NOID)) {
        result *= m_vertices[id].edges()[0].weight();
        id = idL;
    }
    return id;
}
#endif

std::ostream& operator<<(std::ostream& os, const ADBaseValue& adVal) {
    os << "id: " << adVal.id() << "; value: " << adVal.value();
    return os;
}

std::ostream& operator<<(std::ostream& os, const ADValue& adVal) {
    os << "ADValue " << static_cast<const ADBaseValue&>(adVal);
    return os;
}

std::ostream& operator<<(std::ostream& os, const ADVar& var) {
    os << "ADVar " << static_cast<const ADBaseValue&>(var);
    return os;
}

std::ostream& operator<<(std::ostream& os, const ADGraph& graph) {
    os << "ADGraph independent vars: " << graph.m_numIndepVars << "; vertices: " << graph.m_vertices.size();
    for (const auto& vertex : graph.m_vertices) {
        os << std::endl
           << "  Vertex edge L id=" << vertex.edges()[0].id() << ", weight=" << vertex.edges()[0].weight()
           << "; R id=" << vertex.edges()[1].id() << ", weight=" << vertex.edges()[1].weight()
           << "; weight=" << vertex.weight() << "; ord2Weight=" << vertex.ord2Weight();
        VertexId minId = std::numeric_limits<VertexId>::max();
        VertexId maxId = 0;
        os << std::endl << "    ord2Edges: ";
        const char* sep = "";
        for (const auto& edge : vertex.ord2Edges()) {
            os << sep << edge.id() << " wt=" << edge.weight();
            sep = ", ";
            if (edge.id() < minId) minId = edge.id();
            if (edge.id() > maxId) maxId = edge.id();
        }
        if (!vertex.ord2Edges().empty())
            os << " [" << minId << "," << maxId << "]";
        os << std::endl << "    selfOrd2Weight: " << vertex.selfOrd2Weight();
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ADDepVar& var) {
    os << "ADDepVar " << static_cast<const ADBaseValue&>(var)
       << std::endl
       << static_cast<const ADGraph&>(var);
    return os;
}

} //namespace zhad
