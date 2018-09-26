#pragma once

#include <gng.h>
#include <cuckoohash_map.hh>

#include <fstream>
#include <iostream>

#define DYN_VQ

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
        _learningRate = 0.2;
        _radius = 80;
#ifdef DYN_VQ
        _needVq = false;
#else
        _needVq = true;
#endif
        _numIter = 0;
        _vqEpisodes = 0;
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
        if( _numIter <= 500 || _numIter % 1000 == 0 )
        {
            _needVq = true;
            _vqEpisodes = 0;
        }
        else
        {
            _needVq = false;
        }

        if( _vqEpisodes >= 300 )
        {
            _vqEpisodes = 0;
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
            _vqEpisodes++;
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

        double increment = (error / (double)(_gng->nodes().size())) * _learningRate;
        double totalAssignment = 0.0;
        for( auto& kv : weights )
        {
            totalAssignment += 1.0 / (kv.first->learnedTimes() + 1.0);
        }

        for( auto& kv : weights )
        {
            double assignment = 1.0 / (kv.first->learnedTimes() + 1.0);
            // cerr << "asignment=" << assignment << " totalAssignment=" << totalAssignment << endl;
            // assignment = 1.0;
            double wi = kv.first->weight();
            double wipp = wi + (increment * (assignment / totalAssignment) );
            // _weights.insert_or_assign( kv.first, wipp );
            kv.first->weight() = wipp;
            kv.first->learnedTimes() += 1.0;
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
        double ret = n->weight();
        /*
        if( _weights.find( n, ret ) )
        {

        }
        else
        {
            cerr << "*** NEW WEIGHT" << endl;
            ret = gngrand<double>(-1.0, 1.0);
            _weights.insert( n, ret );
        }
        */
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
    int _vqEpisodes;
};
