#pragma once

#include <limits>
#include <iostream>
#include <algorithm>
#include <map>

using namespace std;

#include "gngnode.h"
#include "gngedge.h"

#include "utils.h"

//template< typename ScalarType >
//struct NodeComparator_
//{
//    NodeComparator_( std::vector<ScalarType> in )
//        :_input(in)
//    {

//    }
//    inline bool operator() (const GNGNode<ScalarType>*& lhs, const GNGNode<ScalarType>*& rhs)
//    {
//        // return (struct1.key < struct2.key);
//        return vector_distance(_input, lhs->position()) < vector_distance(_input, rhs->position());
//    }
//    std::vector<ScalarType> _input;
//};

template< typename ScalarType >
class GNG
{

public:
    GNG( size_t inputSize = 2 )
        :_inputSize(inputSize)
    {
        _inCount = 0;
        _Eb = 0.2;
        _En = 0.06;

        _A = 300;
        _ageMax = 100;
        _alpha = 0.5;
        _beta = 0.995;

        init();
    }

    virtual ~GNG()
    {

    }

    std::vector< GNGNode<ScalarType>* > findNearest( vector<ScalarType> in, size_t n_min )
    {
        // Copy from set
        std::map< ScalarType, GNGNode<ScalarType>* > _distances;
        std::vector< GNGNode<ScalarType>* > v;

        for( GNGNode<ScalarType>* n : _nodes )
        {
            // v.push_back( n );
            _distances.insert( std::make_pair(vector_distance(in, n->position()), n ) );
        }

        /*
        std::sort(begin(v),
                  end(v),
                  [in]( GNGNode<ScalarType>*& lhs,  GNGNode<ScalarType>*& rhs)
        {
            return vector_distance(in, lhs->position()) < vector_distance(in, rhs->position());
        });
        */
        // std::sort(v.begin(), v.end(), NodeComparator_<ScalarType>(in) );

        std::vector< GNGNode<ScalarType>* > ret;

        /*
        for( size_t i = 0; i < min( v.size(), n_min ); ++i )
        {
            ret.push_back( v[i] );
        }
        */

        int k = 0;
        for( auto& kv : _distances )
        {
            if( k >= n_min )
                break;
            ret.push_back( kv.second );
            k++;
        }

        return ret;
    }

    ScalarType find( std::vector<ScalarType> input, GNGNode<ScalarType>*& ret )
    {
        GNGNode<ScalarType> * s1 = nullptr;
        ScalarType d1 = std::numeric_limits<ScalarType>::max();
        for( auto s : _nodes )
        {
            ScalarType d = vector_distance( s->position(), input );
            if( d < d1 )
            {
                d1 = d;
                s1 = s;
            }
        }
        ret = s1;
        return d1;
    }

    std::set< GNGNode< ScalarType >* >& nodes()
    {
        return _nodes;
    }

    std::set< GNGEdge<ScalarType>* >& edges()
    {
        return _edges;
    }

    void init()
    {
        // (0) Start with two units a and b at random positions wa and wb
        GNGNode<ScalarType> * a = new GNGNode<ScalarType>( _inputSize, gngrand_position<ScalarType>( _inputSize ) );
        GNGNode<ScalarType> * b = new GNGNode<ScalarType>( _inputSize, gngrand_position<ScalarType>( _inputSize ) );
        _nodes.insert( a );
        _nodes.insert( b );
        // addEdge( a, b );
    }

    double in( const std::vector<ScalarType> epsilon )
    {
        //        cerr << "cur_nodes=" << _nodes.size() << endl;

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
        //        cerr << "d1=" << d1 << " d2=" << d2 << endl;

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
        s1->position() = vector_add( s1->position(), delta_w_s1 );
        for( GNGNode<ScalarType> * n : s1->neighbors() )
        {
            std::vector<ScalarType> delta_w_n = vector_scale( vector_sub( epsilon, n->position() ), _Eb );
            n->position() = vector_add( n->position(), delta_w_n );
        }

        // (6) If s1 and s2 are connected by an edge, set the age of this edge to zero. If
        // such an edge does not exist, create it.
        bool updatedEdge = false;
        for( GNGEdge<ScalarType>* e : _edges )
        {
            if( e->a() == s1 && e->b() == s2 || e->a() == s2 && e->b() == s1 )
            {
                e->age() = 0;
                updatedEdge = true;
                break;
            }
        }
        if( !updatedEdge )
        {
            _edges.insert( new GNGEdge<ScalarType>( s1, s2, 0 ) );
        }

        // (7) Remove edges with an age larger than a_max • If this results in points having
        // no emanating edges, remove them as well.
        std::vector< GNGEdge<ScalarType>* > toRemove;
        for( GNGEdge<ScalarType>* e : _edges )
        {
            if( e->age() >= _ageMax )
                toRemove.push_back(e);
        }
        for( GNGEdge<ScalarType>* e : toRemove )
        {
            _edges.erase( e );
            delete e;
        }
        removeNonConnectedNodes();

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
        if( _inCount % _A == 0 )
        {
            cerr << "_inCount=" << _inCount << " - INSERT !" << endl;
            insertNewNode();
            //            sleep(1);
        }

        // (9) Decrease all error variables by multiplying them with a constant d.
        for( GNGNode<ScalarType>* n : _nodes )
        {
            n->error() *= _beta;
        }

        // (10) If a stopping criterion (e.g., net size or some performance measure) is not
        // yet fulfilled go to step 1.

        _inCount++;

        return d1;

    }

private:
    size_t _inputSize;
    std::set< GNGNode<ScalarType>* > _nodes;
    std::set< GNGEdge<ScalarType>* > _edges;

