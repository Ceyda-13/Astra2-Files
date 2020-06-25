#pragma once

#define SESSION_BUF_LEN		320
#define VERIFY_PACK_LEN		128
#define SESSION_CSSTEP1_LEN	256

#pragma pack(1)

typedef struct PacketXTrapVerify
{
	uint8_t	bHeader;
	uint8_t	bPacketData[VERIFY_PACK_LEN];

} TPacketXTrapCSVerify;

#pragma pack()

class CXTrapManager : public singleton<CXTrapManager>
{
public:
	CXTrapManager();
	virtual ~CXTrapManager();

	bool LoadXTrapModule();
	bool LoadClientMapFile( uint32_t iMapIndex );

	bool CreateClientSession( LPCHARACTER lpCharSession );
	void DestroyClientSession( LPCHARACTER lpCharSession );

	bool Verify_CSStep1( LPCHARACTER lpCharSession, uint8_t* pOutBufData );
	void Verify_CSStep3( LPCHARACTER lpCharSession, uint8_t* pBufData );
private:
	struct			sXTrapContext;
	sXTrapContext*	m_pImpl;

	struct sSessionInfo
	{
		sSessionInfo()
		{
			m_pCheckEvent = nullptr;
			memset(szSessionBuf, 0x00, sizeof(szSessionBuf) );
			memset(szPackBuf, 0x00, sizeof(szPackBuf) );
		}

		uint8_t	szSessionBuf[SESSION_BUF_LEN];
		uint8_t	szPackBuf[VERIFY_PACK_LEN];
		LPEVENT m_pCheckEvent;
	};

#ifdef ENABLE_XTRAP_SYSTEM
	typedef std::unordered_map<uint32_t, sSessionInfo> ClientSessionMap;

	ClientSessionMap	m_mapClientSessions;
#endif

};

