#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "protocol.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "cmd.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "battle.h"
#include "exchange.h"
#include "questmanager.h"
#include "profiler.h"
#include "messenger_manager.h"
#include "party.h"
#include "p2p.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "log.h"
#include "banword.h"
#include "empire_text_convert.h"
#include "unique_item.h"
#include "building.h"
#include "locale_service.h"
#include "gm.h"
#include "spam.h"
#include "ani.h"
#include "motion.h"
#include "OXEvent.h"
#include "locale_service.h"
#include "HackShield.h"
#include "XTrapManager.h"
#include "DragonSoul.h"
#include "belt_inventory_helper.h" // @correction042

#include "input.h"
#ifdef __GROWTH_PET__
#include "new_petsystem.h"
#endif
#ifdef __MINI_GAME__
#include "minigame_manager.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif

#ifdef ENABLE_CHAT_LOGGING // @correction182
static char	__escape_string[1024];
static char	__escape_string2[1024];
#endif

const char* ConvertEmpireIndexToStr(int32_t empireIndex)
{
#define EMPIRE_RED		1
#define EMPIRE_YELLOW	2
#define EMPIRE_BLUE		3

	switch (empireIndex)
	{
	case EMPIRE_RED:
		return "a";
	case EMPIRE_YELLOW:
		return "b";
	case EMPIRE_BLUE:
		return "c";
	default:
		return "";
	}
}

static int32_t __deposit_limit()
{
	return (1000*10000);
}

void SendBlockChatInfo(LPCHARACTER ch, int32_t sec)
{
	if (sec <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채팅 금지 상태입니다."));
		return;
	}

	int32_t hour = sec / 3600;
	sec -= hour * 3600;

	int32_t min = (sec / 60);
	sec -= min * 60;

	char buf[128+1];

	if (hour > 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d 시간 %d 분 %d 초 동안 채팅금지 상태입니다"), hour, min, sec);
	else if (hour > 0 && min == 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d 시간 %d 초 동안 채팅금지 상태입니다"), hour, sec);
	else if (hour == 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d 분 %d 초 동안 채팅금지 상태입니다"), min, sec);
	else
		snprintf(buf, sizeof(buf), LC_TEXT("%d 초 동안 채팅금지 상태입니다"), sec);

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

EVENTINFO(spam_event_info)
{
	char host[MAX_HOST_LENGTH+1];

	spam_event_info()
	{
		::memset( host, 0, MAX_HOST_LENGTH+1 );
	}
};

typedef std::unordered_map<std::string, std::pair<uint32_t, LPEVENT> > spam_score_of_ip_t;
spam_score_of_ip_t spam_score_of_ip;

EVENTFUNC(block_chat_by_ip_event)
{
	spam_event_info* info = dynamic_cast<spam_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "block_chat_by_ip_event> <Factor> Null pointer" );
		return 0;
	}

	const char * host = info->host;

	spam_score_of_ip_t::iterator it = spam_score_of_ip.find(host);

	if (it != spam_score_of_ip.end())
	{
		it->second.first = 0;
		it->second.second = nullptr;
	}

	return 0;
}

bool SpamBlockCheck(LPCHARACTER ch, const char* const buf, const size_t buflen)
{
	if (ch->GetLevel() < g_iSpamBlockMaxLevel)
	{
		spam_score_of_ip_t::iterator it = spam_score_of_ip.find(ch->GetDesc()->GetHostName());

		if (it == spam_score_of_ip.end())
		{
			spam_score_of_ip.insert(std::make_pair(ch->GetDesc()->GetHostName(), std::make_pair(0, (LPEVENT) nullptr)));
			it = spam_score_of_ip.find(ch->GetDesc()->GetHostName());
		}

		if (it->second.second)
		{
			SendBlockChatInfo(ch, event_time(it->second.second) / passes_per_sec);
			return true;
		}

		uint32_t score;
		const char * word = SpamManager::instance().GetSpamScore(buf, buflen, score);

		it->second.first += score;

		if (word)
			sys_log(0, "SPAM_SCORE: %s text: %s score: %u total: %u word: %s", ch->GetName(), buf, score, it->second.first, word);

		if (it->second.first >= g_uiSpamBlockScore)
		{
			spam_event_info* info = AllocEventInfo<spam_event_info>();
			strlcpy(info->host, ch->GetDesc()->GetHostName(), sizeof(info->host));

			it->second.second = event_create(block_chat_by_ip_event, info, PASSES_PER_SEC(g_uiSpamBlockDuration));
			sys_log(0, "SPAM_IP: %s for %u seconds", info->host, g_uiSpamBlockDuration);

			LogManager::instance().CharLog(ch, 0, "SPAM", word);

			SendBlockChatInfo(ch, event_time(it->second.second) / passes_per_sec);

			return true;
		}
	}

	return false;
}

enum
{
	TEXT_TAG_PLAIN,
	TEXT_TAG_TAG,
	TEXT_TAG_COLOR,
	TEXT_TAG_HYPERLINK_START,
	TEXT_TAG_HYPERLINK_END,
	TEXT_TAG_RESTORE_COLOR,
};

int32_t GetTextTag(const char * src, int32_t maxLen, int32_t & tagLen, std::string & extraInfo)
{
	tagLen = 1;

	if (maxLen < 2 || *src != '|')
		return TEXT_TAG_PLAIN;

	const char * cur = ++src;

	if (*cur == '|')
	{
		tagLen = 2;
		return TEXT_TAG_TAG;
	}
	else if (*cur == 'c')
	{
		tagLen = 2;
		return TEXT_TAG_COLOR;
	}
	else if (*cur == 'H')
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_START;
	}
	else if (*cur == 'h')
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_END;
	}

	return TEXT_TAG_PLAIN;
}

void GetTextTagInfo(const char * src, int32_t src_len, int32_t & hyperlinks, bool & colored)
{
	colored = false;
	hyperlinks = 0;

	int32_t len;
	std::string extraInfo;

	for (int32_t i = 0; i < src_len;)
	{
		int32_t tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

		if (tag == TEXT_TAG_HYPERLINK_START)
			++hyperlinks;

		if (tag == TEXT_TAG_COLOR)
			colored = true;

		i += len;
	}
}

int32_t ProcessTextTag(LPCHARACTER ch, const char * c_pszText, size_t len)
{
	int32_t hyperlinks;
	bool colored;

	GetTextTagInfo(c_pszText, len, hyperlinks, colored);

	if (colored == true && hyperlinks == 0)
		return 4;

	if (g_bDisablePrismNeed) // @correction208
		return 0;

	int32_t nPrismCount = ch->CountSpecifyItem(ITEM_PRISM);

	if (nPrismCount < hyperlinks)
		return 1;


	if (!ch->GetMyShop())
	{
		ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
		return 0;
	} else
	{
		int32_t sellingNumber = ch->GetMyShop()->GetNumberByVnum(ITEM_PRISM);
		if(nPrismCount - sellingNumber < hyperlinks)
		{
			return 2;
		} else
		{
			ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
			return 0;
		}
	}

	return 4;
}

int32_t CInputMain::Whisper(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	const TPacketCGWhisper* pinfo = reinterpret_cast<const TPacketCGWhisper*>(data);

	if (uiBytes < pinfo->wSize)
		return -1;

	int32_t iExtraLen = pinfo->wSize - sizeof(TPacketCGWhisper);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

#ifdef ENABLE_WHISPER_CHAT_SPAMLIMIT // @correction184
	if (ch->IncreaseChatCounter() >= 10)
	{
		ch->GetDesc()->DelayedDisconnect(0);
		return (iExtraLen);
	}
#endif

	if (ch->FindAffect(AFFECT_BLOCK_CHAT))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채팅 금지 상태입니다."));
		return (iExtraLen);
	}

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindPC(pinfo->szNameTo);

	if (pkChr == ch)
		return (iExtraLen);

	LPDESC pkDesc = nullptr;

	uint8_t bOpponentEmpire = 0;

	if (test_server)
	{
		if (!pkChr)
			sys_log(0, "Whisper to %s(%s) from %s", "Null", pinfo->szNameTo, ch->GetName());
		else
			sys_log(0, "Whisper to %s(%s) from %s", pkChr->GetName(), pinfo->szNameTo, ch->GetName());
	}

	if (ch->IsBlockMode(BLOCK_WHISPER))
	{
		if (ch->GetDesc())
		{
			TPacketGCWhisper pack;
			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
			pack.wSize = sizeof(TPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(pack));
		}
		return iExtraLen;
	}

	if (!pkChr)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(pinfo->szNameTo);

		if (pkCCI)
		{
			pkDesc = pkCCI->pkDesc;
			pkDesc->SetRelay(pinfo->szNameTo);
			bOpponentEmpire = pkCCI->bEmpire;

			if (test_server)
				sys_log(0, "Whisper to %s from %s (Channel %d Mapindex %d)", "Null", ch->GetName(), pkCCI->bChannel, pkCCI->lMapIndex);
		}
	}
	else
	{
		pkDesc = pkChr->GetDesc();
		bOpponentEmpire = pkChr->GetEmpire();
	}

	if (!pkDesc)
	{
		if (ch->GetDesc())
		{
			TPacketGCWhisper pack;

			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_NOT_EXIST;
			pack.wSize = sizeof(TPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(TPacketGCWhisper));
			sys_log(0, "WHISPER: no player");
		}
	}
	else
	{
		if (ch->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack;
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
				pack.wSize = sizeof(TPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else if (pkChr && pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack;
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_TARGET_BLOCKED;
				pack.wSize = sizeof(TPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else
		{
			uint8_t bType = WHISPER_TYPE_NORMAL;

			char buf[CHAT_MAX_LEN + 1];
			strlcpy(buf, data + sizeof(TPacketCGWhisper), MIN(iExtraLen + 1, sizeof(buf)));
			const size_t buflen = strlen(buf);

			if (true == SpamBlockCheck(ch, buf, buflen))
			{
				if (!pkChr)
				{
					CCI * pkCCI = P2P_MANAGER::instance().Find(pinfo->szNameTo);

					if (pkCCI)
					{
						pkDesc->SetRelay("");
					}
				}
				return iExtraLen;
			}

			// @correction005
			CBanwordManager::instance().ConvertString(buf, buflen);

			if (g_bEmpireWhisper)
				if (!ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
					if (!(pkChr && pkChr->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)))
						if (bOpponentEmpire != ch->GetEmpire() && ch->GetEmpire() && bOpponentEmpire
								&& ch->GetGMLevel() == GM_PLAYER && gm_get_level(pinfo->szNameTo) == GM_PLAYER)
						{
							if (!pkChr)
							{
								bType = ch->GetEmpire() << 4;
							}
							else
							{
								ConvertEmpireText(ch->GetEmpire(), buf, buflen, 10 + 2 * pkChr->GetSkillPower(SKILL_LANGUAGE1 + ch->GetEmpire() - 1));
							}
						}

			int32_t processReturn = ProcessTextTag(ch, buf, buflen);
			if (0!=processReturn)
			{
				if (ch->GetDesc())
				{
					TItemTable * pTable = ITEM_MANAGER::instance().GetTable(ITEM_PRISM);

					if (pTable)
					{
						char buf[128];
						int32_t len;
						if (3==processReturn)
							len = snprintf(buf, sizeof(buf), LC_TEXT("사용할수 없습니다."), pTable->szLocaleName);
						else
							len = snprintf(buf, sizeof(buf), LC_TEXT("%s이 필요합니다."), pTable->szLocaleName);

						if (len < 0 || len >= (int32_t) sizeof(buf))
							len = sizeof(buf) - 1;

						++len;

						TPacketGCWhisper pack;

						pack.bHeader = HEADER_GC_WHISPER;
						pack.bType = WHISPER_TYPE_ERROR;
						pack.wSize = sizeof(TPacketGCWhisper) + len;
						strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));

						ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
						ch->GetDesc()->Packet(buf, len);

						sys_log(0, "WHISPER: not enough %s: char: %s", pTable->szLocaleName, ch->GetName());
					}
				}

				pkDesc->SetRelay("");
				return (iExtraLen);
			}

			if (ch->IsGM())
				bType = (bType & 0xF0) | WHISPER_TYPE_GM;

			if (buflen > 0)
			{
				TPacketGCWhisper pack;

				pack.bHeader = HEADER_GC_WHISPER;
				pack.wSize = sizeof(TPacketGCWhisper) + buflen;
				strlcpy(pack.szEmpire, ConvertEmpireIndexToStr(ch->GetEmpire()), sizeof(pack.szEmpire));
#ifdef __MULTI_LANGUAGE__
				strlcpy(pack.szLanguage, ch->GetLanguage(), sizeof(pack.szLanguage));
#endif
				pack.bType = bType;
				strlcpy(pack.szNameFrom, ch->GetName(), sizeof(pack.szNameFrom));

				TEMP_BUFFER tmpbuf;

				tmpbuf.write(&pack, sizeof(pack));
				tmpbuf.write(buf, buflen);

				pkDesc->Packet(tmpbuf.read_peek(), tmpbuf.size());

#ifdef ENABLE_CHAT_LOGGING // @correction182 // @correction005
				if (ch->IsGM())
				{
					LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), buf, buflen);
					LogManager::instance().EscapeString(__escape_string2, sizeof(__escape_string2), pinfo->szNameTo, sizeof(pack.szNameFrom));
					LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, __escape_string2, "WHISPER", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
				}
#endif
			}
		}
	}
	if(pkDesc)
		pkDesc->SetRelay("");

	return (iExtraLen);
}

