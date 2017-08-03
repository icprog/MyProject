/*!
 ******************************************************************************
 * @file    dev_peb64.h
 * @author  Aaron of TRI
 * @version V0.0.0
 * @date    Wed Apr 12 19:03:20 2017
 * @brief   This file ...
 
 * Naming convention of macros:
   1.    adj...      Adjust field data into its field, ie: #define adjWrOfI2c(_W)   ((_W)<<7)
   2.    ...To...    Convert to something, in some case it can be either "adj..." or "...To...", but this 
                     is more like transform but not only adjust
   3.    getData...  Unlike the previouse two names, this one does not convert or adject the arguments, 
                     instead it use the argument to assist converting or adjusting an hard coded data and
                     which must be explicitly describe in the macro's name
                     ie: #define getDataTRIPhoneNumberDigit(_DIGIT) (0x28328918>>_DIGIT*4 & 0xF)
   4.    drv...      Macros adapting this prefix are fake inline functions
   
  * Terminologies:
   1.    ATE channel: ADATE305 channels, according the spec, there are 2 channels per ADATE305 IC
             
   2.    Channel FPGA: In contrast to Main FPGA, channel FPGAs handle those channels with ADATE305 on it
   
   3.    "Channel Number" or "Peb channel number": 1~96, as of Apr 25 2017, there are 8 ADATE305 ICs per channel FPGA,
             4 channel FPGAs per PEB64, and up to 10 PEB64 slots supported per 6868 machine
             It sums up a PEB64 contains 2*8*4 = 64 channels
             Plus 32 user channels that provide simply a relay without channel FPGA nor ADATE305
             Totally 64+32 = 96 channels
            
 ******************************************************************************
 */
#ifndef __DRV_PEB64_H
#define __DRV_PEB64_H
/* Includes ------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/

/*--------------------------------------------------------For bit operation---*/
// Extract _DAT[_L:_R] and right align, _L should >= _R
#define FIELD( _DAT, _L, _R )                                      ( ( _DAT ) >> ( _R ) & (( 1 << ( ( _L ) - ( _R ) + 1 ) )-1) )
// Extract _D1 and _D2, align _D2 to right, then align _D1 adjoin to left of _D2
#define ALIGN2( _D1, _L1, _R1, _D2, _L2, _R2 )                     ( ( FIELD( _D1, _L1, _R1 ) << ( (_L2) - (_R2) + 1 ) ) | ( FIELD( _D2, _L2, _R2 ) ) )
#define ALIGN3( _D1, _L1, _R1, _D2, _L2, _R2, _D3, _L3, _R3 )      ( ALIGN2( _D1, _L1, _R1, ALIGN2( _D2, _L2, _R2, _D3, _L3, _R3 ), ( (_L2) - (_R2) + (_L3) - (_R3) + 1 ), 0 ) )

/*------------------------------------------------------PEB address mapping---*/
#define READ_DATA_MASK              0xFFFF



// Slot Enable

#define ADDR_ENABLE_SLOTS           0xE0E
#define SLOTS_ALL_OFF               0
//
#define slotIndToBitmask(_IND)      ((1<<(_IND))&0xFFFF)
#define slotNumToBitmask(_NUM)      (slotIndToBitmask((_NUM)-1))



// FPGA Enable

#define ADDR_ONOFF_FPGAS            0x0FF
#define FPGAS_ALL_OFF               0
#define fpgaIndToBitmask(_IND)      ((1<<(_IND))&0xFFFF)
#define fpgaNumToBitmask(_NUM)      (fpgaIndToBitmask((_NUM)-1))


// ID

#define ADDR_READ_PEB_ID            0



// Test Register

#define ADDR_FPGA_TEST_REG          0
#define ADDR_CH_FPGA_TEST_REG       0xF0



// Pattern

// Main FPGA
#define ADDR_PAT_RESET_CHANNELS             0x01
    #define START_RESET_CHANNEL                 0

#define ADDR_PAT_SET_MASTER_BOARD           0x02
    #define THIS_IS_PAT_MASTER                  0x01
	#define THIS_IS_NOT_PAT_MASTER              0x00
#define ADDR_PAT_BURST_START                0x03
    #define START_BURST_MODE                    0    
// Channel FPGA
#define ADDR_PAT_FAILED_LIMIT               0x10
    #define drvGetPatMaxFailLimit()         (0xFFFF)

#define ADDR_PAT_FAILED_REACTION            0x11
    #define FAIL_STOP                         0x0
    #define FAIL_LIMIT                        0x1
    #define FAIL_IGNORE                       0x2
#define ADDR_PAT_BURST_PREPARE              0x01
    #define BURST_PREPARE                       0x01


// Channel Rely Control

