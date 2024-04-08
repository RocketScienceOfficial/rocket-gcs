#pragma once

#define SYSTEM_STATES_COUNT 2

enum class SystemState
{
    GCS,
    ROCKET,
};

void StateInit();
void StateCheck();
bool StateChanged();
SystemState StateGetCurrent();