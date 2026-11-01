/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "contentcatalog.h"

#include "contentsha256.h"

#include <cstdio>
#include <cstring>


namespace
{

ContentFile const Files[] = {
	{ "MULTI.MIX", 11960308, "d1512284ffb8186f499bc11af7874d8ebc7af5047ccdd52854fed1f966cc1b72", CONTENT_PART_BASE },
	{ "patch.mix", 606836, "ef0f70fe7d7ac002d73ed413b8d52923383ba4b9c80519aeed84b6860cc1d462", CONTENT_PART_BASE },
	{ "MAPS01.MIX", 3668236, "bdfb431276a84ab33ea6d49dfd66504ca96654d84a4ab92bb1fdc9910fbec608", CONTENT_PART_BASE },
	{ "MAPS02.MIX", 3503740, "4805410c63dabea61e237798fa6096826b947b67fb744c439fa02afbc8cd551d", CONTENT_PART_BASE },
	{ "SIDECD01.MIX", 12304212, "7ee166720d1a37e5d540bf3ae0bf3389b6de11c0ed748cdda46c2ca8f63e529f", CONTENT_PART_BASE },
	{ "SIDECD02.MIX", 11141636, "1565f74dced0abebcbda10610f429419fde3588613364057e22442889906a50a", CONTENT_PART_BASE },
	// The Steam release ships a different SCORES.MIX than the retail discs; either is accepted.
	{ "SCORES.MIX", 51332268, "52b01c40b49ad406d7c39ebff6f7ffcf2c7de33edb5e875a8ec29439a0bd8dab", CONTENT_PART_BASE, "f82eabd8cccabdaf82ebcdaa26d8effc71a7d3caa3246ec1ff4f4b751de091af" },
	{ "e01scd01.mix", 350116, "43f8e07cdb7590ab1434c0525381e908de8bf117af64e131a482cfdb317acfcf", CONTENT_PART_FIRESTORM },
	{ "e01scd02.mix", 275012, "eba1c52243879f25e66c32de9ecc68886b0caf8a4a2b56fffad9bbd7bd2c349f", CONTENT_PART_FIRESTORM },
	{ "expand01.mix", 36972996, "f1f37a7a9c14bc32dcd308474dcfa9ab7736d57b2a7cbf6e6be1d4d298af6cec", CONTENT_PART_FIRESTORM },
	{ "expand02.mix", 88146260, "42e1d13748a05076bbedc5991e952a8b81ec60d5bb176303f787c1d82f4ce823", CONTENT_PART_FIRESTORM },
	{ "expand03.mix", 466084477, "8bbfae4e2638f3f41c9cc54981f45d0b8e436558b8a9264be71712372d6b841f", CONTENT_PART_FIRESTORM },
	{ "GMENU.MIX", 3911452, "52ffd2e6c92799576dbb78857885e75d45723c8fcf7de8001e72908a8750e728", CONTENT_PART_FIRESTORM },
	{ "maps03.mix", 3714132, "a915c125f68a6a293e6badfd1badedc33f54c1792e83ce29fb832eb3d1b790f2", CONTENT_PART_FIRESTORM },
	{ "SCORES01.MIX", 20079580, "7d1113f96deb60f4903410a9202c32453ab07d0d6dcba0353585b6d4e0725e97", CONTENT_PART_FIRESTORM },
	{ "wdt.mix", 11806764, "3e3624c097939050ceba92af4762895e77c28cafc166130c4b3839be8b29b82f", CONTENT_PART_FIRESTORM },
	{ "wdtvox.mix", 2800932, "444e0b8773a2b24e2d041c04638c64c1434ccb2d7a9593d47fdec5f253282267", CONTENT_PART_FIRESTORM },
	{ "MOVIES01.MIX", 489980956, "e34eb5d34caa0084b95ba04380ff0a9aa9f7906936833ca0e1d6f2b943ebd1f2", CONTENT_PART_MOVIES },
	{ "MOVIES02.MIX", 490656380, "64543f6afaf850f8fb7584e3813ece5ad92f2cab4f48613b349931506dd98462", CONTENT_PART_MOVIES },
	{ "SIZZLE1.VQA", 11340738, "2cea6ee021945dbc18caa4d22dc23ccda48c63f69888f450ba38594205ef203b", CONTENT_PART_MOVIES },
	{ "movies03.mix", 462369668, "fb481dbd306e448fb911357d3cd5ae946ab9171381c40b48286fd6616c9f0c26", CONTENT_PART_FIRESTORM_MOVIES },
	{ "TIBSUN.MIX", 75094736, "23431290b211cd039315095e1987f1c411ecd3b6440861c4874572c92e5f6b2a", CONTENT_PART_BASE },
};


std::uint64_t Part_Bytes(ContentPart part)
{
	std::uint64_t total = 0;
	for (ContentFile const & file : Files) {
		if (file.Part == part) {
			total += file.Size;
		}
	}
	return(total);
}

}


