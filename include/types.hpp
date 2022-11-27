#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include "bits.hpp"
#include <cstdint>
#include <bit>
#include <concepts>
#include <type_traits>

namespace ClassicVCom_Nova64
{
	template <typename T>
	concept WordCompatible = std::convertible_to<T, uint16_t>;

	template <typename T>
	concept DWordCompatible = std::convertible_to<T, uint32_t>;

	template <typename T>
	concept QWordCompatible = std::convertible_to<T, uint64_t>;

	template <typename T>
	concept WordMinimumRequired = (sizeof(T) >= sizeof(uint16_t)) && WordCompatible<T>;

	template <typename T>
	concept DWordMinimumRequired = (sizeof(T) >= sizeof(uint32_t)) && DWordCompatible<T>;

	template <typename T>
	concept QWordMinimumRequired = (sizeof(T) >= sizeof(uint64_t)) && QWordCompatible<T>;

	template <typename T>
	concept WordMaximumRequired = (sizeof(T) <= sizeof(uint16_t)) && WordCompatible<T>;

	template <typename T>
	concept DWordMaximumRequired = (sizeof(T) <= sizeof(uint32_t)) && DWordCompatible<T>;

	template <typename T>
	concept QWordMaximumRequired = (sizeof(T) <= sizeof(uint64_t)) && QWordCompatible<T>;
	
	template <typename T>
	concept WordRequired = (sizeof(T) == sizeof(uint16_t)) && WordCompatible<T>;

	template <typename T>
	concept DWordRequired = (sizeof(T) == sizeof(uint32_t)) && DWordCompatible<T>;

	template <typename T>
	concept QWordRequired = (sizeof(T) == sizeof(uint64_t)) && QWordCompatible<T>;

	template <typename T>
	concept WordAlignmentRequired = (sizeof(T) % sizeof(uint16_t) == 0);

	template <typename T>
	concept DWordAlignmentRequired = (sizeof(T) % sizeof(uint32_t) == 0);

	template <typename T>
	concept QWordAlignmentRequired = (sizeof(T) % sizeof(uint64_t) == 0);

	template <typename T>
	concept ByteDataRequired = requires(T obj)
	{
		{ obj.data } -> std::convertible_to<uint32_t>;
	};

	template <DWordCompatible T, std::endian Endianness>
	struct ByteData
	{
		T data;

		ByteData() : data(0)
		{
		}

		constexpr ByteData(T val)
		{
			data = val;
		}

		template <ByteDataRequired T2>
		constexpr ByteData(const T2 &val)
		{
			data = val.data;
		}

		/*
		template <ByteDataRequired T2>
		constexpr ByteData &operator=(const T2 &val)
		{
			data = val.data;
			return *this;
		}
		*/

		constexpr operator T() const
		{
			return data;
		}

		constexpr void operator++()
		{
			*this = *this + 1;
		}

		constexpr ByteData<T, Endianness> operator++(int)
		{
			ByteData<T, Endianness> result = *this;
			*this = *this + 1;
			return result;
		}
		
		constexpr void operator--()
		{
			*this = *this - 1;
		}

		constexpr ByteData<T, Endianness> operator--(int)
		{
			ByteData<T, Endianness> result = *this;
			*this = *this - 1;
			return result;
		}

		template <DWordCompatible T2>
		constexpr void operator+=(T2 val)
		{
			*this = *this + val;
		}

		template <DWordCompatible T2>
		constexpr void operator-=(T2 val)
		{
			*this = *this - val;
		}
		
		template <DWordCompatible T2>
		constexpr void operator*=(T2 val)
		{
			*this = *this * val;
		}

		template <DWordCompatible T2>
		constexpr void operator/=(T2 val)
		{
			*this = *this / val;
		}

		template <DWordCompatible T2>
		constexpr void operator%=(T2 val)
		{
			*this = *this % val;
		}

		template <DWordCompatible T2>
		constexpr void operator<<=(T2 shift)
		{
			*this = *this << shift;
		}

		template <DWordCompatible T2>
		constexpr void operator>>=(T2 shift)
		{
			*this = *this >> shift;
		}

		template <DWordCompatible T2>
		constexpr void operator|=(T2 val)
		{
			*this = *this | val;
		}

		template <DWordCompatible T2>
		constexpr void operator&=(T2 val)
		{
			*this = *this & val;
		}

		template <DWordCompatible T2>
		constexpr void operator^=(T2 val)
		{
			*this = *this ^ val;
		}
	};

