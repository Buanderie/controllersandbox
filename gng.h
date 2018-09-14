#pragma once

#include <limits>
#include <iostream>
using namespace std;

#include "gngnode.h"
#include "gngedge.h"

#include "utils.h"

template< typename ScalarType >
class GNG
{
public:
    GNG( size_t inputSize = 2 )
        :_inputSize(inputSize)
    {
        init();
    }

    virtual ~GNG()
    {

    }


    void init()
    {
        // (0) Start with two units a and b at random positions wa and wb
        GNGNode<ScalarType> * a = new GNGNode<ScalarType>( _inputSize, gngrand_position<ScalarType>( _inputSize ) );
        GNGNode<ScalarType> * b = new GNGNode<ScalarType>( _inputSize, gngrand_position<ScalarType>( _inputSize ) );
        _nodes.insert( a );
        _nodes.insert( b );
    }

    void in( const std::vector<ScalarType> epsilon )
    {
        // (1) Generate an input signal epsilon according to P(epsilon).

        // (2) Find the nearest unit s1 and the second nearest unit s2
        GNGNode<ScalarType> * s1 = nullptr;
        GNGNode<ScalarType> * s2 = nullptr;
        ScalarType d1 = std::numeric_limits<ScalarType>::max();
        ScalarType d2 = std::numeric_limits<ScalarType>::max();
        for( auto s : _nodes )
        {
            ScalarType d = vector_distance( s->position(), epsilon );
            if( d < d1 )
            {
                d1 = d;
                s1 = s;
            }
        }
        for( auto s : _nodes )
        {
            ScalarType d = vector_distance( s->position(), epsilon );
            if( d < d2 && s != s1 )
            {
                d2 = d;
                s2 = s;
            }
        }
        cerr << "d1=" << d1 << " d2=" << d2 << endl;

        // (3) Increment the age of all the edges emanating from s1
        for( GNGEdge<ScalarType>* e : _edges )
        {
            if( e->a() == s1 || e->b() == s1 )
            {
                e->age() += 1.0;
            }
        }

        // (4) Add the squared distance between the input signal and the nearest unit in
        // input space to a local counter variable:
        // delta_error(s1) = || w_s1 - epsilon ||^2
        ScalarType delta_error_s1 = vector_distance( s1->position(), epsilon, true );

        // (5) Move s1 and its direct topological neighbors towards epsilon by fractions
        // Eb and En, respectively, of the total distance:
        // delta_w_s1 = Eb( epsilon - w_s1 )
        // delta_w_n = En(epsilon - w_n ) for all direct neighbors n of s1
        std::vector<ScalarType> delta_w_s1 = vector_scale( vector_sub( epsilon, s1->position() ), _Eb );
        // WHAT ?
        for( GNGNode<ScalarType> * n : s1->neighbors() )
        {
            std::vector<ScalarType> delta_w_n = vector_scale( vector_sub( epsilon, n->position() ), _Eb );
        }

        // (6) If s1 and s2 are connected by an edge, set the age of this edge to zero. If
        // such an edge does not exist, create it.

        // (7) Remove edges with an age larger than a_max • If this results in points having
        // no emanating edges, remove them as well.

        // (8) If the number of input signals generated so far is an integer multiple of a
        // parameter A, insert a new unit as follows:
        // • Determine the unit q with the maximum accumulated error.
        // • Insert a new unit r halfway between q and its neighbor f with the
        // largest error variable:
        // w_r = 0.5 (w_q + w_f)
        // • Insert edges connecting the new unit r with units q and f, and remove
        // the original edge between q and f.
        // • Decrease the error variables of q and f by multiplying them with a
        // constant d. Initialize the error variable of r with the new value of the
        // error variable of q.

        // (9) Decrease all error variables by multiplying them with a constant d.

        // (10) If a stopping criterion (e.g., net size or some performance measure) is not
        // yet fulfilled go to step 1.
    }

private:
    size_t _inputSize;
    std::set< GNGNode<ScalarType>* > _nodes;
    std::set< GNGEdge<ScalarType>* > _edges;

    ScalarType _Eb;
    ScalarType _En;

protected:

};
