#include <iostream>
#include <fstream>

using namespace std;

#include <gngnode.h>
#include <gng.h>
#include <utils.h>
#include <cercon.h>

#include <opencv2/opencv.hpp>
using namespace cv;

std::vector<double> randBox( double minX, double maxX,
                             double minY, double maxY )
{
    double x = gngrand<double>( minX, maxX );
    double y = gngrand<double>( minY, maxY );
    std::vector<double> ret;
    ret.push_back( x );
    ret.push_back( y );
    return ret;
}

std::vector<double> randSin()
{
    double x = gngrand<double>( -1.0, 1.0 );
    double y = sin( x );
    std::vector<double> ret;
    ret.push_back( x );
    ret.push_back( y );
    return ret;
}

int main( int argc, char** argv )
{
    srand(time(NULL));

    ofstream ofs( "error.csv" );
    ofs << "t,error" << endl;

#ifdef GRAPH
    GNG<double> gng;
    int t = 0;
    while(true)
    {
        std::vector<double> vi = randBox(0, 320, 0, 240 );
        std::vector<double> vi2 = randBox(320, 640, 240, 480);
//        std::vector<double> vi2 = gngrand_position<double>(2, 2, 3);
        for( int k = 0; k < 100; ++k )
        { gng.in( vi );
//        gng.in( vi2 );
        }
        cv::Mat popo = cv::Mat::zeros( cv::Size(640, 480), CV_8UC3 );
        for( auto n : gng.nodes() )
        {
            cv::circle( popo, cv::Point( n->position()[0], n->position()[1] ), 2, Scalar(255,0,255), 2 );
        }
        for( auto e : gng.edges() )
        {
            GNGNode<double>* a = e->a();
            GNGNode<double>* b = e->b();
            cv::Point2d p1( a->position()[0], a->position()[1] );
            cv::Point2d p2( b->position()[0], b->position()[1] );
            cv::line( popo, p1, p2, Scalar(255, 255, 255), 1 );
        }

        std::vector<double> ret;
        GNGNode<double>* n;
        double error = gng.find( vi, n );
        ret = n->position();
        ofs << t << "," << error << endl;

//        imshow("popo", popo);
//        waitKey(5);
        t++;
    }
#else
    CerCon cc(1);
    while(true)
    {
        double r = gngrand<double>(-1.0, 1.0);
        std::vector<double> in = { r };
        double rr = cc.predict(in);
        cerr << "error=" << rr << endl;
    }
#endif

    return 0;
}
