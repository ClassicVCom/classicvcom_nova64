#ifndef _OS_HPP_
#define _OS_HPP_

#include "types.hpp"
#include "cpu.hpp"
#include <array>
#include <vector>
#include <string>

namespace ClassicVCom_Nova64
{
	struct Program;

	struct ProcessState
	{
		// GPR_Data GPR_Registers;
		std::array<QWord_LE, 8> GPR_Registers;
		IndexRegisterData SI;
		IndexRegisterData DI;
		QWord_LE FL;
		MPRegisterData IP;
		MPRegisterData BP;
		MPRegisterData SP;
	};

	struct Process
	{
		std::string name;
		Program *program;
		ProcessState state;
	};

	class Motherboard;

	class Kernel
	{
		public:
			Kernel(Motherboard &motherboard);
			~Kernel();
			bool CreateProcess();
			void DestroyProcess(Word_LE program_id, QWord_LE return_code);
			void LoadProcessState(Word_LE program_id);
			void SaveProcessState(Word_LE program_id);
		private:
			Motherboard &CurrentMotherboard;
			std::vector<Process> ProcessList;
	};
}

#endif