    ScalarType _Eb;
    ScalarType _En;
    ScalarType _ageMax;
    int _A;
    ScalarType _d;

    ScalarType _beta;
    ScalarType _alpha;

    int _inCount;

    void removeNonConnectedNodes()
    {
        std::vector< GNGNode<ScalarType>* > toRemove;
        for( GNGNode<ScalarType>* n : _nodes )
        {
            if( n->neighbors().size() <= 0 )
                toRemove.push_back(n);
        }
        for( GNGNode<ScalarType>* n : toRemove )
        {
            _nodes.erase( n );
            delete n;
        }
    }

    void insertNewNode()
    {
        // • Determine the unit q with the maximum accumulated error.
        // • Insert a new unit r halfway between q and its neighbor f with the
        // largest error variable:
        // w_r = 0.5 (w_q + w_f)
        // • Insert edges connecting the new unit r with units q and f, and remove
        // the original edge between q and f.
        // • Decrease the error variables of q and f by multiplying them with a
        // constant d. Initialize the error variable of r with the new value of the
        // error variable of q.
        ScalarType maxError = std::numeric_limits<ScalarType>::min();
        GNGNode<ScalarType>* maxErrorNode = nullptr;
        for( GNGNode<ScalarType>* n : _nodes )
        {
            //            cerr << "n=" << n << " n_error=" << n->error() << endl;
            if( n->error() > maxError )
            {
                //                cerr << "maxError=" << maxError << " n_error=" << n->error() << endl;
                maxError = n->error();
                maxErrorNode = n;
            }
        }
        //        cerr << "maxErrorNode=" << maxErrorNode << endl;

        //
        if( maxErrorNode && maxErrorNode->neighbors().size() > 0 )
        {
            ScalarType maxNeihborError = std::numeric_limits<ScalarType>::min();
            GNGNode<ScalarType>* maxNeighborErrorNode = nullptr;
            //            cerr << "neighbors.size=" << maxErrorNode->neighbors().size() << endl;
            for( GNGNode<ScalarType>* nn : maxErrorNode->neighbors() )
            {
                if( nn->error() >= maxNeihborError )
                {
                    //                    cerr << "maxNeighborError=" << maxNeihborError << " nn_error=" << nn->error() << endl;
                    maxNeihborError = nn->error();
                    maxNeighborErrorNode = nn;
                }
            }

            if( maxNeighborErrorNode )
            {
                // Insert halfway
                std::vector<ScalarType> w_q = maxErrorNode->position();
                std::vector<ScalarType> w_f = maxNeighborErrorNode->position();
                std::vector<ScalarType> w_r = vector_scale( vector_add( w_q, w_f ), (ScalarType)1 / (ScalarType)2 );
                GNGNode<ScalarType> * f = new GNGNode<ScalarType>( w_r.size(), w_r );
                maxErrorNode->error() *= _alpha;
                maxNeighborErrorNode->error() *= _alpha;
                f->error() = maxErrorNode->error();
                _nodes.insert( f );
                removeEdge( maxErrorNode, maxNeighborErrorNode );
                addEdge( maxErrorNode, f );
                addEdge( f, maxNeighborErrorNode );
            }
        }
    }

    bool removeEdge( GNGNode<ScalarType>* a, GNGNode<ScalarType>* b )
    {
        GNGEdge<ScalarType>* toremove = nullptr;
        for( GNGEdge<ScalarType>* e : _edges )
        {
            if( e->a() == a && e->b() == b || e->a() == b && e->b() == a )
            {
                toremove = e;
                break;
            }
        }
        if( toremove )
        {
            _edges.erase( toremove );
            delete toremove;
            return true;
        }
        else
        {
            return false;
        }
    }

    void addEdge( GNGNode<ScalarType>* a, GNGNode<ScalarType>* b )
    {
        GNGEdge<ScalarType>* e = new GNGEdge<ScalarType>( a, b );
        _edges.insert( e );
    }

protected:

};
