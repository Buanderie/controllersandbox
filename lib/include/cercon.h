#pragma once

#include <gng.h>

#include <fstream>
#include <iostream>

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
    }

    virtual ~CerCon()
    {
        _ofs.close();
    }

    double predict( std::vector<double> in )
    {
        if( in.size() == _inputSize )
        {
            for( int k = 0; k < 100; ++k )
                _gng->in( in );
        }
        GNGNode<double>* ret_gng;
        double error = _gng->find( in, ret_gng );
        cerr << "NEIGH: " << ret_gng->neighbors().size() << endl;
        _ofs << _seenSamples++ << "," << error << endl;
        return error;
    }

private:
    size_t _inputSize;
    GNG<double> * _gng;
    ofstream _ofs;
    int _seenSamples;

protected:

};