std::span<ContentFile const> Content_Files(void)
{
	return(std::span<ContentFile const>(Files, sizeof(Files) / sizeof(Files[0])));
}


char const * Content_Download_Root(void)
{
	return("https://siliconsun-content.fjlaubscher.workers.dev");
}


char const * Content_Download_Key(void)
{
	return("7f0236d1ca66d6cd177139dfce6f3dd68b14ed35a82b27204d82d5bb369ec9c2");
}


std::string Content_Download_Host(void)
{
	std::string host = Content_Download_Root();
	if (host.rfind("https://", 0) == 0) {
		host.erase(0, 8);
	} else if (host.rfind("http://", 0) == 0) {
		host.erase(0, 7);
	}
	while (!host.empty() && host.back() == '/') {
		host.pop_back();
	}
	return(host);
}


bool Content_File_Selected(ContentFile const & file, bool tsmovies, bool firestorm, bool fsmovies)
{
	switch (file.Part) {
		case CONTENT_PART_BASE:
			return(true);
		case CONTENT_PART_MOVIES:
			return(tsmovies);
		case CONTENT_PART_FIRESTORM:
			return(firestorm);
		case CONTENT_PART_FIRESTORM_MOVIES:
			return(fsmovies);
	}
	return(false);
}


bool Content_File_Matches(ContentFile const & file, unsigned char const digest[32])
{
	if (Content_Sha256_Matches(file.Sha256, digest)) {
		return(true);
	}
	return(file.Sha256Alt != nullptr && Content_Sha256_Matches(file.Sha256Alt, digest));
}


std::uint64_t Content_Base_Bytes(void)
{
	return(Part_Bytes(CONTENT_PART_BASE));
}


std::uint64_t Content_TsMovies_Marginal(void)
{
	return(Part_Bytes(CONTENT_PART_MOVIES));
}


std::uint64_t Content_Firestorm_Marginal(void)
{
	return(Part_Bytes(CONTENT_PART_FIRESTORM));
}


std::uint64_t Content_FsMovies_Marginal(void)
{
	return(Part_Bytes(CONTENT_PART_FIRESTORM_MOVIES));
}


std::uint64_t Content_Download_Bytes(bool tsmovies, bool firestorm, bool fsmovies)
{
	std::uint64_t total = Part_Bytes(CONTENT_PART_BASE);
	if (tsmovies) {
		total += Part_Bytes(CONTENT_PART_MOVIES);
	}
	if (firestorm) {
		total += Part_Bytes(CONTENT_PART_FIRESTORM);
	}
	if (fsmovies) {
		total += Part_Bytes(CONTENT_PART_FIRESTORM_MOVIES);
	}
	return(total);
}


std::string Content_Size_Text(std::uint64_t bytes)
{
	char buffer[32];
	double const megabytes = (double)bytes / (1024.0 * 1024.0);
	if (megabytes >= 1024.0) {
		std::snprintf(buffer, sizeof(buffer), "%.1f GB", megabytes / 1024.0);
	} else {
		std::snprintf(buffer, sizeof(buffer), "%.1f MB", megabytes);
	}
	return(buffer);
}


std::string Content_Speed_Text(double bytes_per_second)
{
	if (bytes_per_second < 0.0) {
		bytes_per_second = 0.0;
	}
	return(Content_Size_Text((std::uint64_t)(bytes_per_second + 0.5)) + "/s");
}
