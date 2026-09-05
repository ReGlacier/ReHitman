#pragma once

#include <cstdint>

// ZHM3LevelControl
static constexpr uint16_t MSG_LevelControl_HitmanDied            = 0x081Du;

// ZFightController
static constexpr uint16_t MSG_FightController_RequestNewPosition = 0x0950u;
static constexpr uint16_t MSG_FightController_BackupRequested    = 0x0951u;
static constexpr uint16_t MSG_FightController_RequestCoverFire   = 0x094Fu;
