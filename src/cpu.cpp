#include "motherboard.hpp"
#include "bits.hpp"
#include "system.hpp"
// #include "instructions_inline.hpp"
#include "instructions_templates.hpp"
#include <fmt/core.h>
#include <cstdint>
#include <cstring>
#include <cmath>

/*
consteval ClassicVCom_Nova64::InstructionFunctionTableArray ClassicVCom_Nova64::CreateInstructionFunctionTable()
{
	InstructionFunctionTableArray table;
	for (size_t i = 0; i < table.size(); ++i)
	{
		table[i] = { &Instruction::InvalidInstruction, "Fetched an Invalid Instruction" };
	}
	table[static_cast<uint16_t>(InstructionType::NoOperation)] = { &Instruction::NoOperation, "Fetched a No Operation Instruction" };
	table[static_cast<uint16_t>(InstructionType::SystemCall)] = { &Instruction::SystemCall, "Fetched a System Call Instruction" };
	table[static_cast<uint16_t>(InstructionType::InterruptReturn)] = { &Instruction::InterruptReturn, "Fetched an Interrupt Return Instruction" };
	table[static_cast<uint16_t>(InstructionType::Push)] = { &Instruction::Push, "Fetched a Push Instruction" };
	table[static_cast<uint16_t>(InstructionType::Pop)] = { &Instruction::Pop, "Fetched a Pop Instruction" };
	table[static_cast<uint16_t>(InstructionType::Move)] = { &Instruction::Move, "Fetched a Move Instruction" };
	table[static_cast<uint16_t>(InstructionType::Compare)] = { &Instruction::Compare, "Fetched a Compare Instruction" };
	table[static_cast<uint16_t>(InstructionType::Jump)] = { &Instruction::Jump, "Fetched a Jump Instruction" };
	table[static_cast<uint16_t>(InstructionType::Add)] = { &Instruction::AddSubtract<PerformAddition<uint8_t, int8_t>, PerformAddition<Word_LE, int16_t>, PerformAddition<DWord_LE, int32_t>, PerformAddition<QWord_LE, int64_t>>, "Fetched an Add Instruction" };
	table[static_cast<uint16_t>(InstructionType::Subtract)] = { &Instruction::AddSubtract<PerformSubtraction<uint8_t, int8_t>, PerformSubtraction<Word_LE, int16_t>, PerformSubtraction<DWord_LE, int32_t>, PerformSubtraction<QWord_LE, int64_t>>, "Fetched a Subtract Instruction" };
	table[static_cast<uint16_t>(InstructionType::IncrementDecrement)] = { &Instruction::IncrementDecrement, "Fetched an Increment/Decrement Instruction" };
	table[static_cast<uint16_t>(InstructionType::SetClear)] = { &Instruction::SetClear, "Fetched a Set/Clear Instruction" };
	table[static_cast<uint16_t>(InstructionType::ChipCall)] = { &Instruction::ChipCall, "Fetched a Virtual Chipset Function Call Instruction" };
	table[static_cast<uint16_t>(InstructionType::ShadowFetchAndExecute)] = { &Instruction::ShadowFetchAndExecute, "Fetched a Shadow Fetch and Execute Instruction" };
	return table;
}
*/

consteval ClassicVCom_Nova64::InstructionBaseCallbackTableArray ClassicVCom_Nova64::CreateInstructionBaseCallbackTable()
{
	InstructionBaseCallbackTableArray table;
	for (size_t i = 0; i < table.size(); ++i)
	{
		table[i] = { Instruction::InvalidInstructionType::ExecuteCycle, "Fetched an Invalid Instruction" };
	}
	table[static_cast<uint16_t>(InstructionType::NoOperation)] = { Instruction::NoOperationInstruction::ExecuteCycle, "Fetched a No Operation Instruction" };
	table[static_cast<uint16_t>(InstructionType::SystemCall)] = { Instruction::SystemCallInstruction::ExecuteCycle, "Fetched a System Call Instruction" };
	table[static_cast<uint16_t>(InstructionType::InterruptReturn)] = { Instruction::InterruptReturnInstruction::ExecuteCycle_1, "Fetched an Interrupt Return Instruction" };
	table[static_cast<uint16_t>(InstructionType::Push)] =  { Instruction::PushInstruction::Base_ExecuteCycle, "Fetched a Push Instruction" };
	table[static_cast<uint16_t>(InstructionType::Pop)] = { Instruction::PopInstruction::Base_ExecuteCycle, "Fetched a Pop Instruction" };
	table[static_cast<uint16_t>(InstructionType::Move)] = { Instruction::MoveInstruction::Base_ExecuteCycle, "Fetched a Move Instruction" };
	table[static_cast<uint16_t>(InstructionType::Compare)] = { Instruction::CompareInstruction::Base_ExecuteCycle, "Fetched a Compare Instruction" };
	table[static_cast<uint16_t>(InstructionType::Jump)] = { Instruction::JumpInstruction::Base_ExecuteCycle, "Fetched a Jump Instruction" };
	table[static_cast<uint16_t>(InstructionType::Add)] = { Instruction::AddInstruction::Base_ExecuteCycle, "Fetched an Add Instruction" };
	table[static_cast<uint16_t>(InstructionType::Subtract)] = { Instruction::SubtractInstruction::Base_ExecuteCycle, "Fetched a Subtract Instruction" };
	table[static_cast<uint16_t>(InstructionType::IncrementDecrement)] = { Instruction::IncrementDecrementInstruction::Base_ExecuteCycle, "Fetched an Increment/Decrement Instruction" };
	table[static_cast<uint16_t>(InstructionType::SetClear)] = { Instruction::SetClearInstruction::Flags1_ExecuteCycle, "Fetched a Set/Clear Instruction" };
	table[static_cast<uint16_t>(InstructionType::ChipCall)] = { Instruction::ChipCallInstruction::ExecuteCycle, "Fetched a Virtual Chipset Function Call Instruction" };
	table[static_cast<uint16_t>(InstructionType::ShadowFetchAndExecute)] = { Instruction::ShadowFetchAndExecuteInstruction::ExecuteCycle_1, "Fetched a Shadow Fetch and Execute Instruction" };
	return table;
}

