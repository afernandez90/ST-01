#pragma once
#define ATOMIC_RESTORESTATE 0
#define ATOMIC_BLOCK(x) for(bool atomic_once=true;atomic_once;atomic_once=false)
