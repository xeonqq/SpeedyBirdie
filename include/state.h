#ifndef INCLUDE_STATE_H_
#define INCLUDE_STATE_H_
enum class State { Init, Stop, WarmUp, Feeding, Smoothing, Ready };

extern State g_state;

#endif