#define ADDR_FPGA_CH_STATUS         0x01
#define ADDR_CHN_RLY_GRP_0          0xE0
#define ADDR_CHN_RLY_READ           0xE0    // Reading this address to get the data received while the last writing performed
#define ADDR_CHN_RLY_GRP_1          0xE1    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_GRP_2          0xE2    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_GRP_3          0xE3    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_GRP_4          0xE4    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_GRP_5          0xE5    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_GRP_6          0xE6    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_GRP_7          0xE7    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_USR_0          0xE8    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_USR_1          0xE9    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_USR_2          0xEA    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_USR_3          0xEB    // Behavior on reading this address is undefined
#define ADDR_CHN_RLY_STATUS         0xEF
//
#define ADDR_CHN_RLY_FIRST          ADDR_CHN_RLY_GRP_0
#define ADDR_CHN_RLY_LAST           ADDR_CHN_RLY_USR_3
#define RLY_CHN_AMOUNT_PER_REG      (16/2)
#define TOTAL_RLY_CHN_AMOUNT        ((1+(ADDR_CHN_RLY_LAST)-(ADDR_CHN_RLY_FIRST))*(RLY_CHN_AMOUNT_PER_REG))
#define TOTAL_ATE_RLY_CHN_AMOUNT    ((1+(ADDR_CHN_RLY_GRP_7)-(ADDR_CHN_RLY_FIRST))*(RLY_CHN_AMOUNT_PER_REG))
#define chnRlyChnToAddr(_CHN)       (ADDR_CHN_RLY_FIRST + ((_CHN)>>3))
#define chnRlyChnToBitOffset(_CHN)  (((_CHN) & 0x7)<<1)
#define getDataChnRlyOn(_CHN)       (3 << chnRlyChnToBitOffset(_CHN))
#define getDataChnRlyOff(_CHN)      (2 << chnRlyChnToBitOffset(_CHN))
#define chnRlyChnToOnOffBitmask(_CHN)    (1 << chnRlyChnToBitOffset(_CHN))
#define BITMASK_CHN_WORKING         1



// Channel Memory Control

#define ADDR_CHN_MEM_ADDR           0x03
#define ADDR_CHN_MEM_INST           0x04
#define ADDR_CHN_MEM_PATT           0x06
//
#define PATT_WIDTH                  48
#define INST_WIDTH                  32

// Channel board's ADATE302 Control

// #define ADDR_ATE_WR_CTRL_RD_BUSY    0x37
    // #define chnNumToChnInd(_CHN)            ((_CHN)-1)
    // #define chnNumToAteChipInd(_CHN)        ((chnNumToChnInd(_CHN)>>1)%drvGetAteChipAmountPerFpga())
    // #define chnNumToAteChnInd(_CHN)         (chnNumToChnInd(_CHN)&1)
    // #define chnNumToFpgaNum(_CHN)           (_CHN / drvGetAteChnAmountPerFpga() + drvGetFirstChnFpgaNumOnPeb64())
    // #define adjAteChipSel( _CHIPSEL )       (((_CHIPSEL)&0xFF)<<8)
    // #define adjAteWr( _W )                  (((_W)&0x1)<<7)
    // #define adjAteChnSel( _ATECHNSEL )      (((_ATECHNSEL)&0x3)<<5)
    // #define adjAteAddr( _ADDR_IN_ATE )      (((_ADDR_IN_ATE)&0x1F))
    // #define adjAteCtrlX( _CHIPSEL, _W, _ATECHNSEL, _ADDR_IN_ATE )    ( adjAteChipSel(_CHIPSEL) | adjAteWr(_W) | adjAteChnSel(_ATECHNSEL) | adjAteAddr(_ADDR_IN_ATE) )
    // #define adjAteCtrl( _CHN, _W, _ADDR_IN_ATE )    ( adjAteChipSel(1<<chnNumToAteChipInd(_CHN)) | adjAteWr(_W) | adjAteChnSel(1<<chnNumToAteChnInd(_CHN)) | adjAteAddr(_ADDR_IN_ATE) )
    // #define ATE_WRITE                       1
    // #define ATE_READ                        (!(ATE_WRITE))
    // #define BITMASK_ATE_WORKING             1
// #define ADDR_ATE_DATA               0x38
#define ADDR_PMU_MODE_ENABLE        0x3C
    #define PMU_ENABLE                      (0x0001)
    #define chnNumToPmuChnBitMask(_CHN)     (1<<(((_CHN)-1) % drvGetAteChnAmountPerFpga()))
#define ADDR_PMU_DATA               0x3D
#define ADDR_PMU_HZ_VT_ENABLE       0x3E

// Address of ADATE305's registers
#define ATEADDR_NOP                 0x00
#define ATEADDR_VH_DAC_LEVEL        0x01
    #define vhVoltToDac(_VOLT)              ((unsigned int)((_VOLT+2.5L)*16384.0L/10.0L))
    #define vhDacToVolt(_DAC)               ((double)(_DAC)*10.0L/16384.0L-2.5L)
