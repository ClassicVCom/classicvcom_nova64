#ifndef _CVNE_HPP_
#define _CVNE_HPP_

#include "types.hpp"
#include <cstdint>
#include <array>

namespace ClassicVCom_Nova64
{
	struct CVNE_Header
	{
		DWord_LE magicword; // Must be CVNE
		struct
		{
			Word_LE major;
			Word_LE minor;
		} version;
		QWord_LE primary_code_region_file_address;
		DWord_LE primary_code_region_size;
		uint8_t primary_code_region_flags;
		std::array<uint8_t, 3> reserved_1;
		DWord_LE primary_stack_region_size;
	};

	struct CVNE_UserDefinedMemoryRegionData
	{
		QWord_LE file_address;
		DWord_LE size;
		uint8_t flags;
		std::array<uint8_t, 3> reserved_1;
	};

	struct CVNE_File
	{
		CVNE_Header header;
		std::array<CVNE_UserDefinedMemoryRegionData, 14> user_defined_memory_region_table;
	};
}

#endif