	template <std::endian Endianness>
	using Word = ByteData<uint16_t, Endianness>;

	template <std::endian Endianness>
	using DWord = ByteData<uint32_t, Endianness>;

	template <std::endian Endianness>
	using QWord = ByteData<uint64_t, Endianness>;

	using Word_Native = Word<std::endian::native>;
	using DWord_Native = DWord<std::endian::native>;
	using QWord_Native = QWord<std::endian::native>;

	using Word_LE = Word<std::endian::little>;
	using DWord_LE = DWord<std::endian::little>;
	using QWord_LE = QWord<std::endian::little>;

	using Word_BE = Word<std::endian::big>;
	using DWord_BE = DWord<std::endian::big>;
	using QWord_BE = QWord<std::endian::big>;

	template <>
	constexpr Word_LE::ByteData(uint16_t val)
	{
		data = (std::endian::little == std::endian::native) ? val : ((val >> 8) | (val << 8));
	}

	template <> template <>
	constexpr Word_LE::ByteData(const DWord_LE &val)
	{
		data = (std::endian::little == std::endian::native) ? val.data : (val.data >> 16);
	}

	template <> template <>
	constexpr Word_LE::ByteData(const QWord_LE &val)
	{
		data = (std::endian::little == std::endian::native) ? val.data : (val.data >> 32);
	}

	template <> template <>
	constexpr Word_LE::ByteData(const Word_BE &val)
	{
		data = ((val.data >> 8) | (val.data << 8));
	}

	template <> template <>
	constexpr Word_LE::ByteData(const DWord_BE &val)
	{
		data = (std::endian::little == std::endian::native) ? ((val.data >> 24) | ((val.data & 0xFF0000) >> 8)) : (((val.data & 0xFF00) >> 8) | ((val.data & 0xFF) << 8));
	}

	template <> template <>
	constexpr Word_LE::ByteData(const QWord_BE &val)
	{
		data = (std::endian::little == std::endian::native) ? ((val.data >> 56) | ((val.data & GenerateByteBitmask<uint64_t>(6)) >> 40)) : (((val.data & 0xFF00) >> 8) | ((val.data & 0xFF) << 8));
	}

	template <>
	constexpr DWord_LE::ByteData(uint32_t val)
	{
		data = (std::endian::little == std::endian::native) ? val : ((val >> 24) | ((val & 0xFF0000) >> 8) | ((val & 0xFF00) << 8) | (val << 24));
	}

	template <> template <>
	constexpr DWord_LE::ByteData(const Word_LE &val)
	{
		data = (std::endian::little == std::endian::native) ? val.data : (val.data << 16);
	}

	template <> template <>
	constexpr DWord_LE::ByteData(const QWord_LE &val)
	{
		data = (std::endian::little == std::endian::native) ? val.data : (val.data >> 32);
	}

	template <> template <>
	constexpr DWord_LE::ByteData(const Word_BE &val)
	{
		data = (std::endian::little == std::endian::native) ? (((val.data & 0xFF00) >> 8) | ((val.data & 0xFF) << 8)) : (((val.data & 0xFF00) << 8) | ((val.data & 0xFF) << 24));
	}

	template <> template <>
	constexpr DWord_LE::ByteData(const DWord_BE &val)
	{
		data = ((val.data >> 24) | ((val.data & 0xFF0000) >> 8) | ((val.data & 0xFF00) << 8) | (val.data << 24));
	}

