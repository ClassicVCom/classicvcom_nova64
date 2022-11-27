#ifndef _SYSCALL_HPP_
#define _SYSCALL_HPP_

#include "types.hpp"

namespace ClassicVCom_Nova64
{
	enum class SystemCallType : uint16_t
	{
		OpenClassicVGOS = 0,
		ExecuteChildProgram = 16,
		ExitCurrentProgram = 17,
		LoadFilesystem = 64
	};
}

#endif
