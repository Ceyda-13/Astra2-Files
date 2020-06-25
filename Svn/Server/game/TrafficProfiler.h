#pragma once

class TrafficProfiler: public singleton<TrafficProfiler>
{
	public:

		enum IODirection {
			IODIR_INPUT	= 0,
			IODIR_OUTPUT,
			IODIR_MAX
		};

	public:

		TrafficProfiler( void );

		~TrafficProfiler( void );

		bool	Initialize( uint32_t dwFlushCycle, const char* pszLogFileName );

		bool	Report( IODirection dir, uint8_t byHeader, uint32_t dwSize )
		{
			ComputeTraffic( dir, byHeader, dwSize );
			if ( (uint32_t)(time( nullptr ) - m_tmProfileStartTime) >= m_dwFlushCycle )
				return Flush();
			return true;
		}

		bool	Flush( void );

	private:

		void	InitializeProfiling( void );

		void	ComputeTraffic( IODirection dir, uint8_t byHeader, uint32_t dwSize )
		{

			TrafficInfo& rTrafficInfo = m_aTrafficVec[ dir ][ byHeader ];

			m_dwTotalTraffic += dwSize;
			m_dwTotalPacket += !rTrafficInfo.second;

			rTrafficInfo.first += dwSize;
			rTrafficInfo.second++;
		}

		typedef std::pair< uint32_t, uint32_t >	TrafficInfo;

		typedef std::vector< TrafficInfo > TrafficVec;

		FILE* m_pfProfileLogFile;
		uint32_t m_dwFlushCycle;
		time_t m_tmProfileStartTime;
		uint32_t m_dwTotalTraffic;
		uint32_t m_dwTotalPacket;
		TrafficVec m_aTrafficVec[ IODIR_MAX ];
};


