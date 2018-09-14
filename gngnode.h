#pragma once

#include <vector>
#include <set>

template< typename ScalarType >
class GNGNode
{
public:
    GNGNode( size_t inputSize, const std::vector< ScalarType >& position )
        :_position(position)
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

        return ret;
    }

    std::vector<ScalarType>& position()
    {
        return _position;
    }

private:

protected:
    std::vector< ScalarType > _position;
    std::set< GNGNode< ScalarType >* > _neighbors;

};
