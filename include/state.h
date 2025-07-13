#pragma once

#define SYSTEM_STATES_COUNT 3

enum class SystemState
{
    GCS,
    ROCKET,
    OTHER,
};

void StateInit();
void StateCheck();
bool StateChanged();
SystemState StateGetCurrent();