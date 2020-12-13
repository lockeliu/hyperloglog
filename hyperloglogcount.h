/****************************************************
*         Filename: hyperloglogcount.h
*         Creat Time: 2016-12-29 16:36
*         Author: lockeliu
*         Description: hyperloglog 
******************************************************/
#pragma once 
#include <string>
#include "stdint.h"

using namespace std;

namespace hyperloglogcount
{

	static const double POW_2_32 = 4294967296.0;
	static const double NEGATIVE_POW_2_32 = -4294967296.0;


	enum ERR_CODE
	{
		ERR_PARAM_INVALID = 1,
	};

	class HyperLogLogCount
	{
		public:
			HyperLogLogCount(const uint64_t & iSize);

			~HyperLogLogCount();

            // 初始化内存
            int Init();

			//增加一个记录
			int MayContainAndInsert(const void * Key, uint32_t iLen ); 
			int MayContainAndInsert( const string & sKey ); 
			int MayContainAndInsert(  const uint64_t iKey ); 
			int MayContainAndInsert(  const uint32_t iKey ); 

			//估算UV
			int CountUV( uint64_t & iResult);

		private:

			uint8_t * m_pHLLC;//hllc关联的那块内存
			uint64_t m_iSize;//hllc关联的内存的大小
			uint32_t m_iLog2Size;//m_iLog2Size = log( m_iSize );
			double m_fAlphaMM;//参数值


			//检测参数是否合法
			int CheckBefore();

            //设置桶大小
			int SetSize( uint64_t iSize );
			//计算一个数，二进制形式下后面有多少个零
			uint32_t CountTrailZeros(uint64_t iNum );
	};
}

