#pragma once
#ifdef ENABLE_HSHIELD_SYSTEM
#include <unordered_map> // @correction106
#endif

#ifdef __FreeBSD__
#define UNIX
#ifdef ENABLE_HSHIELD_SYSTEM
#include <AntiCpXSvr.h>
#endif
#undef UNIX
#endif

#pragma pack(1)

typedef struct SPacketGCHSCheck
{
	uint8_t	bHeader;
#ifdef __FreeBSD__
#ifdef ENABLE_HSHIELD_SYSTEM
	AHNHS_TRANS_BUFFER Req;
#endif
#endif
} TPacketGCHSCheck;

#pragma pack()

class CHackShieldImpl
{
	public:
		bool Initialize ();
		void Release ();

		bool CreateClientHandle (uint32_t dwPlayerID);
		void DeleteClientHandle (uint32_t dwPlayerID);

		bool SendCheckPacket (LPCHARACTER ch);
		bool VerifyAck (LPCHARACTER ch, TPacketGCHSCheck* buf);

	private:
#ifdef __FreeBSD__
#ifdef ENABLE_HSHIELD_SYSTEM
		AHNHS_SERVER_HANDLE handle_;

		typedef std::unordered_map<uint32_t, AHNHS_CLIENT_HANDLE> ClientHandleContainer;
		ClientHandleContainer CliehtHandleMap_;

		typedef std::unordered_map<uint32_t, bool> ClientCheckContainer;
		ClientCheckContainer ClientCheckMap_;
#endif
#endif
};