#define ATEADDR_VL_DAC_LEVEL        0x02
    #define vlVoltToDac                     vhVoltToDac
    #define vlDacToVolt                     vhDacToVolt
#define ATEADDR_VT_DAC_LEVEL        0x03
    #define vtVoltToDac                     vhVoltToDac
    #define vtDacToVolt                     vhDacToVolt
#define ATEADDR_VOL_DAC_LEVEL       0x04
    #define volVoltToDac                     vlVoltToDac
    #define volDacToVolt                     vlDacToVolt
#define ATEADDR_VOH_DAC_LEVEL       0x05
    #define vohVoltToDac                     vhVoltToDac
    #define vohDacToVolt                     vhDacToVolt
#define ATEADDR_VCH_DAC_LEVEL       0x06
    #define vcVoltToDac                     vhVoltToDac
    #define vcDacToVolt                     vhDacToVolt
#define ATEADDR_VCL_DAC_LEVEL       0x07
#define ATEADDR_VIOH_DAC_LEVEL      0x08
    #define IohCurrToDac(_CURR)             ((unsigned int)((((_CURR)*1000.0L+6.0L)*16384.0L)/24.0L))
#define ATEADDR_VIOL_DAC_LEVEL      0x09    
    #define IolCurrToDac                    IohCurrToDac
#define ATEADDR_OVD_HI_LEVEL        0x0A    // when ch = ch1
    #define OvdhVoltToDac(_VOLT)            ((unsigned int)((((_VOLT)+3.0L)*16384.0L)/10.0L))
#define ATEADDR_OVD_LO_LEVEL        0x0A    // when ch = ch0
    #define OvdlVoltToDac                   OvdhVoltToDac    
#define ATEADDR_PMUDAC_LEVEL        0x0B
    #define pmuVoltToDac(_VOLT)             ((unsigned int)(((_VOLT)+2.5L)*65536.0L/10.0L))
    #define pmuDacToVolt(_DAC)              ((_DAC)*10.0L/65536.0L-2.5L)
    #define pmuCurrToDac(_AMP,_IPEAK)       ((unsigned int)(0x7FFF+(_AMP)*0x8000/(_IPEAK)))
#define ATEADDR_PE_PMU_ENABLE       0x0C
    #define BIT_PMU_EN                      2
    #define BIT_PMU_FORCE_VT                1
    #define BIT_PE_DISABLE                  0       // When disable: Driver Relay Open
#define ATEADDR_CHANNEL_STATE       0x0D
    #define BIT_CHN_HV_MODE_EN              2
    #define BIT_CHN_LOAD_EN                 1
    #define BIT_CHN_VT_NOT_HZ               0
#define ATEADDR_PMU_STATE           0x0E
    #define BIT_PMU_IN_SEL_B1               9
    #define BIT_PMU_IN_SEL_B0               8
    #define BIT_PMU_SEN_PATH                7
    #define BIT_PMU_CLAMP_EN                5
    #define BIT_PMU_MEAS_CURR_NOT_VOLT      4
    #define BIT_PMU_FORCE_CURR_NOT_VOLT     3
    #define BIT_PMU_RANGE_B2                2
    #define BIT_PMU_RANGE_B1                1
    #define BIT_PMU_RANGE_B0                0
    #define PMU_RANGE_BITMASK               ((1<<BIT_PMU_RANGE_B0)|(1<<BIT_PMU_RANGE_B1)|(1<<BIT_PMU_RANGE_B2))
#define ATEADDR_PMU_MEAS_EN         0x0F
    #define BIT_PMU_MEAS_CH_SEL_B1          2
    #define BIT_PMU_MEAS_CH_SEL_B0          1
    #define BIT_PMU_MEAS_OUT_EN             0
#define ATEADDR_DIFF_COMP_EN        0x10
#define ATEADDR_DAC_MONITOR         0x11
#define ATEADDR_OVD_CHX_ALARM_MASK  0x12
#define ATEADDR_OVD_CHX_ALARM_STATE 0x13    // read only




// Pattern time slot period settings

#define ADDR_TIMESLOT_PERIOD_ADDR           0x07
#define ADDR_TIMESLOT_PERIOD_H              0x08
#define ADDR_TIMESLOT_PERIOD_L              0x09






// Pattern time slot channel settings

#define ADDR_TIMESLOT_CONTENT_SEL           0x0C
    #define adjTimeslotSelMarker( _MRK )            (((_MRK)&0xF)<<4)
        #define DRIVE_MARKER                        0
        #define IO_MARKER                           1
        #define COMPARE_MARKER                      2
    #define adjTimeslotSelChannel( _CHT )           ((_CHT)&0xF)
    #define adjTimslotSel(_MRK, _CHT)               ( adjTimeslotSelMarker(_MRK) | adjTimeslotSelChannel(_CHT) )
    
#define ADDR_TIMESLOT_ADDR                  0x0D
    #define timeslotChtToMemAddr( _TS, _CHT )       (( (_TS)*8+(_CHT) )<<1)
