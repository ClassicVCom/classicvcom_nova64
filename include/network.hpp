#ifndef _NETWORK_HPP_
#define _NETWORK_HPP_

#include "chipset.hpp"

namespace ClassicVCom_Nova64
{
	class Network
	{
		public:
			Network();
			~Network();
			void operator()(unsigned short function, std::array<unsigned long long, 8> args);
			template <HasChipsetId T>
			std::string GetId(T &chipset);
		private:
			const std::string id = "ClassicVNet-2";
	};
}

#endif
