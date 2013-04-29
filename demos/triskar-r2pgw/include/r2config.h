#ifndef __R2CONFIG_H__
#define __R2CONFIG_H__

#include "transport/LedTransport.hpp"
#include "transport/RTcan.hpp"

/*
 * R2P middleware configuration structure
 */

struct Config
{
	typedef RTcan Transport;
};

#endif /* __R2CONFIG_H__ */