#define ADDR_TIMESLOT_DATA_H                0x0E
        #define adjTimeslotLocationHi( _LOC )           (((_LOC)>>16)&0x3FFF)
        #define adjTimeslotDat( _DAT )                  (((_DAT)&0x3)<<14)
    #define adjTimeslotMarkerHi( _LOC, _DAT )       ( adjTimeslotLocationHi(_LOC) | adjTimeslotDat(_DAT) )
    #define adjTimeslotMarkerLo( _LOC )             ( (_LOC)&0xFFFF )
#define ADDR_TIMESLOT_DATA_L                0x0F






// Fail bus

// Main FPGA
#define ADDR_FAIL_MASTER_ENABLE          0x04
    #define adjFailMasterMask(_SITENUM)                     ( 1<<((_SITENUM)-1) )
#define ADDR_SITE_01_ENABLE              0x10
    #define adjSiteEnableMask( _CHNFPGANUM, _LINKNUM )      ( 1<<( ((_CHNFPGANUM)-1)*2 + ((_LINKNUM)-1) ))
    #define siteNumToSiteEnableAddress( _SITENUM )          ( (_SITENUM)-1+(ADDR_SITE_01_ENABLE) )
#define ADDR_SITE_02_ENABLE              0x11
#define ADDR_SITE_03_ENABLE              0x12
#define ADDR_SITE_04_ENABLE              0x13
#define ADDR_SITE_05_ENABLE              0x14
#define ADDR_SITE_06_ENABLE              0x15
#define ADDR_SITE_07_ENABLE              0x16
#define ADDR_SITE_08_ENABLE              0x17
#define ADDR_SITE_09_ENABLE              0x18
#define ADDR_SITE_10_ENABLE              0x19
#define ADDR_SITE_11_ENABLE              0x1A
#define ADDR_SITE_12_ENABLE              0x1B
#define ADDR_SITE_13_ENABLE              0x1C
#define ADDR_SITE_14_ENABLE              0x1D
#define ADDR_SITE_15_ENABLE              0x1E
#define ADDR_SITE_16_ENABLE              0x1F
#define ADDR_FISRT_SITE                  ADDR_SITE_01_ENABLE
#define ADDR_LAST_SITE                   ADDR_SITE_16_ENABLE
    #define totalSiteAmount()            ( (ADDR_LAST_SITE) - (ADDR_FISRT_SITE) + 1 )
    #define lastSiteNum()                ( totalSiteAmount() )

// Channel FPGA
#define ADDR_FAIL_LINK_1_ENABLE          0x12
    #define adjLinkEnableMask( _FGPACHNNUM )                ( 1<<( (_FGPACHNNUM)-1 ))
    #define linkNumToLinkEnableAddress( _LINKNUM )          ( (_LINKNUM)-1+(ADDR_FAIL_LINK_1_ENABLE) )
    #define chnFpgaNumToRealFpgaNum( _CHNFPGANUM )          ( (_CHNFPGANUM) + drvGetFirstChnFpgaNumOnPeb64() -1 )
    #define fpgaNumToChnFpgaNum( _FPGANUM )                 ( (_FPGANUM) - drvGetFirstChnFpgaNumOnPeb64() +1 )
#define ADDR_FAIL_LINK_2_ENABLE          0x13
#define ADDR_FISRT_LINK                  ADDR_FAIL_LINK_1_ENABLE
#define ADDR_LAST_LINK                   ADDR_FAIL_LINK_2_ENABLE





// Match Sites

// Main FPGA

#define ADDR_MATCH_MASTER_ENABLE            0x05
    #define adjMatchMasterMask(_MATCHSITENUM)               ( 1<<((_MATCHSITENUM)-1) )

#define ADDR_MATCH_SITE_01_CH_ENABLE        0x30
    #define MATCH_SITE_ALL_CH_DISABLE                       0
    #define adjChnFpgaNumToMatchEnableBit( _CHNFPGANUM )    ( 1<<((_CHNFPGANUM)-1) )
#define ADDR_MATCH_SITE_02_CH_ENABLE        0x31
#define ADDR_MATCH_SITE_03_CH_ENABLE        0x32
#define ADDR_MATCH_SITE_04_CH_ENABLE        0x33
#define ADDR_MATCH_SITE_05_CH_ENABLE        0x34
#define ADDR_MATCH_SITE_06_CH_ENABLE        0x35
#define ADDR_FISRT_MATCH_SITE               ADDR_MATCH_SITE_01_CH_ENABLE
#define ADDR_LAST_MATCH_SITE                ADDR_MATCH_SITE_06_CH_ENABLE
    #define totalMatchSiteAmount()          ( (ADDR_LAST_MATCH_SITE) - (ADDR_FISRT_MATCH_SITE) + 1 )
    #define lastMatchSiteNum()              ( totalMatchSiteAmount() )
    #define matchSiteNumToMatchSiteAddress( _MATCHSITENUM )             ( (_MATCHSITENUM)-1+ADDR_FISRT_MATCH_SITE )


