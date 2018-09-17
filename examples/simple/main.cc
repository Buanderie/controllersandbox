#include <iostream>

using namespace std;

#include <gngnode.h>
#include <gng.h>
#include <utils.h>

int main( int argc, char** argv )
{
    srand(time(NULL));

    GNG<double> gng;

    for( int k = 0; k < 100000; ++k )
    {
        std::vector<double> vi = gngrand_position<double>(2);
        gng.in( vi );
    }
    return 0;
}
