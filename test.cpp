#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include "stdio.h"
#include "hyperloglogcount.h"

using namespace std;

void ShowUsage( const char * program )
{
    printf("%s teststr size test_size \n", program );
    printf("%s testint size test_size \n", program );
}

int main(int argc, char ** argv )
{
    if( argc < 2 )
    {
        ShowUsage( argv[0] );
        return 0;
    }

    const char * pcFunc = argv[1];
    if ( (strcasecmp(pcFunc, "teststr") == 0) && (argc >= 4) )
    {
        uint32_t iSize = atol( argv[2] );

        hyperloglogcount::HyperLogLogCount oHyperLogLogCount(iSize);
        oHyperLogLogCount.Init();

        uint64_t iTestSize = atoll( argv[3] );
        for( uint64_t i = 1; i <= iTestSize; ++i )
        {
            string s = to_string(i);
            oHyperLogLogCount.MayContainAndInsert( s );
        }
        uint64_t iResult = 0;
        oHyperLogLogCount.CountUV( iResult );
        printf("result: %ld rate: %f\n", iResult , 1.0 * (1.0 * iTestSize - iResult ) / iTestSize);

    }
    else if ( (strcasecmp(pcFunc, "testint") == 0) && (argc >= 4) )
    {
        uint32_t iSize = atoi( argv[2] );

        hyperloglogcount::HyperLogLogCount oHyperLogLogCount(iSize);
        oHyperLogLogCount.Init();

        uint64_t iTestSize = atoll( argv[3] );
        for( uint64_t i = 1; i <= iTestSize; ++i )
        {
            oHyperLogLogCount.MayContainAndInsert( i );
            uint64_t iResult = 0;
            oHyperLogLogCount.CountUV( iResult );
            printf("%ld: result: %ld rate: %f\n", i, iResult , 1.0 * (1.0 * i - iResult ) / i);
        }

    }
    else
    {
        ShowUsage(argv[0]);
    }
    return 0;
}

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL$ $Id$ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

