#ifndef _BITS_HPP_
#define _BITS_HPP_

#include <concepts>
#include <cstdint>

namespace ClassicVCom_Nova64
{
	template <std::integral T>
	consteval T GenerateFieldBitmask(uint16_t bit_offset, uint16_t size)
	{
		if (!size)
		{
			return 0;
		}
		T bitmask = 0x00;
		std::size_t bit_check = bit_offset + size;
		std::size_t type_bit_count = sizeof(T) * 8;
		if (bit_offset >= type_bit_count)
		{
			return 0;
		}
		if (bit_check > type_bit_count)
		{
			size -= bit_check - type_bit_count;
		}
		for (uint16_t i = 0; i < size; ++i)
		{
			bitmask |= static_cast<T>(0x01) << (bit_offset + i);
		}
		return bitmask;
	}

	template <std::integral T>
	consteval T GenerateByteBitmask(std::size_t byte_offset)
	{
		if (byte_offset >= sizeof(T))
		{
			return 0;
		}
		return (static_cast<T>(0xFF) << (byte_offset << 3));
	}
}

#endif
