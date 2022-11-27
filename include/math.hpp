#ifndef _CVCOM_NOVA64_MATH_
#define _CVCOM_NOVA64_MATH_

#include "types.hpp"
#include "cpu.hpp"
#include "instructions.hpp"
#include <concepts>

namespace ClassicVCom_Nova64
{
	template <DWordCompatible T, std::signed_integral T2> requires (sizeof(T) == sizeof(T2))
	inline void PerformAddition(T &accumulator, T &operand, QWord_LE &flags, bool signed_mode, bool carry_mode)
	{
		if (!signed_mode)
		{
			T tmp = accumulator + operand;
			if (carry_mode && (flags & 0x02))
			{
				++tmp;
			}
			flags &= ~(0x0B);
			if (tmp < accumulator)
			{
				flags |= 0x0A;
			}
			if (!tmp)
			{
				flags |= 0x01;
			}
			accumulator = tmp;
		}
		else
		{
			if (carry_mode)
			{
				return;
			}
			T2 tmp = static_cast<T2>(accumulator) + static_cast<T2>(operand);
			flags &= ~(0x09);
			if (tmp < static_cast<T2>(accumulator))
			{
				flags |= 0x08;
			}
			if (!tmp)
			{
				flags |= 0x01;
			}
			accumulator = static_cast<T>(tmp);
		}
	}

	template <DWordCompatible T, std::signed_integral T2> requires (sizeof(T) == sizeof(T2))
	inline void PerformSubtraction(T &accumulator, T &operand, QWord_LE &flags, bool signed_mode, bool borrow_mode)
	{
		if (!signed_mode)
		{
			T tmp = accumulator - operand;
			if (borrow_mode && (flags & 0x02))
			{
				--tmp;
			}
			flags &= ~(0x15);
			if (tmp > accumulator)
			{
				flags |= 0x14;
			}
			if (!tmp)
			{
				flags |= 0x01;
			}
			accumulator = tmp;
		}
		else
		{
			if (borrow_mode)
			{
				return;
			}
			T2 tmp = static_cast<T2>(accumulator) - static_cast<T2>(operand);
			flags &= ~(0x11);
			if (tmp > static_cast<T2>(accumulator))
			{
				flags |= 0x10;
			}
			if (!tmp)
			{
				flags |= 0x01;
			}
			accumulator = static_cast<T>(tmp);
		}
	}
}

#endif
