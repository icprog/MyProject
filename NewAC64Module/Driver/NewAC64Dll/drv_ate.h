/*!
 ******************************************************************************
 * @file    drv_ate.h
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Fri Apr 28 10:43:34 2017
 * @brief   This file ...
 ******************************************************************************
 */
#ifndef __DRV_ATE_H
#define __DRV_ATE_H
/* Includes ------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/
#define ADDR_ATE_WR_CTRL_RD_BUSY    0x37
    #define chnNumToChnInd(_CHN)            ((_CHN)-1)
    #define chnNumToAteChipInd(_CHN)        ((chnNumToChnInd(_CHN)>>1)%drvGetAteChipAmountPerFpga())
    #define chnNumToAteChnInd(_CHN)         (chnNumToChnInd(_CHN)&1)
    #define chnNumToFpgaNum(_CHN)           (chnNumToChnInd(_CHN) / drvGetAteChnAmountPerFpga() + drvGetFirstChnFpgaNumOnPeb64())
    #define adjAteChipSel( _CHIPSEL )       (((_CHIPSEL)&0xFF)<<8)
    #define adjAteWr( _W )                  (((_W)&0x1)<<7)
    #define adjAteChnSel( _ATECHNSEL )      (((_ATECHNSEL)&0x3)<<5)
    #define adjAteAddr( _ADDR_IN_ATE )      (((_ADDR_IN_ATE)&0x1F))
    #define adjAteCtrlX( _CHIPSEL, _W, _ATECHNSEL, _ADDR_IN_ATE )    ( adjAteChipSel(_CHIPSEL) | adjAteWr(_W) | adjAteChnSel(_ATECHNSEL) | adjAteAddr(_ADDR_IN_ATE) )
    #define adjAteCtrl( _CHN, _W, _ADDR_IN_ATE )    ( adjAteChipSel(1<<chnNumToAteChipInd(_CHN)) | adjAteWr(_W) | adjAteChnSel(1<<chnNumToAteChnInd(_CHN)) | adjAteAddr(_ADDR_IN_ATE) )
    #define ATE_WRITE                       1
    #define ATE_READ                        (!(ATE_WRITE))
    #define BITMASK_ATE_WORKING             1
#define ADDR_ATE_DATA               0x38
// #define ADDR_PMU_MODE_ENABLE        0x3C
    // #define PMU_ENABLE                      (0x0001)
// #define ADDR_PMU_DATA               0x3D
// #define ADDR_PMU_HZ_VT_ENABLE       0x3E
/* Macro ---------------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
int ateWaitIdle( void );
int ateWrReg( unsigned int ctrl, unsigned int regData );
int ateRdReg( unsigned int ctrl, unsigned int * pregData );

#endif