// Channel FPGA

#define ADDR_MATCH_TO_FAIL_LINK_SEL         0x16
    #define MATCH_TO_FAIL_LINK_1                    0
    #define MATCH_TO_FAIL_LINK_2                    1
#define ADDR_MATCH_CH_ENABLE                0x17
    #define MATCH_CH_ALL_CH_DISABLE                 0

typedef struct
{
    unsigned int pChnBitsPerChnFPGA[ 4 ];
    
    
} sMatchSite;




// Log Trigger Control

// Channel FPGA
#define ADDR_SET_LOG_MEM_ADDR_FOR_READ  0x0A
#define ADDR_READ_LOG_MEM_DATA          0x0B

#define ADDR_LOG_START_CYCLE_COUNT_L    0x19
#define ADDR_LOG_START_CYCLE_COUNT_H    0x1A
#define ADDR_LOG_START_VECTOR_ADDR_L    0x1B
#define ADDR_LOG_START_VECTOR_ADDR_H    0x1C
#define ADDR_LOG_MODE_CONTROL           0x1D
    #define adjLogModeContorl( _MODE, _RECOVER )            ((((_MODE) & 3)<<1)|( (_RECOVER) & 1 ))

#define ADDR_LOG_MEMORY_STATUS          0x24
    #define getDataLastLogAddr( _STATUS )                   ( (_STATUS) & 0x3FFF )
    #define getDataLogRecover( _STATUS )                    ( ((_STATUS)>>14) & 1 )





// PMU ADC

#define ADDR_ADC_WR_CNT_RD_BUSY     0x39
    #define BIT_ADC_BUSY                    0
#define ADDR_ADC_DATA_H16BIT        0x3A
#define ADDR_ADC_DATA_L16BIT        0x3B
    #define measAdcToVolt(_ADC)     ((_ADC)*10.0L/65536.0L-2.5L)
    //#define measAdcToAmp(_ADC,_R)   ((measAdcToVolt(_ADC)-2.5L)/(5L*(_R)))
#define measAdcToAmp(_ADC,_R)		((measAdcToVolt(_ADC)-2.5L)/(5.0L*(_R)))





// TMU

#define chnNumToTmuInd( _CHN )      ( (((_CHN)-1)>>3) & 1 )
// Channel FPGA - FREQUENCY MEASURE UNIT
#define ADDR_TMU1_FREQ_MODE             0x40
    #define adjTmuFreqStart( _TMU_CMP, _CHSEL )             (ALIGN3( (_TMU_CMP),1,0, 0,0,0, (_CHSEL),2,0 ))
        #define TMU_CMP_H                       0
        #define TMU_CMP_L                       1
        #define TMU_CMP_HL                      2
        #define chnNumToTmuChnSel( _CHN )                           ( ((_CHN)-1) & 0x7 )
    #define workingStateFromTmuFregStt( _STT )                      ( FIELD((_STT), 0, 0) )
#define ADDR_TMU1_FREQ_W_PRD_H_R_CNT_H  0x41
    #define overflowFromTmuCntH(_CNTH)                      (FIELD( (_CNTH),15,15 ))
#define ADDR_TMU1_FREQ_W_PRD_L_R_CNT_L  0x42    
    #define counterFromTmuCntHL( _CNTH, _CNTL )             (ALIGN2( (_CNTH),14,0, (_CNTL),15,0 ))

#define ADDR_TMU2_FREQ_MODE             0x50
#define ADDR_TMU2_FREQ_W_PRD_H_R_CNT_H  0x51
#define ADDR_TMU2_FREQ_W_PRD_L_R_CNT_L  0x52    

// Channel FPGA - PERIOD MEASURE UNIT
#define ADDR_TMU1_PRD_MODE              0x43
    #define adjTmuPrdStart( _TMU_PRD_MODE, _TMU_CMP, _CHSEL )   ( (1<<15) | ((_TMU_PRD_MODE)<<8) | ((_TMU_CMP)<<4) | (_CHSEL) )
    #define adjTmuPrdStop()                                     ( 0 )
        #define TMU_PRD_MODE_R2R                0   // Rising to rising
        #define TMU_PRD_MODE_F2F                1   // Falling to falling
        #define TMU_PRD_MODE_R2F                2   // Rising to falling
        #define TMU_PRD_MODE_F2R                3   // Falling to rising
    #define triggerStateFromTmuPrdStt( _STT )                     ( FIELD( (_STT), 1, 1 ))
    #define workingStateFromTmuPrdStt( _STT )                     ( FIELD( (_STT), 0, 0 ))    
#define ADDR_TMU1_PRD_MEAS_H            0x44
    #define overflowFromTmuPrdMeasH(_MEASH)                     (FIELD( (_MEASH),15,15 ))
