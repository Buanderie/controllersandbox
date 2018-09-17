#pragma once

#include <vector>
#include <set>

template< typename ScalarType >
class GNGNode
{
public:
    GNGNode( size_t inputSize, const std::vector< ScalarType >& position )
        :_position(position), _error(100.0)
    {

    }

    virtual ~GNGNode()
    {

    }

    bool addNeighbor( GNGNode<ScalarType> * n )
    {
        auto ret = _neighbors.insert( n );
        return ret.second;
    }

    bool removeNeighbor( GNGNode<ScalarType> * n )
    {
        return (_neighbors.erase( n ) == 1);
    }

    std::vector< GNGNode<ScalarType>* > neighbors()
    {
        std::vector< GNGNode<ScalarType>* > ret;
        for( GNGNode<ScalarType>* nn : _neighbors )
            ret.push_back( nn );
        return ret;
    }

    std::vector<ScalarType>& position()
    {
        return _position;
    }

    ScalarType& error()
    {
        return _error;
    }

private:

protected:
    std::vector< ScalarType > _position;
    std::set< GNGNode< ScalarType >* > _neighbors;
    ScalarType _error;

};