struct RawPacketToCharacterFunc
{
	const void * m_buf;
	int32_t	m_buf_len;

	RawPacketToCharacterFunc(const void * buf, int32_t buf_len) : m_buf(buf), m_buf_len(buf_len)
	{
	}

	void operator () (LPCHARACTER c)
	{
		if (!c->GetDesc())
			return;

		c->GetDesc()->Packet(m_buf, m_buf_len);
	}
};

struct FEmpireChatPacket
{
	packet_chat& p;
	const char* orig_msg;
	int32_t orig_len;
	char converted_msg[CHAT_MAX_LEN+1];

	uint8_t bEmpire;
	int32_t iMapIndex;
	int32_t namelen;

	FEmpireChatPacket(packet_chat& p, const char* chat_msg, int32_t len, uint8_t bEmpire, int32_t iMapIndex, int32_t iNameLen)
		: p(p), orig_msg(chat_msg), orig_len(len), bEmpire(bEmpire), iMapIndex(iMapIndex), namelen(iNameLen)
	{
		memset( converted_msg, 0, sizeof(converted_msg) );
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetMapIndex() != iMapIndex)
			return;

		d->BufferedPacket(&p, sizeof(packet_chat));

		if (d->GetEmpire() == bEmpire ||
			bEmpire == 0 ||
			d->GetCharacter()->GetGMLevel() > GM_PLAYER ||
			d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
		{
			d->Packet(orig_msg, orig_len);
		}
		else
		{
			size_t len = strlcpy(converted_msg, orig_msg, sizeof(converted_msg));

			if (len >= sizeof(converted_msg))
				len = sizeof(converted_msg) - 1;

			ConvertEmpireText(bEmpire, converted_msg + namelen, len - namelen, 10 + 2 * d->GetCharacter()->GetSkillPower(SKILL_LANGUAGE1 + bEmpire - 1));
			d->Packet(converted_msg, orig_len);
		}
	}
};

struct FYmirChatPacket
{
	packet_chat& packet;
	const char* m_szChat;
	size_t m_lenChat;
	const char* m_szName;

	int32_t m_iMapIndex;
	uint8_t m_bEmpire;
	bool m_ring;

	char m_orig_msg[CHAT_MAX_LEN+1];
	int32_t m_len_orig_msg;
	char m_conv_msg[CHAT_MAX_LEN+1];
	int32_t m_len_conv_msg;

	FYmirChatPacket(packet_chat& p, const char* chat, size_t len_chat, const char* name, size_t len_name, int32_t iMapIndex, uint8_t empire, bool ring)
		: packet(p),
		m_szChat(chat), m_lenChat(len_chat),
		m_szName(name),
		m_iMapIndex(iMapIndex), m_bEmpire(empire),
		m_ring(ring)
	{
		m_len_orig_msg = snprintf(m_orig_msg, sizeof(m_orig_msg), "%s : %s", m_szName, m_szChat) + 1;

		if (m_len_orig_msg < 0 || m_len_orig_msg >= (int32_t) sizeof(m_orig_msg))
			m_len_orig_msg = sizeof(m_orig_msg) - 1;

		m_len_conv_msg = snprintf(m_conv_msg, sizeof(m_conv_msg), "??? : %s", m_szChat) + 1;

		if (m_len_conv_msg < 0 || m_len_conv_msg >= (int32_t) sizeof(m_conv_msg))
			m_len_conv_msg = sizeof(m_conv_msg) - 1;

		ConvertEmpireText(m_bEmpire, m_conv_msg + 6, m_len_conv_msg - 6, 10);
	}

	void operator() (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetMapIndex() != m_iMapIndex)
			return;

		if (m_ring ||
			d->GetEmpire() == m_bEmpire ||
			d->GetCharacter()->GetGMLevel() > GM_PLAYER ||
			d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
		{
			packet.size = m_len_orig_msg + sizeof(TPacketGCChat);

			d->BufferedPacket(&packet, sizeof(packet_chat));
			d->Packet(m_orig_msg, m_len_orig_msg);
		}
		else
		{
			packet.size = m_len_conv_msg + sizeof(TPacketGCChat);

			d->BufferedPacket(&packet, sizeof(packet_chat));
			d->Packet(m_conv_msg, m_len_conv_msg);
		}
	}
};

int32_t CInputMain::Chat(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	const TPacketCGChat* pinfo = reinterpret_cast<const TPacketCGChat*>(data);

	if (uiBytes < pinfo->size)
		return -1;

	const int32_t iExtraLen = pinfo->size - sizeof(TPacketCGChat);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->size, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char buf[CHAT_MAX_LEN - (CHARACTER_NAME_MAX_LEN + 3) + 1];
	strlcpy(buf, data + sizeof(TPacketCGChat), MIN(iExtraLen + 1, sizeof(buf)));
	const size_t buflen = strlen(buf);

	if (buflen > 1 && *buf == '/')
	{
		interpret_command(ch, buf + 1, buflen - 1);
		return iExtraLen;
	}
	
#ifdef ENABLE_CHAT_SPAMLIMIT // @correction183
	if (ch->IncreaseChatCounter() >= 4)
	{
		if (ch->GetChatCounter() == 10)
			ch->GetDesc()->DelayedDisconnect(0);
		return iExtraLen;
	}
#else
	if (ch->IncreaseChatCounter() >= 10)
	{
		if (ch->GetChatCounter() == 10)
		{
			sys_log(0, "CHAT_HACK: %s", ch->GetName());
			ch->GetDesc()->DelayedDisconnect(5);
		}

		return iExtraLen;
	}
#endif

	const CAffect* pAffect = ch->FindAffect(AFFECT_BLOCK_CHAT);

	if (pAffect != nullptr)
	{
		SendBlockChatInfo(ch, pAffect->lDuration);
		return iExtraLen;
	}

	if (true == SpamBlockCheck(ch, buf, buflen))
	{
		return iExtraLen;
	}

	// @correction054 begin
	CBanwordManager::instance().ConvertString(buf, buflen);

	int32_t processReturn = ProcessTextTag(ch, buf, buflen);
	if (0!=processReturn)
	{
		const TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ITEM_PRISM);

		if (nullptr != pTable)
		{
			if (3==processReturn)
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("사용할수 없습니다."), pTable->szLocaleName);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s이 필요합니다."), pTable->szLocaleName);

		}

		return iExtraLen;
	}
	// @correction054 end

	char chatbuf[CHAT_MAX_LEN + 1];
	int32_t len = snprintf(chatbuf, sizeof(chatbuf), "%s : %s", ch->GetName(), buf);

	if (CHAT_TYPE_SHOUT == pinfo->type)
	{
		LogManager::instance().ShoutLog(g_bChannel, ch->GetEmpire(), chatbuf);
#ifdef __MULTI_LANGUAGE__
		len = snprintf(chatbuf, sizeof(chatbuf), "/language/%s//empire/%s/%s |Hwhisper:%s|h(#)|h|r - Lv.%d|h|r : %s", ch->GetLanguage(), ConvertEmpireIndexToStr(ch->GetEmpire()), ch->GetName(), ch->GetName(), ch->GetLevel(), buf);
#endif
	}

	// @correction005
	if (len < 0 || len >= (int32_t) sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	if (pinfo->type == CHAT_TYPE_SHOUT)
	{

		if (ch->GetLevel() < g_iShoutLimitLevel) // @correction219
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("외치기는 레벨 %d 이상만 사용 가능 합니다."), g_iShoutLimitLevel); // @correction219
			return (iExtraLen);
		}

		if (thecore_heart->pulse - (int32_t) ch->GetLastShoutPulse() < passes_per_sec * g_iShoutLimitTime) // @correction221
			return (iExtraLen);

		ch->SetLastShoutPulse(thecore_heart->pulse);

		TPacketGGShout p;

		p.bHeader = HEADER_GG_SHOUT;
		p.bEmpire = ch->GetEmpire();
		strlcpy(p.szText, chatbuf, sizeof(p.szText));

		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShout));

		SendShout(chatbuf, ch->GetEmpire());

		return (iExtraLen);
	}

	TPacketGCChat pack_chat;

	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(TPacketGCChat) + len;
	pack_chat.type = pinfo->type;
	pack_chat.id = ch->GetVID();

	switch (pinfo->type)
	{
		case CHAT_TYPE_TALKING:
			{
				const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();

				if (false)
				{
					std::for_each(c_ref_set.begin(), c_ref_set.end(),
							FYmirChatPacket(pack_chat,
								buf,
								strlen(buf),
								ch->GetName(),
								strlen(ch->GetName()),
								ch->GetMapIndex(),
								ch->GetEmpire(),
								ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)));
				}
				else
				{
					std::for_each(c_ref_set.begin(), c_ref_set.end(),
							FEmpireChatPacket(pack_chat,
								chatbuf,
								len,
								(ch->GetGMLevel() > GM_PLAYER ||
								 ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)) ? 0 : ch->GetEmpire(),
								ch->GetMapIndex(), strlen(ch->GetName())));
#ifdef ENABLE_CHAT_LOGGING // @correction182
					if (ch->IsGM())
					{
						LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, "", "NORMAL", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
#endif
				}
			}
			break;

		case CHAT_TYPE_PARTY:
			{
				if (!ch->GetParty())
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("파티 중이 아닙니다."));
				else
				{
					TEMP_BUFFER tbuf;

					tbuf.write(&pack_chat, sizeof(pack_chat));
					tbuf.write(chatbuf, len);

					RawPacketToCharacterFunc f(tbuf.read_peek(), tbuf.size());
					ch->GetParty()->ForEachOnlineMember(f);
#ifdef ENABLE_CHAT_LOGGING // @correction182
					if (ch->IsGM())
					{
						LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetParty()->GetLeaderPID(), "", "PARTY", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
#endif
				}
			}
			break;

		case CHAT_TYPE_GUILD:
			{
				if (!ch->GetGuild())
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("길드에 가입하지 않았습니다."));
				else
				{
					ch->GetGuild()->Chat(chatbuf);
#ifdef ENABLE_CHAT_LOGGING // @correction182
					if (ch->IsGM())
					{
						LogManager::instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetGuild()->GetID(), ch->GetGuild()->GetName(), "GUILD", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
#endif
				}
			}
			break;

		default:
			sys_err("Unknown chat type %d", pinfo->type);
			break;
	}

	return (iExtraLen);
}

void CInputMain::ItemUse(LPCHARACTER ch, const char * data)
{
	TPacketCGItemUse * p = (TPacketCGItemUse *) data;
	ch->UseItem(p->Cell, NPOS
#ifdef __FAST_USE_GIFTBOX__
, p->wCount
#endif
);
}

void CInputMain::ItemToItem(LPCHARACTER ch, const char * pcData)
{
	TPacketCGItemUseToItem * p = (TPacketCGItemUseToItem *) pcData;
	if (ch)
		ch->UseItem(p->Cell, p->TargetCell);
}

void CInputMain::ItemDrop(LPCHARACTER ch, const char * data)
{
	struct command_item_drop * pinfo = (struct command_item_drop *) data;

	if (!ch)
		return;

	if (pinfo->gold > 0)
		ch->DropGold(pinfo->gold);
	else
		ch->DropItem(pinfo->Cell);
}

void CInputMain::ItemDrop2(LPCHARACTER ch, const char * data)
{

	TPacketCGItemDrop2 * pinfo = (TPacketCGItemDrop2 *) data;


	if (!ch)
		return;
	if (pinfo->gold > 0)
		ch->DropGold(pinfo->gold);
	else
		ch->DropItem(pinfo->Cell, pinfo->count);
}

// @correction126 BEGIN
void CInputMain::ItemDestroy(LPCHARACTER ch, const char * data)
{
	struct command_item_destroy * pinfo = (struct command_item_destroy *) data;
	if (ch)
		ch->DestroyItem(pinfo->Cell, pinfo->type);
}
// @correction126 END

void CInputMain::ItemMove(LPCHARACTER ch, const char * data)
{
	struct command_item_move * pinfo = (struct command_item_move *) data;

	if (ch)
		ch->MoveItem(pinfo->Cell, pinfo->CellTo, pinfo->count);
}

void CInputMain::ItemPickup(LPCHARACTER ch, const char * data)
{
	struct command_item_pickup * pinfo = (struct command_item_pickup*) data;
	if (ch)
		ch->PickupItem(pinfo->vid);
}

void CInputMain::QuickslotAdd(LPCHARACTER ch, const char * data)
{
	struct command_quickslot_add * pinfo = (struct command_quickslot_add *) data;
	// @correction097 BEGIN
	if (pinfo->slot.type == QUICKSLOT_TYPE_ITEM)
	{
		LPITEM item = nullptr;

		TItemPos srcCell(INVENTORY, pinfo->slot.pos);

		if (!(item = ch->GetItem(srcCell)))
			return;

		if (item->GetType() != ITEM_USE && item->GetType() != ITEM_QUEST && item->GetType() != ITEM_PET)
			return;
	}
	// @correction097 END

	ch->SetQuickslot(pinfo->pos, pinfo->slot);
}