#define ADDR_TMU1_PRD_MEAS_L            0x45
    #define measurementFromTmuPrdMeasHL( _MEASH, _MEASL )       (ALIGN2( (_MEASH),14,0, (_MEASL),15,0 ))

#define ADDR_TMU2_PRD_MODE              0x53
#define ADDR_TMU2_PRD_MEAS_H            0x54
#define ADDR_TMU2_PRD_MEAS_L            0x55

// Channel FPGA - EDGE MEASURE UNIT
#define ADDR_TMU1_EDGE_W_MODE_R_STT     0x46
        #define adjEdgeChnCfg( _CHSEL, _CMPSEL, _EDGSEL )       (ALIGN3( (_EDGSEL),0,0, (_CMPSEL),0,0, (_CHSEL),2,0  ))
    #define adjTmuEdgeStart( _CHSEL1, _CMPSEL1, _EDGSEL1, _CHSEL2, _CMPSEL2, _EDGSEL2 )     ( (1<<15) | ALIGN3( adjEdgeChnCfg(_CHSEL2, _CMPSEL2, _EDGSEL2),4,0, 0,2,0, adjEdgeChnCfg(_CHSEL1, _CMPSEL1, _EDGSEL1),4,0 ))
        #define TMU_EDGE_CMP_RISING                                     (0)
        #define TMU_EDGE_CMP_FALLING                                    (1)
    #define adjTmuEdgeStop()                                    ( 0 )
    #define triggerStateFromTmuEdgeStt( _STT )                      ( FIELD( (_STT), 1, 1 ))
    #define workingStateFromTmuEdgeStt( _STT )                      ( FIELD( (_STT), 0, 0 ))
#define ADDR_TMU1_EDGE_MEAS_H           0x47
    #define overflowFromTmuEdgeMeasH(_MEASH)                    (FIELD( (_MEASH),15,15 ))
#define ADDR_TMU1_EDGE_MEAS_L           0x48
    #define measurementFromTmuEdgeMeasHL( _MEASH, _MEASL )      (ALIGN2( (_MEASH),14,0, (_MEASL),15,0 ))

#define ADDR_TMU2_EDGE_W_MODE_R_STT     0x56
#define ADDR_TMU2_EDGE_MEAS_H           0x57
#define ADDR_TMU2_EDGE_MEAS_L           0x58






/*-------------------------------------------------------Names of board IDs---*/
#define BOARD_ID_BIT_MASK   0xFF00

#define PEB64_ID            0x8400
#define PEB64_CH_ID         0x8000

//
#define MAIN_FPGA_NUMBER            1
#define MAIN_FPGA_IND               ((MAIN_FPGA_NUMBER)-1)
/* Macro ---------------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

int getApiVersionString( TCHAR * pCh, int maxLength );
#define getSupportSlotAmount()          (16)

/*-----------------------------------------------------Board presence check---*/
int drvRdMainBrdId( int slotNum );
int drvInquiryFpgaTestReg( int slotNum, int fpgaNum, int inquiryData );
#define drvGetMainFpgaNumOnPeb64()     (MAIN_FPGA_NUMBER)
#define drvGetFirstChnFpgaNumOnPeb64() (MAIN_FPGA_NUMBER+1)
#define drvGetChnFpgaAmountOnPeb64()   (4)
#define drvGetLastChnFpgaNumOnPeb64()  (drvGetFirstChnFpgaNumOnPeb64()+drvGetChnFpgaAmountOnPeb64()-1)
#define drvGetAteChipAmountPerFpga()   (8)
#define drvGetAteChnAmountPerAteChip() (2)
#define drvGetAteChnAmountPerFpga()    (drvGetAteChipAmountPerFpga()*drvGetAteChnAmountPerAteChip())

/*------------------------------------------------------------Relay control---*/
int drvRdChnStatusReg( int slotNum );
int drvWrChnRly( int slotNum, unsigned char chnNum, unsigned char onNotOff );
int drvRdChnRly( int slotNum, unsigned char chnNum, unsigned char * pOnNotOff );
int drvResetAllChnRly( int slotNum );
typedef struct
{
    unsigned char pOnOff[ TOTAL_RLY_CHN_AMOUNT ];
} sChnRlyStatus;
int drvRdChnRlyAll( int slotNum, sChnRlyStatus * psChnRlyStatus, unsigned char firstChn, unsigned char lastChn );

/*------------------------------------------Channel board memory read write---*/
int drvWrChnMemInst( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int * pInstBuf, unsigned int len );
int drvRdChnMemInst( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int * pInstBuf, unsigned int len );
int drvWrChnMemInstFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int fixData, unsigned int len );
int drvRdChnMemInstFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int fixData, unsigned int len );
int drvWrChnMemInstIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int startData, unsigned int len );
int drvRdChnMemInstIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned int startData, unsigned int len );

