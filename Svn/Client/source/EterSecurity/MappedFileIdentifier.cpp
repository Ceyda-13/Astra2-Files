#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include <XOrstr.h>
#include <fstream>
#include "../EterPack/EterPackManager.h"

std::string GetMappedFileHash(const std::string & strFileName)
{
	std::string strHash = "";

	if (strFileName.empty())
		return strHash;

	if (!CEterPackManager::Instance().isExist(strFileName.c_str()))
		return strHash;

	CMappedFile pMappedFile;
	const uint8_t* c_pbMap;
	if (!pMappedFile.Create(strFileName.c_str(), (const void**)&c_pbMap, 0, 0))
		return strHash;

	if (!c_pbMap)
		return strHash;

	strHash = CAnticheatManager::Instance().GetMd5(c_pbMap, pMappedFile.Size());

	return strHash;
}

static struct stCheckFileList {
	const char*			c_szFileName;
	const char*			c_szFileMd5;
} st_FileHashList[] = {

	{ "d:\\ymir work\\pc\\warrior\\horse\\wait.msa",		"0ca623f31bb662c795e30f76dcac761c" },
	{ "d:\\ymir work\\pc\\warrior\\horse\\damage.msa",		"87fba70b9c39e55256c9bd8fefd24daa" },
	{ "d:\\ymir work\\pc\\warrior\\horse\\wait_1.msa",		"24dfcdca31d9df539075ee70e375f9fd" },
	{ "d:\\ymir work\\pc\\warrior\\horse\\wait_2.msa",		"1411f53f4a1afd48bf9ff2829f2f2422" },
	{ "d:\\ymir work\\pc\\warrior\\horse_onehand_sword\\combo_01.msa",		"2da4eabbad6b135d58376964cc28d232" },
	{ "d:\\ymir work\\pc\\warrior\\horse_onehand_sword\\combo_02.msa",		"a8d91cbdfd240ea663b65eff927ab268" },
	{ "d:\\ymir work\\pc\\warrior\\horse_onehand_sword\\combo_03.msa",		"d2e579ab6d07d77513f5cebc06db3efd" },
	{ "d:\\ymir work\\pc\\warrior\\horse_twohand_sword\\combo_01.msa",		"a396a7adf6ada434541d1c902bb441a6" },
	{ "d:\\ymir work\\pc\\warrior\\horse_twohand_sword\\combo_02.msa",		"44aed62a36f4e8f49d5d192c81eded91" },
	{ "d:\\ymir work\\pc\\warrior\\horse_twohand_sword\\combo_03.msa",		"2d1fe846f2424ae305e933f39975c197" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_01.msa",		"d5157530faf7a7a62dbe3305e33d7901" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_02.msa",		"3fc937d2ec1b1bb1fccb2e5436265a43" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_03.msa",		"5b770381ea510dc5e9cc29568559773a" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_04.msa",		"d270c27a30897c6f7616c39640823cd1" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_05.msa",		"788c9e55d9f071efd9bbebc8e41b3a7c" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_06.msa",		"44ba15627638829ab8037063c6610aeb" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_07.msa",		"16e493a84ee8f2baf98c76374063cb7a" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage.msa",		"e0045a196e8ad146deb258b497a428c3" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage_1.msa",		"699e31e825807db87729a27dfdf20595" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage_2.msa",		"d8a8d4d56d0aec535d43169af4450240" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage_3.msa",		"c343b40fb5594d8a9c98346a5e93bcd4" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\wait.msa",		"3c2af5d10675b5226aa9ed6d3f6545b3" },
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\wait_1.msa",		"2de7f5289caff1b94ba5d502346e1147" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_01.msa",		"f36abeffe48b8ea28914811bd4b64f18" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_02.msa",		"bb1aaa77b804167e7000fc9ad409f527" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_03.msa",		"839bb72a140c7a65b3e9b6443c145521" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_04.msa",		"789179dea92aadb4a507ca316f136f08" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_05.msa",		"9bdaf95b42814659434cd085ca76c6af" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_06.msa",		"2f9999a679d6369758e7115bb3bc7409" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_07.msa",		"08e4430e4c5b1baaf2c5941163af2d89" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage.msa",		"82846659be5aee19fb0856aba9eb76e8" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage_1.msa",		"37286497efb18312494fa677eba2dae6" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage_2.msa",		"1a3f94d699fc1d2a153cc45760afe683" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage_3.msa",		"e537ae7129b969e9724aa50cadf69300" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\wait.msa",		"94b5e6770f2064dfe4de9a95ba39813a" },
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\wait_1.msa",		"1d6296e6409bdc18ce5026648ba412d8" },
	{ "d:\\ymir work\\pc2\\warrior\\horse\\wait.msa",		"16778aff5ff09643fbebbc22eb86c6f4" },
	{ "d:\\ymir work\\pc2\\warrior\\horse\\damage.msa",		"beeda94a120f910f3d3010c10375565f" },
	{ "d:\\ymir work\\pc2\\warrior\\horse\\wait_1.msa",		"6e321f5231c494709bc9f98f1e12d8fe" },
	{ "d:\\ymir work\\pc2\\warrior\\horse\\wait_2.msa",		"d093c07ef088453ed134f7e31902e75f" },
	{ "d:\\ymir work\\pc2\\warrior\\horse_onehand_sword\\combo_01.msa",		"c890fbdd915e871755bfaff87b5f67c8" },
	{ "d:\\ymir work\\pc2\\warrior\\horse_onehand_sword\\combo_02.msa",		"8ba16a0400a0a322b5512ec75471ca45" },
	{ "d:\\ymir work\\pc2\\warrior\\horse_onehand_sword\\combo_03.msa",		"166974b69f818a10231420fa663dbf05" },
	{ "d:\\ymir work\\pc2\\warrior\\horse_twohand_sword\\combo_01.msa",		"c890fbdd915e871755bfaff87b5f67c8" },
	{ "d:\\ymir work\\pc2\\warrior\\horse_twohand_sword\\combo_02.msa",		"8ba16a0400a0a322b5512ec75471ca45" },
	{ "d:\\ymir work\\pc2\\warrior\\horse_twohand_sword\\combo_03.msa",		"166974b69f818a10231420fa663dbf05" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_01.msa",		"b4f2f87dcb0ee872502a451eabbdd086" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_02.msa",		"be7bbae448d4a62a9fd1fcf642580727" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_03.msa",		"cdb2e9c6d5efa621211a2bf7ef7c1f67" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_04.msa",		"da5711d76b339c03ba7a3a5811f46733" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_05.msa",		"b202f808205cedcb47b6d785a35b1227" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_06.msa",		"18c1c5c34011ef07b7732ce568f4d439" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_07.msa",		"5738278fc72a9c6714d76377dc48175d" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage.msa",		"fe56aeba3cb3b5aa1d4ce6af75626c52" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage_1.msa",		"1311492d9afd90fc7944a1d89eca2ffd" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage_2.msa",		"53448c011f4c9a1eff2eacd335d6fa6b" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage_3.msa",		"d92a48a931dd61022cf93368d7e19d54" },
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\wait.msa",		"5136dfd0a90be2b8a00190d20b659b41" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_01.msa",		"233cb85c0dbd23020f804a065af7d26d" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_02.msa",		"ae29d852563ac302d0bc4a871de1850a" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_03.msa",		"62d2e7bcb5f30e7444a993a29c537ebf" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_04.msa",		"8b332510db6e80137069a2145c71b433" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_05.msa",		"4852e2f8407ce850d62d93f9d0a31797" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_06.msa",		"567a9621eff569b1ff5ac46dea5727e1" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_07.msa",		"5a4f2a2a42bb81e2d1eb654167862dae" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage.msa",		"1dbf1a0ec6dae0b9eed828c019028415" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage_1.msa",		"d3446aae7b110f945488c8e680381cfe" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage_2.msa",		"5c44f19f91633c87395bf6072f8f67f1" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage_3.msa",		"c6c9fc0a27872f02f5eb5cd629220063" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\wait.msa",		"73b2273118785801b5e9137ade4b0a9b" },
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\wait_1.msa",		"2155db1b9ce6d4b7af4f4cbc9d07a89f" },
	{ "d:\\ymir work\\pc\\sura\\horse\\wait.msa",		"2b0d37fd55e2fb2acd5d1dbf465133c8" },
	{ "d:\\ymir work\\pc\\sura\\horse\\damage.msa",		"35b1026d8c540ce9e8edf993bb2994f0" },
	{ "d:\\ymir work\\pc\\sura\\horse\\wait_1.msa",		"51401340b97eb31a6e498ab9d1262552" },
	{ "d:\\ymir work\\pc\\sura\\horse\\wait_2.msa",		"422b721045995199e6b20d28fcd0566e" },
	{ "d:\\ymir work\\pc\\sura\\horse_onehand_sword\\combo_01.msa",		"9a5adbbca72a36b33cfb6c9a863601a9" },
	{ "d:\\ymir work\\pc\\sura\\horse_onehand_sword\\combo_02.msa",		"8c9c03aa5c3ca131c3098f137acc48d4" },
	{ "d:\\ymir work\\pc\\sura\\horse_onehand_sword\\combo_03.msa",		"ca15426e919e5dffe6250509fad2facd" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_01.msa",		"df317c0a02efed237b6e3fbf4127e391" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_02.msa",		"aa87cfa880cfc2783640f2a4be7481d0" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_03.msa",		"a942f917b7e91d2fc9372bfa9501fb57" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_05.msa",		"ee93b1531d6c10ce7912f2965d3cf30b" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_06.msa",		"d0da6073ae42f9eb4a0770da13a9e1cd" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_07.msa",		"4f8d5f50141a76d8ca867ebf8e6e646f" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage.msa",		"5c2eeadc59f827aff4ca1b6e40e1cef9" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage_1.msa",		"954009ab7aab6fc6453dbf84212e7c8c" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage_2.msa",		"9d90289df446a4108beaf14317475989" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage_3.msa",		"77a03999132991bf29533fabd42e97e7" },
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\wait.msa",		"1dc1411f3cd9340d261ace45f1677d95" },
	{ "d:\\ymir work\\pc2\\sura\\horse\\wait.msa",		"80e7437176e3e38d553df9d8eb0fa332" },
	{ "d:\\ymir work\\pc2\\sura\\horse\\damage.msa",		"692c9fbcf5346f30b692112a7f2fbe4f" },
	{ "d:\\ymir work\\pc2\\sura\\horse\\wait_1.msa",		"76c93c9b623adfc899c3a41b3521b0b9" },
	{ "d:\\ymir work\\pc2\\sura\\horse\\wait_2.msa",		"f2b73a2d725caf5f1efa116ab20679f7" },
	{ "d:\\ymir work\\pc2\\sura\\horse_onehand_sword\\combo_01.msa",		"b43c70b3773a6324a5e5d7b3fef37128" },
	{ "d:\\ymir work\\pc2\\sura\\horse_onehand_sword\\combo_02.msa",		"f987156931f48e5c334083dc0c92cd58" },
	{ "d:\\ymir work\\pc2\\sura\\horse_onehand_sword\\combo_03.msa",		"bca22009b68f322c439faec2fb364b99" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_01.msa",		"8e43dd66dc6a4bdca5606b3eded0966f" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_02.msa",		"452d63b47dcae13002d5ee6b1f51cb03" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_03.msa",		"412f193a5a53ad76f911632f37ab8d5e" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_05.msa",		"7a6f9d39e7dcb99898b15991c3f029e5" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_06.msa",		"afbdaf93fa1eebf98b4c6b8684c221d7" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_07.msa",		"5b13e6a9d37d11994ba008a93e04fc14" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage.msa",		"d8ef234e871ece93422e4ee0aa7905df" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage_1.msa",		"6c9455346f29a988c08f2692baab857f" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage_2.msa",		"fa9a74de488ead952bfe37adc00a38d0" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage_3.msa",		"eacab3e94ae43734fb1374261c149667" },
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\wait.msa",		"bff4169ef380de051bb2e9e223659d7e" },
	{ "d:\\ymir work\\pc\\shaman\\horse\\wait.msa",		"27ebb6ba26e58889af2c687b0d8c1c45" },
	{ "d:\\ymir work\\pc\\shaman\\horse\\damage.msa",		"7765f15c43118741f11e5bc903813d93" },
	{ "d:\\ymir work\\pc\\shaman\\horse\\wait_1.msa",		"17594eb9e82238295279ac37ac6d5674" },
	{ "d:\\ymir work\\pc\\shaman\\horse\\wait_2.msa",		"6a9eb02eff9780a095d1aa34a2972ae7" },
	{ "d:\\ymir work\\pc\\shaman\\horse_bell\\combo_01.msa",		"6d8f6b97d6741a06a47dd7fe8161d43e" },
	{ "d:\\ymir work\\pc\\shaman\\horse_bell\\combo_02.msa",		"3e80d822310f0662a3e56c2904c387eb" },
	{ "d:\\ymir work\\pc\\shaman\\horse_bell\\combo_03.msa",		"86a0c859be02e5d1bcc1d0ac1c95ce04" },
	{ "d:\\ymir work\\pc\\shaman\\horse_fan\\combo_01.msa",		"76b5a2c37ddb73c160e359939cb47a55" },
	{ "d:\\ymir work\\pc\\shaman\\horse_fan\\combo_02.msa",		"353a4c23a301971e3f172d7e08446709" },
	{ "d:\\ymir work\\pc\\shaman\\horse_fan\\combo_03.msa",		"59f4785c25f72d58e1e6180f1e5ea974" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_01.msa",		"64f344462b865a3344ce0d685cff8235" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_02.msa",		"a768f5b8bd0e5302d5395650f5400021" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_03.msa",		"079c71a4c0335846f3ca4ae4af49fe19" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_04.msa",		"f65b936c605d5a21b4dc82b64011564a" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_05.msa",		"d8ef61abe76034bc0c02813e4a1fd4cb" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_06.msa",		"b2a995d3588c97c9bad1414caaf3f301" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_07.msa",		"44c2906d0c890cdd8ca08b6a3b5e56f5" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage.msa",		"3e844783f33716f5b0cd284a174bb454" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage_1.msa",		"46b91f0ea281300ce6d7616eab077cd8" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage_2.msa",		"8a58c0918197230678e7d27fb4f32f2e" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage_3.msa",		"bc807b7065bdc5558402121d8b564498" },
	{ "d:\\ymir work\\pc\\shaman\\bell\\wait.msa",		"54dafb99955165a86016f97d944e9480" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_01.msa",		"94f3da3c71754a785cceb2b823416065" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_02.msa",		"981728b4fd08592382bced4239d6794e" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_03.msa",		"472836837d763c64ef628cae88a4ff7a" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_04.msa",		"5fd1e8c47f4400dea637e6147b752b8a" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_05.msa",		"dca3b159b7b7b7dece5e2c27d70be817" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_06.msa",		"a0a351380fc7bbff9af2f5011af15cfb" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_07.msa",		"7da70b0b199b9dca1a983829cdab5af4" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage.msa",		"0166e6237592256968686e28cde4967a" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage_1.msa",		"49766293ac4a2454d9d2b54f30c81ac3" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage_2.msa",		"533ca579d7cc26cfdaa8ae1246b714ae" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage_3.msa",		"a2d37c6f86d466dc358b53164db3920d" },
	{ "d:\\ymir work\\pc\\shaman\\fan\\wait.msa",		"937bebcef86f2ca964b7f025cf237843" },
	{ "d:\\ymir work\\pc2\\shaman\\horse\\wait.msa",		"a7fe1f9638c804e51237587d29ea015e" },
	{ "d:\\ymir work\\pc2\\shaman\\horse\\damage.msa",		"fc53f784e2beea6124f022eb2a98aaaa" },
	{ "d:\\ymir work\\pc2\\shaman\\horse\\wait_1.msa",		"244d12f9ecefa0eea627100036ee81dc" },
	{ "d:\\ymir work\\pc2\\shaman\\horse\\wait_2.msa",		"fd8ccddfc8b9d29237dc45839012aa22" },
	{ "d:\\ymir work\\pc2\\shaman\\horse_bell\\combo_01.msa",		"60ac9e1adc37627d31495117324523ce" },
	{ "d:\\ymir work\\pc2\\shaman\\horse_bell\\combo_02.msa",		"311fe2d7e7bd0b9cf58c410b313177bb" },
	{ "d:\\ymir work\\pc2\\shaman\\horse_bell\\combo_03.msa",		"2d249d798ee52062d85c702cc77729f5" },
	{ "d:\\ymir work\\pc2\\shaman\\horse_fan\\combo_01.msa",		"cfbb6000cd13d7d008884d9246691823" },
	{ "d:\\ymir work\\pc2\\shaman\\horse_fan\\combo_02.msa",		"1f253af8807c871b7ea208df054dab86" },
	{ "d:\\ymir work\\pc2\\shaman\\horse_fan\\combo_03.msa",		"c3734420ec204e1bac3dc2b94b3452a0" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_01.msa",		"349fa05a137a5db5a12496e6aa86f08e" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_02.msa",		"33b1e2dbb9ea33d4c4d000de9e7eed12" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_03.msa",		"1a6c17baf1ee32cad3cdcc752739e256" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_04.msa",		"99d0dca2b3011b3736eaf722d5d21fb0" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_05.msa",		"136cf92be58f70c6e74f84b4c0b7ccbe" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_06.msa",		"4f1feee0c3c5fdeb347a07d1b0b414d6" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_07.msa",		"2130313b154e0f1580581e0371a93b9d" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage.msa",		"831bf91871eac39ee3050264ee8cd00c" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage_1.msa",		"deee7bf7c69dacdc69de241930ead6a6" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage_2.msa",		"a1e4693b796f47ba25cef21d65f0dbca" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage_3.msa",		"469e57fd795bbf8c427c2db365998d40" },
	{ "d:\\ymir work\\pc2\\shaman\\bell\\wait.msa",		"c4fe3c56cb903fc6427fb8ff8a9d710f" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_01.msa",		"38a93281e809ce7e0f0108ec00993afe" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_02.msa",		"1619aa1805c11c163ea970f6b30aaed7" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_03.msa",		"05bdf0a697b4920122f2336e1ff83d95" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_04.msa",		"6cc7070c3f39c2927fb16bd8f8903bb6" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_05.msa",		"7006282d38c5afbe8828072f2136a417" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_06.msa",		"0f90014d3dc240963e26ac1624810cc6" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_07.msa",		"1d270ab4403260a0b389b6b8ecdb049d" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage.msa",		"1a65161363b63fbe2e2ab3eb331d277d" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage_1.msa",		"55a28565dc102a71d0ee020bc9542233" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage_2.msa",		"da00afd68ff04f26865f98ad9a8ed116" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage_3.msa",		"498ef042775d7c3fbe446c355952b2d1" },
	{ "d:\\ymir work\\pc2\\shaman\\fan\\wait.msa",		"d2ea05392a01357125fba1f8c78fcf14" },
	{ "d:\\ymir work\\pc\\assassin\\horse\\wait.msa",		"f2086de51696036178459b889587deae" },
	{ "d:\\ymir work\\pc\\assassin\\horse\\damage.msa",		"85bdab5b8f816d2bc45c8aac9d5d2b62" },
	{ "d:\\ymir work\\pc\\assassin\\horse\\wait_1.msa",		"f24889b7693f3ad052f79f920011297d" },
	{ "d:\\ymir work\\pc\\assassin\\horse\\wait_2.msa",		"93b31c58c57379746631f8328fa5abbb" },
	{ "d:\\ymir work\\pc\\assassin\\horse_onehand_sword\\combo_01.msa",		"40430673d7205417d9952451daf11da0" },
	{ "d:\\ymir work\\pc\\assassin\\horse_onehand_sword\\combo_02.msa",		"41292c01fedd999cf2f9f45f548c5e5a" },
	{ "d:\\ymir work\\pc\\assassin\\horse_onehand_sword\\combo_03.msa",		"cdf20c9fe6d1b0582eef419afaed5077" },
	{ "d:\\ymir work\\pc\\assassin\\horse_dualhand_sword\\combo_01.msa",		"e6fb9bdcd20616f7000114c6661caef2" },
	{ "d:\\ymir work\\pc\\assassin\\horse_dualhand_sword\\combo_02.msa",		"bb2763e11c3a16632608f6b04a0a713e" },
	{ "d:\\ymir work\\pc\\assassin\\horse_dualhand_sword\\combo_03.msa",		"13c608748b3fce510b11596549b68946" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_01.msa",		"d24236f81e0aeff2f685ae9bf1fa6362" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_02.msa",		"c651d1653e87edaacdd149b73d709c20" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_03.msa",		"371b7ae31b5be9d64812b7a0a95150c5" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_04.msa",		"ae2ef8fe64e02da7f988bcb737062dc1" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_05.msa",		"52831f4375e6be175020a7fecd88c575" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_06.msa",		"ca931d7d1fb88a24bd29482ffb26a8f3" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_07.msa",		"6b792ef10ec48e41d0ddf985709a2c95" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage.msa",		"643460a7e6ae9755a0bca0cbd4228541" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage_1.msa",		"f27898c4387fccb32385b926c87f053f" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage_2.msa",		"14a70c66c0a6704263a17bf40d91440f" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage_3.msa",		"14cae205eead54082b96795e15dec0aa" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\wait.msa",		"45fafacabe1a857f03a6fefe57bcac45" },
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\wait_1.msa",		"b3015cc56e760e029cb43fea1bb234fe" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_01.msa",		"7708aa483eb0c380a11025346201d058" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_02.msa",		"4f9693b505bb92393d812fa4faa7a2f7" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_03.msa",		"fec496f6b3e15c48111dabfaf1289d20" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_04.msa",		"924eb0298433f7a3bd27ae99c209b60a" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_05.msa",		"aba2b8f27156cd5ad364fdfa11ca457a" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_06.msa",		"79532fda61e3dab7a6d0c080ed462e4d" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_07.msa",		"527dcf3ac220d4824b2dc76e86e498b6" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_08.msa",		"2da5ddac3e619b2eceb6ea93644bd109" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage.msa",		"1c9abdf3e779b3e9a7af70813a5bf2ae" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage_1.msa",		"c63b5d9b17487edff2e74173367e6543" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage_2.msa",		"31454e5d262d49124da0421886dc9278" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage_3.msa",		"081e408128052ca0e5b778ccfe7a0243" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\wait.msa",		"51fbbf00bff084b5e2f9dacdaf9e9cd4" },
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\wait_1.msa",		"4f95af0aba137dc73e1a2adf23d8ec6d" },
	{ "d:\\ymir work\\pc\\assassin\\bow\\wait.msa",		"79bedce37aba3c547abaf1204a775f95" },
	{ "d:\\ymir work\\pc\\assassin\\bow\\wait_1.msa",		"0f61c0e1dcf1ea05e4ac969d3252e78e" },
	{ "d:\\ymir work\\pc2\\assassin\\horse\\wait.msa",		"ad7b770b62e11a463b1a023e606d9482" },
	{ "d:\\ymir work\\pc2\\assassin\\horse\\damage.msa",		"1b1e8e92216e51c6eba79f6ee1252421" },
	{ "d:\\ymir work\\pc2\\assassin\\horse\\wait_1.msa",		"b65628d976796882396be37c2e6260cd" },
	{ "d:\\ymir work\\pc2\\assassin\\horse\\wait_2.msa",		"7f366debaf2cea3331e0850226f1e22f" },
	{ "d:\\ymir work\\pc2\\assassin\\horse_onehand_sword\\combo_01.msa",		"dadc6462c1cd239218a963f63f6e6b3a" },
	{ "d:\\ymir work\\pc2\\assassin\\horse_onehand_sword\\combo_02.msa",		"83cf6eead20ef300af9ce9d68f55b5b9" },
	{ "d:\\ymir work\\pc2\\assassin\\horse_onehand_sword\\combo_03.msa",		"bbd93f7ca23347f58e26b149f78d215c" },
	{ "d:\\ymir work\\pc2\\assassin\\horse_dualhand_sword\\combo_01.msa",		"ec3f3692d5dab712649129df2aa075b8" },
	{ "d:\\ymir work\\pc2\\assassin\\horse_dualhand_sword\\combo_02.msa",		"d4861235b901a7f50d27f452dbc68588" },
	{ "d:\\ymir work\\pc2\\assassin\\horse_dualhand_sword\\combo_03.msa",		"7efa998c45c487f7767d68945ee6d017" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_01.msa",		"a3f032fa9f2faa0f3d7ed19a441bcf9d" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_02.msa",		"d1bb7ff76603068f7c76463452b2bde8" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_03.msa",		"81b5c276d9d1ab617888a164bde01836" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_04.msa",		"873943e90e5139dc850dfae24f9e6af0" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_05.msa",		"6fb1f5381951fad0238ca749cbb34442" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_06.msa",		"5d1c3b4ee30f70bc9ae46ca45c6ceaad" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_07.msa",		"302ea8fad102ebd7376ffbb448a26023" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage.msa",		"87902cd60ee97743082a6154246e5966" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage_1.msa",		"88ca6e5af55d8af73dc9a49a07ba6d7d" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage_2.msa",		"e59f9c6a0f784bba7dec8ff57a56c3b7" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage_3.msa",		"f12826d61b88224481ca94b837046656" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\wait.msa",		"c54f6bf69694e039234a206b5f64b29d" },
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\wait_1.msa",		"bf855de2d8aa0750d57903ac0432e908" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_01.msa",		"487d02bfa4499062c94355c0c1fde610" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_02.msa",		"317d3558ea68415aece4d23be6577725" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_03.msa",		"12a01be86e6a8f4330c08bb33c5b11e1" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_04.msa",		"d56e2e0f5c38d4c1c080dbadca989052" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_05.msa",		"f64a43dbc4eda4468d6513c77881e98f" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_06.msa",		"6981a954ab687374965e3f5cc29774f4" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_07.msa",		"c49deb4a7f685e938a564031ffc27e6c" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_08.msa",		"c49deb4a7f685e938a564031ffc27e6c" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage.msa",		"fd7d292dd830c5afda12c0745711d458" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage_1.msa",		"8ef1f318f9a8b57e83274ea7c354d30c" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage_2.msa",		"9a22acf954cd8f72f69239c7e6697f1a" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage_3.msa",		"5277d4e913f93a71071e58f393c2a06d" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\wait.msa",		"5368e10d8cf302d0a2886de0d59de762" },
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\wait_1.msa",		"f4b56182da6fe7da06c7fc649505e0b5" },
	{ "d:\\ymir work\\pc2\\assassin\\bow\\wait.msa",		"4a83cc37ed104876cb032319816b04c1" },
	{ "d:\\ymir work\\pc2\\assassin\\bow\\wait_1.msa",		"05b9f0658227239349eb803f40f35442" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\wait.msa",		"876a671bb1bf58a56537872637c8c81b" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\front_damage.msa",		"e84d3b76afb2acb803eb3b2dd113f5e4" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\wait1.msa",		"3a9c8d00ee09d32367d76238f9404891" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\wait2.msa",		"cdd1251745f274de26bd4025d0a7cb8f" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse_claw\\combo_01.msa",		"2e05606ae605b0020efb48c78567a5c0" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse_claw\\combo_02.msa",		"f73ddfd65da257206b56f58bb2ab2ace" },
	{ "d:\\ymir work\\pc3\\wolfman\\horse_claw\\combo_03.msa",		"ccc86e18cde87559157c6caae0082763" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_01.msa",		"c50414a4199ee50d6a9b34ebde79f24c" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_02.msa",		"3b8c5d6440a8a9e41b9a2ca10bbee7be" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_03.msa",		"0cd10eb843256047b2eca2f58b51c6ad" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_04.msa",		"0c4fa2863e39634576d097b2c723be77" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_05.msa",		"f5338d83c0f3938e4f7e3b5eb5b37da8" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_06.msa",		"1c6d9f038e4d36880e2b3516b135361b" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_07.msa",		"a023cdba2749ea6e7b4a00f3c29b766c" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\back_damage.msa",		"4a8deecf0c4c6e0dabe2ed1ac3cd3dcc" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\back_damage1.msa",		"152fd1c8b60fd6f6186fbdfdf727f509" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\front_damage.msa",		"8be1279e45125f52bc9b9eca03b74ecc" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\front_damage1.msa",		"2fb491222616a91ab5380104ea8cd4f0" },
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\wait.msa",		"30bfce58e5d1f7b1ba6dfcf12a8e70de" },
	{ "metin2_map_a1\\000000\\attr.atr",		"760a2199a461964e3b3460a9ca8cef99" },
	{ "metin2_map_a1\\001001\\attr.atr",		"471dbdf339b3f1687f41904527676a64" },
	{ "metin2_map_a3\\000000\\attr.atr",		"9ef79c0ed2330e3db76df921ae981fd0" },
	{ "metin2_map_a3\\001001\\attr.atr",		"0d662913588f70e150d0626ae0185525" },
	{ "metin2_map_b1\\000000\\attr.atr",		"08a3fa27624b39735b4bf409f9a3632a" },
	{ "metin2_map_b1\\001001\\attr.atr",		"099b99b4adec67fedc53f7615f3aa892" },
	{ "metin2_map_b3\\000000\\attr.atr",		"ef85978d327801877207a4d8fb73d8c2" },
	{ "metin2_map_b3\\001001\\attr.atr",		"4d0d9d6a86629131f715ac4598f84827" },
	{ "metin2_map_c1\\000000\\attr.atr",		"c5a714a3677086aaccf7309edf795782" },
	{ "metin2_map_c1\\001001\\attr.atr",		"2c2da08f1f02f184ef75c0012b2b53d9" },
	{ "metin2_map_c3\\000000\\attr.atr",		"856526ae5579c9b598145b0b603e714e" },
	{ "metin2_map_c3\\001001\\attr.atr",		"07881f0a9c4ec9326dbb3f2c0537979f" },
	{ "metin2_map_monkeydungeon\\001001\\attr.atr",		"208fe753339985c9c260b068bc8b4578" },
	{ "metin2_map_monkeydungeon_02\\001001\\attr.atr",		"208fe753339985c9c260b068bc8b4578" },
	{ "metin2_map_monkeydungeon_03\\001001\\attr.atr",		"208fe753339985c9c260b068bc8b4578" },
	{ "metin2_map_spiderdungeon\\001001\\attr.atr",		"54671e12e49438f891df56a3ac9e77ea" },
	{ "metin2_map_trent\\001001\\attr.atr",		"53b7f62e328327776e542a56eeeef8e4" },
	{ "metin2_map_trent02\\001001\\attr.atr",		"300c26ae6cc2282a026300fb77519b37" },
	{ "XXX", 0 },
};

#ifdef _DEBUG
void HashDump(const char* cArgFormat, ...)
{
	char szTmpString[2000];

	va_list vaArgList;
	va_start(vaArgList, cArgFormat);
	wvsprintfA(szTmpString, cArgFormat, vaArgList);
	va_end(vaArgList);

	std::ofstream f("hashs.txt", std::ofstream::out | std::ofstream::app);
	f << szTmpString << std::endl;
	f.close();
}

void DumpMappedFileHashes()
{
	/*for (size_t i = 0; strcmp(st_FileHashList[i].c_szFileName, "XXX"); ++i)
	{
		auto strCurrentFileName = st_FileHashList[i].c_szFileName;
		auto strHash = GetMappedFileHash(strCurrentFileName);

		//HashDump("\t{ \"%s\",		\"%s\" }, strCurrentFileNam);
	}*/
}
#endif

void CAnticheatManager::CheckMappedFiles()
{
	for (size_t i = 0; strcmp(st_FileHashList[i].c_szFileName, XOR("XXX")); ++i)
	{
		auto c_szCurrentFileName = st_FileHashList[i].c_szFileName;
		auto strCurrentFileHash = std::string(st_FileHashList[i].c_szFileMd5);

		auto strCorrectFileHash = GetMappedFileHash(c_szCurrentFileName);

		if (!strCorrectFileHash.empty() && strCorrectFileHash != strCurrentFileHash)
		{
#ifdef _DEBUG
			DumpMappedFileHashes();
#endif
			CCheatDetectQueueMgr::Instance().AppendDetection(MAPPED_FILE_VIOLATION, i, "");
			ExitByAnticheat(MAPPED_FILE_VIOLATION, i, 0, true);
		}
	}
}
