/*!
 ******************************************************************************
 * @file    utils.c
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Fri Apr 28 11:58:55 2017
 * @brief   This file ...
 ******************************************************************************
 */
#define __UTILS_C
/* Private includes ----------------------------------------------------------*/
#include <stdafx.h>
#include <Windows.h>
#include "utils.h"
#if _DEBUG
#include <conio.h>
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

void delayms( float ms )
{
    LARGE_INTEGER StartingTime, EndingTime, PollingTime;
    LARGE_INTEGER Frequency;

    QueryPerformanceCounter(&StartingTime);
    
    QueryPerformanceFrequency(&Frequency); 
    
    EndingTime.QuadPart = StartingTime.QuadPart + (long long)(Frequency.QuadPart * ms / 1000.0F);
    // Activity to be timed
    
    while (1)
    {
        QueryPerformanceCounter(&PollingTime);
        if ( PollingTime.QuadPart >= EndingTime.QuadPart )
			break;
    }
}

long long getTickstamp( void )
{
    LARGE_INTEGER CurrentTime;
    
    QueryPerformanceCounter(&CurrentTime);
    
    return CurrentTime.QuadPart;
}

long long getTicksPerSecond( void )
{
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency); 
	return Frequency.QuadPart;
}