int drvWrChnMemPatt( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long * pPattBuf, unsigned int len );
int drvRdChnMemPatt( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long * pPattBuf, unsigned int len );
int drvWrChnMemPattFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long fixData48, unsigned int len );
int drvRdChnMemPattFixData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long fixData48, unsigned int len );
int drvWrChnMemPattIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long startData48, unsigned int len );
int drvRdChnMemPattIncData( int slotNum, int fpgaNum, unsigned int startAddr, unsigned long long startData48, unsigned int len );

/*-------------------------------------------Channel Board ADATE305 control---*/
int drvWrAteHzMode( int slotNum, unsigned char chnNum );
int drvWrAtePmuMode( int slotNum, unsigned char chnNum );
int drvWrAtePatMode( int slotNum, unsigned char chnNum );

// PMU mode
typedef struct
{
    unsigned char const regFieldVal;
    double const ohm;
    double const forceIPeakAmp;
    TCHAR * ptext;
} sAteCurrRangeInfo;

typedef enum
{
    ATE_CURR_02uA = 0
  , ATE_CURR_20uA
  , ATE_CURR_200uA
  , ATE_CURR_2mA
  , ATE_CURR_32mA
} sAteCurrIndex;

int drvGetAtePmuCurrRangeText( sAteCurrIndex gsAteCurrIndex, TCHAR const ** ppChText );
int drvWrAtePmuState( int slotNum, unsigned char chnNum, sAteCurrIndex gsAteCurrIndex, unsigned char forceCurrNotVolt, unsigned char measCurrNotCurr );
int drvWrAteForceVolt( int slotNum, unsigned char chnNum, double volt, sAteCurrIndex gsAteCurrIndex , double clampH, double clampL, float delayms );
int drvWrAteForceCurr( int slotNum, unsigned char chnNum, double amp, sAteCurrIndex gsAteCurrIndex , double clampH, double clampL, float delayms );

int drvWrAteVtMode( int slotNum, unsigned char chnNum );

int drvWrAteClampH( int slotNum, unsigned char chnNum, double value );
int drvWrAteClampL( int slotNum, unsigned char chnNum, double value );

int drvRdAtePmuVolt( int slotNum, unsigned char chnNum, double * pvolt );
int drvWrAtePmuVolt( int slotNum, unsigned char chnNum, double volt );

int drvRdAteVt( int slotNum, unsigned char chnNum, double * pvolt );
int drvWrAteVt( int slotNum, unsigned char chnNum, double volt );

// PATTERN mode
int drvWrAteVh( int slotNum, unsigned char chnNum, double volt );
int drvRdAteVh( int slotNum, unsigned char chnNum, double * pvolt );
int drvWrAteVl( int slotNum, unsigned char chnNum, double volt );
int drvRdAteVl( int slotNum, unsigned char chnNum, double * pvolt );
int drvWrAteVoh( int slotNum, unsigned char chnNum, double volt );
int drvWrAteVol( int slotNum, unsigned char chnNum, double volt );
int drvWrAteIoh( int slotNum, unsigned char chnNum, double curr );
int drvWrAteIol( int slotNum, unsigned char chnNum, double curr );
int drvWrAteOvdh( int slotNum, unsigned char chnNum, double volt );
int drvWrAteOvdl( int slotNum, unsigned char chnNum, double volt );
int drvWrAteActiveLoadOnOff( int slotNum, unsigned char chnNum, int enablenotDisable );

// ATE ADC Measure
int drvRdAteAD( int slotNum, unsigned char chnNum, unsigned int count, long long * pAdcVal32 );
int drvWrAteMeasCurr( int slotNum, unsigned char chnNum, unsigned int count, double * pAmps );
int drvWrAteMeasVolt( int slotNum, unsigned char chnNum, unsigned int count, double * pVolts );
int drvWrAteMeasOff( int slotNum, int chnNum );

// Pattern Time Slot Setup
int drvWrPatCht( int slotNum
               , int chnNum
               , unsigned int ts
               , unsigned int cht
               , unsigned int d0Loc
               , unsigned int d0Dat
               , unsigned int d1Loc
               , unsigned int d1Dat
               , unsigned int cLoc
               , unsigned int cLoc2
               , unsigned int cDat
               , unsigned int cMod
               , unsigned int io0Loc
               , unsigned int io0Dat
               , unsigned int io1Loc
               , unsigned int io1Dat );

typedef struct
{
    unsigned int d0Loc  : 30;
    unsigned int d0Dat  : 2;
    unsigned int d1Loc  : 30;
    unsigned int d1Dat  : 2;
    unsigned int cLoc   : 30;
    unsigned int cLoc2  : 30;
    unsigned int cDat   : 2;    //todo: adapt new spec
    unsigned int cMod   : 2;    //todo: adapt new spec
    unsigned int io0Loc : 30;
    unsigned int io0Dat : 2;
    unsigned int io1Loc : 30;
    unsigned int io1Dat : 2;
} sTimeslotChannelInfo;
int drvRdPatCht( sTimeslotChannelInfo * psTimeslotChannelInfo
               , int slotNum
               , int chnNum
               , unsigned int ts
               , unsigned int cht);