void CInputMain::QuickslotDelete(LPCHARACTER ch, const char * data)
{
	struct command_quickslot_del * pinfo = (struct command_quickslot_del *) data;
	ch->DelQuickslot(pinfo->pos);
}

void CInputMain::QuickslotSwap(LPCHARACTER ch, const char * data)
{
	struct command_quickslot_swap * pinfo = (struct command_quickslot_swap *) data;
	ch->SwapQuickslot(pinfo->pos, pinfo->change_pos);
}

int32_t CInputMain::Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	TPacketCGMessenger* p = (TPacketCGMessenger*) c_pData;

	if (uiBytes < sizeof(TPacketCGMessenger))
		return -1;

	c_pData += sizeof(TPacketCGMessenger);
	uiBytes -= sizeof(TPacketCGMessenger);

	switch (p->subheader)
	{
		case MESSENGER_SUBHEADER_CG_ADD_BY_VID:
			{
				if (uiBytes < sizeof(TPacketCGMessengerAddByVID))
					return -1;

				TPacketCGMessengerAddByVID * p2 = (TPacketCGMessengerAddByVID *) c_pData;
				LPCHARACTER ch_companion = CHARACTER_MANAGER::instance().Find(p2->vid);

				if (!ch_companion)
					return sizeof(TPacketCGMessengerAddByVID);

				if (ch->IsObserverMode())
					return sizeof(TPacketCGMessengerAddByVID);

				if (ch_companion->IsBlockMode(BLOCK_MESSENGER_INVITE))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 메신져 추가 거부 상태입니다."));
					return sizeof(TPacketCGMessengerAddByVID);
				}

				LPDESC d = ch_companion->GetDesc();

				if (!d)
					return sizeof(TPacketCGMessengerAddByVID);

				if (ch->GetGMLevel() == GM_PLAYER && ch_companion->GetGMLevel() != GM_PLAYER)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<메신져> 운영자는 메신져에 추가할 수 없습니다."));
					return sizeof(TPacketCGMessengerAddByVID);
				}

				if (ch->GetDesc() == d)
					return sizeof(TPacketCGMessengerAddByVID);

				MessengerManager::instance().RequestToAdd(ch, ch_companion);
			}
			return sizeof(TPacketCGMessengerAddByVID);

		case MESSENGER_SUBHEADER_CG_ADD_BY_NAME:
			{
				if (uiBytes < CHARACTER_NAME_MAX_LEN)
					return -1;

				char name[CHARACTER_NAME_MAX_LEN + 1];
				strlcpy(name, c_pData, sizeof(name));

				if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(name) != GM_PLAYER)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<메신져> 운영자는 메신져에 추가할 수 없습니다."));
					return CHARACTER_NAME_MAX_LEN;
				}

				LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(name);

				if (!tch)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님은 접속되 있지 않습니다."), name);
				else
				{
					if (tch == ch)
						return CHARACTER_NAME_MAX_LEN;

					if (tch->IsBlockMode(BLOCK_MESSENGER_INVITE) == true)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 메신져 추가 거부 상태입니다."));
					}
					else
					{
						MessengerManager::instance().RequestToAdd(ch, tch);
					}
				}
			}
			return CHARACTER_NAME_MAX_LEN;

		case MESSENGER_SUBHEADER_CG_REMOVE:
			{
				if (uiBytes < CHARACTER_NAME_MAX_LEN)
					return -1;

				char char_name[CHARACTER_NAME_MAX_LEN + 1];
				strlcpy(char_name, c_pData, sizeof(char_name));
				MessengerManager::instance().RemoveFromList(ch->GetName(), char_name);
				MessengerManager::instance().RemoveFromList(char_name, ch->GetName()); // @correction024
			}
			return CHARACTER_NAME_MAX_LEN;

		default:
			sys_err("CInputMain::Messenger : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

int32_t CInputMain::Shop(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	TPacketCGShop * p = (TPacketCGShop *) data;

	if (uiBytes < sizeof(TPacketCGShop))
		return -1;

	if (test_server)
		sys_log(0, "CInputMain::Shop() ==> SubHeader %d", p->subheader);

	const char * c_pData = data + sizeof(TPacketCGShop);
	uiBytes -= sizeof(TPacketCGShop);

	switch (p->subheader)
	{
		case SHOP_SUBHEADER_CG_END:
			sys_log(1, "INPUT: %s SHOP: END", ch->GetName());
			CShopManager::instance().StopShopping(ch);
			return 0;

		case SHOP_SUBHEADER_CG_BUY:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint8_t))
					return -1;

				uint8_t bPos = *(c_pData + 1);
				sys_log(1, "INPUT: %s SHOP: BUY %d", ch->GetName(), bPos);
				CShopManager::instance().Buy(ch, bPos);
				return (sizeof(uint8_t) + sizeof(uint8_t));
			}

		case SHOP_SUBHEADER_CG_SELL:
			{
				if (uiBytes < sizeof(uint8_t))
					return -1;

				uint8_t pos = *c_pData;

				sys_log(0, "INPUT: %s SHOP: SELL", ch->GetName());
				CShopManager::instance().Sell(ch, pos);
				return sizeof(uint8_t);
			}

		case SHOP_SUBHEADER_CG_SELL2:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint16_t))
					return -1;

				uint8_t pos = *(c_pData++);
				uint16_t count = *(c_pData);

				sys_log(0, "INPUT: %s SHOP: SELL2", ch->GetName());
				CShopManager::instance().Sell(ch, pos, count);
				return sizeof(uint8_t) + sizeof(uint16_t);
			}

		default:
			sys_err("CInputMain::Shop : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

void CInputMain::OnClick(LPCHARACTER ch, const char * data)
{
	struct command_on_click *	pinfo = (struct command_on_click *) data;
	LPCHARACTER			victim;

	if ((victim = CHARACTER_MANAGER::instance().Find(pinfo->vid)))
		victim->OnClick(ch);
#ifdef __OFFLINE_PRIVATE_SHOP__
	else
		CShopManager::instance().StartOfflinePrivateShop(ch, pinfo->vid);
#endif
}

void CInputMain::Exchange(LPCHARACTER ch, const char * data)
{
	struct command_exchange * pinfo = (struct command_exchange *) data;
	LPCHARACTER	to_ch = nullptr;

	if (!ch->CanHandleItem())
		return;

	int32_t iPulse = thecore_pulse();

	if ((to_ch = CHARACTER_MANAGER::instance().Find(pinfo->arg1)))
	{
		if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			to_ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래 후 %d초 이내에 창고를 열수 없습니다."), g_nPortalLimitTime);
			return;
		}

		if( true == to_ch->IsDead() )
		{
			return;
		}
	}

	sys_log(0, "CInputMain()::Exchange()  SubHeader %d ", pinfo->sub_header);

	if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래 후 %d초 이내에 창고를 열수 없습니다."), g_nPortalLimitTime);
		return;
	}


	switch (pinfo->sub_header)
	{
		case EXCHANGE_SUBHEADER_CG_START:
			if (!ch->GetExchange())
			{
				if ((to_ch = CHARACTER_MANAGER::instance().Find(pinfo->arg1)))
				{
					if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("창고를 연후 %d초 이내에는 거래를 할수 없습니다."), g_nPortalLimitTime);

						if (test_server)
							ch->ChatPacket(CHAT_TYPE_INFO, "[TestOnly][Safebox]Pulse %d LoadTime %d PASS %d", iPulse, ch->GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
						return;
					}

					if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
					{
						to_ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("창고를 연후 %d초 이내에는 거래를 할수 없습니다."), g_nPortalLimitTime);


						if (test_server)
							to_ch->ChatPacket(CHAT_TYPE_INFO, "[TestOnly][Safebox]Pulse %d LoadTime %d PASS %d", iPulse, to_ch->GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
						return;
					}

					if (ch->GetGold() >= GOLD_MAX)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("액수가 20억 냥을 초과하여 거래를 할수가 없습니다.."));

						sys_err("[OVERFLOG_GOLD] START (%llu) id %u name %s ", ch->GetGold(), ch->GetPlayerID(), ch->GetName());
						return;
					}

					if (to_ch->IsPC())
					{
						if (quest::CQuestManager::instance().GiveItemToPC(ch->GetPlayerID(), to_ch))
						{
							sys_log(0, "Exchange canceled by quest %s %s", ch->GetName(), to_ch->GetName());
							return;
						}
					}


					if (ch->GetMyShop() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중일경우 개인상점을 열수가 없습니다."));
						return;
					}

					ch->ExchangeStart(to_ch);
				}
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ITEM_ADD:
			if (quest::CQuestManager::instance().GetEventFlag("EXCHANGE_ITEM_ADD") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				break;
			}

			if (ch->GetExchange())
			{
				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->AddItem(pinfo->Pos, pinfo->arg2);
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ITEM_DEL:
			if (quest::CQuestManager::instance().GetEventFlag("EXCHANGE_ITEM_DEL") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				break;
			}
			if (ch->GetExchange())
			{
				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->RemoveItem(pinfo->arg1);
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ELK_ADD:
			if (quest::CQuestManager::instance().GetEventFlag("EXCHANGE_ELK_ADD") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				break;
			}
			if (ch->GetExchange())
			{
				const uint64_t nTotalGold = ch->GetExchange()->GetCompany()->GetOwner()->GetGold() + pinfo->arg1;

				if (GOLD_MAX <= nTotalGold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 총금액이 20억 냥을 초과하여 거래를 할수가 없습니다.."));

					sys_err("[OVERFLOW_GOLD] ELK_ADD (%llu) id %u name %s ",
							ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
							ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
						   	ch->GetExchange()->GetCompany()->GetOwner()->GetName());

					return;
				}

				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->AddGold(pinfo->arg1);
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ACCEPT:
			if (ch->GetExchange())
			{
				sys_log(0, "CInputMain()::Exchange() ==> ACCEPT ");
				ch->GetExchange()->Accept(true);
			}

			break;

		case EXCHANGE_SUBHEADER_CG_CANCEL:
			if (ch->GetExchange())
				ch->GetExchange()->Cancel();
			break;
	}
}

void CInputMain::Position(LPCHARACTER ch, const char * data)
{
	struct command_position * pinfo = (struct command_position *) data;

	switch (pinfo->position)
	{
		case POSITION_GENERAL:
			ch->Standup();
			break;

		case POSITION_SITTING_CHAIR:
			ch->Sitdown(0);
			break;

		case POSITION_SITTING_GROUND:
			ch->Sitdown(1);
			break;
	}
}

static const int32_t ComboSequenceBySkillLevel[3][8] =
{
	{ 14, 15, 16, 17,  0,  0,  0,  0 },
	{ 14, 15, 16, 18, 20,  0,  0,  0 },
	{ 14, 15, 16, 18, 19, 17,  0,  0 },
};

#define COMBO_HACK_ALLOWABLE_MS	100

bool CheckComboHack(LPCHARACTER ch, uint8_t bArg, uint32_t dwTime, bool CheckSpeedHack)
{
	if (!gHackCheckEnable) // @correction120
		return false;

	if (ch->IsStun() || ch->IsDead())
		return false;

	int32_t ComboInterval = dwTime - ch->GetLastComboTime();
	int32_t HackScalar = 0;
#if 0
	sys_log(0, "COMBO: %s arg:%u seq:%u delta:%d checkspeedhack:%d",
			ch->GetName(), bArg, ch->GetComboSequence(), ComboInterval - ch->GetValidComboInterval(), CheckSpeedHack);
#endif
	if (bArg == 14)
	{
		ch->SetComboSequence(1);
		ch->SetValidComboInterval((int32_t) (ani_combo_speed(ch, 1) / (ch->GetPoint(POINT_ATT_SPEED) / 100.f)));
		ch->SetLastComboTime(dwTime);
	}
	else if (bArg > 14 && bArg < 22)
	{
		int32_t idx = MIN(2, ch->GetComboIndex());

		if (ch->GetComboSequence() > 5)
		{
			HackScalar = 1;
			ch->SetValidComboInterval(300);
			sys_log(0, "COMBO_HACK: 5 %s combo_seq:%d", ch->GetName(), ch->GetComboSequence());
		}
		else if (bArg == 21 &&
				 idx == 2 &&
				 ch->GetComboSequence() == 5 &&
				 ch->GetJob() == JOB_ASSASSIN &&
				 ch->GetWear(WEAR_WEAPON) &&
				 ch->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
			ch->SetValidComboInterval(300);
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		else if (bArg == 21 && idx == 2 && ch->GetComboSequence() == 5 && ch->GetJob() == JOB_WOLFMAN && ch->GetWear(WEAR_WEAPON) && ch->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_CLAW)
			ch->SetValidComboInterval(300);
#endif
		else if (ComboSequenceBySkillLevel[idx][ch->GetComboSequence()] != bArg)
		{
			HackScalar = 1;
			ch->SetValidComboInterval(300);

			sys_log(0, "COMBO_HACK: 3 %s arg:%u valid:%u combo_idx:%d combo_seq:%d",
					ch->GetName(),
					bArg,
					ComboSequenceBySkillLevel[idx][ch->GetComboSequence()],
					idx,
					ch->GetComboSequence());
		}
		else
		{
			if (CheckSpeedHack && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
			{
				HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 100;

				sys_log(0, "COMBO_HACK: 2 %s arg:%u interval:%d valid:%u atkspd:%u riding:%s",
						ch->GetName(),
						bArg,
						ComboInterval,
						ch->GetValidComboInterval(),
						ch->GetPoint(POINT_ATT_SPEED),
						ch->IsRiding() ? "yes" : "no");
			}

			if (ch->IsRiding())
				ch->SetComboSequence(ch->GetComboSequence() == 1 ? 2 : 1);
			else
				ch->SetComboSequence(ch->GetComboSequence() + 1);

			ch->SetValidComboInterval((int32_t) (ani_combo_speed(ch, bArg - 13) / (ch->GetPoint(POINT_ATT_SPEED) / 100.f)));
			ch->SetLastComboTime(dwTime);
		}
	}
	else if (bArg == 13)
	{
		if (ch->GetRaceNum() >= MAIN_RACE_MAX_NUM)
		{

			float normalAttackDuration = CMotionManager::instance().GetNormalAttackDuration(ch->GetRaceNum());
			int32_t k = (int32_t) (normalAttackDuration / ((float) ch->GetPoint(POINT_ATT_SPEED) / 100.f) * 900.f);
			ch->SetValidComboInterval(k);
			ch->SetLastComboTime(dwTime);
		}
	}
	else
	{
		if (ch->GetDesc()->DelayedDisconnect(number(2, 9)))
		{
			LogManager::instance().HackLog("Hacker", ch);
			sys_log(0, "HACKER: %s arg %u", ch->GetName(), bArg);
		}

		HackScalar = 10;
		ch->SetValidComboInterval(300);
	}

	if (HackScalar)
	{
		if (get_dword_time() - ch->GetLastMountTime() > 1500)
			ch->IncreaseComboHackCount(1 + HackScalar);

		ch->SkipComboAttackByTime(ch->GetValidComboInterval());
	}

	return HackScalar;
}

void CInputMain::Move(LPCHARACTER ch, const char * data)
{
	if (!ch->CanMove())
		return;

	struct command_move * pinfo = (struct command_move *) data;

	if (pinfo->bFunc >= FUNC_MAX_NUM && !(pinfo->bFunc & 0x80))
	{
		sys_err("invalid move type: %s", ch->GetName());
		return;
	}



	{
		const float fDist = DISTANCE_SQRT((ch->GetX() - pinfo->lX) / 100, (ch->GetY() - pinfo->lY) / 100);
		// @fixme106 (changed 40 to 60)
		if (((false == ch->IsRiding() && fDist > 25) || fDist > 40) && OXEVENT_MAP_INDEX != ch->GetMapIndex())
		{
#ifdef ENABLE_HACK_TELEPORT_LOG // @correction005
			{
				const PIXEL_POSITION & warpPos = ch->GetWarpPosition();

				if (warpPos.x == 0 && warpPos.y == 0)
					LogManager::instance().HackLog("Teleport", ch);
			}
#endif
			sys_log(0, "MOVE: %s trying to move too far (dist: %.1fm) Riding(%d)", ch->GetName(), fDist, ch->IsRiding());

			ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
			ch->Stop();
			return;
		}
#ifdef ENABLE_CHECK_GHOSTMODE // @correction185
		if (ch->IsPC() && ch->IsDead())
		{
			sys_log(0, "MOVE: %s trying to move as dead", ch->GetName());

			ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
			ch->Stop();
			return;
		}
#endif
		uint32_t dwCurTime = get_dword_time();
		bool CheckSpeedHack = (false == ch->GetDesc()->IsHandshaking() && dwCurTime - ch->GetDesc()->GetClientTime() > 7000);

		if (CheckSpeedHack)
		{
			int32_t iDelta = (int32_t) (pinfo->dwTime - ch->GetDesc()->GetClientTime());
			int32_t iServerDelta = (int32_t) (dwCurTime - ch->GetDesc()->GetClientTime());

			iDelta = (int32_t) (dwCurTime - pinfo->dwTime);

			if (iDelta >= 30000)
			{
				sys_log(0, "SPEEDHACK: slow timer name %s delta %d", ch->GetName(), iDelta);
				ch->GetDesc()->DelayedDisconnect(3);
			}
			else if (iDelta < -(iServerDelta / 50))
			{
				sys_log(0, "SPEEDHACK: DETECTED! %s (delta %d %d)", ch->GetName(), iDelta, iServerDelta);
				ch->GetDesc()->DelayedDisconnect(3);
			}
		}

		if (pinfo->bFunc == FUNC_COMBO && g_bCheckMultiHack)
		{
			CheckComboHack(ch, pinfo->bArg, pinfo->dwTime, CheckSpeedHack);
		}
	}

	if (pinfo->bFunc == FUNC_MOVE)
	{
		if (ch->GetLimitPoint(POINT_MOV_SPEED) == 0)
			return;

		ch->SetRotation(pinfo->bRot * 5);
		ch->ResetStopTime();

		ch->Goto(pinfo->lX, pinfo->lY);
	}
	else
	{
		if (pinfo->bFunc == FUNC_ATTACK || pinfo->bFunc == FUNC_COMBO)
			ch->OnMove(true);
		else if (pinfo->bFunc & FUNC_SKILL)
		{
			const int32_t MASK_SKILL_MOTION = 0x7F;
			uint32_t motion = pinfo->bFunc & MASK_SKILL_MOTION;

			if (!ch->IsUsableSkillMotion(motion))
			{
				const char* name = ch->GetName();
				uint32_t job = ch->GetJob();
				uint32_t group = ch->GetSkillGroup();

				char szBuf[256];
				snprintf(szBuf, sizeof(szBuf), "SKILL_HACK: name=%s, job=%d, group=%d, motion=%d", name, job, group, motion);
				LogManager::instance().HackLog(szBuf, ch->GetDesc()->GetAccountTable().login, ch->GetName(), ch->GetDesc()->GetHostName());
				sys_log(0, "%s", szBuf);

				if (test_server)
				{
					ch->GetDesc()->DelayedDisconnect(number(2, 8));
					ch->ChatPacket(CHAT_TYPE_INFO, szBuf);
				}
				else
				{
					ch->GetDesc()->DelayedDisconnect(number(150, 500));
				}
			}

			ch->OnMove();
		}

		ch->SetRotation(pinfo->bRot * 5);
		ch->ResetStopTime();

		ch->Move(pinfo->lX, pinfo->lY);
		ch->Stop();
		ch->StopStaminaConsume();
	}

	TPacketGCMove pack;

	pack.bHeader      = HEADER_GC_MOVE;
	pack.bFunc        = pinfo->bFunc;
	pack.bArg         = pinfo->bArg;
	pack.bRot         = pinfo->bRot;
	pack.dwVID        = ch->GetVID();
	pack.lX           = pinfo->lX;
	pack.lY           = pinfo->lY;
	pack.dwTime       = pinfo->dwTime;
	pack.dwDuration   = (pinfo->bFunc == FUNC_MOVE) ? ch->GetCurrentMoveDuration() : 0;

	ch->PacketAround(&pack, sizeof(TPacketGCMove), ch);
}

void CInputMain::Attack(LPCHARACTER ch, const uint8_t header, const char* data)
{
	if (nullptr == ch)
		return;

	struct type_identifier
	{
		uint8_t header;
		uint8_t type;
	};

	const struct type_identifier* const type = reinterpret_cast<const struct type_identifier*>(data);

	if (type->type > 0)
	{
		if (false == ch->CanUseSkill(type->type))
		{
			return;
		}

		switch (type->type)
		{
			case SKILL_GEOMPUNG:
			case SKILL_SANGONG:
			case SKILL_YEONSA:
			case SKILL_KWANKYEOK:
			case SKILL_HWAJO:
			case SKILL_GIGUNG:
			case SKILL_PABEOB:
			case SKILL_MARYUNG:
			case SKILL_TUSOK:
			case SKILL_MAHWAN:
			case SKILL_BIPABU:
			case SKILL_NOEJEON:
			case SKILL_CHAIN:
			case SKILL_HORSE_WILDATTACK_RANGE:
				if (HEADER_CG_SHOOT != type->header)
				{
					if (test_server)
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Attack :name[%s] Vnum[%d] can't use skill by attack(warning)"), type->type);
					return;
				}
				break;
		}
	}

	switch (header)
	{
		case HEADER_CG_ATTACK:
			{
				if (nullptr == ch->GetDesc())
					return;

				const TPacketCGAttack* const packMelee = reinterpret_cast<const TPacketCGAttack*>(data);

				ch->GetDesc()->AssembleCRCMagicCube(packMelee->bCRCMagicCubeProcPiece, packMelee->bCRCMagicCubeFilePiece);

				LPCHARACTER	victim = CHARACTER_MANAGER::instance().Find(packMelee->dwVID);

				if (nullptr == victim || ch == victim)
					return;

				if (packMelee->dwCrc != 0x63B0BD88)
					return;

				switch (victim->GetCharType())
				{
					case CHAR_TYPE_NPC:
					case CHAR_TYPE_WARP:
					case CHAR_TYPE_GOTO:
						return;
				}

				if (packMelee->bType > 0)
				{
					if (false == ch->CheckSkillHitCount(packMelee->bType, victim->GetVID()))
					{
						return;
					}
				}

				ch->Attack(victim, packMelee->bType);
			}
			break;

		case HEADER_CG_SHOOT:
			{
				const TPacketCGShoot* const packShoot = reinterpret_cast<const TPacketCGShoot*>(data);

				ch->Shoot(packShoot->bType);
			}
			break;
	}
}

int32_t CInputMain::SyncPosition(LPCHARACTER ch, const char * c_pcData, size_t uiBytes)
{
	const TPacketCGSyncPosition* pinfo = reinterpret_cast<const TPacketCGSyncPosition*>( c_pcData );

	if (uiBytes < pinfo->wSize)
		return -1;

	int32_t iExtraLen = pinfo->wSize - sizeof(TPacketCGSyncPosition);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	if (0 != (iExtraLen % sizeof(TPacketCGSyncPositionElement)))
	{
		sys_err("invalid packet length %d (name: %s)", pinfo->wSize, ch->GetName());
		return iExtraLen;
	}

	int32_t iCount = iExtraLen / sizeof(TPacketCGSyncPositionElement);

	if (iCount <= 0)
		return iExtraLen;

	static const int32_t nCountLimit = 16;

	if( iCount > nCountLimit )
	{
		sys_err( "Too many SyncPosition Count(%d) from Name(%s)", iCount, ch->GetName() );
		iCount = nCountLimit;
	}

	TEMP_BUFFER tbuf;
	LPBUFFER lpBuf = tbuf.getptr();

	TPacketGCSyncPosition * pHeader = (TPacketGCSyncPosition *) buffer_write_peek(lpBuf);
	buffer_write_proceed(lpBuf, sizeof(TPacketGCSyncPosition));

	const TPacketCGSyncPositionElement* e =
		reinterpret_cast<const TPacketCGSyncPositionElement*>(c_pcData + sizeof(TPacketCGSyncPosition));

	timeval tvCurTime;
	gettimeofday(&tvCurTime, nullptr);

	for (int32_t i = 0; i < iCount; ++i, ++e)
	{
		LPCHARACTER victim = CHARACTER_MANAGER::instance().Find(e->dwVID);

		if (!victim)
			continue;

		switch (victim->GetCharType())
		{
			case CHAR_TYPE_NPC:
			case CHAR_TYPE_WARP:
			case CHAR_TYPE_GOTO:
				continue;
		}

		if (!victim->SetSyncOwner(ch))
			continue;

		const float fDistWithSyncOwner = DISTANCE_SQRT( (victim->GetX() - ch->GetX()) / 100, (victim->GetY() - ch->GetY()) / 100 );
		static const float fLimitDistWithSyncOwner = 2500.f + 1000.f;
		if (fDistWithSyncOwner > fLimitDistWithSyncOwner)
		{
			if (ch->GetSyncHackCount() < g_iSyncHackLimitCount)
			{
				ch->SetSyncHackCount(ch->GetSyncHackCount() + 1);
				continue;
			}
			else
			{
				LogManager::instance().HackLog( "SYNC_POSITION_HACK", ch );

				sys_err( "Too far SyncPosition DistanceWithSyncOwner(%f)(%s) from Name(%s) CH(%d,%d) VICTIM(%d,%d) SYNC(%d,%d)",
					fDistWithSyncOwner, victim->GetName(), ch->GetName(), ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY(),
					e->lX, e->lY );

				ch->GetDesc()->SetPhase(PHASE_CLOSE);

				return -1;
			}
		}

		const float fDist = DISTANCE_SQRT( (victim->GetX() - e->lX) / 100, (victim->GetY() - e->lY) / 100 );
		static const int32_t g_lValidSyncInterval = 100 * 1000;
		const timeval &tvLastSyncTime = victim->GetLastSyncTime();
		timeval *tvDiff = timediff(&tvCurTime, &tvLastSyncTime);

		if (tvDiff->tv_sec == 0 && tvDiff->tv_usec < g_lValidSyncInterval)
		{
			if (ch->GetSyncHackCount() < g_iSyncHackLimitCount)
			{
				ch->SetSyncHackCount(ch->GetSyncHackCount() + 1);
				continue;
			}
			else
			{
				LogManager::instance().HackLog( "SYNC_POSITION_HACK", ch );

				sys_err( "Too often SyncPosition Interval(%ldms)(%s) from Name(%s) VICTIM(%d,%d) SYNC(%d,%d)",
					tvDiff->tv_sec * 1000 + tvDiff->tv_usec / 1000, victim->GetName(), ch->GetName(), victim->GetX(), victim->GetY(),
					e->lX, e->lY );

				ch->GetDesc()->SetPhase(PHASE_CLOSE);

				return -1;
			}
		}
		else if( fDist > 25.0f )
		{
			LogManager::instance().HackLog( "SYNC_POSITION_HACK", ch );

			sys_err( "Too far SyncPosition Distance(%f)(%s) from Name(%s) CH(%d,%d) VICTIM(%d,%d) SYNC(%d,%d)",
				   	fDist, victim->GetName(), ch->GetName(), ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY(),
				  e->lX, e->lY );

			ch->GetDesc()->SetPhase(PHASE_CLOSE);

			return -1;
		}
		else
		{
			victim->SetLastSyncTime(tvCurTime);
			victim->Sync(e->lX, e->lY);
			buffer_write(lpBuf, e, sizeof(TPacketCGSyncPositionElement));
		}
	}

	if (buffer_size(lpBuf) != sizeof(TPacketGCSyncPosition))
	{
		pHeader->bHeader = HEADER_GC_SYNC_POSITION;
		pHeader->wSize = buffer_size(lpBuf);

		ch->PacketAround(buffer_read_peek(lpBuf), buffer_size(lpBuf), ch);
	}

	return iExtraLen;
}

void CInputMain::FlyTarget(LPCHARACTER ch, const char * pcData, uint8_t bHeader)
{
	TPacketCGFlyTargeting * p = (TPacketCGFlyTargeting *) pcData;
	ch->FlyTarget(p->dwTargetVID, p->x, p->y, bHeader);
}

void CInputMain::UseSkill(LPCHARACTER ch, const char * pcData)
{
	TPacketCGUseSkill * p = (TPacketCGUseSkill *) pcData;
	ch->UseSkill(p->dwVnum, CHARACTER_MANAGER::instance().Find(p->dwVID));
}

void CInputMain::ScriptButton(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGScriptButton * p = (TPacketCGScriptButton *) c_pData;
	sys_log(0, "QUEST ScriptButton pid %d idx %u", ch->GetPlayerID(), p->idx);

	quest::PC* pc = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
	if (pc && pc->IsConfirmWait())
	{
		quest::CQuestManager::instance().Confirm(ch->GetPlayerID(), quest::CONFIRM_TIMEOUT);
	}
	else if (p->idx & 0x80000000)
	{
		quest::CQuestManager::Instance().QuestInfo(ch->GetPlayerID(), p->idx & 0x7fffffff);
	}
	else
	{
		quest::CQuestManager::Instance().QuestButton(ch->GetPlayerID(), p->idx);
	}
}

void CInputMain::ScriptAnswer(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGScriptAnswer * p = (TPacketCGScriptAnswer *) c_pData;
	sys_log(0, "QUEST ScriptAnswer pid %d answer %d", ch->GetPlayerID(), p->answer);

	if (p->answer > 250)
	{
		quest::CQuestManager::Instance().Resume(ch->GetPlayerID());
	}
	else
	{
		quest::CQuestManager::Instance().Select(ch->GetPlayerID(),  p->answer);
	}
}


void CInputMain::ScriptSelectItem(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGScriptSelectItem* p = (TPacketCGScriptSelectItem*) c_pData;
	sys_log(0, "QUEST ScriptSelectItem pid %d answer %d", ch->GetPlayerID(), p->selection);
	quest::CQuestManager::Instance().SelectItem(ch->GetPlayerID(), p->selection);
}

void CInputMain::QuestInputString(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGQuestInputString * p = (TPacketCGQuestInputString*) c_pData;

	char msg[65];
	strlcpy(msg, p->msg, sizeof(msg));
	sys_log(0, "QUEST InputString pid %u msg %s", ch->GetPlayerID(), msg);

	quest::CQuestManager::Instance().Input(ch->GetPlayerID(), msg);
}

void CInputMain::QuestConfirm(LPCHARACTER ch, const void* c_pData)
{
	TPacketCGQuestConfirm* p = (TPacketCGQuestConfirm*) c_pData;
	LPCHARACTER ch_wait = CHARACTER_MANAGER::instance().FindByPID(p->requestPID);
	if (p->answer)
		p->answer = quest::CONFIRM_YES;
	sys_log(0, "QuestConfirm from %s pid %u name %s answer %d", ch->GetName(), p->requestPID, (ch_wait)?ch_wait->GetName():"", p->answer);
	if (ch_wait)
	{
		quest::CQuestManager::Instance().Confirm(ch_wait->GetPlayerID(), (quest::EQuestConfirmType) p->answer, ch->GetPlayerID());
	}
}

void CInputMain::Target(LPCHARACTER ch, const char * pcData)
{
	TPacketCGTarget * p = (TPacketCGTarget *) pcData;

	building::LPOBJECT pkObj = building::CManager::instance().FindObjectByVID(p->dwVID);

	if (pkObj)
	{
		TPacketGCTarget pckTarget;
		pckTarget.header = HEADER_GC_TARGET;
		pckTarget.dwVID = p->dwVID;
		ch->GetDesc()->Packet(&pckTarget, sizeof(TPacketGCTarget));
	}
	else
		ch->SetTarget(CHARACTER_MANAGER::instance().Find(p->dwVID));
}

void CInputMain::Warp(LPCHARACTER ch, const char * pcData)
{
	ch->WarpEnd();
}

void CInputMain::SafeboxCheckin(LPCHARACTER ch, const char * c_pData)
{
	if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		return;

	TPacketCGSafeboxCheckin * p = (TPacketCGSafeboxCheckin *) c_pData;

	if (!ch->CanHandleItem())
		return;

	CSafebox * pkSafebox = ch->GetSafebox();
	LPITEM pkItem = ch->GetItem(p->ItemPos);

	if (!pkSafebox || !pkItem)
		return;

	if (pkItem->IsEquipped()) // @correction086
		return;

	if (pkItem->GetCell() >= INVENTORY_MAX_NUM && IS_SET(pkItem->GetFlag(), ITEM_FLAG_IRREMOVABLE))
	{
	    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고로 옮길 수 없는 아이템 입니다."));
	    return;
	}

	if (!pkSafebox->IsEmpty(p->bSafePos, pkItem->GetSize()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
		return;
	}

	if (pkItem->GetVnum() == UNIQUE_ITEM_SAFEBOX_EXPAND)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 이 아이템은 넣을 수 없습니다."));
		return;
	}

	if( IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SAFEBOX) )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 이 아이템은 넣을 수 없습니다."));
		return;
	}

	if (true == pkItem->isLocked())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 이 아이템은 넣을 수 없습니다."));
		return;
	}

	// @correction060 BEGIN
	if (ITEM_BELT == pkItem->GetType() && CBeltInventoryHelper::IsExistItemInBeltInventory(ch))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("벨트 인벤토리에 아이템이 존재하면 해제할 수 없습니다."));
		return;
	}
	// @correction060 END

	pkItem->RemoveFromCharacter();
	if (!pkItem->IsDragonSoul())
		ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, p->ItemPos.cell, 255);
	pkSafebox->Add(p->bSafePos, pkItem);

	char szHint[128];
	snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(), pkItem->GetCount());
	LogManager::instance().ItemLog(ch, pkItem, "SAFEBOX PUT", szHint);
}

