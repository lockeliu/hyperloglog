/****************************************************
*         Filename: hyperloglogcount.c
*         Creat Time: 2016-12-29 16:36
*         Author: lockeliu
*         Description: hyperloglog 
******************************************************/

#include "hyperloglogcount.h"
#include "murmurhash.h"
#include <math.h>
#include <stdio.h>

namespace hyperloglogcount 
{
	HyperLogLogCount :: HyperLogLogCount(const uint64_t & iSize)
	{
		m_pHLLC = NULL;
		m_iSize = iSize;
		m_iLog2Size = 0;
		m_fAlphaMM = 0.0;
	}

	HyperLogLogCount :: ~HyperLogLogCount()
	{

        if( m_pHLLC != NULL)
            delete []m_pHLLC;
	}

    int HyperLogLogCount :: Init()
    {
        int iRet = 0;
        iRet = SetSize( m_iSize );
        if( iRet ) return iRet;
        m_pHLLC = new uint8_t[m_iSize];
        if( m_pHLLC == NULL) return -1;
    }

	//二分法
	//计算一个数，二进制格式下，后面零的个数
	uint32_t HyperLogLogCount :: CountTrailZeros( uint64_t iNum )
	{
		uint64_t iNumTmp;
		uint32_t iResult = 63;

		if (iNum == 0)
			return 64;

		iNumTmp = iNum << 32;    if (iNumTmp != 0) {  iResult -= 32; iNum = iNumTmp; }
		iNumTmp = iNum << 16;    if (iNumTmp != 0) {  iResult -= 16; iNum = iNumTmp; }
		iNumTmp = iNum << 8;     if (iNumTmp != 0) {  iResult -= 8; iNum = iNumTmp; }
		iNumTmp = iNum << 4;     if (iNumTmp != 0) {  iResult -= 4; iNum = iNumTmp; }
		iNumTmp = iNum << 2;     if (iNumTmp != 0) {  iResult -= 2; iNum = iNumTmp; }

		return iResult - (uint32_t)((iNum << 1) >> 63);

	}

	//设置hllc关联的内存的大小
	//0 suc
	//other error
	int HyperLogLogCount :: SetSize( uint64_t iSize )
	{
		if( iSize & ( iSize - 1 ) ) return ERR_PARAM_INVALID;

		//init
		m_iSize = iSize;
		m_iLog2Size = CountTrailZeros( iSize );

		switch (m_iLog2Size) {
			case 4:
				m_fAlphaMM = 0.673 * m_iSize * m_iSize;
				break;
			case 5:
				m_fAlphaMM = 0.697 * m_iSize * m_iSize;
				break;
			case 6:
				m_fAlphaMM = 0.709 * m_iSize * m_iSize;
				break;
			default:
				m_fAlphaMM = (0.7213 / (1 + 1.079 / m_iSize) ) * m_iSize * m_iSize;
		}
		return 0;
	}

	//检测参数是否合法
	//0 suc
	//1 fail
	int HyperLogLogCount :: CheckBefore()
	{
		if( NULL == m_pHLLC ) return 1;
		if( 0 == m_iSize  ) return 2;
		if( 0 == m_iLog2Size ) return 3;

		return 0;
	}

	//增加一个记录，支持并发
	//0 suc
	//other error
	//并不知道这个key是否本来就存在的
	int HyperLogLogCount :: MayContainAndInsert(const void * Key, uint32_t iLen )
	{
		int iRet = 0;
		if( CheckBefore() ) 
		{
			printf("ERR: %s CheckBefore fail! i size %ld log2size %u alphaMM %f", 
					__func__, m_iSize, m_iLog2Size, m_fAlphaMM );
			return ERR_PARAM_INVALID;
		}


		uint64_t iHash = (uint64_t) murmurhash64_no_seed( (void *)Key, iLen ); 
		uint64_t iIndex = iHash >> ( 64  - m_iLog2Size );

		uint8_t iResult = (uint8_t)CountTrailZeros( ( ( iHash << m_iLog2Size ) >> m_iLog2Size ) ) + 1;

		uint8_t iHLLC = 0;
		while( iHLLC = m_pHLLC[iIndex] , iResult > iHLLC )
		{
			//test and set 支持并发
			iRet = __sync_bool_compare_and_swap( &(m_pHLLC[iIndex]), iHLLC, iResult );
			if( 0 == iRet )
			{
				continue;
			}
			return 0;
		}

		return 0;

	}

	//增加一个记录，支持并发
	//0 suc
	//other error
	//并不知道这个key是否本来就存在的
	int HyperLogLogCount :: MayContainAndInsert( const string & sKey )
	{
		return MayContainAndInsert( sKey.c_str(), sKey.size() );
	}

	//增加一个记录，支持并发
	//0 suc
	//other error
	//并不知道这个key是否本来就存在的
	int HyperLogLogCount :: MayContainAndInsert( const uint64_t iKey )
	{
		return MayContainAndInsert( &iKey, sizeof(uint64_t ) );
	}

	//增加一个记录，支持并发
	//0 suc
	//other error
	//并不知道这个key是否本来就存在的
	int HyperLogLogCount :: MayContainAndInsert( const uint32_t iKey )
	{
		return MayContainAndInsert( &iKey, sizeof( uint32_t ) );
	}

	//估算UV
	//0 suc
	//other error
	int HyperLogLogCount :: CountUV( uint64_t & iResult )
	{
		if( CheckBefore() )
		{
			printf("ERR: %s CheckBefore fail! i size %ld log2size %u alphaMM %f", 
					__func__, m_iSize, m_iLog2Size, m_fAlphaMM );
			return ERR_PARAM_INVALID;
		}

		double fSum  = 0.0 , fEstimate = 0.0;
		uint32_t iZeros = 0;

		for(uint32_t i = 0; i < m_iSize; ++i )
		{
			fSum += pow( 2, ( -1 * m_pHLLC[i] ) );
		}

		fEstimate = m_fAlphaMM * ( 1.0 / fSum );

		if( fEstimate <= 2.5 * m_iSize )//小范围
		{
			for( uint32_t i = 0 ; i < m_iSize; ++i )
			{
				if( m_pHLLC[i] == 0 ) 
				{
					++iZeros;
				}
			}

			iResult =  (uint64_t)round( 1.0 * m_iSize * log( 1.0 * m_iSize / iZeros ) );
		}
		else if( fEstimate <=  (1.0 / 30.0 * POW_2_32 ) )//中范围
		{
			iResult = (uint64_t) round( fEstimate );
		}else //大范围
		{
			iResult = (uint64_t) round( (NEGATIVE_POW_2_32 * log(1.0 - (fEstimate / POW_2_32))));
		}
		return  0;
	}

}

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL$ $Id$ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

