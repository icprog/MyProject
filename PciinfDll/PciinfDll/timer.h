#ifndef __TIMER_H
#define __TIMER_H

#include "pciimport.h"
#include "ifcapi_6850.h"

// 95/08/01 Tom MB
#ifdef BUILDING_TIMER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

extern "C" DLLEXPORT extern void ATE_PciDelayTimeMS(double tm);
extern "C" DLLEXPORT extern void dlyms(float tm);
// 95/08/01 Tom ME

#endif