void CInputMain::SafeboxCheckout(LPCHARACTER ch, const char * c_pData, bool bMall)
{
	TPacketCGSafeboxCheckout * p = (TPacketCGSafeboxCheckout *) c_pData;

	if (!ch->CanHandleItem())
		return;

	CSafebox * pkSafebox;

	if (bMall)
		pkSafebox = ch->GetMall();
	else
		pkSafebox = ch->GetSafebox();

	if (!pkSafebox)
		return;

	LPITEM pkItem = pkSafebox->Get(p->bSafePos);

	if (!pkItem)
		return;

	if (!ch->IsEmptyItemGrid(p->ItemPos, pkItem))
		return;

	if (pkItem->IsDragonSoul())
	{
		if (bMall)
		{
			DSManager::instance().DragonSoulItemInitialize(pkItem);
		}

		if (DRAGON_SOUL_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
			return;
		}

		TItemPos DestPos = p->ItemPos;
		if (!DSManager::instance().IsValidCellForThisItem(pkItem, DestPos))
		{
			int32_t iCell = ch->GetEmptyDragonSoulInventory(pkItem);
			if (iCell < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
				return ;
			}
			DestPos = TItemPos (DRAGON_SOUL_INVENTORY, iCell);
		}

		pkSafebox->Remove(p->bSafePos);
		pkItem->AddToCharacter(ch, DestPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}
	else
	{
		if (DRAGON_SOUL_INVENTORY == p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
			return;
		}
		// @correction042
		if (p->ItemPos.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(pkItem))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이 아이템은 벨트 인벤토리로 옮길 수 없습니다."));
			return;
		}

		pkSafebox->Remove(p->bSafePos);
		pkItem->AddToCharacter(ch, p->ItemPos);
		ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
	}

	uint32_t dwID = pkItem->GetID();
	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_FLUSH, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&dwID, sizeof(uint32_t));

	char szHint[128];
	snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(), pkItem->GetCount());
	if (bMall)
		LogManager::instance().ItemLog(ch, pkItem, "MALL GET", szHint);
	else
		LogManager::instance().ItemLog(ch, pkItem, "SAFEBOX GET", szHint);
}

