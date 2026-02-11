#ifndef _AI_H_
#define _AI_H_

#include "core.h"
#include <stdint.h>

Command ai_move(GameState *state);
Command ai_demand_suit(GameState *state);

#endif
