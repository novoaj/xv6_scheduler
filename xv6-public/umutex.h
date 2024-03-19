#ifndef UMUTEX_H
#define UMUTEX_H
#include "spinlock.h"
typedef struct {
  // Lock state, ownership, etc.
  int state;
  int ownership;
  struct spinlock lk;
} mutex;

#endif