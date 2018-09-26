#pragma once

#include <gng.h>
#include <cuckoohash_map.hh>

#include <fstream>
#include <iostream>

// #define DYN_VQ

using namespace std;

class CerCon
{
public:
    CerCon( size_t inputSize )
        :_inputSize( inputSize )
    {
        _gng = new GNG<double>( inputSize );
        _ofs.open( "error.csv" );
        _ofs << "t,error" << endl;
        _seenSamples = 0;
        _learningRate = 0.1;
        _radius = 15;
#ifdef DYN_VQ
        _needVq = false;
#else
        _needVq = true;
#endif
        _numIter = 0;
    }

    virtual ~CerCon()
    {
        _ofs.close();
    }

    void train( std::vector<double> in, double error )
    {

        cerr << "iter=" << _numIter << endl;

        // Check if we need VQ update
        GNGNode<double>* ret_node;
        double vqError = _gng->find( in, ret_node );
#ifdef DYN_VQ
        if( vqError >= 0.5 || _numIter <= 2000 )
        {
            _needVq = true;
        }
        else
        {
            _needVq = false;
        }
#endif

        cerr << "current VQ error: " << vqError << endl;
        // Present input vector for VQ
        if( _needVq )
        {
            double avgVqError = 0.0;
            int numVqIter = 200;
            if( in.size() == _inputSize )
            {
                for( int k = 0; k < numVqIter; ++k )
                {
                    double verr = _gng->in( in );
                    avgVqError += verr;
                }
            }
            avgVqError /= (double)numVqIter;
            cerr << "avgVqError=" << avgVqError << endl;
#ifndef DYN_VQ
            if( _numIter > 300 )
            {
                _needVq = false;
            }
#endif
        }

        // Perform a least-square iteration...
        // (1) Retrieve the N closest nodes from input
        std::vector< GNGNode<double>* > closest = _gng->findNearest( in, _radius );
        // (2) Retrieve weights
        std::map< GNGNode<double>*, double > weights;
        for( GNGNode<double>* n : closest )
        {
            weights.insert( make_pair(n, getWeight(n)) );
        }


        /*
        // Debug print
        cerr << "hashmap.size()=" << _weights.size() << endl;
        // Current weight
        for( double w : weights )
        {
            cerr << "weight: " << w << endl;
        }
        */

        double increment = (error / (double)(_weights.size())) * _learningRate;
        for( auto& kv : weights )
        {
            double wi = _weights.find( kv.first );
            double wipp = wi + increment;
            _weights.insert_or_assign( kv.first, wipp );
        }

        _numIter++;

    }

    double predict( std::vector<double> in )
    {
        std::vector< GNGNode<double>* > nearestNodes = _gng->findNearest( in, _radius );
        return computeFinalResult( nearestNodes );
    }

private:
    size_t _inputSize;
    GNG<double> * _gng;
    cuckoohash_map< GNGNode<double>*, double > _weights;
    ofstream _ofs;
    int _seenSamples;
    int _radius;

    bool _needVq;

    double _learningRate;

    double getWeight( GNGNode<double>* n )
    {
        double ret;
        if( _weights.find( n, ret ) )
        {

        }
        else
        {
            cerr << "*** NEW WEIGHT" << endl;
            ret = gngrand<double>(-1.0, 1.0);
            _weights.insert( n, ret );
        }
        return ret;
    }

    double computeFinalResult( std::vector< GNGNode<double>* > nodes )
    {
        double sum = 0.0;
        // Compute the sum of all weights...
        for( GNGNode<double>* n : nodes )
        {
            sum += getWeight( n );
        }
        return sum;
    }

protected:
    int _numIter;

};
