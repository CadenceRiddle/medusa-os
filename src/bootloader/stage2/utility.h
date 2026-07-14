#pragma once

#include "stdint.h"

// Rounds a number up to an alignment boundary.
uint32_t align(uint32_t number, uint32_t alignTo);