	template <> template <>
	constexpr DWord_LE::ByteData(const QWord_BE &val)
	{
		data = (std::endian::little == std::endian::native) ? ((val.data >> 56) | ((val.data & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((val.data & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((val.data & GenerateByteBitmask<uint64_t>(4)) >> 8)) : (((val.data & 0xFF000000) >> 24) | ((val.data & 0xFF0000) >> 8) | ((val.data & 0xFF00) << 8) | ((val.data & 0xFF) << 24));
	}

	template <>
	constexpr QWord_LE::ByteData(uint64_t val)
	{
		data = (std::endian::little == std::endian::native) ? val : ((val >> 56) | ((val & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((val & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((val & GenerateByteBitmask<uint64_t>(4)) >> 8) | ((val & GenerateByteBitmask<uint64_t>(3)) << 8) | ((val & GenerateByteBitmask<uint64_t>(2)) << 24) | ((val & GenerateByteBitmask<uint64_t>(1)) << 40) | (val << 56));
	}

	template <> template <>
	constexpr QWord_LE::ByteData(const Word_LE &val)
	{
		data = (std::endian::little == std::endian::native) ? val.data : (static_cast<uint64_t>(val.data) << 48);
	}

	template <> template <>
	constexpr QWord_LE::ByteData(const DWord_LE &val)
	{
		data = (std::endian::little == std::endian::native) ? val.data : (static_cast<uint64_t>(val.data) << 32);
	}

	template <> template <>
	constexpr QWord_LE::ByteData(const Word_BE &val)
	{
		data = (std::endian::little == std::endian::native) ? ((val.data >> 8) | (val.data << 8)) : ((static_cast<uint64_t>(val.data & 0xFF00) << 40) | ((static_cast<uint64_t>(val.data & 0xFF) << 56)));
	}

	template <> template <>
	constexpr QWord_LE::ByteData(const DWord_BE &val)
	{
		data = (std::endian::little == std::endian::native) ? ((val.data >> 24) | ((val.data & 0xFF0000) >> 8) | ((val.data & 0xFF00) << 8) | (val.data << 24)) : ((static_cast<uint64_t>(val.data & 0xFF000000) << 8) | (static_cast<uint64_t>(val.data & 0xFF0000) << 24) | (static_cast<uint64_t>(val.data & 0xFF00) << 40) | (static_cast<uint64_t>(val.data) << 56));
	}

	template <> template <>
	constexpr QWord_LE::ByteData(const QWord_BE &val)
	{
		data = ((val.data >> 56) | ((val.data & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((val.data & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((val.data & GenerateByteBitmask<uint64_t>(4)) >> 8) | ((val.data & GenerateByteBitmask<uint64_t>(3)) << 8) | ((val.data & GenerateByteBitmask<uint64_t>(2)) << 24) | ((val.data & GenerateByteBitmask<uint64_t>(1)) << 40) | (val.data << 56));
	}

	template <>
	constexpr Word_BE::ByteData(uint16_t val)
	{
		data = (std::endian::big == std::endian::native) ? val : ((val >> 8) | (val << 8));
	}

	template <> template <>
	constexpr Word_BE::ByteData(const Word_LE &val)
	{
		data = ((val.data >> 8) | (val.data << 8));
	}

	template <> template <>
	constexpr Word_BE::ByteData(const DWord_LE &val)
	{
		data = (std::endian::big == std::endian::native)  ? ((val.data >> 24) | ((val.data & 0xFF0000) >> 8)) : (((val.data & 0xFF00) >> 8) | ((val.data & 0xFF) << 8));
	}

	template <> template <>
	constexpr Word_BE::ByteData(const QWord_LE &val)
	{
		data = (std::endian::big == std::endian::native) ? ((val.data >> 56) | ((val.data & GenerateByteBitmask<uint64_t>(6)) >> 40)) : (((val.data & 0xFF00) >> 8) | ((val.data & 0xFF) << 8));
	}

	template <> template <>
	constexpr Word_BE::ByteData(const DWord_BE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : (val.data >> 16);
	}

	template <> template <>
	constexpr Word_BE::ByteData(const QWord_BE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : (val.data >> 32);
	}

	template <>
	constexpr DWord_BE::ByteData(uint32_t val)
	{
		data = (std::endian::big == std::endian::native) ? val : ((val >> 24) | ((val & 0xFF0000) >> 8) | ((val & 0xFF00) << 8) | (val << 24));
	}

	template <> template <>
	constexpr DWord_BE::ByteData(const Word_LE &val)
	{
		data = (std::endian::big == std::endian::native) ? (((val.data & 0xFF00) >> 8) | ((val.data & 0xFF) << 8)) : (((val.data & 0xFF00) << 8) | ((val.data & 0xFF) << 24));
	}

	template <> template <>
	constexpr DWord_BE::ByteData(const DWord_LE &val)
	{
		data = ((val.data >> 24) | ((val.data & 0xFF0000) >> 8) | ((val.data & 0xFF00) << 8) | (val.data << 24));
	}

	template <> template <>
	constexpr DWord_BE::ByteData(const QWord_LE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : ((val.data >> 56) | ((val.data & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((val.data & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((val.data & GenerateByteBitmask<uint64_t>(4)) >> 8) | ((val.data & 0xFF000000) >> 24) | ((val.data & 0xFF0000) >> 8) | ((val.data & 0xFF00) << 8) | ((val.data & 0xFF) << 24));
	}

	template <> template <>
	constexpr DWord_BE::ByteData(const Word_BE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : (val.data << 16);
	}

	template <> template <>
	constexpr DWord_BE::ByteData(const QWord_BE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : (val.data >> 32);
	}

	template <>
	constexpr QWord_BE::ByteData(uint64_t val)
	{
		data = (std::endian::big == std::endian::native) ? val : ((val >> 56) | ((val & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((val & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((val & GenerateByteBitmask<uint64_t>(4)) >> 8) | ((val & GenerateByteBitmask<uint64_t>(3)) << 8) | ((val & GenerateByteBitmask<uint64_t>(2)) << 24) | ((val & GenerateByteBitmask<uint64_t>(1)) << 40) | (val << 56));
	}

	template <> template <>
	constexpr QWord_BE::ByteData(const Word_LE &val)
	{
		data = (std::endian::big == std::endian::native) ? ((val.data >> 8) | (val.data << 8)) : ((static_cast<uint64_t>(val.data & 0xFF00) << 40) | ((static_cast<uint64_t>(val.data & 0xFF) << 56)));
	}

	template <> template <>
	constexpr QWord_BE::ByteData(const DWord_LE &val)
	{
		data = (std::endian::big == std::endian::native) ? ((val.data >> 24) | ((val.data & 0xFF0000) >> 8) | ((val.data & 0xFF00) << 8) | (val.data << 24)) : ((static_cast<uint64_t>(val.data & 0xFF000000) << 8) | (static_cast<uint64_t>(val.data & 0xFF0000) << 24) | (static_cast<uint64_t>(val.data & 0xFF00) << 40) | (static_cast<uint64_t>(val.data) << 56));
	}

	template <> template <>
	constexpr QWord_BE::ByteData(const QWord_LE &val)
	{
		data = ((val.data >> 56) | ((val.data & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((val.data & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((val.data & GenerateByteBitmask<uint64_t>(3)) >> 8) | ((val.data & GenerateByteBitmask<uint64_t>(3)) << 8) | ((val.data & GenerateByteBitmask<uint64_t>(2)) << 24) | ((val.data & GenerateByteBitmask<uint64_t>(1)) << 40) | (val.data << 56));
	}

	template <> template <>
	constexpr QWord_BE::ByteData(const Word_BE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : (static_cast<uint64_t>(val.data) << 48);
	}

	template <> template <>
	constexpr QWord_BE::ByteData(const DWord_BE &val)
	{
		data = (std::endian::big == std::endian::native) ? val.data : (static_cast<uint64_t>(val.data) << 32);
	}

	template <>
	constexpr Word_LE::operator uint16_t() const
	{
		return (std::endian::little == std::endian::native) ? data : ((data >> 8) | (data << 8));
	}

	template <>
	constexpr DWord_LE::operator uint32_t() const
	{
		return (std::endian::little == std::endian::native) ? data : ((data >> 24) | ((data & 0xFF0000) >> 8) | ((data & 0xFF00) << 8) | (data << 24));
	}

	template <>
	constexpr QWord_LE::operator uint64_t() const
	{
		return (std::endian::little == std::endian::native) ? data : ((data >> 56) | ((data & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((data & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((data & GenerateByteBitmask<uint64_t>(4)) >> 8) | ((data & GenerateByteBitmask<uint64_t>(3)) << 8) | ((data & GenerateByteBitmask<uint64_t>(2)) << 24) | ((data & GenerateByteBitmask<uint64_t>(1)) << 40) | (data << 56));
	}

	template <>
	constexpr Word_BE::operator uint16_t() const
	{
		return (std::endian::big == std::endian::native) ? data : ((data >> 8) | (data << 8));
	}

	template <>
	constexpr DWord_BE::operator uint32_t() const
	{
		return (std::endian::big == std::endian::native) ? data : ((data >> 24) | ((data & 0xFF0000) >> 8) | ((data & 0xFF00) << 8) | (data << 24));
	}

	template <>
	constexpr QWord_BE::operator uint64_t() const
	{
		return (std::endian::big == std::endian::native) ? data : ((data >> 56) | ((data & GenerateByteBitmask<uint64_t>(6)) >> 40) | ((data & GenerateByteBitmask<uint64_t>(5)) >> 24) | ((data & GenerateByteBitmask<uint64_t>(4)) >> 8) | ((data & GenerateByteBitmask<uint64_t>(3)) << 8) | ((data & GenerateByteBitmask<uint64_t>(2)) << 24) | ((data & GenerateByteBitmask<uint64_t>(1)) << 40) | (data << 56));
	}
}

#endif