void CInputMain::SafeboxItemMove(LPCHARACTER ch, const char * data)
{
	struct command_item_move * pinfo = (struct command_item_move *) data;

	if (!ch->CanHandleItem())
		return;

	if (!ch->GetSafebox())
		return;

	ch->GetSafebox()->MoveItem(pinfo->Cell.cell, pinfo->CellTo.cell, pinfo->count);
}

void CInputMain::PartyInvite(LPCHARACTER ch, const char * c_pData)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	TPacketCGPartyInvite * p = (TPacketCGPartyInvite*) c_pData;

	LPCHARACTER pInvitee = CHARACTER_MANAGER::instance().Find(p->vid);

	if (!pInvitee || !ch->GetDesc() || !pInvitee->GetDesc())
	{
		sys_err("PARTY Cannot find invited character");
		return;
	}

	ch->PartyInvite(pInvitee);
}

void CInputMain::PartyInviteAnswer(LPCHARACTER ch, const char * c_pData)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	TPacketCGPartyInviteAnswer * p = (TPacketCGPartyInviteAnswer*) c_pData;

	LPCHARACTER pInviter = CHARACTER_MANAGER::instance().Find(p->leader_vid);


	if (!pInviter)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티요청을 한 캐릭터를 찾을수 없습니다."));
	else if (!p->accept)
		pInviter->PartyInviteDeny(ch->GetPlayerID());
	else
		pInviter->PartyInviteAccept(ch);
}

void CInputMain::PartySetState(LPCHARACTER ch, const char* c_pData)
{
	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
		return;
	}

	TPacketCGPartySetState* p = (TPacketCGPartySetState*) c_pData;

	if (!ch->GetParty())
		return;

	if (ch->GetParty()->GetLeaderPID() != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 리더만 변경할 수 있습니다."));
		return;
	}

	if (!ch->GetParty()->IsMember(p->pid))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 상태를 변경하려는 사람이 파티원이 아닙니다."));
		return;
	}

	uint32_t pid = p->pid;
	sys_log(0, "PARTY SetRole pid %d to role %d state %s", pid, p->byRole, p->flag ? "on" : "off");

	switch (p->byRole)
	{
		case PARTY_ROLE_NORMAL:
			break;

		case PARTY_ROLE_ATTACKER:
		case PARTY_ROLE_TANKER:
		case PARTY_ROLE_BUFFER:
		case PARTY_ROLE_SKILL_MASTER:
		case PARTY_ROLE_HASTE:
		case PARTY_ROLE_DEFENDER:
			if (ch->GetParty()->SetRole(pid, p->byRole, p->flag))
			{
				TPacketPartyStateChange pack;
				pack.dwLeaderPID = ch->GetPlayerID();
				pack.dwPID = p->pid;
				pack.bRole = p->byRole;
				pack.bFlag = p->flag;
				db_clientdesc->DBPacket(HEADER_GD_PARTY_STATE_CHANGE, 0, &pack, sizeof(pack));
			}
			break;

		default:
			sys_err("wrong byRole in PartySetState Packet name %s state %d", ch->GetName(), p->byRole);
			break;
	}
}