ClassicVCom_Nova64::CPU::CPU(double cycles_per_second, Motherboard *CurrentMotherboard) : current_cycle_state(CycleState::Fetch), current_instruction_callback{ *this, std::array<QWord_LE, 4> { 0, 0, 0, 0 }, std::vector<QWord_LE>(0), nullptr }, cycle_accumulator(0.0), running(false), CurrentMotherboard(CurrentMotherboard), ProgramVectorTable(4096), /* instruction_function_table(CreateInstructionFunctionTable()), */ instruction_base_callback_table(CreateInstructionBaseCallbackTable())
{
	if (cycles_per_second < 1.0)
	{
		cycles_per_second = 1.0;
	}
	this->cycles_per_second = cycles_per_second;
	this->cycle_rate = 1.0 / cycles_per_second;
}

ClassicVCom_Nova64::CPU::~CPU()
{
}

void ClassicVCom_Nova64::CPU::SetupProgramVectorTables()
{
	for (size_t i = 0; i < 4096; ++i)
	{
		ProgramVectorTable[i].descriptor = &CurrentMotherboard->program_descriptor_table[i];
	}
}

void ClassicVCom_Nova64::CPU::RunCycles()
{
	static double lowest_process_time = 0.0;
	static double highest_process_time = 0.0;
	static double average_process_time = 0.0;
	static double times_processed = 0.0;

	/*
	static double lowest_mips_rate = 0.0;
	static double highest_mips_rate = 0.0;
	static double average_mips_rate = 0.0;
	static double instructions_processed = 0.0;
	static double current_cycle_count = 0.0;
	*/

	std::chrono::high_resolution_clock::time_point current_tp = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> cycle_time = current_tp - cycle_tp;
	if (cycle_time.count() > 0.25)
	{
		cycle_time = std::chrono::duration<double>(0.25);
	}
	cycle_accumulator += cycle_time.count();
	cycle_tp = current_tp;
	if (cycle_accumulator >= cycle_rate)
	{
		uint32_t cycles = static_cast<uint32_t>(cycle_accumulator / cycle_rate);
		// uint32_t cycles_processed = 0;
		cycle_accumulator -= static_cast<double>(cycles) * cycle_rate;
		// fmt::print("Cycles Acquired:  {}\n", cycles);
		// for (; cycles_processed < cycles; )
		Timer &MainTimer = CurrentMotherboard->MainTimer;
		for (uint32_t cycles_processed = 0; cycles_processed < cycles; ++cycles_processed)
		{
			/*
			Word_LE current_program_id = (IP.memory_control & 0xFFF);
			uint8_t current_region_id = ((IP.memory_control & 0xF000) >> 12);
			*/
			// MainTimer.RunTimers(*this);
			MainTimer.RunTimer<0>(*this);
			MainTimer.RunTimer<1>(*this);
			/*
			MainTimer.RunTimer<2>(*this);
			MainTimer.RunTimer<3>(*this);
			MainTimer.RunTimer<4>(*this);
			MainTimer.RunTimer<5>(*this);
			MainTimer.RunTimer<6>(*this);
			MainTimer.RunTimer<7>(*this);
			*/
			switch (current_cycle_state)
			{
				case CycleState::Fetch:
				{
					// current_cycle_count += 1.0;
					bool interrupt = false;
					SR &= ~(0x03);
					ProgramMemoryControlData current_program_memory_control = GetProgramMemoryControlData(IP);
					Program &CurrentProgram = ProgramVectorTable[current_program_memory_control.program_id];
					while (irq_queue.size() > 0 && !(SR & 0x04) && !interrupt)
					{
						ChipsetInterruptRequestData irq = irq_queue.front();
						switch (irq.chipset)
						{
							case 0x03:
							{
								ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(CurrentMotherboard->MainInput, irq.interrupt);
								size_t system_region_id = (current_program_memory_control.program_id * 16) + idt_entry.ISR_control.region;
								if (CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x04)
								{
									interrupt = true;
									data_bus = std::bit_cast<QWord_LE>(CrossRegionJumpData { idt_entry.address, idt_entry.ISR_control.region });
									current_instruction_callback.callback = Instruction::Interrupt::ExecuteCycle_1;
									/*
									PushDataToStack<uint8_t>(*this, current_program_memory_control.region_id);
									PushDataToStack<DWord_LE>(*this, IP.address);
									PushDataToStack<QWord_LE>(*this, FL);
									FL &= ~(0x100);
									SR |= 0x04;
									// cycles_processed += 3;
									IP.address = idt_entry.address;
									SetProgramRegion(IP, idt_entry.ISR_control.region);
									current_program_memory_control = GetProgramMemoryControlData(IP);
									*/
								}
								else
								{
									irq_queue.pop_front();
								}
								break;
							}
							case 0x04:
							{
								ChipsetInterruptDescriptorTableEntryData idt_entry = GetInterrupt(CurrentMotherboard->MainTimer, irq.interrupt);
								size_t system_region_id = (current_program_memory_control.program_id * 16) + idt_entry.ISR_control.region;
								if (CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x04)
								{
									interrupt = true;
									data_bus = std::bit_cast<QWord_LE>(CrossRegionJumpData { idt_entry.address, idt_entry.ISR_control.region });
									current_instruction_callback.callback = Instruction::Interrupt::ExecuteCycle_1;
									/*
									PushDataToStack<uint8_t>(*this, current_program_memory_control.region_id);
									PushDataToStack<DWord_LE>(*this, IP.address);
									PushDataToStack<QWord_LE>(*this, FL);
									FL &= ~(0x100);
									SR |= 0x04;
									IP.address = idt_entry.address;
									SetProgramRegion(IP, idt_entry.ISR_control.region);
									current_program_memory_control = GetProgramMemoryControlData(IP);
									*/
								}
								else
								{
									irq_queue.pop_front();
								}
								break;
							}
						}
					}
					if (interrupt)
					{
						current_cycle_state = CycleState::Execute;
						continue;
					}
					uint8_t *current_region = CurrentProgram.memory_region[current_program_memory_control.region_id];
					size_t system_region_id = (current_program_memory_control.program_id * 16) + current_program_memory_control.region_id;
					if (IP.address >= CurrentMotherboard->memory_region_table[system_region_id].memory_region_size)
					{
						if (CurrentMotherboard->memory_region_table[system_region_id].memory_region_control.memory_region_flags & 0x20)
						{
							IP.address %= CurrentMotherboard->memory_region_table[system_region_id].memory_region_size;
						}
						else
						{
							fmt::print("Wraparound disallowed in current code region.\n");
							running = false;
							CurrentMotherboard->OSKernel.DestroyProcess(current_program_id, 0);
							break;
						}
					}
					BaseInstructionData &current_instruction = reinterpret_cast<BaseInstructionData &>(current_instruction_callback.instruction_data[0]);
					memcpy(&current_instruction, &current_region[IP.address], sizeof(BaseInstructionData));
					// memcpy(&CurrentInstruction, &current_region[IP.address], sizeof(BaseInstructionData));
					// fmt::print("{}\n", instruction_function_table[CurrentInstruction.instruction_type].display_str);
					IP.address += sizeof(BaseInstructionData);
					current_instruction_callback.callback = instruction_base_callback_table[current_instruction.instruction_type].callback;
					// fmt::print("{}\n", instruction_base_callback_table[current_instruction.instruction_type].display_str);
					// ++cycles_processed;
					current_cycle_state = CycleState::Execute;
					break;
				}
				case CycleState::Execute:
				{
					// current_cycle_count += 1.0;
					// instruction_function_table[CurrentInstruction.instruction_type].func(*this, CurrentInstruction, cycles_processed);
					current_instruction_callback.callback(current_instruction_callback);
					if (current_instruction_callback.callback == nullptr)
					{
						current_cycle_state = CycleState::Fetch;
						/*
						double current_mips_rate = cycles_per_second / current_cycle_count;
						if (lowest_mips_rate == 0.0 || current_mips_rate < lowest_mips_rate)
						{
							lowest_mips_rate = current_mips_rate;
						}
						if (current_mips_rate > highest_mips_rate)
						{
							highest_mips_rate = current_mips_rate;
						}
						average_mips_rate += current_mips_rate;
						current_cycle_count = 0.0;
						instructions_processed += 1.0;
						*/
					}
					/*
					if (cycles_processed >= 20)
					{
						running = false;
						CurrentMotherboard->OSKernel.DestroyProcess(current_program_id, 0);
					}
					*/
					break;
				}
			}
			if (!running)
			{
				break;
			}
		}
		// cycles_left = cycles_processed - cycles;
		std::chrono::duration<double> cycle_process_time = std::chrono::high_resolution_clock::now() - cycle_tp;
		double real_cycle_process_time = cycle_process_time.count();
		if (lowest_process_time == 0.0 || real_cycle_process_time < lowest_process_time)
		{
			lowest_process_time = real_cycle_process_time;
	 	}
		if (real_cycle_process_time > highest_process_time)
		{
			highest_process_time = cycle_process_time.count();
		}
		times_processed += 1.0;
		average_process_time += cycle_process_time.count();
		fmt::print("Cycle Process Time:  {}\n", cycle_process_time.count());
		fmt::print("Average Cycle Process Time:  {}\n", average_process_time / times_processed);
		fmt::print("Lowest Cycle Process Time:  {}\n", lowest_process_time);
		fmt::print("Highest Cycle Process Time:  {}\n", highest_process_time);
		/*
		fmt::print("Average MIPS Rate:  {}\n", average_mips_rate / instructions_processed);
		fmt::print("Lowest MIPS Rate:  {}\n", lowest_mips_rate);
		fmt::print("Highest MIPS Rate:  {}\n", highest_mips_rate);
		*/
	}
}

