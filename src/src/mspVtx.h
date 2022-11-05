#pragma once

#include <stdint.h>

void mspQueryFlightController(uint32_t time_ms);
void mspBuildPacket(void);
void mspProcessSerial(void);
void mspUpdate(uint32_t now);
void mspReset();