void CInputMain::PartyRemove(LPCHARACTER ch, const char* c_pData)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티에서 추방할 수 없습니다."));
		return;
	}

	TPacketCGPartyRemove* p = (TPacketCGPartyRemove*) c_pData;

	if (!ch->GetParty())
		return;

	LPPARTY pParty = ch->GetParty();
	if (pParty->GetLeaderPID() == ch->GetPlayerID())
	{
		if (ch->GetDungeon())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던젼내에서는 파티원을 추방할 수 없습니다."));
		}
		else
		{
			if(pParty->IsPartyInDungeon(351))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티>던전 안에 파티원이 있어 파티를 해산 할 수 없습니다."));
				return;
			}

			if (p->pid == ch->GetPlayerID() || pParty->GetMemberCount() == 2)
			{
				CPartyManager::instance().DeleteParty(pParty);
			}
			else
			{
				LPCHARACTER B = CHARACTER_MANAGER::instance().FindByPID(p->pid);
				if (B)
				{
					B->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 추방당하셨습니다."));
				}
				pParty->Quit(p->pid);
			}
		}
	}
	else
	{
		if (p->pid == ch->GetPlayerID())
		{
			if (ch->GetDungeon())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던젼내에서는 파티를 나갈 수 없습니다."));
			}
			else
			{
				if (pParty->GetMemberCount() == 2)
				{
					CPartyManager::instance().DeleteParty(pParty);
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 나가셨습니다."));
					pParty->Quit(ch->GetPlayerID());
				}
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 다른 파티원을 탈퇴시킬 수 없습니다."));
		}
	}
}

void CInputMain::AnswerMakeGuild(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGAnswerMakeGuild* p = (TPacketCGAnswerMakeGuild*) c_pData;

	// @correction092 BEGIN
	if (ch->GetGold() < 200000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Yetersiz yang!");
		return;
	}

	if (ch->GetLevel() < 40)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Yetersiz level!");
		return;
	}
	// @correction092 END

	if (get_global_time() - ch->GetQuestFlag("guild_manage.new_disband_time") <
			CGuildManager::instance().GetDisbandDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 해산한 후 %d일 이내에는 길드를 만들 수 없습니다."),
				quest::CQuestManager::instance().GetEventFlag("guild_disband_delay"));
		return;
	}

	if (get_global_time() - ch->GetQuestFlag("guild_manage.new_withdraw_time") <
			CGuildManager::instance().GetWithdrawDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 탈퇴한 후 %d일 이내에는 길드를 만들 수 없습니다."),
				quest::CQuestManager::instance().GetEventFlag("guild_withdraw_delay"));
		return;
	}

	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::instance();

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, p->guild_name, sizeof(cp.name));

	if (cp.name[0] == 0 || !check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("적합하지 않은 길드 이름 입니다."));
		return;
	}

	uint32_t dwGuildID = gm.CreateGuild(cp);

	if (dwGuildID)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> [%s] 길드가 생성되었습니다."), cp.name);

		// @correction005
		int32_t GuildCreateFee = 200000;

		ch->ChangeGold(false, GuildCreateFee);
		DBManager::instance().SendMoneyLog(MONEY_LOG_GUILD, ch->GetPlayerID(), -GuildCreateFee);

		char Log[128];
		snprintf(Log, sizeof(Log), "GUILD_NAME %s MASTER %s", cp.name, ch->GetName());
		LogManager::instance().CharLog(ch, 0, "MAKE_GUILD", Log);

		ch->RemoveSpecifyItem(GUILD_CREATE_ITEM_VNUM, 1);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드 생성에 실패하였습니다."));
}

void CInputMain::PartyUseSkill(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGPartyUseSkill* p = (TPacketCGPartyUseSkill*) c_pData;
	if (!ch->GetParty())
		return;

	if (ch->GetPlayerID() != ch->GetParty()->GetLeaderPID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티 기술은 파티장만 사용할 수 있습니다."));
		return;
	}

	switch (p->bySkillIndex)
	{
		case PARTY_SKILL_HEAL:
			ch->GetParty()->HealParty();
			break;
		case PARTY_SKILL_WARP:
			{
				LPCHARACTER pch = CHARACTER_MANAGER::instance().Find(p->vid);
				if (pch)
					ch->GetParty()->SummonToLeader(pch->GetPlayerID());
				else
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 소환하려는 대상을 찾을 수 없습니다."));
			}
			break;
	}
}

void CInputMain::PartyParameter(LPCHARACTER ch, const char * c_pData)
{
	TPacketCGPartyParameter * p = (TPacketCGPartyParameter *) c_pData;

	if (ch->GetParty())
		ch->GetParty()->SetParameter(p->bDistributeMode);
}

