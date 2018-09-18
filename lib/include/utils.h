#pragma once

#include <vector>

#include <unistd.h>
#include <math.h>

template< typename ScalarType >
ScalarType interpolation(ScalarType x1, ScalarType x2, ScalarType y1, ScalarType y2, ScalarType xInt)
{
    return y1 + (y2 - y1) / (x2 - x1)*(xInt - x1);
}

template< typename ScalarType >
ScalarType gngrand( ScalarType minVal = 0.0, ScalarType maxVal=1.0 )
{
    ScalarType rv = (ScalarType)rand() / (ScalarType)RAND_MAX;
    return interpolation( (ScalarType)0.0, (ScalarType)1.0, minVal, maxVal, rv );
}

template< typename ScalarType >
std::vector<ScalarType> gngrand_position( size_t size, ScalarType minVal=0.0, ScalarType maxVal=1.0 )
{
    std::vector< ScalarType > ret;
    ret.resize(size);
    for( int i = 0; i < ret.size(); ++i )
    {
        ret[ i ] = gngrand<ScalarType>(minVal, maxVal);
    }
    return ret;
}

template< typename ScalarType >
ScalarType vector_distance( const std::vector<ScalarType>& a,
                            const std::vector<ScalarType>& b,
                            bool retSquared = false )
{
    ScalarType sum = 0;
    // TODO: Assert a.Size() == b.size
    for( int i = 0; i < a.size(); ++i )
    {
        sum += (a[i] - b[i])*(a[i] - b[i]);
    }
    if( !retSquared )
        sum = (ScalarType)sqrt( (double)sum );
    return sum;
}

template< typename ScalarType >
std::vector<ScalarType> vector_sub( const std::vector<ScalarType>& a,
                                    const std::vector<ScalarType>& b )
{
    std::vector< ScalarType > ret( a.size() );
    for( int i = 0; i < a.size(); ++i )
    {
        ret[i] = a[i] - b[i];
    }
    return ret;
}

template< typename ScalarType >
std::vector<ScalarType> vector_add( const std::vector<ScalarType>& a,
                                    const std::vector<ScalarType>& b )
{
    std::vector< ScalarType > ret( a.size() );
    for( int i = 0; i < a.size(); ++i )
    {
        ret[i] = a[i] + b[i];
    }
    return ret;
}

template< typename ScalarType >
std::vector<ScalarType> vector_scale( const std::vector<ScalarType>& a,
                                      ScalarType s )
{
    std::vector< ScalarType > ret( a.size() );
    for( int i = 0; i < a.size(); ++i )
    {
        ret[i] = a[i] * s;
    }
    return ret;
}

