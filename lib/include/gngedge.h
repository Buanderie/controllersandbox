#pragma once

#include "gngnode.h"

template< typename ScalarType >
class GNGEdge
{
public:
    GNGEdge( GNGNode<ScalarType> * a, GNGNode<ScalarType> * b, ScalarType age=((ScalarType)0) )
        :_a(a), _b(b)
    {
        _a->addNeighbor( b );
        _b->addNeighbor( a );
    }

    virtual ~GNGEdge()
    {
        _a->removeNeighbor( _b );
        _b->removeNeighbor( _a );
    }

    ScalarType& age()
    {
        return _age;
    }

    GNGNode<ScalarType>* a()
    {
        return _a;
    }

    GNGNode<ScalarType>* b()
    {
        return _b;
    }

private:
    GNGNode<ScalarType> * _a;
    GNGNode<ScalarType> * _b;

    ScalarType _age;

protected:

};