size_t GetSubPacketSize(const GUILD_SUBHEADER_CG& header)
{
	switch (header)
	{
		case GUILD_SUBHEADER_CG_DEPOSIT_MONEY:				return sizeof(int32_t);
		case GUILD_SUBHEADER_CG_WITHDRAW_MONEY:				return sizeof(int32_t);
		case GUILD_SUBHEADER_CG_ADD_MEMBER:					return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_REMOVE_MEMBER:				return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:			return 10;
		case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:		return sizeof(uint8_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_OFFER:						return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_CHARGE_GSP:					return sizeof(int32_t);
		case GUILD_SUBHEADER_CG_POST_COMMENT:				return 1;
		case GUILD_SUBHEADER_CG_DELETE_COMMENT:				return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_REFRESH_COMMENT:			return 0;
		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:		return sizeof(uint32_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_USE_SKILL:					return sizeof(TPacketCGGuildUseSkill);
		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:		return sizeof(uint32_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:		return sizeof(uint32_t) + sizeof(uint8_t);
	}

	return 0;
}

int32_t CInputMain::Guild(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	if (uiBytes < sizeof(TPacketCGGuild))
		return -1;

	const TPacketCGGuild* p = reinterpret_cast<const TPacketCGGuild*>(data);
	const char* c_pData = data + sizeof(TPacketCGGuild);

	uiBytes -= sizeof(TPacketCGGuild);

	const GUILD_SUBHEADER_CG SubHeader = static_cast<GUILD_SUBHEADER_CG>(p->subheader);
	const size_t SubPacketLen = GetSubPacketSize(SubHeader);

	if (uiBytes < SubPacketLen)
	{
		return -1;
	}

	CGuild* pGuild = ch->GetGuild();

	if (nullptr == pGuild)
	{
		if (SubHeader != GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드에 속해있지 않습니다."));
			return SubPacketLen;
		}
	}

	switch (SubHeader)
	{
		case GUILD_SUBHEADER_CG_DEPOSIT_MONEY:
			{
				return SubPacketLen;

				const int32_t gold = MIN(*reinterpret_cast<const int32_t*>(c_pData), __deposit_limit());

				if (gold < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 잘못된 금액입니다."));
					return SubPacketLen;
				}

				if ((int32_t)ch->GetGold() < gold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 가지고 있는 돈이 부족합니다."));
					return SubPacketLen;
				}

				pGuild->RequestDepositMoney(ch, gold);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_WITHDRAW_MONEY:
			{
				return SubPacketLen;

				const int32_t gold = MIN(*reinterpret_cast<const int32_t*>(c_pData), 500000);

				if (gold < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 잘못된 금액입니다."));
					return SubPacketLen;
				}

				pGuild->RequestWithdrawMoney(ch, gold);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_ADD_MEMBER:
			{
				const uint32_t vid = *reinterpret_cast<const uint32_t*>(c_pData);
				LPCHARACTER newmember = CHARACTER_MANAGER::instance().Find(vid);

				if (!newmember)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그러한 사람을 찾을 수 없습니다."));
					return SubPacketLen;
				}

				// @correction064 BEGIN (+newmember ispc check)
				if (!ch->IsPC() || !newmember->IsPC())
					return SubPacketLen;
				// @correction064 END

				// @correction005
				pGuild->Invite(ch, newmember);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_REMOVE_MEMBER:
			{
				if (pGuild->UnderAnyWar() != 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전 중에는 길드원을 탈퇴시킬 수 없습니다."));
					return SubPacketLen;
				}

				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				LPCHARACTER member = CHARACTER_MANAGER::instance().FindByPID(pid);

				if (member)
				{
					if (member->GetGuild() != pGuild)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방이 같은 길드가 아닙니다."));
						return SubPacketLen;
					}

					if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 강제 탈퇴 시킬 권한이 없습니다."));
						return SubPacketLen;
					}

					member->SetQuestFlag("guild_manage.new_withdraw_time", get_global_time());
					pGuild->RequestRemoveMember(member->GetPlayerID());

					// @correction005
					if (g_bGuildInviteLimit) // @correction213
						DBManager::instance().Query("REPLACE INTO guild_invite_limit VALUES(%d, %d)", pGuild->GetID(), get_global_time());
				}
				else
				{
					if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 강제 탈퇴 시킬 권한이 없습니다."));
						return SubPacketLen;
					}

					if (pGuild->RequestRemoveMember(pid))
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 강제 탈퇴 시켰습니다."));
					else
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그러한 사람을 찾을 수 없습니다."));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:
			{
				char gradename[GUILD_GRADE_NAME_MAX_LEN + 1];
				strlcpy(gradename, c_pData + 1, sizeof(gradename));

				const TGuildMember * m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 직위 이름을 변경할 권한이 없습니다."));
				}
				else if (*c_pData == GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장의 직위 이름은 변경할 수 없습니다."));
				}
				else if (!check_name(gradename))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 적합하지 않은 직위 이름 입니다."));
				}
				else
				{
					pGuild->ChangeGradeName(*c_pData, gradename);
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:
			{
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 직위 권한을 변경할 권한이 없습니다."));
				}
				else if (*c_pData == GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장의 권한은 변경할 수 없습니다."));
				}
				else
				{
					pGuild->ChangeGradeAuth(*c_pData, *(c_pData + 1));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_OFFER:
			{
				uint32_t offer = *reinterpret_cast<const uint32_t*>(c_pData);

				// @correction005
				if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드가 이미 최고 레벨입니다."));
				}
				else
				{
					offer /= 100;
					offer *= 100;

					if (pGuild->OfferExp(ch, offer))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> %u의 경험치를 투자하였습니다."), offer);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 경험치 투자에 실패하였습니다."));
					}
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHARGE_GSP:
			{
				const int32_t offer = *reinterpret_cast<const int32_t*>(c_pData);
				const int32_t gold = offer * 100;

				if (offer < 0 || gold < offer || gold < 0 || (int32_t)ch->GetGold() < gold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 돈이 부족합니다."));
					return SubPacketLen;
				}

				if (!pGuild->ChargeSP(ch, offer))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 용신력 회복에 실패하였습니다."));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_POST_COMMENT:
			{
				const size_t length = *c_pData;

				if (length > GUILD_COMMENT_MAX_LEN)
				{
					sys_err("POST_COMMENT: %s comment too int32_t (length: %u)", ch->GetName(), length);
					ch->GetDesc()->SetPhase(PHASE_CLOSE);
					return -1;
				}

				if (uiBytes < 1 + length)
					return -1;

				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (length && !pGuild->HasGradeAuth(m->grade, GUILD_AUTH_NOTICE) && *(c_pData + 1) == '!')
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 공지글을 작성할 권한이 없습니다."));
				}
				else
				{
					std::string str(c_pData + 1, length);
					pGuild->AddComment(ch, str);
				}

				return (1 + length);
			}

		case GUILD_SUBHEADER_CG_DELETE_COMMENT:
			{
				const uint32_t comment_id = *reinterpret_cast<const uint32_t*>(c_pData);

				pGuild->DeleteComment(ch, comment_id);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_REFRESH_COMMENT:
			pGuild->RefreshComment(ch);
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:
			{
				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t grade = *(c_pData + sizeof(uint32_t));
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 직위를 변경할 권한이 없습니다."));
				else if (ch->GetPlayerID() == pid)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장의 직위는 변경할 수 없습니다."));
				else if (grade == 1)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장으로 직위를 변경할 수 없습니다."));
				else
					pGuild->ChangeMemberGrade(pid, grade);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_USE_SKILL:
			{
				const TPacketCGGuildUseSkill* p = reinterpret_cast<const TPacketCGGuildUseSkill*>(c_pData);

				pGuild->UseSkill(p->dwVnum, ch, p->dwPID);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:
			{
				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t is_general = *(c_pData + sizeof(uint32_t));
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 장군을 지정할 권한이 없습니다."));
				}
				else
				{
					if (!pGuild->ChangeMemberGeneral(pid, is_general))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 더이상 장수를 지정할 수 없습니다."));
					}
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:
			{
				const uint32_t guild_id = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t accept = *(c_pData + sizeof(uint32_t));

				CGuild * g = CGuildManager::instance().FindGuild(guild_id);

				if (g)
				{
					if (accept)
						g->InviteAccept(ch);
					else
						g->InviteDeny(ch->GetPlayerID());
				}
			}
			return SubPacketLen;

	}

	return 0;
}

void CInputMain::Fishing(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGFishing* p = (TPacketCGFishing*)c_pData;
	ch->SetRotation(p->dir * 5);
	ch->fishing();
	return;
}

void CInputMain::ItemGive(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGGiveItem* p = (TPacketCGGiveItem*) c_pData;
	LPCHARACTER to_ch = CHARACTER_MANAGER::instance().Find(p->dwTargetVID);

	if (to_ch)
		ch->GiveItem(to_ch, p->ItemPos);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아이템을 건네줄 수 없습니다."));
}

void CInputMain::Hack(LPCHARACTER ch, const char * c_pData)
{
	TPacketCGHack * p = (TPacketCGHack *) c_pData;

	char buf[sizeof(p->szBuf)];
	strlcpy(buf, p->szBuf, sizeof(buf));

	sys_err("HACK_DETECT: %s %s", ch->GetName(), buf);

	ch->GetDesc()->SetPhase(PHASE_CLOSE);
}

int32_t CInputMain::MyShop(LPCHARACTER ch, const char * c_pData, size_t uiBytes)
{
	TPacketCGMyShop * p = (TPacketCGMyShop *) c_pData;
	int32_t iExtraLen = p->bCount * sizeof(TShopItemTable);

	if (uiBytes < sizeof(TPacketCGMyShop) + iExtraLen)
		return -1;

	if (ch->GetGold() >= GOLD_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("소유 돈이 20억냥을 넘어 거래를 핼수가 없습니다."));
		sys_log(0, "MyShop ==> OverFlow Gold id %u name %s ", ch->GetPlayerID(), ch->GetName());
		return (iExtraLen);
	}

	if (ch->IsStun() || ch->IsDead())
		return (iExtraLen);

	if (ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중일경우 개인상점을 열수가 없습니다."));
		return (iExtraLen);
	}

	sys_log(0, "MyShop count %d", p->bCount);
	ch->OpenMyShop(p->szSign, (TShopItemTable *) (c_pData + sizeof(TPacketCGMyShop)), p->bCount
#ifdef __OFFLINE_PRIVATE_SHOP__
, p->iTime, p->iTitle, p->iPoly
#endif
	);
	return (iExtraLen);
}

void CInputMain::Refine(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGRefine* p = reinterpret_cast<const TPacketCGRefine*>(c_pData);

	if (ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->GetMyShop() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO,  LC_TEXT("창고,거래창등이 열린 상태에서는 개량을 할수가 없습니다"));
		ch->ClearRefineMode();
		return;
	}

	if (p->type == 255)
	{
		ch->ClearRefineMode();
		return;
	}

	if (p->pos >= INVENTORY_MAX_NUM)
	{
		ch->ClearRefineMode();
		return;
	}

	LPITEM item = ch->GetInventoryItem(p->pos);

	if (!item)
	{
		ch->ClearRefineMode();
		return;
	}

	ch->SetRefineTime();

	if (p->type == REFINE_TYPE_NORMAL)
	{
		sys_log (0, "refine_type_noraml");
		ch->DoRefine(item);
	}
	else if (p->type == REFINE_TYPE_SCROLL || p->type == REFINE_TYPE_HYUNIRON || p->type == REFINE_TYPE_MUSIN || p->type == REFINE_TYPE_BDRAGON
#ifdef __RITUEL_STONE__
|| p->type == REFINE_TYPE_RITUALS_SCROLL
#endif
)
	{
		sys_log (0, "refine_type_scroll, ...");
		ch->DoRefineWithScroll(item);
	}
	else if (p->type == REFINE_TYPE_MONEY_ONLY)
	{
		const LPITEM item = ch->GetInventoryItem(p->pos);

		if (nullptr != item)
		{
			if (500 <= item->GetRefineSet())
			{
				LogManager::instance().HackLog("DEVIL_TOWER_REFINE_HACK", ch);
			}
			else
			{
				if (ch->GetQuestFlag("deviltower_zone.can_refine"))
				{
					if (ch->DoRefine(item, true)) // @correction099
						ch->SetQuestFlag("deviltower_zone.can_refine", 0);
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "사귀 타워 완료 보상은 한번까지 사용가능합니다.");
			}
		}
	}

	ch->ClearRefineMode();
}

int32_t CInputMain::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		d->SetPhase(PHASE_CLOSE);
		return (0);
	}

	int32_t iExtraLen = 0;

	if (test_server && bHeader != HEADER_CG_MOVE)
		sys_log(0, "CInputMain::Analyze() ==> Header [%d] ", bHeader);

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_CHAT:
			if (test_server)
			{
				char* pBuf = (char*)c_pData;
				sys_log(0, "%s", pBuf + sizeof(TPacketCGChat));
			}

			if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_WHISPER:
			if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_MOVE:
			Move(ch, c_pData);
			// @correction030 (removed CheckClientVersion since useless in here)
			// @correction005
			break;

		case HEADER_CG_CHARACTER_POSITION:
			Position(ch, c_pData);
			break;

		case HEADER_CG_ITEM_USE:
			if (!ch->IsObserverMode())
				ItemUse(ch, c_pData);
			break;

		case HEADER_CG_ITEM_DROP:
			if (!ch->IsObserverMode())
			{
				ItemDrop(ch, c_pData);
			}
			break;

		case HEADER_CG_ITEM_DROP2:
			if (!ch->IsObserverMode())
				ItemDrop2(ch, c_pData);
			break;

// @correction126 BEGIN
		case HEADER_CG_ITEM_DESTROY:
			if (!ch->IsObserverMode())
				ItemDestroy(ch, c_pData);
			break;
// @correction126 END

		case HEADER_CG_ITEM_MOVE:
			if (!ch->IsObserverMode())
				ItemMove(ch, c_pData);
			break;

		case HEADER_CG_ITEM_PICKUP:
			if (!ch->IsObserverMode())
				ItemPickup(ch, c_pData);
			break;

		case HEADER_CG_ITEM_USE_TO_ITEM:
			if (!ch->IsObserverMode())
				ItemToItem(ch, c_pData);
			break;

		case HEADER_CG_ITEM_GIVE:
			if (!ch->IsObserverMode())
				ItemGive(ch, c_pData);
			break;

		case HEADER_CG_EXCHANGE:
			if (!ch->IsObserverMode())
				Exchange(ch, c_pData);
			break;

		case HEADER_CG_ATTACK:
		case HEADER_CG_SHOOT:
			if (!ch->IsObserverMode())
			{
				Attack(ch, bHeader, c_pData);
			}
			break;

		case HEADER_CG_USE_SKILL:
			if (!ch->IsObserverMode())
				UseSkill(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_ADD:
			QuickslotAdd(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_DEL:
			QuickslotDelete(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_SWAP:
			QuickslotSwap(ch, c_pData);
			break;

		case HEADER_CG_SHOP:
			if ((iExtraLen = Shop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_MESSENGER:
			if ((iExtraLen = Messenger(ch, c_pData, m_iBufferLeft))<0)
				return -1;
			break;

		case HEADER_CG_ON_CLICK:
			OnClick(ch, c_pData);
			break;

		case HEADER_CG_SYNC_POSITION:
			if ((iExtraLen = SyncPosition(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_ADD_FLY_TARGETING:
		case HEADER_CG_FLY_TARGETING:
			FlyTarget(ch, c_pData, bHeader);
			break;

		case HEADER_CG_SCRIPT_BUTTON:
			ScriptButton(ch, c_pData);
			break;

		case HEADER_CG_SCRIPT_SELECT_ITEM:
			ScriptSelectItem(ch, c_pData);
			break;

		case HEADER_CG_SCRIPT_ANSWER:
			ScriptAnswer(ch, c_pData);
			break;

		case HEADER_CG_QUEST_INPUT_STRING:
			QuestInputString(ch, c_pData);
			break;

		case HEADER_CG_QUEST_CONFIRM:
			QuestConfirm(ch, c_pData);
			break;

		case HEADER_CG_TARGET:
			Target(ch, c_pData);
			break;

		case HEADER_CG_WARP:
			Warp(ch, c_pData);
			break;

		case HEADER_CG_SAFEBOX_CHECKIN:
			SafeboxCheckin(ch, c_pData);
			break;

		case HEADER_CG_SAFEBOX_CHECKOUT:
			SafeboxCheckout(ch, c_pData, false);
			break;

		case HEADER_CG_SAFEBOX_ITEM_MOVE:
			SafeboxItemMove(ch, c_pData);
			break;

		case HEADER_CG_MALL_CHECKOUT:
			SafeboxCheckout(ch, c_pData, true);
			break;

		case HEADER_CG_PARTY_INVITE:
			PartyInvite(ch, c_pData);
			break;

		case HEADER_CG_PARTY_REMOVE:
			PartyRemove(ch, c_pData);
			break;

		case HEADER_CG_PARTY_INVITE_ANSWER:
			PartyInviteAnswer(ch, c_pData);
			break;

		case HEADER_CG_PARTY_SET_STATE:
			PartySetState(ch, c_pData);
			break;

		case HEADER_CG_PARTY_USE_SKILL:
			PartyUseSkill(ch, c_pData);
			break;

		case HEADER_CG_PARTY_PARAMETER:
			PartyParameter(ch, c_pData);
			break;

		case HEADER_CG_ANSWER_MAKE_GUILD:
#ifdef ENABLE_NEWGUILDMAKE // @correction145
			ch->ChatPacket(CHAT_TYPE_INFO, "<%s> AnswerMakeGuild disabled", __FUNCTION__);
#else
			AnswerMakeGuild(ch, c_pData);
#endif
			break;

		case HEADER_CG_GUILD:
			if ((iExtraLen = Guild(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_FISHING:
			Fishing(ch, c_pData);
			break;

		case HEADER_CG_HACK:
			Hack(ch, c_pData);
			break;

		case HEADER_CG_MYSHOP:
			if ((iExtraLen = MyShop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_REFINE:
			Refine(ch, c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION:
			Version(ch, c_pData);
			break;

		case HEADER_CG_HS_ACK:
			if (isHackShieldEnable)
			{
				CHackShieldManager::instance().VerifyAck(d->GetCharacter(), c_pData);
			}
			break;

		case HEADER_CG_XTRAP_ACK:
			{
				TPacketXTrapCSVerify* p = reinterpret_cast<TPacketXTrapCSVerify*>((void*)c_pData);
				CXTrapManager::instance().Verify_CSStep3(d->GetCharacter(), p->bPacketData);
			}
			break;
		case HEADER_CG_DRAGON_SOUL_REFINE:
			{
				TPacketCGDragonSoulRefine* p = reinterpret_cast <TPacketCGDragonSoulRefine*>((void*)c_pData);
				switch(p->bSubType)
				{
				case DS_SUB_HEADER_CLOSE:
					ch->DragonSoul_RefineWindow_Close();
					break;
				case DS_SUB_HEADER_DO_REFINE_GRADE:
					{
						DSManager::instance().DoRefineGrade(ch, p->ItemGrid);
					}
					break;
				case DS_SUB_HEADER_DO_REFINE_STEP:
					{
						DSManager::instance().DoRefineStep(ch, p->ItemGrid);
					}
					break;
				case DS_SUB_HEADER_DO_REFINE_STRENGTH:
					{
						DSManager::instance().DoRefineStrength(ch, p->ItemGrid);
					}
					break;
				}
			}
			break;
#ifdef __ACCE_SYSTEM__ // @correction191
		case HEADER_CG_ACCE:
			Acce(ch, c_pData);
			break;
#endif
#ifdef __CHANGE_LOOK__ // @correction192
		case HEADER_CG_CL:
			ChangeLook(ch, c_pData);
			break;
#endif
#ifdef __SEND_TARGET_INFO__
		case HEADER_CG_TARGET_INFO_LOAD:
			TargetInfoLoad(ch, c_pData);
			break;
#endif
#ifdef __GROWTH_PET__
		case HEADER_CG_PET_SET_NAME:
			BraveRequestPetName(ch, c_pData);
			break;
#endif
#ifdef __SHOW_CHEST_DROP__
		case HEADER_CG_CHEST_DROP_INFO:
			ChestDropInfo(ch, c_pData);
			break;
#endif

#ifdef __MINI_GAME_RUMI__
		case HEADER_CG_MINI_GAME_RUMI:
			if ((iExtraLen = MiniGameRumi(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		case HEADER_CG_ATTENDANCE_BUTTON_CLICK:
			CMiniGameManager::instance().AttendanceButtonClick(ch, c_pData);
			break;
		case HEADER_CG_ATTENDANCE_REQUEST_REWARD_LIST:
			CMiniGameManager::instance().RequestRewardList(ch);
			break;
		case HEADER_CG_ATTENDANCE_REQUEST_DATA:
			CMiniGameManager::instance().RequestData(ch, c_pData);
			break;
#endif

#ifdef __MINI_GAME_FISH__
		case HEADER_CG_FISH_EVENT:
			if ((iExtraLen = FishEvent(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef __MINI_GAME_CATCH_KING__
		case HEADER_CG_MINI_GAME_CATCH_KING:
			if ((iExtraLen = MiniGameCatchKing(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif
#ifdef __SWITCHBOT__
		case HEADER_CG_SWITCHBOT:
			if ((iExtraLen = Switchbot(ch, c_pData, m_iBufferLeft)) < 0)
			{
				return -1;
			}
			break;
#endif
	}
	return (iExtraLen);
}

int32_t CInputDead::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		return 0;
	}

	int32_t iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_CHAT:
			if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;

			break;

		case HEADER_CG_WHISPER:
			if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;

			break;

		case HEADER_CG_HACK:
			Hack(ch, c_pData);
			break;

		default:
			return (0);
	}

	return (iExtraLen);
}

#ifdef __ACCE_SYSTEM__ // @correction191
void CInputMain::Acce(LPCHARACTER pkChar, const char* c_pData)
{
	if (!pkChar || !c_pData)
		return;

	quest::PC * pPC = quest::CQuestManager::instance().GetPCForce(pkChar->GetPlayerID());
	if (pPC && pPC->IsRunning())
		return;
	
	TPacketAcce * sPacket = (TPacketAcce*) c_pData;
	switch (sPacket->subheader)
	{
		case ACCE_SUBHEADER_CG_CLOSE:
			pkChar->CloseAcce();
			break;
		case ACCE_SUBHEADER_CG_ADD:
			pkChar->AddAcceMaterial(sPacket->tPos, sPacket->bPos);
			break;
		case ACCE_SUBHEADER_CG_REMOVE:
			pkChar->RemoveAcceMaterial(sPacket->bPos);
			break;
		case ACCE_SUBHEADER_CG_REFINE:
			pkChar->RefineAcceMaterials();
			break;
		default:
			break;
	}
}
#endif

#ifdef __CHANGE_LOOK__ // @correction192
void CInputMain::ChangeLook(LPCHARACTER pkChar, const char* c_pData)
{
	quest::PC * pPC = quest::CQuestManager::instance().GetPCForce(pkChar->GetPlayerID());
	if (pPC->IsRunning())
		return;
	
	TPacketChangeLook * sPacket = (TPacketChangeLook*) c_pData;
	switch (sPacket->subheader)
	{
		case CL_SUBHEADER_CLOSE:
			{
				pkChar->ChangeLookWindow(false);
			}
			break;
		case CL_SUBHEADER_ADD:
			{
				pkChar->AddClMaterial(sPacket->tPos, sPacket->bPos);
			}
			break;
		case CL_SUBHEADER_REMOVE:
			{
				pkChar->RemoveClMaterial(sPacket->bPos);
			}
			break;
		case CL_SUBHEADER_REFINE:
			{
				pkChar->RefineClMaterials();
			}
			break;
		default:
			break;
	}
}
#endif

#ifdef __SEND_TARGET_INFO__
void CInputMain::TargetInfoLoad(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGTargetInfoLoad* p = (TPacketCGTargetInfoLoad*)c_pData;
	TPacketGCTargetInfo pInfo;
	pInfo.header = HEADER_GC_TARGET_INFO;
	static std::vector<LPITEM> s_vec_item;
	s_vec_item.clear();
	LPITEM pkInfoItem;
	LPCHARACTER m_pkChrTarget = CHARACTER_MANAGER::instance().Find(p->dwVID);

	if (!ch || !m_pkChrTarget)
		return;

	// if (m_pkChrTarget && (m_pkChrTarget->IsMonster() || m_pkChrTarget->IsStone()))
	// {
		// if (thecore_heart->pulse - (int32_t) ch->GetLastTargetInfoPulse() < passes_per_sec * 3)
			// return;

		// ch->SetLastTargetInfoPulse(thecore_heart->pulse);

	if (ITEM_MANAGER::instance().CreateDropItemVector(m_pkChrTarget, ch, s_vec_item) && (m_pkChrTarget->IsMonster() || m_pkChrTarget->IsStone()))
	{
		if (s_vec_item.size() == 0);
		else if (s_vec_item.size() == 1)
		{
			pkInfoItem = s_vec_item[0];
			pInfo.dwVID	= m_pkChrTarget->GetVID();
			pInfo.race = m_pkChrTarget->GetRaceNum();
			pInfo.dwVnum = pkInfoItem->GetVnum();
			pInfo.count = pkInfoItem->GetCount();
			ch->GetDesc()->Packet(&pInfo, sizeof(TPacketGCTargetInfo));
		}
		else
		{
			int32_t iItemIdx = s_vec_item.size() - 1;
			while (iItemIdx >= 0)
			{
				pkInfoItem = s_vec_item[iItemIdx--];

				if (!pkInfoItem)
				{
					sys_err("pkInfoItem null in vector idx %d", iItemIdx + 1);
					continue;
				}

					pInfo.dwVID	= m_pkChrTarget->GetVID();
					pInfo.race = m_pkChrTarget->GetRaceNum();
					pInfo.dwVnum = pkInfoItem->GetVnum();
					pInfo.count = pkInfoItem->GetCount();
					ch->GetDesc()->Packet(&pInfo, sizeof(TPacketGCTargetInfo));
			}
		}
	}
	// }
}
#endif


#ifdef __GROWTH_PET__
void CInputMain::BraveRequestPetName(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc()) { return; }
	int32_t vid = ch->GetEggVid();
	if (vid == 0) { return; }

	TPacketCGRequestPetName* p = (TPacketCGRequestPetName*)c_pData;

	/*if (ch->GetGold() < 100000) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[PetSystem] ihtiyacınız olan 100.000 yang");
		return;
	}*/

	if (ch->CountSpecifyItem(vid) > 0 && check_name(p->petname) != 0) {
		//DBManager::instance().SendMoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), -100000);
		//ch->PointChange(POINT_GOLD, -100000, true);
		ch->RemoveSpecifyItem(vid, 1);
		LPITEM pSummonItem = ch->AutoGiveItem(vid + 300, 1);
		if (pSummonItem)
		{
			pSummonItem->SetSocket(1, 0);
			pSummonItem->SetSocket(2, time(nullptr));
			for (int32_t b = 0; b < 3; b++)
				pSummonItem->SetSocket(b + 3, number(1, 30));
			for (int32_t b = 0; b < 3; b++)
				pSummonItem->SetForceAttribute(b, 0, -1);

			pSummonItem->SetForceAttribute(4, 0, 0); // evo, lock
			pSummonItem->SetForceAttribute(5, 1, number(1, 6)); // level, type

			std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery("INSERT INTO new_petsystem VALUES(%u, '%s', 0)", pSummonItem->GetID(), p->petname));
		}
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "[PetSystem]Hatali Pet ismi!");
}
#endif

#ifdef __SHOW_CHEST_DROP__
void CInputMain::ChestDropInfo(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGChestDropInfo* p = (TPacketCGChestDropInfo*)c_pData;

	LPITEM item = nullptr;

	if (/*!ch->IsValidItemPosition(p->tPos) || */!(item = ch->GetItem(p->tPos)))
		return;

	std::vector<TChestDropInfoTable> vec_ItemList;
	ITEM_MANAGER::instance().GetChestItemList(item->GetVnum(), vec_ItemList);

	TPacketGCChestDropInfo packet;
	packet.bHeader = HEADER_GC_CHEST_DROP_INFO;
	packet.wSize = sizeof(packet) + sizeof(TChestDropInfoTable) * vec_ItemList.size();
	packet.tPos = p->tPos;

	ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
	ch->GetDesc()->Packet(&vec_ItemList[0], sizeof(TChestDropInfoTable) * vec_ItemList.size());
}
#endif

#ifdef __MINI_GAME_RUMI__
int32_t CInputMain::MiniGameRumi(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGMiniGameRumi* p = (TPacketCGMiniGameRumi*)data;

	if (uiBytes < sizeof(TPacketCGMiniGameRumi))
		return -1;

	const char* c_pData = data + sizeof(TPacketCGMiniGameRumi);
	uiBytes -= sizeof(TPacketCGMiniGameRumi);

	switch (p->subheader)
	{
	case MINI_GAME_RUMI_SUBHEADER_CG_START:
		ch->MiniGameRumiStart();
		return 0;

	case MINI_GAME_RUMI_SUBHEADER_CG_EXIT:
		ch->MiniGameRumiExit();
		return 0;

	case MINI_GAME_RUMI_SUBHEADER_CG_HAND_CARD_CLICK:
	{
		if (uiBytes < sizeof(int32_t) + sizeof(uint8_t))
			return -1;

		int32_t bAdd = *(c_pData);
		uint8_t bIndex = *(c_pData + sizeof(int32_t));
		ch->MiniGameRumiHandCardClick(bAdd, bIndex);
		return sizeof(int32_t) + sizeof(uint8_t);
	}
	case MINI_GAME_RUMI_SUBHEADER_CG_DECK_CARD_CLICK:
		ch->MiniGameRumiDeckCardClick();
		return 0;

	case MINI_GAME_RUMI_SUBHEADER_CG_FIELD_CARD_CLICK:
	{
		if (uiBytes < sizeof(uint8_t))
			return -1;

		uint8_t bIndex = *(c_pData);
		ch->MiniGameRumiFieldCardClick(bIndex);
		return sizeof(uint8_t);
	}
	default:
		sys_err("CInputMain::MiniGameRumi : Unknown subheader %d : %s", p->subheader, ch->GetName());
		break;
	}

	return 0;
}
#endif
#ifdef __MINI_GAME_FISH__
int32_t CInputMain::FishEvent(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGFishEvent* p = (TPacketCGFishEvent*)data;

	if (uiBytes < sizeof(TPacketCGFishEvent))
		return -1;

	const char* c_pData = data + sizeof(TPacketCGFishEvent);
	uiBytes -= sizeof(TPacketCGFishEvent);

	switch (p->bSubheader)
	{
	case FISH_EVENT_SUBHEADER_CG_BOX_USE:
	{
		if (uiBytes < sizeof(uint8_t) + sizeof(uint16_t))
			return -1;

		uint8_t bWindow = *(c_pData++);
		uint16_t wCell = *(c_pData);

		// sys_log(0, "INPUT: %s FISH_EVENT: USE_BOX", ch->GetName());

		ch->FishEventBoxUse(TItemPos(bWindow, wCell));
		return (sizeof(uint8_t) + sizeof(uint16_t));
	}

	case FISH_EVENT_SUBHEADER_CG_SHAPE_ADD:
	{
		if (uiBytes < sizeof(uint8_t))
			return -1;

		uint8_t shapePos = *c_pData;

		sys_log(0, "INPUT: %s FISH_EVENT: ADD_SHAPE", ch->GetName());

		ch->FishEventShapeAdd(shapePos);
		return sizeof(uint8_t);
	}

	case FISH_EVENT_SUBHEADER_CG_REQUEST_BLOCK:
	{
		ch->FishEventRequestBlock();
		return 0;
	}

	default:
		sys_err("CInputMain::FishEvent : Unknown subheader %d : %s", p->bSubheader, ch->GetName());
		break;
	}

	return 0;
}
#endif

#ifdef __MINI_GAME_CATCH_KING__
int32_t CInputMain::MiniGameCatchKing(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGMiniGameCatchKing* p = (TPacketCGMiniGameCatchKing*)data;

	if (uiBytes < sizeof(TPacketCGMiniGameCatchKing))
		return -1;

	// const char * c_pData = data + sizeof(TPacketCGMiniGameCatchKing);
	uiBytes -= sizeof(TPacketCGMiniGameCatchKing);

	switch (p->bSubheader)
	{
	case 0:
		ch->MiniGameCatchKingStartGame(p->bSubArgument);
		return 0;

	case 1:
		ch->MiniGameCatchKingDeckCardClick();
		return 0;

	case 2:
		ch->MiniGameCatchKingFieldCardClick(p->bSubArgument);
		return 0;

	case 3:
		ch->MiniGameCatchKingGetReward();
		return 0;

	default:
		sys_err("CInputMain::MiniGameCatchKing : Unknown subheader %d : %s", p->bSubheader, ch->GetName());
		break;
	}

	return 0;
}
#endif

#ifdef __SWITCHBOT__
int32_t CInputMain::Switchbot(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGSwitchbot* p = reinterpret_cast<const TPacketCGSwitchbot*>(data);

	if (uiBytes < sizeof(TPacketCGSwitchbot))
	{
		return -1;
	}

	const char* c_pData = data + sizeof(TPacketCGSwitchbot);
	uiBytes -= sizeof(TPacketCGSwitchbot);

	switch (p->subheader)
	{
	case SUBHEADER_CG_SWITCHBOT_START:
	{
		size_t extraLen = sizeof(TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
		if (uiBytes < extraLen)
		{
			return -1;
		}

		std::vector<TSwitchbotAttributeAlternativeTable> vec_alternatives;

		for (uint8_t alternative = 0; alternative < SWITCHBOT_ALTERNATIVE_COUNT; ++alternative)
		{
			const TSwitchbotAttributeAlternativeTable* pAttr = reinterpret_cast<const TSwitchbotAttributeAlternativeTable*>(c_pData);
			c_pData += sizeof(TSwitchbotAttributeAlternativeTable);

			vec_alternatives.emplace_back(*pAttr);
		}

		CSwitchbotManager::Instance().Start(ch->GetPlayerID(), p->slot, vec_alternatives);
		return extraLen;
	}

	case SUBHEADER_CG_SWITCHBOT_STOP:
	{
		CSwitchbotManager::Instance().Stop(ch->GetPlayerID(), p->slot);
		return 0;
	}
	}

	return 0;
}
#endif