int drvWrPatTs( int slotNum
               , int fpgaNum
               , unsigned int ts
               , unsigned int tm );

int drvRdPatTs( int slotNum, int fpgaNum, int ts, unsigned int * pdata );

int drvPatBurstPrepareIgnoreFailed( int slotNum );
int drvPatBurstPrepareStopWhenAllSiteFailed( int slotNum );
int drvPatBurstPrepareStopFailedReachLimit( int slotNum, unsigned int limit16bit );
int drvPatMasterStartBurst( void );
int drvPatSetMaster( int slotNum );
int drvPatResetChannels( int slotNum );




// Site mapping of fail bus
int drvFailLinkClearHooks( int slotNum, int chnFpgaNum, int linkNum );               // Bit operation
int drvFailLinkHookChn( int slotNum, int chnFpgaNum, int linkNum, int fpgaChnNum );  // Bit operation
int drvWrFailLinkHookReg( int slotNum, int chnFpgaNum, int linkNum, unsigned int data );          // Word operation

int drvFailSiteClearHooks( int slotNum, int siteNum );                               // Bit operation
int drvFailSiteHookLink( int slotNum, int siteNum, int chnFpgaNum, int linkNum );    // Bit operation
int drvWrFailSiteHookReg( int slotNum, int siteNum, unsigned int data );             // Word operation
int drvFailSiteElectMaster( void );


// Slot decide its match site
int drvMatchSiteClearFpgaEnable( int slotNum );
int drvMatchSiteSetFpgaEnable( int slotNum, int matchSiteNum, unsigned int fgpaEnBits );
int drvMatchSiteClearLinkChannel( int slotNum, int chnFpgaNum );
int drvMatchSiteSetLinkChannel( int slotNum, int chnFpgaNum, unsigned int fpgaChnEnBits );
int drvMatchSiteElectMaster( void );


// TMU
int drvWrAteTmuMode( int slotNum, unsigned char chnNum );

int drvTmuStartMeasFreq( int slotNum, int chnNum, unsigned int tmuCmp, unsigned int periodInNanoSec );
int drvTmuGetFreqMeasResult( int slotNum, int chnNum, unsigned int * pBusyNotDone, unsigned int * pOverflow, unsigned int * pCounter );
#define calFreqFromPeriodAndCounter(_PRD, _CNT)         ( ((double)(_PRD)) / ((double)(_CNT)) )

int drvTmuStartMeasPeriod( int slotNum, int chnNum, unsigned int tmuCmp, unsigned int prdMode );
int drvTmuStopMeasPeriod( int slotNum, int chnNum );
int drvTmuGetPeriodMeasResult( int slotNum, int chnNum, unsigned int * pBusyNotDone, unsigned int * pTriggered, unsigned int * pOverflow, unsigned int * pPeriodInNanoSec );


int drvTmuStartMeasEdge( int slotNum
                       , unsigned int chnNum1
                       , unsigned int tmuCmp1
                       , unsigned int tmuEdge1
                       , unsigned int chnNum2
                       , unsigned int tmuCmp2
                       , unsigned int tmuEdge2 );
int drvTmuStopMeasEdge( int slotNum, int chnNum );
int drvTmuGetEdgeMeasResult( int slotNum, int chnNum, unsigned int * pBusyNotDone, unsigned int * pTriggered, unsigned int * pOverflow, unsigned int * pPeriodInNanoSec );




// Log Trigger Setup

int drvLogSetVectorAddr( int slotNum, int fpgaNum, unsigned int addr );
int drvLogSetCycleCount( int slotNum, int fpgaNum, unsigned int cycleCount );
typedef enum { NORMAL_MODE = 0, FAIL_ONLY_MODE = 2, REPEAT_FOLD_MODE = 3} sLogMode;
int drvLogSetControl( int slotNum, int fpgaNum, sLogMode gsLogMode, unsigned int recoverEnable );

int drvGetLastLogAddress( int slotNum, int fpgaNum, unsigned int * pRecovered, unsigned int * pAddress );

typedef struct
{
    unsigned int cycleCount;
    unsigned int vectorAddr;
    unsigned int LoopCount;
    unsigned int siteFail;
    unsigned int channelFail;
    unsigned int channelsCompareFail;
} sLogData;
int drvReadLogMem( int slotNum, int fpgaNum, unsigned int startAddress, unsigned int lastAddress, sLogData * psLogData );

// For development purpose
int drvRdAteReg( int slotNum, unsigned char chnNum, int addr, int * rdata );
int drvWrAteReg( int slotNum, unsigned char chnNum, int addr, int data );


#endif