void ClassicVCom_Nova64::CPU::LoadTestProgram()
{
	const std::array<uint8_t, 176> test_program = {
		0x20, 0x00, 0x07, 0x04, 0x01, 0x00, 0x04, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x07, 0x04, 0x01, 0x00, 0x04, 0x00,
		0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x19, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xD0, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
		0x20, 0x00, 0x07, 0x04, 0x01, 0x00, 0x04, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x31, 0x00, 0x01, 0x00, 0x50, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	const std::array<uint8_t, 16> test_data_region = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	const std::array<uint8_t, 112> test_code_region = {
		0x44, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00,
		0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	memcpy(&CurrentMotherboard->memory[0x00], test_program.data(), test_program.size());
	memcpy(&CurrentMotherboard->memory[0xB0], test_data_region.data(), test_data_region.size());
	memcpy(&CurrentMotherboard->memory[0xC0], test_code_region.data(), test_code_region.size());
	CurrentMotherboard->program_descriptor_table[0].start_addr = 0x00000000;
	CurrentMotherboard->program_descriptor_table[0].program_executable_mem_size = 280;
	CurrentMotherboard->program_descriptor_table[0].program_data = 0;
	CurrentMotherboard->memory_region_table[0].memory_region_start_addr = 0x00000000;
	CurrentMotherboard->memory_region_table[0].memory_region_size = 176;
	CurrentMotherboard->memory_region_table[0].memory_region_control.memory_region_flags = 0x04;
	CurrentMotherboard->memory_region_table[1].memory_region_start_addr = 0xB0;
	CurrentMotherboard->memory_region_table[1].memory_region_size = 16;
	CurrentMotherboard->memory_region_table[1].memory_region_control.memory_region_flags = 0x03;
	CurrentMotherboard->memory_region_table[2].memory_region_start_addr = 0xC0;
	CurrentMotherboard->memory_region_table[2].memory_region_size = 112;
	CurrentMotherboard->memory_region_table[2].memory_region_control.memory_region_flags = 0x06;
	CurrentMotherboard->memory_region_table[15].memory_region_start_addr = 0x120;
	CurrentMotherboard->memory_region_table[15].memory_region_size = 32;
	CurrentMotherboard->memory_region_table[15].memory_region_control.memory_region_flags = 0x23;
	CurrentMotherboard->OSKernel.CreateProcess();
	// RunProgram(*this, CurrentMotherboard->memory.data());
	/*
	CurrentMotherboard->program_descriptor_table[0].program_control |= 0x01;
	Program TestProgram(&CurrentMotherboard->program_descriptor_table[0], CurrentMotherboard->memory.data(), &CurrentMotherboard->memory_region_table[0]);
	ProgramVectorTable.push_back(std::move(TestProgram));
	*/
	BP.memory_control = (1 << 12);
	BP.address = 0x00;
	SP.memory_control = (15 << 12);
}
