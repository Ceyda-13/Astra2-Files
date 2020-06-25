#include "stdafx.h"
#include "MarkManager.h"

#ifdef _WIN32
#include <direct.h>
#endif

#define OLD_MARK_INDEX_FILENAME "guild_mark.idx"
#define OLD_MARK_DATA_FILENAME "guild_mark.tga"

static Pixel * LoadOldGuildMarkImageFile()
{
	FILE * fp = fopen(OLD_MARK_DATA_FILENAME, "rb");

	if (!fp)
	{
		sys_err("cannot open %s", OLD_MARK_INDEX_FILENAME);
		return nullptr;
	}

	int32_t dataSize = 512 * 512 * sizeof(Pixel);
	Pixel * dataPtr = (Pixel *) malloc(dataSize);

	fread(dataPtr, dataSize, 1, fp);

	fclose(fp);

	return dataPtr;
}

bool GuildMarkConvert(const std::vector<uint32_t> & vecGuildID)
{
#ifndef _WIN32
	mkdir("mark", S_IRWXU);
#else
	_mkdir("mark");
#endif

#ifndef _WIN32
	if (0 != access(OLD_MARK_INDEX_FILENAME, F_OK))
#else
	if (0 != _access(OLD_MARK_INDEX_FILENAME, 0))
#endif
		return true;

	FILE* fp = fopen(OLD_MARK_INDEX_FILENAME, "r");

	if (!fp)
		return false;

	Pixel* oldImagePtr = LoadOldGuildMarkImageFile();

	if (!oldImagePtr)
	{
		fclose(fp);
		return false;
	}

	sys_log(0, "Guild Mark Converting Start.");

	char line[256];
	uint32_t guild_id;
	uint32_t mark_id;
	Pixel mark[SGuildMark::SIZE];

	while (fgets(line, sizeof(line) - 1, fp))
	{
		sscanf(line, "%u %u", &guild_id, &mark_id);

		if (find(vecGuildID.begin(), vecGuildID.end(), guild_id) == vecGuildID.end())
		{
			sys_log(0, "  skipping guild ID %u", guild_id);
			continue;
		}

		uint32_t row = mark_id / 32;
		uint32_t col = mark_id % 32;

		if (row >= 42)
		{
			sys_err("invalid mark_id %u", mark_id);
			continue;
		}

		uint32_t sx = col * 16;
		uint32_t sy = row * 12;

		Pixel* src = oldImagePtr + sy * 512 + sx;
		Pixel* dst = mark;

		for (int32_t y = 0; y != SGuildMark::HEIGHT; ++y)
		{
			for (int32_t x = 0; x != SGuildMark::WIDTH; ++x)
				*(dst++) = *(src+x);

			src += 512;
		}

		CGuildMarkManager::instance().SaveMark(guild_id, (uint8_t *) mark);
		line[0] = '\0';
	}

	free(oldImagePtr);
	fclose(fp);

#ifndef _WIN32
	system("mv -f guild_mark.idx guild_mark.idx.removable");
	system("mv -f guild_mark.tga guild_mark.tga.removable");
#else
	system("move /Y guild_mark.idx guild_mark.idx.removable");
	system("move /Y guild_mark.tga guild_mark.tga.removable");
#endif

	sys_log(0, "Guild Mark Converting Complete.");

	return true;
}

