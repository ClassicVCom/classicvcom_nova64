#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include <array>
#include <concepts>

namespace ClassicVCom_Nova64
{
	template <std::integral T, size_t size>
	consteval std::array<T, size> GenerateIdentityMap()
	{
		std::array<T, size> data;
		for (size_t i = 0; i < size; ++i)
		{
			data[i] = i;
		}
		return data;
	}
};

#endif
