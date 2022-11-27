#ifndef _DMA_HPP_
#define _DMA_HPP_

#include "chipset.hpp"
#include "types.hpp"
#include "cpu.hpp"
#include <array>

namespace ClassicVCom_Nova64
{
	enum class DMAChipsetFunction
	{
		InitializeAllDMAChannels = 0
	};

	struct DMAChannelData
	{
		QWord_LE dma_control_flags;
		QWord_LE source_data;
		QWord_LE destination_data;
		DWord_LE source_transfer_buffer_size;
		DWord_LE destination_transfer_buffer_size;
	};

	class DMA
	{
		public:
			DMA();
			~DMA();
			void operator()(Word_LE &function, GPR_Data &args);

			template <HasChipsetId T>
			friend std::string GetId(T &chipset);
		private:
			const std::string id = "NovaDMA-64";
			struct
			{
				QWord_LE dma_channel_gate_register;
			} Registers;
			std::array<DMAChannelData, 16> dma_channel;
	};
}

#endif
