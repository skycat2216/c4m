
// build info: 
//   version  : 3.6.12
//   time     : 10/15/2020 08:19:43
//   device   : m128
//   platform : avr
//   mcu      : atmega128
//   fcpu     : 11059200

// developed by MVMC-lab

// you can get lastest version at https://gitlab.com/MVMC-lab/c4mlib/c4mlib/tags

#ifndef C4MLIB_H
#define C4MLIB_H


#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>


#include "spi.cfg"
#include "newmode.cfg"
#include "adc.cfg"
#include "tim.cfg"
#include "pwm.cfg"
#include "interrupt.cfg"
#include "remo_reg.cfg"
#include "twi.cfg"
#include "uart.cfg"
#include "ext.cfg"

#define F_CPU 11059200UL

/*-- macro section start -----------------------------------------------------*/
/**
 * @def SETBIT(ADDRESS, BIT)
 * @ingroup macro_macro
 * @brief 將 ADDRESS 指定的 BIT 設置為 1。
 */
#define SETBIT(ADDRESS, BIT) ((ADDRESS) |= (1 << BIT))

/**
 * @def CLRBIT(ADDRESS, BIT)
 * @ingroup macro_macro
 * @brief 將 ADDRESS 指定的 BIT 清除為 0。
 */
#define CLRBIT(ADDRESS, BIT) ((ADDRESS) &= ~(1 << BIT))

/**
 * @def CHKBIT(ADDRESS, BIT)
 * @ingroup macro_macro
 * @brief 檢查 ADDRESS 指定的 BIT 是 1 或 0。
 */
#define CHKBIT(ADDRESS, BIT) (((ADDRESS) & (1 << BIT)) == (1 << BIT))

/**
 * @def REGFPT(REG_P, MASK, SHIFT, DATA)
 * @ingroup macro_macro
 * @brief 依照指定的 MASK, SHIFT, DATA 去讀取暫存器
 */
#define REGFPT(REG_P, MASK, SHIFT, DATA)                                       \
    (*((volatile char *)REG_P) = ((*((volatile char *)REG_P) & (~MASK)) |      \
                                  (((DATA) << (SHIFT)) & (MASK))))

/**
 * @def REGFGT(REG_P, MASK, SHIFT, DATA_P)
 * @ingroup macro_macro
 * @brief 依照指定的 MASK, SHIFT, DATA_P 去寫入暫存器
 */
#define REGFGT(REG_P, MASK, SHIFT, DATA_P)                                     \
    (*((volatile char *)DATA_P) =                                              \
         ((*((volatile char *)REG_P) & (MASK)) >> (SHIFT)))

/**
 * @def REGPUT(REG_P, BYTES, DATA_P)
 * @ingroup macro_macro
 * @brief 依照指定BYTES數的 *DATA_P 去寫入暫存器 *REG_P 。
 */
#define REGPUT(REG_P, BYTES, DATA_P)                                           \
    do {                                                                       \
        for (signed char __putptr = BYTES - 1; __putptr >= 0; __putptr--) {    \
            *((volatile char *)REG_P + __putptr) =                             \
                *((volatile char *)DATA_P + __putptr);                         \
        }                                                                      \
    } while (0)

/**
 * @def REGGET(REG_P, BYTES, DATA_P)
 * @ingroup macro_macro
 * @brief 依照指定BYTES數從暫存器 *REG_P 取得數值存入DATA_P。
 */
#define REGGET(REG_P, BYTES, DATA_P)                                           \
    do {                                                                       \
        for (char __getptr = 0; __getptr < BYTES; __getptr++) {    \
            *((volatile char *)DATA_P + __getptr) =                            \
                *((volatile char *)REG_P + __getptr);                          \
        }                                                                      \
    } while (0)

/**
 * @def HIBYTE16(SOURCE)
 * @ingroup macro_macro
 * @brief
 */
#define HIBYTE16(SOURCE) (SOURCE >> 8)

/**
 * @def LOBYTE16(SOURCE)
 * @ingroup macro_macro
 * @brief
 */
#define LOBYTE16(SOURCE) (SOURCE & 0xff)

/**
 * @brief 無回傳、參數為(void *)型態之函式型態。
 */
typedef void (*Func_t)(void *);

#define DISABLE 0  ///< 0 啟用 @ingroup macro_macro
#define ENABLE 1  ///< 1 關閉 @ingroup macro_macro

/**
 * @brief 確認 EN 是否為 ENABLE 或 DISABLE
 * @ingroup macro_macro
 */
#define IS_ENABLE_OR_DISABLE(EN) ((EN) == DISABLE || (EN) == ENABLE)

#define INPUT 0  ///< 0 輸入 @ingroup macro_macro
#define OUTPUT 1  ///< 1 輸出 @ingroup macro_macro

/**
 * @brief 確認 INOUT 是否為 INPUT 或 OUTPUT
 * @ingroup macro_macro
 */
#define IS_INPUT_OR_OUTPUT(INOUT) ((INOUT) == INPUT || (INOUT) == OUTPUT)
/*-- macro section end -------------------------------------------------------*/

/*-- hwimp section start -----------------------------------------------------*/

/**
 * @brief 用於定義結構體以存放工作方塊管理參數
 * 
 * @param state 禁致能及錯誤狀態
 * @param FBFunc_p 功能方塊執行函式指標
 * @param FBPara_p 功能方塊結執行函式傳參結構指標
 */
typedef struct {
    volatile uint8_t state;
    Func_t FBFunc_p;
    void *FBPara_p;
} TaskBlock_t;

/**
 * @brief
 *用於為不同硬體中斷定義出管理用結構體以備後續使用
 */
typedef struct {
    uint8_t MaxTask;               //<最大Task數
    uint8_t Total;                 //<目前登記的總Task數
    volatile TaskBlock_t *Task_p;  //<Task串列
} HWIntStr_t;

/**
 * @brief
 *被 internal ISR( )呼叫執行登錄在特定中斷中所有己登錄的工作方塊
 * @param Str_p 特定硬體中斷管理用結構體指標，結構體提供己登錄工作資料
 */
void HWInt_step(HWIntStr_t *Str_p);

/**
 * @brief
 *將工作方塊登錄分享特定硬體中斷服務常式。其工作項目包括：
 *把功能方塊函式及結構體指標，存入硬體中斷管理結構體，取得工作編號TaskId
 * @param Str_p 特定硬體中斷服務常式分享管理結構體指標
 * @param FBFunc_p 功能方塊執行函式指標
 * @param FBPara_p 功能方塊參數結構體指標
 * @return uint8_t TaskID
 */
uint8_t HWInt_reg(HWIntStr_t *Str_p, Func_t FBFunc_p, void *FBPara_p);

/**
 * @brief
 *禁致能己登錄特定硬體中斷中等待由特定中斷於中斷時執行的工作方塊
 * @param Str_p 特定硬體中斷服務常式分享管理結構體指標
 * @param TaskID 登錄進特定硬體中斷時所取得的工作識別編號
 * @param Enable 禁致能參數，1: 致能，0:禁能
 * @return uint8_t
 */
uint8_t HWInt_en(HWIntStr_t *Str_p, uint8_t TaskID, uint8_t Enable);

#define SPI_HW_NUM 1
extern HWIntStr_t *SpiIntStrList_p[SPI_HW_NUM];

#define TIM_HW_NUM 4
extern HWIntStr_t *TimIntStrList_p[TIM_HW_NUM];

#define UART_HW_NUM 2
extern HWIntStr_t *UartTxIntStrList_p[UART_HW_NUM];
extern HWIntStr_t *UartRxIntStrList_p[UART_HW_NUM];

#define PWM_HW_NUM 4
extern HWIntStr_t *PwmIntStrList_p[PWM_HW_NUM];


//=========ADC中斷=========

/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define ADCHWINT_LAY(INTSTR, ININUM, TASKNUM)          \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    AdcIntStrList_p[ININUM] = &INTSTR

/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define ADCHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)          \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.ADCSTR_ssp = &HWISTR;                           \
        FBPARASTR.ADCTaskId = TaskId;                             \
    }                                                             \
    HWInt_en(FBPARASTR.ADCSTR_ssp, FBPARASTR.ADCTaskId, ENABLE)

//=========計時中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define TIMHWINT_LAY(INTSTR, ININUM, TASKNUM)          \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    TimIntStrList_p[ININUM] = &INTSTR

/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define TIMHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)          \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.TIMSTR_ssp = &HWISTR;                           \
        FBPARASTR.TIMTaskId = TaskId;                             \
    }                                                             \
    HWInt_en(FBPARASTR.TIMSTR_ssp, FBPARASTR.TIMTaskId, ENABLE)

//=========PWM中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define PWMHWINT_LAY(INTSTR, ININUM, TASKNUM)          \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    PwmIntStrList_p[ININUM] = &INTSTR

/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define PWMHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)          \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.PWMSTR_ssp = &HWISTR;                           \
        FBPARASTR.PWMTaskId = TaskId;                             \
    }                                                             \
    HWInt_en(FBPARASTR.PWMSTR_ssp, FBPARASTR.PWMTaskId, ENABLE)

//=========外部中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define EXTHWINT_LAY(INTSTR, ININUM, TASKNUM)          \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    ExtIntStrList_p[ININUM] = &INTSTR


/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define EXTHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)          \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.EXTSTR_ssp = &HWISTR;                           \
        FBPARASTR.EXTTaskId = TaskId;                             \
    }                                                             \
    HWInt_en(FBPARASTR.EXTSTR_ssp, FBPARASTR.EXTTaskId, ENABLE)

//======================通訊============================

//=========UART TX中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define UARTTXHWINT_LAY(INTSTR, ININUM, TASKNUM)       \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    UartTxIntStrList_p[ININUM] = &INTSTR


/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define UARTTXHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)       \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.UARTTXSTR_ssp = &HWISTR;                        \
        FBPARASTR.UARTTXTaskId = TaskId;                          \
    }                                                             \
    HWInt_en(FBPARASTR.UARTTXSTR_ssp, FBPARASTR.UARTTXTaskId, ENABLE)

//=========UART RX中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define UARTRXHWINT_LAY(INTSTR, ININUM, TASKNUM)       \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    UartRxIntStrList_p[ININUM] = &INTSTR

/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define UARTRXHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)       \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.UARTRXSTR_ssp = &HWISTR;                        \
        FBPARASTR.UARTRXTaskId = TaskId;                          \
    }                                                             \
    HWInt_en(FBPARASTR.UARTRXSTR_ssp, FBPARASTR.UARTRXTaskId, ENABLE)

//=========SPI中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define SPIHWINT_LAY(INTSTR, ININUM, TASKNUM)          \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    SpiIntStrList_p[ININUM] = &INTSTR

/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define SPIHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)          \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.SPISTR_ssp = &HWISTR;                           \
        FBPARASTR.SPITaskId = TaskId;                             \
    }                                                             \
    HWInt_en(FBPARASTR.SPISTR_ssp, FBPARASTR.SPITaskId, ENABLE)

//=========TWI中斷=========
/**
 * @brief
 *呼叫本巨集會建立一個中斷服務分享器管理結構體HWIntStr_t以及搭配的分享工作表單，
 *並且將它掛進特定類型硬體中斷查詢表單XXXIntList_p[n]中，等待INTERNAL_ISR取用並執行
 *
 * @param INTSTR 中斷服務分享器的結構體命名
 * @param ININUM 本中斷服務分享器的結構體佔同類型中斷的編序
 * @param TASKNUM 允許被登錄至本中斷服務分享器中執行的工作最高數目
 */
#define TWIHWINT_LAY(INTSTR, ININUM, TASKNUM)          \
    HWIntStr_t INTSTR = {0};                           \
    INTSTR.MaxTask = TASKNUM;                          \
    {                                                  \
        static TaskBlock_t INTSTR##_TASKLIST[TASKNUM]; \
        INTSTR.Task_p = INTSTR##_TASKLIST;             \
    }                                                  \
    TwiIntStrList_p[ININUM] = &INTSTR

/**
 * @brief 
 * 本巨集協助使用者將功能方塊登錄為降頻工作執行器之其中一個工作，同時初始其禁致能
 * @param HWISTR 中斷服務分享器的結構體
 * @param FBFUNCA 使用者自訂ISR
 * @param FBPARASTR 使用者自訂ISR中需要用到的傳參結構
 * @param ENABLE 禁致能本工作在降頻執行器
 */
#define TWIHWINT_REG(HWISTR, FBFUNCA, FBPARASTR, ENABLE)          \
    {                                                             \
        uint8_t TaskID = HWInt_reg(&HWISTR, FBFUNCA, &FBPARASTR); \
        FBPARASTR.TWISTR_ssp = &HWISTR;                           \
        FBPARASTR.TWITaskId = TaskId;                             \
    }                                                             \
    HWInt_en(FBPARASTR.TWISTR_ssp, FBPARASTR.TWITaskId, ENABLE)

#define ADC_HW_NUM 1
extern HWIntStr_t *AdcIntStrList_p[ADC_HW_NUM];

#define EXT_HW_NUM 8
extern HWIntStr_t *ExtIntStrList_p[EXT_HW_NUM];

#define TWI_HW_NUM 1
extern HWIntStr_t *TwiIntStrList_p[TWI_HW_NUM];
/*-- hwimp section end -------------------------------------------------------*/

/*-- interrupt section start -------------------------------------------------*/
/**
 * @brief FuncBlock 工作管理結構
 * @ingroup interrupt_struct
 *
 * 此結構被應用在硬體中斷、IFD中斷中，為一項工作。
 * 結構中包含該工作是否開啟、觸發時要執行哪個函式、以及該函式的傳參。
 */
typedef struct {
    volatile uint8_t enable;  ///< 禁致能
    Func_t func_p;            ///< 觸發時執行函式
    void* funcPara_p;         ///< 觸發時執行函式之傳參
} FuncBlockStr_t;

/**
 * @brief type Array to content the pointer the phase and divi info of registed
 * Task
 *
 * @param Divi  Frequecy Divider of the
 * @param DiviCount Execute when DiviCount = Divi[i]
 * @param phase Execute when counter value = phase[i]
 */
typedef struct {
    volatile uint8_t Divi;
    volatile uint8_t DiviCount;
    volatile uint8_t phase;
} PhaseDivi_t;

/**
 * @brief 中斷除頻管理用結構體的住址指標
 *
 * @param Cycle Counts per period
 * @param Counter   count value
 * @param TaskId    Task Identification gotten
 * @param MaxTask   Maximum capacity for tasks to be Registed
 * @param Total Total number of Registed Tasks
 * @param Period_p Bytes of the HardWare Register 0
 * @param HWRegByte Hard ware Register for adjust period
 * @param Divi_p    pointer of PhaseDivi List
 * @param Task_p    pointer of Task List
 */
typedef struct {
    volatile uint8_t Cycle;
    volatile uint8_t Counter;
    volatile uint8_t TaskId;
    uint8_t MaxTask;
    uint8_t Total;
    uint8_t HWRegByte;
    volatile void *HWReg_p;
    volatile PhaseDivi_t *Divi_p;
    volatile TaskBlock_t *Task_p;
    uint16_t *Period_p;
} FreqReduStr_t;

/**
 * @brief 降頻工作執行器登錄函式，供使用者叫用以將功能方塊排入中斷除頻的排程表中
 *
 * @param FRSTR_ssp 中斷除頻管理用結構體的住址指標
 * @param FbFunc_p 中斷除頻後觸發執行之工作函式之住址指標(使用者資料)
 * @param FbPara_p 中斷除頻後觸發執行之工作函式專用結構體之住址指標(使用者傳參)
 * @param cycle
 * 每個循環週期計數次數，必需與其他登錄同一中斷除頻器的其他工作的循環週期相吻合
 * @param divi
 * @param phase 每個循環中，執行工作時機的中斷計數值
 */
uint8_t FreqRedu_reg(FreqReduStr_t* FRSTR_ssp, Func_t FbFunc_p, void* FbPara_p,
                     uint8_t divi, uint8_t phase);

/**
 * @brief
 * 降頻工作執行器禁致能函式，供使用者呼叫禁致能己登錄進中斷除頻中待執行工作方塊，僅有致能者，會在中斷除頻後被執行
 *
 * @param FRSTR_ssp 中斷除頻管理用結構體的住址指標
 * @param Task_Id 中斷除頻工作編號
 * @param Enable 禁致能狀態
 */
uint8_t FreqRedu_en(FreqReduStr_t* FRSTR_ssp, uint8_t Task_Id, uint8_t Enable);

/**
 * @brief
 * 降頻工作執行器執行函式，可與結構體共組成功能方塊之可執行函式，組成後可登錄進中斷服務函式待執行
 *
 * @param void_p 中斷除頻管理用結構體的住址指標
 */
void FreqRedu_step(void* void_p);

/**
 * @brief
 * 本巨集協助使用者定義降頻工作執行器結構體，緩衝列，然後鏈結兩者完成結構體的組建
 *
 * @param FRSTR 降頻執行器結構體名稱FreqReduStr_t type structure for this
 * Frequence Reduce executor
 * @param TASKNUM 降頻執行器容納登錄工作數 Maximum Number of Tasks alow to be
 * registed into this Frequence Reduce executor
 * @param CYCLE 降頻週期
 */
#define FREQREDU_LAY(FRSTR, TASKNUM, CYCLE, HWREGADD, REGBYTE, PERIODMATADD)   \
    FreqReduStr_t FRSTR = {0}; /*define structure*/                            \
    {                                                                          \
        static PhaseDivi_t                                                     \
            FRSTR##_DIVILIST[TASKNUM]; /*define phase and freq. divider list*/ \
        static TaskBlock_t FRSTR##_TASKLIST[TASKNUM]; /*define task list*/     \
        FRSTR.Cycle = CYCLE;                                                   \
        /*Initial the Cycle Number*/                                           \
        FRSTR.HWRegByte = REGBYTE;                                             \
        /*Initial Bytes of Hard ware Register */                               \
        FRSTR.HWReg_p = HWREGADD;                                              \
        /*Initial Hard ware Register Address*/                                 \
        FRSTR.Period_p = PERIODMATADD;                                         \
        /*Initial the Periodic list Matrix */                                  \
        FRSTR.MaxTask = TASKNUM; /*Initial the MaxTask*/                       \
        FRSTR.Task_p =                                                         \
            FRSTR##_TASKLIST; /*Net the list to the Task structure*/           \
        FRSTR.Divi_p =                                                         \
            FRSTR##_DIVILIST; /*Net the list to the Divi structure*/           \
    }

/**
 * @brief IFD工作結構
 * @ingroup interrupt_struct
 *
 * 負責管理一項工作的觸發週期、相位、計數等參數。
 * 為IFD管理器底下使用的結構之一。
 */
typedef struct {
    volatile uint16_t cycle;    ///< 計數觸發週期。
    volatile uint16_t phase;    ///< 計數觸發相位。
    volatile uint16_t counter;  ///< 計數器，計數器固定頻率上數。
    volatile uint8_t enable;  ///< 禁致能控制，決定本逾時ISR是否致能。
    volatile Func_t func_p;  ///< 執行函式指標，為無回傳、傳參為void*函式。
    void* funcPara_p;  ///< 中斷中執行函式專用結構化資料住址指標。
} IntFreqDivISR_t;

/**
 * @brief IFD管理器結構
 * @ingroup interrupt_struct
 *
 * 提供給中斷除頻功能，Interrupt Frequence Diveder(IFD)相關函式使用的結構。
 * 負責管理登記好的IFD工作。
 */
typedef struct {
    uint8_t total;  ///< 紀錄已註冊IFD工作數量
    volatile IntFreqDivISR_t fb[MAX_IFD_FUNCNUM];  ///< 紀錄所有已註冊IFD工作
} IntFreqDivStr_t;

/**
 * @brief 初始化IFD管理器
 *
 * @param IntFreqDivStr_p IFD管理器結構指標
 */
void IntFreqDiv_net(IntFreqDivStr_t* IntFreqDivStr_p);

/**
 * @brief 註冊一項工作到IFD管理器中。
 * @ingroup interrupt_func
 *
 * @param IntFreqDivStr_p IFD管理器的指標。
 * @param FbFunc_p 要註冊的函式，為無回傳、傳參為void*函式。
 * @param FbPara_p 要註冊函式的傳參。
 * @param cycle 循環週期。
 * @param phase 觸發相位，循環週期中的第幾次計數觸發。
 * @return uint8_t IFD工作編號。
 *
 * 此函式會在IFD管理器建立一項IFD工作，並會回傳其在管理器中的工作編號。
 * IFD工作預設為關閉，可以使用 IntFreqDiv_en 開啟。當 IntFreqDiv_step
 * 執行一次時，IFD工作的計數器便會上數，並依據參數設定的循環週期、
 * 觸發相位來決定要不要觸發並執行IFD工作。
 */
uint8_t IntFreqDiv_reg(IntFreqDivStr_t* IntFreqDivStr_p, Func_t FbFunc_p,
                       void* FbPara_p, uint16_t cycle, uint16_t phase);

/**
 * @brief 啟用/關閉指定的IFD工作。
 * @ingroup interrupt_func
 *
 * @param IntFreqDivStr_p IFD管理器的指標。
 * @param Fb_Id 要啟用的IFD工作編號。
 * @param enable 是否啟用，1:啟用、0:關閉。
 *
 * 此函式可以控制一項IFD工作要不要啟用，依照工作編號去開關IFD管理器中相對應編號
 * 的IFD工作。啟用後IFD工作中的計數器才會開始計數，也才能被觸發。
 *
 * 若輸入的編號還沒有被註冊，將不會有任何動作。
 */
void IntFreqDiv_en(IntFreqDivStr_t* IntFreqDivStr_p, uint8_t Fb_Id,
                   uint8_t enable);

/**
 * @brief 透過IFD管理器計數一次。
 * @ingroup interrupt_func
 *
 * @param IntFreqDivStr_p IFD管理器的指標。
 *
 * 此函式會去計數IFD管理器供登記並啟用的IFD工作，若計數大小與觸發相位相等時，
 * 便會執行IFD工作。
 *
 * 此函式可以被註冊到硬體中斷中，如此一來，中斷觸發時便會計數並觸發工作。也可
 * 以透過手動呼叫此函式來計數。
 */
void IntFreqDiv_step(IntFreqDivStr_t* IntFreqDivStr_p);
/*-- interrupt section end ---------------------------------------------------*/

/*-- hardware section start --------------------------------------------------*/
/**
 * @brief tim flag put 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char TIM_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief tim flag get 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char TIM_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief tim put 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TIM_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief tim get 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TIM_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief 與 TIM_fpt 相同。
 */
char PWM_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief 與 TIM_fgt 相同。
 */
char PWM_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief 與 TIM_put 相同。
 */
char PWM_put(volatile unsigned char *REG_p, char Bytes, void* Data_p);

/**
 * @brief 與 TIM_get 相同。
 */
char PWM_get(volatile unsigned char *REG_p, char Bytes, void* Data_p);

/**
 * @brief dio flag put 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief dio flag get 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
             void *Data_p);

/**
 * @brief dio put 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief dio get 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief 與 DIO_fpt 相同。
 */
char EXT_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief 與 DIO_fgt 相同。
 */
char EXT_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief adc flag put 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */

char ADC_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief adc flag get 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char ADC_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
             void *Data_p);

/**
 * @brief adc put 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char ADC_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief adc get 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char ADC_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief adc set 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char ADC_set(volatile unsigned char *REG_p, char Mask, char Shift, char Data);


/**
 * @brief spi flag put 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char SPI_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief spi flag get 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char SPI_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief spi put 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char SPI_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief spi get 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char SPI_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

#include <avr/interrupt.h>
#undef ISR
#ifdef __cplusplus
#    define ISR(vector, ...)                                                 \
        extern "C" void vector##_routine(void) __attribute__((__INTR_ATTRS)) \
            __VA_ARGS__;                                                     \
        void vector##_routine(void)
#else
#    define ISR(vector, ...)                                                   \
        void vector##_routine(void) __attribute__((__INTR_ATTRS)) __VA_ARGS__; \
        void vector##_routine(void)
#endif

/**
 * @brief EEPROM設定函式
 *
 * @ingroup eeprom_func
 * @param Address 要寫入的EEPROM位址。
 * @param Bytes   資料大小。
 * @param Data_p  資料指標。
 */
void EEPROM_set(int Address, char Bytes, void *Data_p);

/**
 * @brief EEPROM get 函式
 *
 * @ingroup eeprom_func
 * @param Address 要讀取的EEPROM位址。
 * @param Bytes   資料大小。
 * @param Data_p  資料指標。
 */
void EEPROM_get(int Address, char Bytes, void *Data_p);

/**
 * @brief twi flag put 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 */
char TWI_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief twi flag get 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 */
char TWI_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
             void *Data_p);

/**
 * @brief twi put 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TWI_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief twi get 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TWI_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief twi set 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 */
char TWI_set(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief UART flag put函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char UART_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief ASA UART flag get函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char UART_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
              void *Data_p);

/**
 * @brief ASA UART put函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char UART_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief ASA UART get函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char UART_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);
/*-- hardware section end ----------------------------------------------------*/

/*-- device section start ----------------------------------------------------*/
/**
 * @brief 進行硬體初始化的動作，包含asabus、stdio初始化。
 * @ingroup device_func
 *
 * 針對不同的硬體平台，將硬體初始化，包含下列動作：
 *   1. STDIO 及硬體初始化：請參照 C4M_STDIO_init。
 *   2. ASABUS ID 硬體初始化：請參照 ASABUS_ID_init。
 *   3. ASABUS SPI 硬體初始化：請參照 ASABUS_SPI_init。
 *   4. ASABUS UART 硬體初始化：請參照 ASABUS_UART_init。
 *   5. EEPROM 初始化及讀取裝置ID。
 *   6. 外掛函式系統的初始化。
 */
void C4M_DEVICE_set(void);
/*-- device section end ------------------------------------------------------*/

/*-- stdio section start -----------------------------------------------------*/
/**
 * @brief 進行標準IO進行初始化的動作。
 * @ingroup stdio_func
 *
 * 依據各個硬體平台去連結不同的通訊方式到 standard IO 中，
 * 便有printf、scanf等的標準IO操作函式可以使用。
 */
void C4M_STDIO_init(void);
/*-- stdio section end -------------------------------------------------------*/

/*-- asabus section start ----------------------------------------------------*/
/**
 * @brief 初始化ASABUS上的ID腳位。
 * @ingroup asabus_func
 *
 * ASABUS 上的 ID 腳位共有三隻，分別為ADDR0、ADDR1、ADDR2。
 * 呼叫此函式將會對前述三隻腳位進行初始化，設定為輸出。
 *
 * 以下是常見的板子上的ADDR腳位
 * asam128_v2:
 *  - ADDR0 : PF5
 *  - ADDR1 : PF6
 *  - ADDR2 : PF7
 *
 * 注意：在呼叫 C4M_DEVIDE_set 的時候也會執行此函式。
 */
void ASABUS_ID_init(void);

/**
 * @brief 設定ASABUS上使用的ID，0~7。
 * @ingroup asabus_func
 *
 * @param id 要設定的 ID 編號，0~7。
 *
 * 可以設定的ID為0~7，轉換成2進位每個bit分別對應到 ADDR0、ADDR1、ADDR2。
 * 若該bit被設定為1，則會令對應腳位輸出 1(logic high)。如果傳入 id
 * 編號非0~7，則不會進行設定的動作。
 *
 * 以下是常見的板子上的ADDR腳位<br>
 *  - asam128_v2:
 *    - ADDR0 : PF5
 *    - ADDR1 : PF6
 *    - ADDR2 : PF7
 */
void ASABUS_ID_set(char id);

/**
 * @brief 初始化ASABUS上的UART硬體。
 * @ingroup asabus_func
 *
 * 針對 ASABUS 上所使用的 UART 硬體進行初始化，會設定鮑率為38400、
 * 停止位元為1、傳輸大小為8位元。
 *
 * 注意：在呼叫 C4M_DEVIDE_set 的時候也會執行此函式。
 * 
 * 以下是常見的板子使用的uart硬體編號：<br>
 *  - asam128_v2: uart0
 */
void ASABUS_UART_init(void);

/**
 * @brief 將資料透過 ASABUS UART 傳送。
 * @ingroup asabus_func
 *
 * 等待並接收 ASABUS UART 來的資料，沒有逾時機制，
 * 所以若硬體錯誤將不會離開此函式。
 */
void ASABUS_UART_transmit(char data);

/**
 * @brief 等待並接收 ASABUS UART 來的資料。
 * @ingroup asabus_func
 *
 * @return char 接收回來的資料。
 *
 * 沒有逾時機制，所以若硬體錯誤將不會離開此函式。
 * 所以若硬體錯誤將不會離開此函式。
 */
char ASABUS_UART_receive(void);

/**
 * @brief 初始化ASABUS上的SPI硬體。
 * @ingroup asabus_func
 *
 * 針對 ASABUS 上所使用的 SPI 硬體進行初始化，設定為master模式，
 * SCK頻率則會因硬體而異。
 *
 * 注意：在呼叫 C4M_DEVIDE_set 的時候也會執行此函式。
 *
 * 以下是常見的板子使用的spi及初始化參數：<br>
 *  - asam128_v2: spi
 *    - SCK 頻率: fosc/64 = 11059200/64 = 172.8 kHz。
 *    - CPOL = 0，前緣為上升緣。
 *    - CPHA = 0，取樣時間為前緣。
 */
void ASABUS_SPI_init(void);

/**
 * @brief 與 ASABUS SPI 交換資料。
 * @ingroup asabus_func
 *
 * @param data 要交換的資料。
 * @return char 交換回來的資料。
 *
 * 沒有逾時機制，所以若硬體錯誤將不會離開此函式。
 * 
 * 以下是常見的板子使用的uart硬體編號：<br>
 *  - asam128_v2: spi
 */
char ASABUS_SPI_swap(char data);
/*-- asabus section end ------------------------------------------------------*/

/*-- asahmi section start ----------------------------------------------------*/
// HMI macro
#define HMI_TYPE_I8 \
    0  ///< 資料型態編號 0，int8_t、char                @ingroup asahmi_macro
#define HMI_TYPE_I16 \
    1  ///< 資料型態編號 1，int16_t、int                @ingroup asahmi_macro
#define HMI_TYPE_I32 \
    2  ///< 資料型態編號 2，int32_t、long int           @ingroup asahmi_macro
#define HMI_TYPE_I64 \
    3  ///< 資料型態編號 3，int64_t                     @ingroup asahmi_macro
#define HMI_TYPE_UI8 \
    4  ///< 資料型態編號 4，uint8_t、unsigned char      @ingroup asahmi_macro
#define HMI_TYPE_UI16 \
    5  ///< 資料型態編號 5，uint16_t、unsigned int      @ingroup asahmi_macro
#define HMI_TYPE_UI32 \
    6  ///< 資料型態編號 6，uint32_t、unsigned long int @ingroup asahmi_macro
#define HMI_TYPE_UI64 \
    7  ///< 資料型態編號 7，uint64_t                    @ingroup asahmi_macro
#define HMI_TYPE_F32 \
    8  ///< 資料型態編號 8，float、double               @ingroup asahmi_macro
#define HMI_TYPE_F64 \
    9  ///< 資料型態編號 9，AVR不支援64位元浮點數        @ingroup asahmi_macro

// HMI declaration
/**
 * @brief 發送陣列(1D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的個數。
 * @param Data_p 存放陣列的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼：：
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *
 * 透過開發版上與PC的接口發送一維陣列資料，發送以 Data_p 為記憶體開頭的陣列資料
 * ，通常為一維的C陣列，會讀取 Num * 形態大小 Bytes的記憶體，若存取記憶體大小超
 * 過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參數 Num
 * 與實際陣列的個數吻和。
 */
char HMI_put_array(char Type, char Num, void *Data_p);

/**
 * @brief 從HMI接收陣列(1D)。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的資料個數。
 * @param Data_p 陣列的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼：：
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非陣列封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：陣列個數對應錯誤，發送端與接收端的資料個數不吻合。
 *
 * 透過開發版上與PC的接口接收一維陣列資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_get_array(char Type, char Num, void *Data_p);

/**
 * @brief 發送矩陣(2D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 矩陣的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *
 * 透過開發版上與PC的接口發送二維矩陣資料，發送以 Data_p 為記憶體開頭的矩陣資料
 * ，通常為二維的C陣列，會讀取 Dim1 * Dim2 * 形態大小 Bytes的記憶體，若存取記
 * 憶體大小超過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參
 * 數Dim1、Dim2與實際矩陣的個數吻和。
 */
char HMI_put_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 從HMI接收矩陣(2D)。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 存放矩陣的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：矩陣維度一對應錯誤，發送端與接收端的維度一大小不吻合。
 *   - 6：矩陣維度二對應錯誤，發送端與接收端的維度二大小不吻合。
 *
 * 透過開發版上與PC的接口接收二維矩陣資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_get_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 發送結構到HMI。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 結構的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char  錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_put_struct(const char *FormatString, int Bytes, void *Data_p);

/**
 * @brief 從HMI接收結構。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 存放結構的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：結構格式字串對應錯誤，發送端與接收端的結構格式字串不吻合。
 *   - 5：結構大小對應錯誤，發送端與接收端的結構大小不吻合。
 *
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_get_struct(const char *FormatString, int Bytes, void *Data_p);

/**
 * @brief 主動同步後，發送陣列(1D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的個數。
 * @param Data_p 存放陣列的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。
 * 透過開發版上與PC的接口發送一維陣列資料，發送以 Data_p 為記憶體開頭的陣列資料
 * ，通常為一維的C陣列，會讀取 Num * 形態大小 Bytes的記憶體，若存取記憶體大小超
 * 過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參數 Num
 * 與實際陣列的個數吻和。
 */
char HMI_snput_array(char Type, char Num, void *Data_p);

/**
 * @brief 主動同步後，從HMI接收陣列(1D)。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的資料個數。
 * @param Data_p 陣列的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非陣列封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：陣列個數對應錯誤，發送端與接收端的資料個數不吻合。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口接收一維陣列資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_snget_array(char Type, char Num, void *Data_p);

/**
 * @brief 主動同步後，發送矩陣(2D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 矩陣的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口發送二維矩陣資料，發送以 Data_p 為記憶體開頭的矩陣資料
 * ，通常為二維的C陣列，會讀取 Dim1 * Dim2 * 形態大小 Bytes的記憶體，若存取記
 * 憶體大小超過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參
 * 數Dim1、Dim2與實際矩陣的個數吻和。
 */
char HMI_snput_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 主動同步後，從HMI接收矩陣(2D)。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 存放矩陣的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：矩陣維度一對應錯誤，發送端與接收端的維度一大小不吻合。
 *   - 6：矩陣維度二對應錯誤，發送端與接收端的維度二大小不吻合。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口接收二維矩陣資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_snget_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 主動同步後，發送結構到HMI。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 結構的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char  錯誤代碼： <br>
 *   - 0：成功無誤。<br>
 *   - 1：資料總大小超過30000，請自行切分，並發送。<br>
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_snput_struct(const char *FormatString, int Bytes, void *Data_p);

/**
 * @brief 主動同步後，從HMI接收結構。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 存放結構的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char  錯誤代碼： <br>
 *   - 0：成功無誤。<br>
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。<br>
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。<br>
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。<br>
 *   - 4：結構格式字串對應錯誤，發送端與接收端的結構格式字串不吻合。<br>
 *   - 5：結構大小對應錯誤，發送端與接收端的結構大小不吻合。<br>
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_snget_struct(const char *FormatString, int Bytes, void *Data_p);
/*-- asahmi section end ------------------------------------------------------*/

/*-- asatwi section start ----------------------------------------------------*/
/**
 * @brief ASA TWI Master 多位元組傳送函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：1、2、3、4、5、6。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待送資料位元組數。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *
 * ASA TWI Master Transmit 函式，依照功能分為6種Mode，通訊方式如下：
 *  - mode 1：
 *      TWI通訊第一 Bytes 為 [RegAdd | Data_p[Bytes-1]] ，並由高至低傳輸，
 *      使用者須自行將Data_p的最高位元向右位移RegAdd bits數。
 *      此mode中的RegAdd為控制旗標(control flag)。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 2：
 *      TWI通訊第一 Bytes 為 [RegAdd | Data_p[0]] ，並由低至高傳輸，
 *      使用者須自行將Data_p的最高位元向右位移RegAdd bits數。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 3：TWI通訊不指定RegAdd，並由高至低傳輸。
 *  - mode 4：TWI通訊不指定RegAdd，並由低至高傳輸。
 *  - mode 5：TWI通訊指定RegAdd，並由高至低傳輸。
 *  - mode 6：TWI通訊指定RegAdd，並由低至高傳輸。
 */
char TWIM_trm(char mode, char SLA, char RegAdd, char Bytes, uint8_t *Data_p, uint16_t WaitTick);

/**
 * @brief ASA TWI Master 多位元組接收函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：1、2、3、4、5、6。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待收資料位元組數。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *
 * ASA TWI Master receive 函式，依照功能分為6種Mode，通訊方式如下：
 *  - mode 1：TWI通訊第一Bytes為[RegAdd | Data_p[Bytes-1]]，並由高至低接收。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - mode 2：TWI通訊第一Bytes為[RegAdd | Data_p[0]]，並由低至高接收。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - mode 3：TWI通訊不指定RegAdd，並由高至低接收。
 *  - mode 4：TWI通訊不指定RegAdd，並由低至高接收。
 *  - mode 5：TWI通訊指定RegAdd，並由高至低接收。
 *  - mode 6：TWI通訊指定RegAdd，並由低至高接收。
 */
char TWIM_rec(char mode, char SLA, char RegAdd, char Bytes, uint8_t *Data_p,
              uint16_t WaitTick);

/**
 * @brief ASA TWI Master 旗標式傳送函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：3、5。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     待送旗標向左位移。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * TWI Master(主)旗標式資料傳輸是由TWIM_trm()與TWIM_rec()實現，
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - mode 3：
 *      使用TWIM_rec() mode 3讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用TWIM_trm() mode 3傳輸資料。
 *
 *  - mode 5：
 *      使用TWIM_rec() mode 5讀取指定RegAdd中的資料，將資料左移後，
 *      用遮罩取資料，最後使用TWIM_trm() mode 5傳輸資料到指定RegAdd。
 */
char TWIM_ftm(char mode, char SLA, char RegAdd, char Mask, char Shift,
              uint8_t *Data_p, uint16_t WaitTick);

/**
 * @brief ASA TWI Master 旗標式接收函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：3、5。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     接收旗標向右位移。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * TWI Master(主)旗標式資料傳輸是由TWIM_rec()實現，
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - mode 3：使用TWIM_rec() mode 3讀取資料，將資料左移後，用遮罩取資料。
 *  - mode 5：
 *      使用TWIM_rec() mode 5讀取指定RegAdd中的資料，將資料左移後，
 *      用遮罩取資料。
 */
char TWIM_frc(char mode, char SLA, char RegAdd, char Mask, char Shift,
              uint8_t *Data_p, uint16_t WaitTick);

/**
 * @brief TWI Slave Mode 1 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 1 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 1，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由高至低傳輸。
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個
 *      Register(Reg_ID = 2)，由高至低接收。
 *
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS1_step(void);

/**
 * @brief TWI Slave Mode 2 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 2 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 2，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由低至高傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個
 *      Register(Reg_ID = 2)，由低至高接收。
 *
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS2_step(void);

/**
 * @brief TWI Slave Mode 3 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 3 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 3，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由高至低傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個
 *      Register(Reg_ID = 2)，由高至低接收。
 *
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS3_step(void);

/**
 * @brief TWI Slave Mode 4 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 4 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 4，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由低至高傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個，
 *      Register(Reg_ID = 2)，由低至高接收。
 *  
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS4_step(void);

/**
 * @brief TWI Slave Mode 5 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 * TWI Slave(僕) Mode 5 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 5，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的指定Register，由高至低傳輸。
 * 
 *  - TWI Slave Recive：
 *      將 Master 接收到的資料儲存到 RemoRW_reg 所註冊的指定 
 *      Register，由高至低接收。
 */
void ASA_TWIS5_step(void);

/**
 * @brief TWI Slave Mode 6 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 * TWI Slave(僕) Mode 6 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 6，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的指定Register，由低至高傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的指定
 *      Register，由低至高接收。
 */
void ASA_TWIS6_step(void);

#define TIMEOUTSETTING 500000  ///< TWI逾時設定   @ingroup asatwi_macro
#define CF_BIT 3               ///< TWI Contral Flag bit數設定 @ingroup asatwi_macro
#define CF_MASK 0xE0           ///< TWI Contral Flag 遮罩 @ingroup asatwi_macro
#define TWAR_MASK 0xFE         ///< TWI TWAR 遮罩 @ingroup asatwi_macro
#define TIMEOUT_FLAG 0X01      ///< TWI 逾時期標 @ingroup asatwi_macro
#define REG_MAX_COUNT 20       ///< TWI 暫存計數器最大值 @ingroup asatwi_macro
#define BUFF_MAX_SZ 32         ///< TWI BUFFER最大值 @ingroup asatwi_macro

/**
 * @brief TWI Hardware 傳輸
 *
 * @ingroup     asatwi_func
 * @param reg   傳輸資料。
 * @return  uint8_t 錯誤代碼：
 *                   - 1：Timeout
 *                   - 其餘編號：參考TWI狀態列表。
 *
 * 將欲傳送資料存入TWI Data Register (TWDR) 中，並對TWI Configer Register
 * (TWCR)中的TWINT和TWEN寫入。
 */
uint8_t TWI_Reg_tram(uint8_t reg);

/**
 * @brief TWI Hardware 接收 ACK回覆
 *
 * @ingroup     asatwi_func
 * @param data  指標指向資料。
 * @return  uint8_t 錯誤代碼：
 *                   - 1：Timeout
 *                   - 其餘編號：參考TWI狀態列表。
 *
 * 對TWI Configer Register (TWCR)中的TWINT、TWEN和TWEA寫入，並將TWI Data
 * Register (TWDR) 接收到的資料存入指定位址指標中。
 */
uint8_t TWI_Reg_rec(uint8_t *data);
/**
 * @brief TWI Hardware 接收 NACK回覆
 *
 * @ingroup     asatwi_func
 * @param data  指標指向資料。
 * @return  uint8_t 錯誤代碼：
 *                   - 1：Timeout
 *                   - 其餘編號：參考TWI狀態列表。
 *
 * 對TWI Configer Register (TWCR)中的TWINT和TWEN寫入，並將TWI Data
 * Register (TWDR) 接收到的資料存入指定位址指標中。
 */
uint8_t TWI_Reg_recNack(uint8_t *data);
/**
 * @brief TWI Hardware Acknowledge
 *
 * @ingroup asatwi_func
 * @param ack_p Acknowledge Flag
 *
 * 決定TWI Configer Register 中的Acknowledge bit(TWEA)是否要enable。
 */
void TWICom_ACKCom(uint8_t ack_p);
/**
 * @brief TWI Hardware Stop
 *
 * @ingroup asatwi_func
 * @param stop_p Stop Flag
 * 
 * 決定TWI Configer Register 中的Stop bit(TWSTO)是否要enable。
 */
void TWICom_Stop(uint8_t stop_p);
/**
 * @brief TWI Hardware Start
 *
 * @ingroup asatwi_func
 * @param _start start Flag
 *
 * 決定TWI Configer Register 中的Start bit(TWSTA)是否要enable。
 */
uint8_t TWICom_Start(uint8_t _start);
/*-- asatwi section end ------------------------------------------------------*/

/*-- asauart section start ---------------------------------------------------*/
/**
 * @brief ASA UART Master 多位元組傳送函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 1, 2, 3。
 * @param UartID    目標裝置的UART ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待送資料位元組數。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * ASA 作為 Master端 透過UART通訊傳送資料。
 * Uart Master端 傳送給 Slave端，依照封包不同，分作11種 Mode ，如以下所示：
 *  - Mode 0：
 *      封包組成 為 [Header]、[UID]、[RegAdd]、[Data]、[Rec(Header)]。
 *      先後傳送資料 [ASAUART_CMD_HEADER (0xAA)]、[UID]、[RegAdd]，隨後再根據
 *      資料筆數[Bytes]由低到高丟出資料，傳完資料後會傳送最後一筆接收的資料
 *      [checksum] 給Slave端驗證，Slave會回傳當作 [ASAUART_RSP_HEADER(0xAB)]
 *      成功資訊或錯誤資訊(0x06)。
 *  - Mode 1：
 *      封包組成 為 [ Data+CF(Control Flag) ]。
 *      傳送第一筆資料為 [RegAdd | Data_p[0]]，剩餘的傳輸資料由低到高傳送。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 2：
 *      封包組成 為 [ CF(Control Flag)+Data ]。
 *      傳送第一筆資料為 [RegAdd |Data_p[Bytes-1]]，剩餘的傳輸資料由高到低傳送。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 3：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由低到高傳送資料。
 *  - Mode 4：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由高到低傳送資料。
 *  - Mode 5：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5 專門用來接收資料。
 *      先傳送一筆資料為 [RegAdd]，接著由低到高傳送資料 [Data]。
 *  - Mode 6：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5 專門用來接收資料。
 *      先傳送一筆資料 [RegAdd]，接著由高到低傳送資料 [Data]。
 *  - Mode 7：
 *      封包組成 為 [ W (1bit，0)+RegAdd(7bit) ]、[Data]。
 *      先傳送一筆資料 [RegAdd]，接著由低到高傳送資料 [Data]。
 *  - Mode 8：
 *      封包組成 為 [ W (1bit，0)+RegAdd(7bit) ] 、[Data]。
 *      先傳送一筆資料 [RegAdd]，接著由高到低傳送資料 [Data]。
 *  - Mode 9：
 *      封包組成 為 [ RegAdd(7bit)+W (1bit，0) ]、[Data]。
 *      先傳送一筆資料 [RegAdd<<1]，接著由低到高傳送資料 [Data]。
 *  - Mode 10：
 *      封包組成 為 [ RegAdd(7bit)+W (1bit，0) ]、[Data]。
 *      先傳送一筆資料 [RegAdd<<1]，接著由高到低傳送資料 [Data]。
 */
char UARTM_trm(char Mode, char UartID, char RegAdd, char Bytes, void *Data_p,uint16_t WaitTick);

/**
 * @brief ASA UART Master 多位元組接收函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 1, 2, 3。
 * @param UartID    UART僕ID：   目標裝置的UART ID
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待收資料位元組數。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * Uart Master端 傳送給 Slave端，依照封包不同，分作11種 Mode ，如以下所示：
 *  - Mode 0：
 *      封包組成 為 [Header]、[UID]、[RegAdd]、[Data]、[Rec(Header)]。
 *      先後傳送資料 [ASAUART_CMD_HEADER (0xAA)]、[UID]、[RegAdd]、[checksum]
 *      後，讀取Slave端回傳的[ASAUART_RSP_HEADER]，確認無誤後 再根據資料筆數
 *      [Bytes] 由低到高接收資料，接收資料完後會接收Slave傳送的最後一筆資料
 *      [checksum] 進行比對，無誤後將資料存取起來。
 *  - Mode 1：
 *      封包組成 為 [ Data+CF(Control Flag) ]。
 *      注意：限定搭配 UARTM_trm()後 使用。
 *      由低到高接收資料。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 2：
 *      封包組成 為 [ CF(Control Flag)+Data ]。
 *      注意：限定搭配 UARTM_trm()後 使用。
 *      由高到低接收資料。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 3：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由低到高接收資料。
 *  - Mode 4：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由高到低接收資料。
 *  - Mode 5：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5
 *      專門用來接收資料。先傳送一筆資料為 [RegAdd] ，
 *      接著由低到高接收資料 [Data]。
 *  - Mode 6：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5
 *      專門用來接收資料。先傳送一筆資料 [RegAdd]，
 *      接著由高到低接收資料 [Data]。
 *  - Mode 7：
 *      封包組成 為 [ R (1bit，1)+RegAdd(7bit) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最低位元、R位元佔最高位元
 *      [0x80 | RegAdd]，接著由低到高接收資料 [Data]。
 *  - Mode 8：
 *      封包組成 為 [ R (1bit，1)+RegAdd(7bit) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最低位元、R位元佔最高位元
 *      [0x80 | RegAdd]，接著由高到低接收資料 [Data]。
 *  - Mode 9：
 *      封包組成 為 [ RegAdd(7bit)+R (1bit，1) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最高位元、R位元佔最低位元
 *      [RegAdd<<1 | 0x01]，接著由低到高接收資料 [Data]。
 *  - Mode 10：
 *      封包組成 為 [ RegAdd(7bit)+R (1bit，1) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最高位元、R位元佔最低位元
 *      [RegAdd<<1 | 0x01]，接著由高到低接收資料 [Data]。
 */
char UARTM_rec(char Mode, char UartID, char RegAdd, char Bytes, void *Data_p,uint16_t WaitTick);

/**
 * @brief ASA UART Master 旗標式傳送函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 3, 5, 7, 9。
 * @param UartID    UART僕ID：      目標的裝置UART ID
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     待送旗標向左位移。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * Uart Master 旗標式資料傳輸是由UARTM_trm()與UARTM_rec()實現。
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - Mode 0： 使用UARTM_rec() Mode 0 讀取指定UID、RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 0
 *    傳輸資料到指定RegAdd。
 *  - Mode 3： 使用UARTM_rec() Mode 3 讀取資料，將資料左移後，
 *    用遮罩取資料，最後使用UARTM_trm() Mode 3 傳輸資料。
 *  - Mode 5： 使用UARTM_rec() Mode 5 讀取指定RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 5
 *    傳輸資料到指定RegAdd。
 *  - Mode 7： 使用UARTM_rec() Mode 7 讀取指定RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 7
 *    傳輸資料到指定RegAdd。
 *  - Mode 9： 使用UARTM_rec() Mode 9 讀取指定RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 9
 *    傳輸資料到指定RegAdd。
 *
 */
char UARTM_ftm(char Mode, char UartID, char RegAdd, char Mask, char Shift,
               char *Data_p, uint16_t WaitTick);

/**
 * @brief ASA UART Master 旗標式接收函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 3, 5 ,7 ,9。
 * @param UartID    UART僕ID：      目標的裝置UART ID
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     接收旗標向右位移。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * Uart Master 旗標式資料接收是由TWIM_rec()實現。
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - Mode 0：
 *      使用UARTM_rec() Mode 0 讀取指定UID、RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 *  - Mode 3：
 *      使用UARTM_rec() Mode 3 讀取資料，將資料左移後，
 *      用遮罩取資料。
 *  - Mode 5：
 *      使用UARTM_rec() Mode 5 讀取指定RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 *  - Mode 7：
 *      使用UARTM_rec() Mode 7 讀取指定RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 *  - Mode 9：
 *      使用UARTM_rec() Mode 9 讀取指定RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 */
char UARTM_frc(char Mode, char UartID, char RegAdd, char Mask, char Shift,
               char *Data_p, uint16_t WaitTick);

/**
 * @brief Slave端 Mode 0 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 *
 * @warning 必須先將UART Interrupt致能，並注意是否已於 RemoReg_init()
 * 內實作將此函式連接至uart_hal.c內的UARTS_inst.rx_compelete_cb。
 */
void ASA_UARTS0_rx_step(void);

/**
 * @brief Slave端 Mode 1 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 *
 */
void ASA_UARTS1_rx_step(void);

/**
 * @brief Slave端 Mode 2 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */

void ASA_UARTS2_rx_step(void);

/**
 * @brief Slave端 Mode 3 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS3_rx_step(void);

/**
 * @brief Slave端 Mode 4 串列埠 Rx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS4_rx_step(void);

/**
 * @brief Slave端 Mode 5 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS5_rx_step(void);

/**
 * @brief Slave端 Mode 6 串列埠 Rx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS6_rx_step(void);

/**
 * @brief Slave端 Mode 7 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS7_rx_step(void);

/**
 * @brief Slave端 Mode 8 串列埠 Rx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS8_rx_step(void);

/**
 * @brief Slave端 Mode 9 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS9_rx(void);

/**
 * @brief Slave端 Mode 10 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS10_rx_step(void);

/**
 * @brief Slave端 Mode 0 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS0_tx_step(void);

/**
 * @brief Slave端 Mode 1 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS1_tx_step(void);

/**
 * @brief Slave端 Mode 2 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS2_tx_step(void);

/**
 * @brief Slave端 Mode 3 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS3_tx_step(void);

/**
 * @brief Slave端 Mode 4 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS4_tx_step(void);

/**
 * @brief Slave端 Mode 5 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS5_tx_step(void);

/**
 * @brief Slave端 Mode 6 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS6_tx_step(void);

/**
 * @brief Slave端 Mode 7 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS7_tx_step(void);

/**
 * @brief Slave端 Mode 8 串列埠 Tx中斷 執行片段。。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。。
 */
void ASA_UARTS8_tx_step(void);

/**
 * @brief Slave端 Mode 9 串列埠 Tx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS9_tx_step(void);

/**
 * @brief Slave端 Mode 10 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS10_tx_step(void);
/*-- asauart section end -----------------------------------------------------*/

/*-- asaspi section start ----------------------------------------------------*/
/**
 * @brief ASA SPI Master 多位元組傳送函式。
 *
 * @ingroup asaspi_func
 * @param mode    SPI通訊模式，目前支援：0~10。
 * @param ASAID   ASA介面卡的ID編號。
 * @param RegAdd  遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes   資料筆數。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @param *Data_p 待送資料指標。
 * @return char   錯誤代碼：
 *                 - 0：成功無誤。
 *                 - 5：模式選擇錯誤。
 *                 - 其他：錯誤。
 *
 * SPI Master傳輸給Slave裝置，依照功用分為10種mode，傳送方式如下：
 *  - mode 0 ：
 *      具check的SPI通訊方式，第一筆傳送給Slave端為[W | RegAdd]，
 *      之後開始傳送資料Data_p[0]直到Data[Bytes-1]，每次傳送給Slave端，
 *      Slave端會回傳上一筆資料給Master端，Master端會進行檢查，如果不一樣，
 *      再傳送最後一筆資料給Slave的時候，Slave會回傳非0的錯誤資訊。
 *
 *  - mode 1 ：
 *      SPI通訊的第一筆為[RegAdd | Data_p[Bytes-1]]，
 *      剩餘的傳輸資料由低到高傳輸。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 2 ：
 *      SPI通訊的第一筆為[RegAdd | Data_p[0]]，
 *      剩餘的傳輸資料由高到低傳輸。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 3 ：SPI通訊的傳輸資料由低到高傳輸，純送資料，未送RegAdd。
 *  - mode 4 ：SPI通訊的傳輸資料由高到低傳輸，純送資料，未送RegAdd。
 *  - mode 5 ：SPI通訊的第一筆為[RegAdd]，傳輸資料由低到高傳輸。
 *  - mode 6 ：SPI通訊的第一筆為[RegAdd]，傳輸資料由高到低傳輸。
 *  - mode 7 ：SPI通訊的第一筆為[W | RegAdd]，傳輸資料由低到高傳輸。
 *  - mode 8 ：SPI通訊的第一筆為[W | RegAdd]，傳輸資料由高到低傳輸。
 *  - mode 9 ：SPI通訊的第一筆為[(RegAdd<<1) | W]，傳輸資料由低到高傳輸。
 *  - mode 10：SPI通訊的第一筆為[(RegAdd<<1) | W]，傳輸資料由高到低傳輸。
 *  - mode 100以上，為外掛式SPI Master(主)傳輸資料。
 */
char ASA_SPIM_trm(char mode, char ASAID, char RegAdd, char Bytes, void *Data_p,
                  uint16_t WaitTick);

/**
 * @brief ASA SPI Master 多位元組接收函式。
 *
 * @ingroup asaspi_func
 * @param mode    SPI通訊模式，目前支援：0、3~10。
 * @param ASAID   ASA介面卡的ID編號。
 * @param RegAdd  控制旗標(control flag)或遠端讀寫暫存器的ID.
 * @param Bytes   資料筆數。
 * @param WaitTick 位元組間延遲時間，單位為 1us。
 * @param *Data_p 待收資料指標。
 * @return char   錯誤代碼：
 *                 - 0：成功無誤。
 *                 - 5：模式選擇錯誤。
 *                 - 其他：錯誤。
 *
 * SPI Master從Slave裝置接收資料，依照功用分為10種mode，接收方式如下：
 *  - mode 0 ：
 *      具check的SPI通訊方式，第一筆傳送給Slave端為[RegAdd]，
 *      之後開始由低到高接收資料依序儲存到 *Data_p，
 *      接收完成後會傳送最後一筆接收的資料給Slave端驗證，
 *      Slave會回傳正確或錯誤資訊。
 *
 *  - mode 1 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 3 ：SPI通訊的第一筆為[0x00]]，純接資料，由低到高依序存入*Data_p。
 *  - mode 4 ：SPI通訊的第一筆為[0x00]]，純接資料，由高到低依序存入*Data_p。
 *  - mode 5 ：SPI通訊的，和Slave交換資料由低到高傳輸。
 *  - mode 6 ：SPI通訊的，和Slave交換資料由高到低傳輸。
 *  - mode 7 ：SPI通訊的第一筆為[R | RegAdd]，之後和Slave交換資料由低到高傳輸。
 *  - mode 8 ：SPI通訊的第一筆為[R | RegAdd]，之後和Slave交換資料由高到低傳輸。
 *  - mode 9 ：
 *      SPI通訊和Slave交換資料第一筆為[(RegAdd<<1) | R]，
 *      之後交換資料由低到高傳輸。
 *  - mode 10：
 *      SPI通訊和Slave交換資料第一筆為[(RegAdd<<1) | R]，
 *      之後交換資料由高到低傳輸。
 *  - mode 100以上，為外掛式SPI Master(主)接收資料。
 */
char ASA_SPIM_rec(char mode, char ASAID, char RegAdd, char Bytes, void *Data_p,
                  uint16_t WaitTick);

/**
 * @brief ASA SPI Master 旗標式接收函式。
 *
 * @ingroup asaspi_func
 * @param mode     SPI通訊模式，目前支援：0、3~10。
 * @param ASAID    ASA介面卡的ID編號。
 * @param RegAdd   遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask     位元組遮罩。
 * @param Shift    接收旗標向右位移。
 * @param WaitTick 位元組間延遲時間，單位為 1us。
 * @param *Data_p  待收資料指標。
 * @return char    錯誤代碼：
 *                  - 0：成功無誤。
 *                  - 5：模式選擇錯誤。
 *                  - 其他：錯誤。
 *
 * SPI Master從Slave裝置接收資料，依照功用分為10種mode，接收方式如下：
 *  - mode 0 ：
 *    具check的SPI通訊方式，先使用ASA_SPIM_rec mode 0 讀取資料，
 *    將資料左移後，用遮罩取資料，最後放到Data_p裡面。
 *
 *  - mode 1 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 3 ：
 *      使用ASA_SPIM_rec mode 3 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 4 ：回傳5，表示模式選擇錯誤。
 *  - mode 5 ：
 *      使用ASA_SPIM_rec mode 5 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 6 ：
 *      使用ASA_SPIM_rec mode 6 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 7 ：
 *      使用ASA_SPIM_rec mode 7 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 8 ：回傳5，表示模式選擇錯誤。
 *  - mode 9 ：
 *      使用ASA_SPIM_rec mode 9 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 10：回傳5，表示模式選擇錯誤。
 *  - mode 100以上，為外掛式SPI Master(主)旗標式資料接收。
 */
char ASA_SPIM_frc(char mode, char ASAID, char RegAdd, char Mask, char Shift,
                  char *Data_p, uint16_t WaitTick);

/**
 * @brief SPI Master(主)旗標式資料傳輸。
 *
 * @ingroup asaspi_func
 * @param mode     SPI通訊模式，目前支援：0、3~10。
 * @param ASAID    ASA介面卡的ID編號。
 * @param RegAdd   控制旗標(control flag)或遠端讀寫暫存器的ID.
 * @param Mask     位元組遮罩。
 * @param Shift    發送資料向左位移。
 * @param WaitTick 位元組間延遲時間，單位為 1us。
 * @param *Data_p  待送資料指標。
 * @return char    錯誤代碼。
 *                  - 0：成功無誤。
 *                  - 5：模式選擇錯誤。
 *                  - 其他：錯誤。
 *
 * SPI Master從Slave裝置接收資料，依照功用分為10種mode，接收方式如下：
 *  - mode 0 ：
 *      具check的SPI通訊方式，先使用ASA_SPIM_rec mode 0讀取資料，
 *      將資料左移後，用遮罩取資料，最後使用ASA_SPIM_trm mode 0傳輸資料。
 *  - mode 1 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 3 ：
 *      使用ASA_SPIM_rec mode 3讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 3傳輸資料。
 *  - mode 4 ：回傳5，表示模式選擇錯誤。
 *  - mode 5 ：
 *      使用ASA_SPIM_rec mode 5讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 5傳輸資料。
 *  - mode 6 ：
 *      使用ASA_SPIM_rec mode 6讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 6傳輸資料。
 *  - mode 7 ：
 *      使用ASA_SPIM_rec mode 7讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 7傳輸資料。
 *  - mode 8 ：回傳5，表示模式選擇錯誤。
 *  - mode 9 ：
 *      使用ASA_SPIM_rec mode 9讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 9傳輸資料。
 *  - mode 10：回傳5，表示模式選擇錯誤。
 *  - mode 100以上，為外掛式SPI Master(主)旗標式資料傳輸。
 */
char ASA_SPIM_ftm(char mode, char ASAID, char RRegAdd, char WRegAdd, char Mask, char Shift,
                  char *Data_p, uint16_t WaitTick);

/**
 * @brief SPI Master(主)向記憶體(flash)傳輸資料。
 *
 * @ingroup asaspi_func
 * @param mode      SPI通訊模式。
 * @param ASAID     ASA介面卡的ID編號。
 * @param RegAdd    命令或指令.
 * @param AddBytes  記憶體位置的資料筆數。
 * @param MemAdd_p  指標指向記憶體位置。
 * @param DataBytes 資料筆數。
 * @param *Data_p   指標指向資料。
 * @return char     錯誤代碼：
 *                   - 0：成功無誤。
 *                   - 5：模式選擇錯誤。
 *
 * SPI Master(主)向記憶體(flash)傳輸資料，傳輸模式如下：
 *  - mode 5 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由低到高傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由低到高傳送DataBytes筆Data_p的資料。
 * 
 *  - mode 6 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由高到低傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由高到低傳送DataBytes筆Data_p的資料。
 * 
 *  - mode x ：回傳5，表示模式選擇錯誤。
 */
char SPIM_Mem_trm(char mode, char ASAID, char RegAdd, char AddBytes,
                  void *MemAdd_p, char DataBytes, void *Data_p);

/**
 * @brief SPI Master(主)向記憶體(flash)接收資料。
 *
 * @ingroup asaspi_func
 * @param mode      SPI通訊模式。
 * @param ASAID     ASA介面卡的ID編號。
 * @param RegAdd    命令或指令.
 * @param AddBytes  記憶體位置的資料筆數。
 * @param MemAdd_p  指標指向記憶體位置。
 * @param DataBytes 資料筆數。
 * @param *Data_p   指標指向資料。
 * @return char     錯誤代碼：
 *                   - 0：成功無誤。
 *                   - 5：模式選擇錯誤。
 *
 * SPI Master(主)向記憶體(flash)傳輸資料，傳輸模式如下：
 *  - mode 5 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由低到高傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由低到高接收DataBytes筆資料至Data_p。
 * 
 *  - mode 6 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由高到低傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由高到低接收DataBytes筆資料至Data_p。
 * 
 *  - mode x ：回傳5，表示模式選擇錯誤。
 */
char SPIM_Mem_rec(char mode, char ASAID, char RegAdd, char AddBytes,
                  void *MemAdd_p, char DataBytes, void *Data_p);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 0。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 0傳輸給Slave裝置，具檢
 * 查機制，當Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會
 * 由Master發送命令[R/W + RegAdd]，決定是要Read或Write Slave端的哪個暫
 * 存器，之後根據Master端傳送的資料，接收或傳送給Master。
 */
void ASA_SPIS0_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 1。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 1傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[3bits 控制旗標 + RegAdd]，之後對Slave端註冊的RegAdd寫入資
 * 料。
 */
void ASA_SPIS1_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 2。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 2傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[RegAdd + 3bits 控制旗標]，之後對Slave端註冊的RegAdd寫入資
 * 料。
 */
void ASA_SPIS2_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 3。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，使用者須先設定是要Read還是Write，之後
 * 使用Mode 3傳輸或接收Slave裝置，當Master cs(chip select)拉低觸發中斷
 * ，會執行此函式，之後由低到高傳輸或接收Slave資料。
 */
void ASA_SPIS3_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 4。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，使用者須先設定是要Read還是Write，之後
 * 使用Mode 4傳輸或接收Slave裝置，當Master cs(chip select)拉低觸發中斷
 * ，會執行此函式，之後由高到低傳輸或接收Slave資料。
 */
void ASA_SPIS4_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 7。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[R/W + RegAdd]，之後對Slave端註冊的RegAdd由低到高寫入或讀取
 * 資料。
 */
void ASA_SPIS7_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 8。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[R/W + RegAdd]，之後對Slave端註冊的RegAdd由高到低寫入或讀取
 * 資料。
 */
void ASA_SPIS8_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 9。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[RegAdd + R/W]，之後對Slave端註冊的RegAdd由低到高寫入或讀取
 * 資料。
 */
void ASA_SPIS9_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 10。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[RegAdd + R/W]，之後對Slave端註冊的RegAdd由高到低寫入或讀取
 * 資料。
 */
void ASA_SPIS10_step(void);
/*-- asaspi section end ------------------------------------------------------*/

/*-- time section start ------------------------------------------------------*/
#define MAX_TIMEOUT_ISR 20

typedef void (*ISRFunc)(void);
typedef struct {
    volatile uint16_t
        time_limit;  // 時間限制        當計數器上數到該數值時觸發逾時中斷
    volatile uint16_t counter;  // 計數器          計數器固定頻率上數
    volatile uint8_t* p_postSlot;  // POST欄住址 一指標指向POST欄住址，為POST &
                                   // SLOT機制的子元件
    volatile uint8_t enable;  // 禁致能控制      決定本逾時ISR是否致能
    ISRFunc p_ISRFunc;        // 逾時中斷函式    逾時中斷函式指標
} TimeoutISR_t;

typedef struct {
    uint8_t total;  // 紀錄已有多少逾時中斷已註冊
    volatile TimeoutISR_t
        timeoutISR_inst[MAX_TIMEOUT_ISR];  // 紀錄所有已註冊的逾時中斷資料結構
} TimerCntStrType;
volatile TimerCntStrType TimerCntStr_inst;
/*-- time section end --------------------------------------------------------*/

/*-- rtpio section start -----------------------------------------------------*/
/**
 * @brief RealTimeUpCount結構原型
 *
 * @ingroup rtupcount_struct
 * @param Task_Id 中斷中功能方塊名單編號。
 * @param TrigCount 觸發次數計數值。
 *
 * 計數值紀錄執行step函式次數，用以追蹤次數。
 */
typedef struct {
    uint8_t Task_Id;             ///<中斷中功能方塊名單編號。
    volatile uint8_t TrigCount;  ///<觸發次數計數值。
    uint8_t NextTaskNum;         // Capacity of the next task list
    uint8_t* NextTask_esp;       // points to next task list
} RealTimeISRCountStr_t;

/**
 * @brief 將觸發計數+1。
 *
 * @ingroup rtupcount_func
 * @param VoidStr_p 要執行的結構指標。
 *
 * 執行觸發次數值+1，可登錄在中斷服務常式中執行。
 */
void RealTimeISRCount_step(void* VoidStr_p);

#define RT_ISRCOUNT_LAY(RTCSTR, NEXTTASKNUM)                                     \
    RealTimeISRCountStr_t RTCSTR = {                                            \
        .Task_Id     = 0, /*Initial TaskId to be zero */                        \
        .TrigCount   = 0, /*Initial Triger Counter */                           \
        .NextTaskNum = NEXTTASKNUM, /*Initial ListNum */                        \
    }



/**
 * @defgroup rtpio_macro
 * @defgroup rtpio_func
 * @defgroup rtpio_struct
 */

/* Public Section Start */
#define RTPIO_MAX_DATA_LENGTH                                                  \
    2  ///<硬體暫存器位元最大大小  @ingroup rtpio_macro

/**
 * @brief RealTimeRegIO結構原型
 *
 * @ingroup rtpio_struct
 *
 * 存取硬體暫存器位址和其大小後續讀寫用。
 * 資料緩衝暫存區則供資料存放，當執行step函式時將資料讀寫暫存區。
 * 計數值則紀錄執行step函式次數，用以追蹤次數。
 */
typedef struct {
    volatile uint8_t* Reg_p;     ///< 存放硬體暫存器指標。
    uint8_t Bytes;               ///< 硬體暫存器大小 (最多2 Bytes) 。
    uint8_t Task_Id;               ///< 中斷中功能方塊名單編號。
    volatile uint8_t* Data_p;    ///<儲存變數位址
    volatile uint8_t TrigCount;  ///< 觸發次數計數值
    uint8_t NextTaskNum;
    uint8_t** NextTask_esp;
} RealTimeRegIOStr_t;

/**
 * @brief RealTimeFlag結構原型
 *
 * @ingroup rtpio_struct
 *
 * 存取硬體暫存器位址和其大小後續讀寫用。
 * 資料緩衝暫存區則供資料存放，當執行step函式時將旗標資料讀寫暫存區。
 * 計數值則紀錄執行step函式次數，用以追蹤次數。
 */
typedef struct {
    volatile uint8_t* Reg_p;      ///< 存放硬體暫存器指標。
    uint8_t Task_Id;                ///< 中斷中功能方塊名單編號。
    uint8_t Mask;                 ///< 旗標讀寫遮罩。
    uint8_t Shift;                ///< 旗標讀寫平移位元。
    volatile uint8_t* Data_p;  ///< 資料暫存緩衝區。
    volatile uint8_t TrigCount;   ///< 觸發次數計數值。
    uint8_t NextTaskNum;
    uint8_t** NextTask_esp;
} RealTimeFlagIOStr_t;

/**
 * @brief 執行一次暫存器讀取，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param VoidStr_p 要執行的結構指標。
 *
 * 執行硬體暫存器輸入埠之讀取並轉存至資料結構內的資料暫存區，並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimeRegGet_step(void* VoidStr_p);

/**
 * @brief 執行一次暫存器寫入，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param VoidStr_p 要執行的結構指標。
 *
 * 執行資料結構內的資料暫存區之讀取並轉存至硬體暫存器輸出埠，並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimeRegPut_step(void* VoidStr_p);

/**
 * @brief 執行一次旗標讀取，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param VoidStr_p 要執行的結構指標。
 *
 * 執行硬體暫存器輸入埠之讀取並轉存至資料結構內的資料暫存區，並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimeFlagGet_step(void* VoidStr_p);

/**
 * @brief 執行一次旗標寫入，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param VoidStr_p 要執行的結構指標。
 *
 * 執行資料結構內的資料暫存區之讀取並轉存至硬體暫存器輸出埠，
 * 並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimeFlagPut_step(void* VoidStr_p);

#define RT_REG_IO_LAY(RTRIOSTR,NEXTTASKNUM,HWREGADD,BYTES,DATA_P)             \
    RealTimeRegIOStr_t RTRIOSTR={                                           \
        .Task_Id     = 0, /*Initial TaskId to be zero */                    \
        .TrigCount   = 0,  /*Initial Triger Counter */                      \
        .NextTaskNum = NEXTTASKNUM, /*Initial ListNum */                    \
        .Reg_p       = HWREGADD,  /*Initial pointer be zero */              \
        .Bytes       = BYTES,  /* load Bytes*/                              \
        .Data_p      = DATA_P                                               \
    }
    
#define RT_FLAG_IO_LAY(RTFIOSTR,NEXTTASKNUM,HWREGADD,MASK,SHIFT,DATA_P)       \
    RealTimeFlagIOStr_t RTFIOSTR={  /*define structure */                   \
        .Task_Id     = 0, /*Initial TaskId to be zero */                    \
        .TrigCount   = 0,  /*Initial Triger Counter */                      \
        .NextTaskNum = NEXTTASKNUM, /*Initial ListNum */                    \
        .Reg_p       = HWREGADD, /*Initial pointer be zero */               \
        .Mask        = MASK,  /* load mask*/                                \
        .Shift       = SHIFT,  /*load shift */                              \
        .Data_p      = DATA_P  /*FlagsValue Variable pointer */             \
    } 
/*-- rtpio section end -------------------------------------------------------*/

/*-- databuffer section start ------------------------------------------------*/
/**
 * @brief 供管理結構體鏈結到前後級工作之緩衝區欄。
 * @ingroup databuffer_macro
 */
#define NETBBF(task_p, num, buffer_p) task_p→BBF[num] = buffer_p

/**
 * @brief BATCHBUFF_LAY 資料連結初始化
 *
 * @param BBSTR 定義批次緩衝暫存器建構
 * @param List[MAXNUM] 定義資料陣列用矩陣
 * @ingroup databuffer_macro
 */
#define BATCHBUFF_LAY(BBSTR, MAXNUM)                                           \
    static BatchBFStr_t BBSTR = {0};                                                  \
    BBSTR.Depth        = MAXNUM;                                               \
    {                                                                          \
        static uint8_t List[MAXNUM] = {0};                                     \
        BBSTR.List_p                = List;                                    \
    }

/* Public Section Start */
/**
 * @brief 定義BBF工作結構體以管理暫存器狀態之結構
 * @ingroup databuffer_struct
 */
typedef struct {
    uint8_t State;  //<批次緩衝暫存器狀態: 0 = 可寫入 ; 1 = 可讀取.
    uint8_t Index;  //<讀寫索引.
    uint8_t Total;  //<總資料位元組數.
    uint8_t Depth;  //<暫存器總長度.
    void* List_p;   //<暫存器矩陣指標.
} BatchBFStr_t;

/**
 * @ingroup databuffer_func
 *
 * @brief BatchBF_put 資料寫入函式，供前級工作呼叫以存入新資料。
 *
 * @param *BBF_p 欲存入資料的批次緩衝區指標。
 * @param *Data_p 提供存入資料的變數或矩陣指標。
 * @param Bytes 資料位元組數。
 * @return 批之緩衝區執行完畢後剩餘空間/執行失敗時不足的空間。
 */
int8_t BatchBF_put(BatchBFStr_t* BBF_p, void* Data_p, uint8_t Bytes);

/**
 * @ingroup databuffer_func
 *
 * @brief BatchBF_get 資料讀出函式:供後級工作呼叫以讀取緩衝區內資料。
 *
 * @param *BBF_p 提供存入資料的批次緩衝區指標。
 * @param *Data_p 欲存入資料的變數或矩陣指標。
 * @param uint8_t Bytes 資料位元組數。
 * @return 批之緩衝區執行完畢後剩餘資料/執行失敗時不足的資料量。
 */
int8_t BatchBF_get(BatchBFStr_t* BBF_p, void* Data_p, uint8_t Bytes);

/**
 * @ingroup databuffer_func
 *
 * @brief BatchBF_full 設定批次緩衝區為滿。
 *
 * @param *BBF_p 欲清空批次緩衝區指標。
 * @return 批之緩衝區執行完畢後狀態。
 */
uint8_t BatchBF_full(BatchBFStr_t* BBF_p);

/**
 * @ingroup databuffer_func
 *
 * @brief BatchBF_clr 供前級工作呼叫以清空緩衝區。
 *
 * @param *BBF_p 欲清空批次緩衝區指標。
 * @return 批之緩衝區執行完畢後狀態。
 */
uint8_t BatchBF_clr(BatchBFStr_t* BBF_p);

#define NETRBF(task, num, buffer_p)                                            \
    task.RBF[num] = buffer_p  ///< 供管理結構體鏈結到串列通訊工作的結構體

/**
 * @brief 供使用者創建並連結實體空間至緩衝器結構體中
 * @ingroup rtpio_macro
 */
#define REMOBUFF_LAY(RBSTR, DDEPTH, VDEPTH)                                    \
    static RemoBFStr_t RBSTR = {0};                                                   \
    {                                                                          \
        static uint8_t RBSTR##_DLIST[DDEPTH]    = {0};                           \
        static RemoVari_t RBSTR##_VLIST[VDEPTH] = {0};                           \
        RBSTR.DList_p                         = RBSTR##_DLIST;                   \
        RBSTR.VList_p                         = RBSTR##_VLIST;                   \
    }                                                                          \
    RBSTR.DataDepth = DDEPTH;                                                  \
    RBSTR.VariDepth = VDEPTH;

/**
 * @brief 提供remo變數位址連結及資訊儲存
 * @ingroup remobf_struct
 */
typedef struct {
    uint8_t Bytes;  ///< 變數位元數
    void* Vari_p;   ///< 變數位址
} RemoVari_t;

/**
 * @brief 提供remobf結構原形
 * @ingroup remobf_struct
 */
typedef struct {
    uint8_t State;        ///< Buffer狀態
    uint8_t DataIndex;    ///< 資料讀取索引
    uint8_t VariIndex;    ///< 變數讀取索引
    uint8_t ByteIndex;    ///< 位元讀取索引
    uint8_t DataTotal;    ///< 資料總位元數
    uint8_t VariTotal;    ///< 變數總數
    uint8_t DataDepth;    ///< Buffer最大總位元數,
    uint8_t VariDepth;    ///< Remo變數最大總數量,
    RemoVari_t* VList_p;  ///< 變數儲存空間指標
    uint8_t* DList_p;     ///< Buffer空間指標
} RemoBFStr_t;

/**
 * @brief 將要提供遠端讀寫的變數大小及位址寫入結構體中並回傳遠端讀寫編號
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param Data_p 變數位址
 * @param Bytes 變數位元數
 * @return uint8_t 遠端讀寫編號(0~254)
 */
uint8_t RemoBF_reg(RemoBFStr_t* Str_p, void* Data_p, uint8_t Bytes);

/**
 * @brief 將buffer內的資料寫入至對應的讀寫變數位址中
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param RegId 對應變數的編號 0~254:對應編號 255:所有已註冊變數
 * @return uint8_t 錯誤代碼
 * - 0: 正常
 * - 1: State錯誤Buffer尚未填滿
 */
uint8_t RemoBF_put(RemoBFStr_t* Str_p, uint8_t RegId);

/**
 * @brief 將1byte的資料填入至buffer中
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param RegId 對應變數的編號 0~254:對應編號 255:所有已註冊變數
 * @param Data 欲寫入之資料值
 * @return int8_t 回傳剩餘Bytes數
 */
int8_t RemoBF_temp(RemoBFStr_t* Str_p, uint8_t RegId, uint8_t Data);

/**
 * @brief 取得1byte對應編碼變數中的值
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param RegId 對應變數的編號 0~254:對應編號 255:所有已註冊變數
 * @param Data_p 放入取得值的位址
 * @return int8_t 回傳剩餘Bytes數
 */
int8_t RemoBF_get(RemoBFStr_t* Str_p, uint8_t RegId, void* Data_p);

/**
 * @brief 歸零remobf結構體中的資料索引
 *
 * @ingroup remobf_func
 * @param Str_p Str_p remobf結構指標
 * @return uint8_t 錯誤代碼 0: 正常
 */
uint8_t RemoBF_clr(RemoBFStr_t* Str_p);

// FIXME: 註解依照coding sytle更正
#define BefTrig        0  //觸發前
#define AftTrig        1  //己觸發
#define Wait4Read      2  //緩衝區己滿可讀
#define StateErrorCode -127

#define NETCBF(task_p, num, buffer_p) task_p→CBF[num] = buffer_p
#define SCOPEBUFF_LAY(SBSTR, MAXNUM, PreC)                                     \
    static ScopeBFStr_t SBSTR = {                                                     \
        .State    = BefTrig,                                                   \
        .Pindex   = 0,                                                         \
        .Gindex   = 0,                                                         \
        .Total    = 0,                                                         \
        .Depth    = MAXNUM,                                                    \
        .PreCount = PreC,                                                      \
    };                                                                         \
    {                                                                          \
        static uint8_t List[MAXNUM] = {0};                                     \
        SBSTR.List_p                = List;                                    \
    }

/**
 * @brief 定義buffer結構成員
 *
 */
typedef struct {
    uint8_t State;     // state of Buffer
    int16_t Pindex;    // Matrix Put Index of Buffer
    int16_t Gindex;    // Matrix Get Index of Buffer
    uint8_t Total;     // Total Data Bytes
    uint8_t Depth;     // length of the buffer,
    uint8_t PreCount;  // Preset value when counter is triggered
    void* List_p;      // Pointer for Data Matrix
} ScopeBFStr_t;

/**
 * @brief 資料寫入函式
 *
 * @param ScopeBF_p 傳入結構指標
 * @param Data_p 資料空間指標
 * @param Bytes 欲寫入資料 bytes 數
 * @return uint8_t 總資料筆數
 */
int8_t ScopeBF_put(ScopeBFStr_t* ScopeBF_p, void* Data_p, uint8_t Bytes);

/**
 * @brief 觸發計數函式
 *
 * @param ScopeBF_p 傳入結構指標
 * @return uint8_t 總資料筆數
 */
int8_t ScopeBF_tri(ScopeBFStr_t* ScopeBF_p);

/**
 * @brief 資料讀出函式
 *
 * @param ScopeBF_p 傳入結構指標
 * @param Data_p 資料空間指標
 * @param Bytes 欲讀出資料 bytes 數
 * @return uint8_t 總資料筆數
 */
int8_t ScopeBF_get(ScopeBFStr_t* ScopeBF_p, void* Data_p, uint8_t Bytes);

/**
 * @brief 資料設滿函式
 *
 * @param ScopeBF_p 傳入結構指標
 * @return uint8_t 總資料筆數
 */
uint8_t ScopeBF_full(ScopeBFStr_t* ScopeBF_p);

/**
 * @brief 資料清空函式，供前級工作呼叫以清空緩衝區。
 *
 * @param ScopeBF_p 傳入結構指標
 * @return uint8_t 總資料筆數
 */
uint8_t ScopeBF_clr(ScopeBFStr_t* ScopeBF_p);

/**
 * @brief   供管理結構體鏈結到前後級工作之緩衝區欄
 * @ingroup databuffer_macro
 */
#define NETFBF(task_p, num, buffer_p) task_p->FBF[num] = buffer_p

/**
 * @brief 供暫存資料之矩陣，鍊結至管理先進先出之結構體，以及初始化先進先出結構體
 * @param Str 定義批次緩衝暫存器建構
 * @param List[MAXNUM] 定義資料陣列用矩陣
 * @ingroup databuffer_macro
 */
#define FIFOBUFF_LAY(FBSTR, MAXNUM)                                            \
    static FifoBFStr_t FBSTR = {0};                                                   \
    FBSTR.Depth       = MAXNUM;                                                \
    {                                                                          \
        static uint8_t List[MAXNUM] = {0};                                     \
        FBSTR.List_p                = List;                                    \
    }

/**
 * @brief   定義先進先出結構體以管理先進先出緩衝區狀態之結構
 * @ingroup databuffer_struct
 */
typedef struct {
    uint8_t PIndex;  ///< FIFO Buffer 的寫入索引
    uint8_t GIndex;  ///< FIFO Buffer 的讀取索引
    uint8_t Total;   ///< FIFO Buffer 中目前已存入的資料數
    uint8_t Depth;   ///< FIFO Buffer 中可存入的資料入，即為大小
    void* List_p;    ///< FIFO Buffer 中的資料起始記憶體位置
} FifoBFStr_t;

/**
 * @ingroup databuffer_func
 *
 * @brief 資料寫入函式，供前級工作呼叫以存入新資料。
 *
 * @param *FBF_p     欲存入資料的批次緩衝區指標。
 * @param *Data_p    提供存入資料的變數或矩陣指標。
 * @param Bytes     資料位元組數。
 * @return
 * 尚餘緩衝空間容量位元組數n，若n大於等於0代表剩餘n位元組可存入，否則為不足n位元組。
 */
int8_t FifoBF_put(FifoBFStr_t* FBF_p, void* Data_p, uint8_t Bytes);

/**
 * @ingroup databuffer_func
 *
 * @brief 資料讀出函式，供前級工作呼叫以讀出新資料。
 *
 * @param *FBF_p     欲讀出資料的批次緩衝區指標。
 * @param *Data_p    提供讀出資料的變數或矩陣指標。
 * @param Bytes     資料位元組數
 * @return
 * 尚餘緩衝空間容量位元組數n，若n大於等於0代表剩餘n位元組可讀出，否則為不足n位元組。
 */
int8_t FifoBF_get(FifoBFStr_t* FBF_p, void* Data_p, uint8_t Bytes);

/**
 * @ingroup databuffer_func
 *
 * @brief 供前級工作呼叫以清空緩衝區。
 *
 * @param *FBF_p     欲清空批次緩衝區指標。
 * @return 緩衝區可存入之資料位元數
 */
uint8_t FifoBF_clr(FifoBFStr_t* FBF_p);
/*-- databuffer section end --------------------------------------------------*/

/*-- hardwareinit section start ----------------------------------------------*/
/**
 * @brief flag put 初始化集中管理結構
 *
 * @param Reg_p 愈初始化之目標暫存器之記憶體位址 Register Pointer
 * @param Mask   遮罩。依照愈初始化暫存器中目標旗標填寫
 * @param Shift  向左位移。依照愈初始化暫存器中目標旗標填寫
 */
typedef struct {
    volatile unsigned char *Reg_p;
    unsigned char Mask;
    unsigned char Shift;
} HWFgGpPara_t;

/**
 * @brief put 初始化集中管理結構
 *
 * @param Reg_p 愈初始化之目標暫存器之記憶體位址 Register Pointer
 * @param Bytes 愈傳輸資料長度。依照目標暫存器長度填寫
 */
typedef struct {
    volatile unsigned char *Reg_p;
    unsigned char Bytes;
} HWRegPara_t;

/**
 * @brief HardWare_init函式專用結構
 *
 * @param FgGpNum
 * @param RegNum
 * @param FgGpPara_p 愈初始化之目標暫存器之記憶體位址 Register Pointer
 * @param RegPara_p
 * @param List_p
 */
typedef struct {
    unsigned char FgGpNum;
    unsigned char RegNum;
    HWFgGpPara_t *FgGpPara_p;
    HWRegPara_t *RegPara_p;
    void *List_p[2];
} HardWareSet_t;

/**
 * @brief 硬體初始化函式
 *
 * @param str_p HardWareSet結構之指標
 */
char hardware_set(HardWareSet_t *str_p);

typedef struct {
    uint8_t DDXn;   // set ext port in pin DDD0~DDD3, DDE4:DDE7
    uint8_t ISCn0;  // EXT Sense Control  ISC00~ ISC70
    uint8_t INTn;   // External Interrupt Request  INT0~ INT7
    uint8_t Total;  // Total Bytes of Flag Group Datum
} EXTFgGpData_t;

typedef struct {
    uint8_t Total;  // Total Bytes of Registers
} EXTRegData_t;

#define EXT4_HW_LAY()                                                          \
    static HardWareSet_t EXT4HWSet_str = {0};                                  \
    {                                                                          \
        static HWFgGpPara_t EXT4FGParaList[3] = {                              \
            {&DDRE, 0x10, 4}, {&EICRB, 0x03, 0}, {&EIMSK, 0x10, 4}};           \
        static EXTFgGpData_t EXT4FgGpData_str = EXT4_FG_DATA_INT;              \
        static HWRegPara_t EXT4RegParaList[1] = {0};                           \
        static EXTRegData_t EXT4RegData_str   = EXT4_REG_DATA_INT;             \
        EXT4HWSet_str.FgGpPara_p              = EXT4FGParaList;                \
        EXT4HWSet_str.RegPara_p               = EXT4RegParaList;               \
        EXT4HWSet_str.List_p[0]               = &EXT4FgGpData_str;             \
        EXT4HWSet_str.List_p[1]               = &EXT4RegData_str;              \
    }                                                                          \
    EXT4HWSet_str.FgGpNum = 3; /*HW Setting Flag-group number*/                \
    EXT4HWSet_str.RegNum  = 0; /*HW Setting Register Number*/

#define EXT5_HW_LAY()                                                          \
    static HardWareSet_t EXT5HWSet_str = {0};                                  \
    {                                                                          \
        static HWFgGpPara_t EXT5FGParaList[3] = {                              \
            {&DDRE, 0x20, 5},                                                  \
            {&EICRB, 0x0c, 2},                                                 \
            {&EIMSK, 0x20,                                                     \
             5}}; /*define HWFlagGroup Parameters and initial it */            \
        static EXTFgGpData_t EXT5FgGpData_str = EXT5_FG_DATA_INT;              \
        static HWRegPara_t EXT5RegParaList[1] = {                              \
            0}; /*define HWREG Parameters and initial it */                    \
        static EXTRegData_t EXT5RegData_str =                                  \
            EXT5_REG_DATA_INT; /*define HWREG Data struct and initial it */    \
        EXT5HWSet_str.FgGpPara_p =                                             \
            EXT5FGParaList; /*connect Flag Group Para */                       \
        EXT5HWSet_str.RegPara_p = EXT5RegParaList; /*connect register Para */  \
        EXT5HWSet_str.List_p[0] =                                              \
            &EXT5FgGpData_str; /*connect Flag Group Data */                    \
        EXT5HWSet_str.List_p[1] = &EXT5RegData_str; /*connect register Data */ \
    }                                                                          \
    EXT5HWSet_str.FgGpNum = 3; /*HW Setting Flag-group number*/                \
    EXT5HWSet_str.RegNum  = 0; /*HW Setting Register Number*/

#define EXT6_HW_LAY()                                                          \
    static HardWareSet_t EXT6HWSet_str = {0};                                  \
    {                                                                          \
        static HWFgGpPara_t EXT6FGParaList[3] = {                              \
            {&DDRE, 0x40, 6},                                                  \
            {&EICRB, 0x30, 4},                                                 \
            {&EIMSK, 0x40,                                                     \
             6}}; /*define HWFlagGroup Parameters and initial it */            \
        static EXTFgGpData_t EXT6FgGpData_str =                                \
            EXT6_FG_DATA_INT; /*define HWFlagGroup Data struct and initial it  \
                               */                                              \
        static HWRegPara_t EXT6RegParaList[1] = {                              \
            0}; /*define HWREG Parameters and initial it */                    \
        static EXTRegData_t EXT6RegData_str =                                  \
            EXT6_REG_DATA_INT; /*define HWREG Data struct and initial it */    \
        EXT6HWSet_str.FgGpPara_p =                                             \
            EXT6FGParaList; /*connect Flag Group Para */                       \
        EXT6HWSet_str.RegPara_p = EXT6RegParaList; /*connect register Para */  \
        EXT6HWSet_str.List_p[0] =                                              \
            &EXT6FgGpData_str; /*connect Flag Group Data */                    \
        EXT6HWSet_str.List_p[1] = &EXT6RegData_str; /*connect register Data */ \
    }                                                                          \
    EXT6HWSet_str.FgGpNum = 3; /*HW Setting Flag-group number*/                \
    EXT6HWSet_str.RegNum  = 0; /*HW Setting Register Number*/

#define EXT7_HW_LAY()                                                          \
    static HardWareSet_t EXT7HWSet_str = {0};                                  \
    {                                                                          \
        static HWFgGpPara_t EXT7FGParaList[3] = {                              \
            {&DDRE, 0x80, 7},                                                  \
            {&EICRB, 0xc0, 6},                                                 \
            {&EIMSK, 0x80,                                                     \
             7}}; /*define HWFlagGroup Parameters and initial it */            \
        static EXTFgGpData_t EXT7FgGpData_str = EXT7_FG_DATA_INT;              \
        static HWRegPara_t EXT7RegParaList[1] = {                              \
            0}; /*define HWREG Parameters and initial it */                    \
        static EXTRegData_t EXT7RegData_str =                                  \
            EXT7_REG_DATA_INT; /*define HWREG Data struct and initial it */    \
        EXT7HWSet_str.FgGpPara_p =                                             \
            EXT7FGParaList; /*connect Flag Group Para */                       \
        EXT7HWSet_str.RegPara_p = EXT7RegParaList; /*connect register Para */  \
        EXT7HWSet_str.List_p[0] = &EXT7FgGpData_str;                           \
        EXT7HWSet_str.List_p[1] = &EXT7RegData_str; /*connect register Data */ \
    }                                                                          \
    EXT7HWSet_str.FgGpNum = 3;                                                 \
    EXT7HWSet_str.RegNum  = 0;


typedef struct {
    uint8_t SPI2SPEED;  // Double SPI Speed
    uint8_t SPR0_1;     // Clock Prescale
    uint8_t CPHASE;     // SynchRdWt
    uint8_t CPOLAR;     // SynchRiseDown
    uint8_t MSSELECT;   // MSSelect
    uint8_t DORDER;     // ByteOrder
    uint8_t SPEn;
    uint8_t SPIEn;
    uint8_t DDx0_3;     // MOSI, SCK, /SS set output. MISO set input.
    uint8_t ASA_ADDR;
    uint8_t SPI_CS8_B;
    uint8_t Total;      // Total Bytes of Flag Group Datum
} SPIFgGpData_t;

typedef struct {
    uint8_t Total;    // Total Bytes of Registers
} SPIRegData_t;

#define SPI_HW_LAY()                                                          \
    static HardWareSet_t SPIHWSet_str = {0};                                  \
    {                                                                         \
        static HWFgGpPara_t SPIFGParaList[11] = {                             \
            {&SPSR, 0x01, 0},                                                 \
            {&SPCR, 0x03, 0},                                                 \
            {&SPCR, 0x04, 2},                                                 \
            {&SPCR, 0x08, 3},                                                 \
            {&SPCR, 0x10, 4},                                                 \
            {&SPCR, 0x20, 5},                                                 \
            {&SPCR, 0x40, 6},                                                 \
            {&SPCR, 0x80, 7},                                                 \
            {&DDRB, 0x0F, 0},                                                 \
            {&DDRF, 0xE0, 5},                                                 \
            {&DDRB, 0xF0, 4}};                                                \
        static HWRegPara_t SPIRegParaList[0];                                 \
        static SPIFgGpData_t SPIFgGpData_str = SPI_FG_DATA_INT;               \
        static SPIRegData_t SPIRegData_str = SPI_REG_DATA_INT;                \
        SPIHWSet_str.FgGpPara_p = SPIFGParaList; /*connect Flag Group Para */ \
        SPIHWSet_str.RegPara_p = SPIRegParaList; /*connect register Para */   \
        SPIHWSet_str.List_p[0] = &SPIFgGpData_str;                            \
        SPIHWSet_str.List_p[1] = &SPIRegData_str; /*connect register Data */  \
    }                                                                         \
    SPIHWSet_str.FgGpNum = 11; /*HW Setting Flag-group number*/               \
    SPIHWSet_str.RegNum = 0;  /*HW Setting Register Number*/



/**
 * @defgroup twi_set_macro
 * @defgroup twi_set_struct
 * @defgroup twi_set_func
 */

/**
 * @brief 提供twi初始化所需旗標設定結構
 * @ingroup twi_set_struct
 */
typedef struct {
    uint8_t TWPSn0_1;  ///< prescaler
    uint8_t TWIEn;     ///< TWI中斷致能
    uint8_t TWENn;     ///< TWI致能
    uint8_t TWEAn;     ///< ACK致能
    uint8_t TWAn1_7;   ///< TWI ADDR
    uint8_t BCEn;      ///< Broadcast
    uint8_t DDx0_1;    ///< TWI腳位輸出入方向
    uint8_t Total;     ///< TWI設定旗標結構總bytes數
} TWIFgGpData_t;

/**
 * @brief 提供twi初始化所需暫存器設定結構
 * @ingroup twi_set_struct
 */
typedef struct {
    uint8_t TWBRn;  ///< TWI Bit rate
    uint8_t Total;  ///< Total Bytes of Registers
} TWIRegData_t;

/**
 * @brief   提供TWI硬體初始化所需之設定佈局巨集
 * @ingroup twi_set_macro
 * 提供TWIHWSet_str結構給hardware_set函式使用
 */
#define TWI_HW_LAY()                                                           \
    static HardWareSet_t TWIHWSet_str = {0};                                   \
    {                                                                          \
        static HWFgGpPara_t TWIFGParaList[7] = {                               \
            {&TWSR, 0x03, 0}, {&TWCR, 0x01, 0}, {&TWCR, 0x04, 2},              \
            {&TWCR, 0x40, 6}, {&TWAR, 0xFE, 1}, {&TWAR, 0x01, 0},              \
            {&DDRD, 0x03, 0},                                                  \
        };                                                                     \
        static HWRegPara_t TWIRegParaList[1] = {{&TWBR, 1}};                   \
        static TWIFgGpData_t TWIFgGpData_str = TWI_FG_DATA_INT;                \
        static TWIRegData_t TWIRegData_str   = TWI_REG_DATA_INT;               \
        TWIHWSet_str.FgGpPara_p              = TWIFGParaList;                  \
        TWIHWSet_str.RegPara_p               = TWIRegParaList;                 \
        TWIHWSet_str.List_p[0]               = &TWIFgGpData_str;               \
        TWIHWSet_str.List_p[1]               = &TWIRegData_str;                \
    }                                                                          \
    TWIHWSet_str.FgGpNum = 7;                                                  \
    TWIHWSet_str.RegNum  = 1;



/**
 * @defgroup adc_set_macro adc_set macros
 * @defgroup adc_set_struct adc_set structs
 * @defgroup adc_set_func adc_set functions
 */

/**
 * @brief   提供ADC硬體初始化所需之旗標設定結構
 * @ingroup adc_set_struct
 */
typedef struct {
    uint8_t MUXn;    // ADC Input source
    uint8_t REFSn;   // Reference source
    uint8_t ADCBIT;  // Use 8bit or 10bit (ADLAR)
    uint8_t ADPCn;   // ADC prescaler (ADPSn)
    uint8_t ADFRUN;  // ADC free running (ADFR)
    uint8_t ADCEN;   // ADC convert enable (ADEN)
    uint8_t ADIntE;  // ADC interrupt enable (ADIE)
    uint8_t DDFn;    // ADC input pin
    uint8_t Total;   // Total number of flag group
} ADCFgGpData_t;

/**
 * @brief   提供ADC硬體初始化所需之暫存器設定結構
 * @ingroup adc_set_struct
 */
typedef struct {
    uint8_t total;  // ADC don't have data need for reg
} ADCRegData_t;

/**
 * @brief   提供ADC硬體初始化所需之設定佈局
 * @ingroup adc_set_macro
 */
#define ADC_HW_LAY()                                                    \
    static HardWareSet_t ADCHWSet_str = {0};                                   \
    {                                                                   \
        static HWFgGpPara_t ADCFGParaList[8] = {                        \
            {&ADMUX, 0x1F, 0},  {&ADMUX, 0xC0, 6},  {&ADMUX, 0x20, 5},  \
            {&ADCSRA, 0x07, 0}, {&ADCSRA, 0x20, 5}, {&ADCSRA, 0x80, 7}, \
            {&ADCSRA, 0x08, 3}, {&DDRF, 0x0F, 0}};                      \
        static HWRegPara_t ADCRegParaList[0];                           \
        static ADCFgGpData_t ADCFgGpData_str = ADC_FG_DATA_INT;         \
        static ADCRegData_t ADCRegData_str = ADC_REG_DATA_INT;          \
        ADCHWSet_str.FgGpNum = 8;                                       \
        ADCHWSet_str.RegNum = 0;                                        \
        ADCHWSet_str.FgGpPara_p = ADCFGParaList;                        \
        ADCHWSet_str.RegPara_p = ADCRegParaList;                        \
        ADCHWSet_str.List_p[0] = &ADCFgGpData_str;                      \
        ADCHWSet_str.List_p[1] = &ADCRegData_str;                       \
    }


typedef struct {
    uint8_t ASn;      // ClockSource
    uint8_t WGMn0_1;  // WaveSelection
    uint8_t CSn0_2;   // TIME0FreqDivide
    uint8_t COMn0_1;  // TIME0WareOut
    uint8_t TIMSKn;
    uint8_t DDx;      // WaveOutPin
    uint8_t Total;    // Total Bytes of Flag Group Datum
} TIM0FgGpData_t;

typedef struct {
    uint8_t OCRn;   // Adjust TIMER0 Cycle
    uint8_t Total;  // Total Bytes of Flag Group Datum
} TIM0_2RegData_t;

typedef struct {
    uint8_t WGMn0_1;   // WaveSelection
    uint8_t WGMn2_3;   // WaveSelection
    uint8_t CSn0_2;    // TIME1FreqDivide
    uint8_t COMnA0_1;  // TIME1WareOut
    uint8_t TIMSKn;
    uint8_t DDx;       // WaveOutPin
    uint8_t Total;     // Total Bytes of Flag Group Datum
} TIM1_3FgGpData_t;

typedef struct {
    uint16_t OCRn;  // Adjust TIMER1 Cycle
    uint8_t Total;  // Total Bytes of Flag Group Datum
} TIM1_3RegData_t;

typedef struct {
    uint8_t WGMn0_1;  // WaveSelection
    uint8_t CSn0_2;   // TIME0FreqDivide
    uint8_t COMn0_1;  // TIME0WareOut
    uint8_t TIMSKn;
    uint8_t DDx;      // WaveOutPin
    uint8_t Total;    // Total Bytes of Flag Group Datum
} TIM2FgGpData_t;

#define TIM0_HW_LAY()                                                          \
    HWFgGpPara_t TIM0FGParaList[6] = {                                         \
        {&ASSR, 0x08, 3},                                                      \
        {&TCCR0, 0x48, 3},                                                     \
        {&TCCR0, 0x07, 0},                                                     \
        {&TCCR0, 0x30, 4},                                                     \
        {&TIMSK, 0x02, 1},                                                     \
        {&DDRB, 0x10, 4}}; /*define HWFlagGroup Parameters and initial it */   \
    TIM0FgGpData_t TIM0FgGpData_str =                                          \
        TIM0_FG_DATA_INT; /*define HWFlagGroup Data struct and initial it      \
                           */                                                  \
    HWRegPara_t TIM0RegParaList[1] = {                                         \
        {&OCR0, 1}}; /*define HWREG Parameters and initial it */               \
    TIM0_2RegData_t TIM0RegData_str =                                          \
        TIM0_2_REG_DATA_INT; /*define HWREG Data struct and initial it */      \
    HardWareSet_t TIM0HWSet_str = {                                            \
        .FgGpNum    = 6,                 /*HW Setting Flag-group number*/      \
        .RegNum     = 1,                 /*HW Setting Register Number*/        \
        .FgGpPara_p = TIM0FGParaList,    /*connect Flag Group Para */          \
        .RegPara_p  = TIM0RegParaList,   /*connect register Para */            \
        .List_p[0]  = &TIM0FgGpData_str, /*connect Flag Group Data */          \
        .List_p[1]  = &TIM0RegData_str,  /*connect register Data */            \
    }; /*define  HardWareSet structure and initial it */

#define TIM1_HW_LAY()                                                          \
    HWFgGpPara_t TIM1FGParaList[6] = {                                         \
        {&TCCR1A, 0x03, 0},                                                    \
        {&TCCR1B, 0x18, 3},                                                    \
        {&TCCR1B, 0x07, 0},                                                    \
        {&TCCR1A, 0xc0, 6},                                                    \
        {&TIMSK, 0x10, 4},                                                     \
        {&DDRB, 0x20, 5}}; /*define HWFlagGroup Parameters and initial it */   \
    TIM1_3FgGpData_t TIM1_3FgGpData_str =                                      \
        TIM1_3_FG_DATA_INT; /*define HWFlagGroup Data struct and initial it */ \
    HWRegPara_t TIM1RegParaList[1] = {                                         \
        {&OCR1AL, 2}}; /*define HWREG Parameters and initial it */             \
    TIM1_3RegData_t TIM1_3RegData_str =                                        \
        TIM1_3_REG_DATA_INT; /*define HWREG Data struct and initial it */      \
    HardWareSet_t TIM1HWSet_str = {                                            \
        .FgGpNum    = 6,                   /*HW Setting Flag-group number*/    \
        .RegNum     = 1,                   /*HW Setting Register Number*/      \
        .FgGpPara_p = TIM1FGParaList,      /*connect Flag Group Para */        \
        .RegPara_p  = TIM1RegParaList,     /*connect register Para */          \
        .List_p[0]  = &TIM1_3FgGpData_str, /*connect Flag Group Data */        \
        .List_p[1]  = &TIM1_3RegData_str   /*connect register Data */          \
    }; /*define  HardWareSet structure and initial it */

#define TIM2_HW_LAY()                                                          \
    HWFgGpPara_t TIM2FGParaList[5] = {                                         \
        {&TCCR2, 0x48, 3},                                                     \
        {&TCCR2, 0x07, 0},                                                     \
        {&TCCR2, 0x30, 4},                                                     \
        {&TIMSK, 0x80, 7},                                                     \
        {&DDRB, 0x80, 7}}; /*define HWFlagGroup Parameters and initial it */   \
    TIM2FgGpData_t TIM2FgGpData_str =                                          \
        TIM2_FG_DATA_INT; /*define HWFlagGroup Data struct and initial it */   \
    HWRegPara_t TIM2RegParaList[1] = {                                         \
        {&OCR2, 1}}; /*define HWREG Parameters and initial it */               \
    TIM0_2RegData_t TIM2RegData_str =                                          \
        TIM0_2_REG_DATA_INT; /*define HWREG Data struct and initial it */      \
    HardWareSet_t TIM2HWSet_str = {                                            \
        .FgGpNum    = 5,                 /*HW Setting Flag-group number*/      \
        .RegNum     = 1,                 /*HW Setting Register Number*/        \
        .FgGpPara_p = TIM2FGParaList,    /*connect Flag Group Para */          \
        .RegPara_p  = TIM2RegParaList,   /*connect register Para */            \
        .List_p[0]  = &TIM2FgGpData_str, /*connect Flag Group Data */          \
        .List_p[1]  = &TIM2RegData_str};  /*connect register Data */

#define TIM3_HW_LAY()                                                          \
    HWFgGpPara_t TIM3FGParaList[6] = {                                         \
        {&TCCR3A, 0x03, 0},                                                    \
        {&TCCR3B, 0x18, 3},                                                    \
        {&TCCR3B, 0x07, 0},                                                    \
        {&TCCR3A, 0xC0, 6},                                                    \
        {&ETIMSK, 0x10, 4},                                                    \
        {&DDRE, 0x08, 3}}; /*define HWFlagGroup Parameters and initial it */   \
    TIM1_3FgGpData_t TIM1_3FgGpData_str =                                      \
        TIM1_3_FG_DATA_INT; /*define HWFlagGroup Data struct and initial       \
                               it */                                           \
    HWRegPara_t TIM3RegParaList[1] = {                                         \
        {&OCR3AL, 2}}; /*define HWREG Parameters and initial it */             \
    TIM1_3RegData_t TIM1_3RegData_str =                                        \
        TIM1_3_REG_DATA_INT; /*define HWREG Data struct and initial it */      \
    HardWareSet_t TIM1_3HWSet_str = {                                          \
        .FgGpNum    = 6,                   /*HW Setting Flag-group number*/    \
        .RegNum     = 1,                   /*HW Setting Register Number*/      \
        .FgGpPara_p = TIM3FGParaList,      /*connect Flag Group Para */        \
        .RegPara_p  = TIM3RegParaList,     /*connect register Para */          \
        .List_p[0]  = &TIM1_3FgGpData_str, /*connect Flag Group Data */        \
        .List_p[1]  = &TIM1_3RegData_str   /*connect register Data */          \
    }; /*define  HardWareSet structure and initial it */


typedef struct {
    uint8_t UMPn0_1;   // Parity Check
    uint8_t USBSn;     // Uart Stop Bits
    uint8_t UCSZn0_1;  // UART Word bits
    uint8_t TXENn;     // TX enable
    uint8_t RXENn;     // RX enable
    uint8_t TXCIEn;    // TX Interrupt enable
    uint8_t RXCIEn;    // TX Interrupt enable
    uint8_t U2Xn;      //  DoubleBaudRate
    uint8_t DDREn;     // TX out RX in
    uint8_t Total;     // Total Number of the Flag Groups
} UARTFgGpData_t;

typedef struct {
    uint8_t UBRRnH;
    uint8_t UBRRnL;  // 2 bytes Baudrate for both Transmitter and Receiver
    uint8_t Total;   // Total cumulation Bytes of all Registers
} UARTRegData_t;

#define UART1_HW_LAY()                                                         \
    static HardWareSet_t Uart1HWSet_str = {0};                                        \
    {                                                                          \
        static HWFgGpPara_t Uart1FGParaList[9] = {                             \
            {&UCSR1C, 0x30, 4}, {&UCSR1C, 0x08, 3}, {&UCSR1C, 0x06, 1},        \
            {&UCSR1B, 0x08, 3}, {&UCSR1B, 0x10, 4}, {&UCSR1B, 0x40, 6},        \
            {&UCSR1B, 0x80, 7}, {&UCSR1A, 0x02, 1}, {&DDRD, 0x0c, 2}};         \
        static UARTFgGpData_t Uart1FgGpData_str = Uart1_FG_DATA_INT;           \
        static HWRegPara_t Uart1RegParaList[2] = {{&UBRR1H, 1}, {&UBRR1L, 1}}; \
        static UARTRegData_t Uart1RegData_str = Uart1_REG_DATA_INT;            \
        Uart1HWSet_str.FgGpPara_p = Uart1FGParaList;                           \
        Uart1HWSet_str.RegPara_p = Uart1RegParaList;                           \
        Uart1HWSet_str.List_p[0] = &Uart1FgGpData_str;                         \
        Uart1HWSet_str.List_p[1] = &Uart1RegData_str;                          \
    }                                                                          \
    Uart1HWSet_str.FgGpNum = 9;                                                \
    Uart1HWSet_str.RegNum = 2;


/**
 * @defgroup pwm_set_macro
 * @defgroup pwm_set_struct
 * @defgroup pwm_set_func
 */

/**
 * @brief   提供PWM0硬體初始化所需之旗標設定結構
 * @ingroup pwm_set_struct
 *
 * @param ASn ClockSource0
 * @param WGMn0_1 WaveMode0
 * @param CSn0_2 ClockSource1
 * @param COMn0_1 WaveOut0
 * @param DDx WaveOutPin
 * @param TotalByte 結構中參數數量
 */
typedef struct {
    uint8_t ASn;
    uint8_t WGMn0_1;
    uint8_t CSn0_2;
    uint8_t COMn0_1;
    uint8_t TIMSKn;
    uint8_t DDx;
    uint8_t TotalByte;
} PWM0FgGpData_t;

/**
 * @brief   提供PWM2硬體初始化所需之旗標設定結構
 * @ingroup pwm_set_struct
 *
 * @param WGMn0_1 WaveMode0
 * @param CSn0_2 ClockSource1
 * @param COMn0_1 WaveOut0
 * @param DDx WaveOutPin
 * @param TotalByte 結構中參數數量
 */
typedef struct {
    uint8_t WGMn0_1;
    uint8_t CSn0_2;
    uint8_t COMn0_1;
    uint8_t TIMSKn;
    uint8_t DDx;
    uint8_t TotalByte;
} PWM2FgGpData_t;

/**
 * @brief   提供PWM0與PWM2硬體初始化所需之暫存器設定結構
 * @ingroup pwm_set_struct
 */
typedef struct {
    uint8_t OCRn;
    uint8_t TotalByte;
} PWM0_2RegData_t;

/**
 * @brief   提供PWM0硬體初始化所需之設定佈局
 * @ingroup pwm_set_macro
 */
#define PWM0_HW_LAY()                                                          \
    HWFgGpPara_t PWM0FGParaList[6] = {{&ASSR, 0x08, 3},  {&TCCR0, 0x48, 3},    \
                                      {&TCCR0, 0x07, 0}, {&TCCR0, 0x30, 4},    \
                                      {&TIMSK, 0x01, 0}, {&DDRB, 0x10, 4}};    \
                                                                               \
    PWM0FgGpData_t PWM0FgGpData_str = PWM0_FG_DATA_INT;                        \
    HWRegPara_t PWM0RegParaList[1]  = {{&OCR0, 1}};                            \
    PWM0_2RegData_t PWM0RegData_str = PWM0_2_REG_DATA_INT;                     \
                                                                               \
    HardWareSet_t PWM0HWSet_str = {.FgGpNum    = 6,                            \
                                   .RegNum     = 1,                            \
                                   .FgGpPara_p = PWM0FGParaList,               \
                                   .RegPara_p  = PWM0RegParaList,              \
                                   .List_p[0]  = &PWM0FgGpData_str,            \
                                   .List_p[1]  = &PWM0RegData_str};

/**
 * @brief   提供PWM2硬體初始化所需之設定佈局
 * @ingroup pwm_set_macro
 */
#define PWM2_HW_LAY()                                                          \
    HWFgGpPara_t PWM2FGParaList[5] = {{&TCCR2, 0x48, 3},                       \
                                      {&TCCR2, 0x07, 0},                       \
                                      {&TCCR2, 0x30, 4},                       \
                                      {&TIMSK, 0x40, 6},                       \
                                      {&DDRB, 0x80, 7}};                       \
                                                                               \
    PWM2FgGpData_t PWM2FgGpData_str = PWM2_FG_DATA_INT;                        \
    HWRegPara_t PWM2RegParaList[1]  = {{&OCR2, 1}};                            \
    PWM0_2RegData_t PWM2RegData_str = PWM0_2_REG_DATA_INT;                     \
                                                                               \
    HardWareSet_t PWM2HWSet_str = {.FgGpNum    = 5,                            \
                                   .RegNum     = 1,                            \
                                   .FgGpPara_p = PWM2FGParaList,               \
                                   .RegPara_p  = PWM2RegParaList,              \
                                   .List_p[0]  = &PWM2FgGpData_str,            \
                                   .List_p[1]  = &PWM2RegData_str};

/**
 * @brief   提供PWM1與PWM3硬體初始化所需之旗標設定結構
 * @ingroup pwm_set_struct
 *
 * @param WGMn0_1 WaveMode0
 * @param WGMn2_3 WaveMode1
 * @param CSn0_2 ClockSource1
 * @param COMnA0_1 WaveOut0
 * @param COMnB0_1 WaveOut1
 * @param COMnC0_1 WaveOut2
 * @param DDxA WaveOutPin
 * @param DDxB WaveOutPin
 * @param DDxC WaveOutPin
 * @param TotalByte 結構中參數數量
 */
typedef struct {
    uint8_t WGMn0_1;
    uint8_t WGMn2_3;
    uint8_t CSn0_2;
    uint8_t COMnA0_1;
    uint8_t COMnB0_1;
    uint8_t COMnC0_1;
    uint8_t DDxA;
    uint8_t DDxB;
    uint8_t DDxC;
    uint8_t TIMSKn;
    uint8_t TotalByte;
} PWM1_3FgGpData_t;

/**
 * @brief   提供PWM1與PWM3硬體初始化所需之暫存器設定結構
 * @ingroup pwm_set_struct
 */
typedef struct {
    uint16_t ICRn;
    uint16_t OCRnA;
    uint16_t OCRnB;
    uint16_t OCRnC;
    uint8_t TotalByte;
} PWM1_3RegData_t;

/**
 * @brief   提供PWM1硬體初始化所需之設定佈局
 * @ingroup pwm_set_macro
 */
#define PWM1_HW_LAY()                                                          \
    HWFgGpPara_t PWM1FGParaList[10] = {{&TCCR1A, 0x03, 0}, {&TCCR1B, 0x18, 3}, \
                                       {&TCCR1B, 0x07, 0}, {&TCCR1A, 0xc0, 6}, \
                                       {&TCCR1A, 0x30, 4}, {&TCCR1A, 0x0c, 2}, \
                                       {&DDRB, 0xe0, 5},   {&DDRB, 0x40, 6},   \
                                       {&DDRB, 0x80, 7},   {&TIMSK, 0x04, 2}}; \
                                                                               \
    HWRegPara_t PWM1RegParaList[4] = {                                         \
        {&ICR1L, 2}, {&OCR1AL, 2}, {&OCR1BL, 2}, {&OCR1CL, 2}};                \
                                                                               \
    PWM1_3FgGpData_t PWM1FgGpData_str = PWM1_3_FG_DATA_INT;                    \
    PWM1_3RegData_t PWM1RegData_str   = PWM1_3_REG_DATA_INT;                   \
                                                                               \
    HardWareSet_t PWM1HWSet_str = {.FgGpNum    = 10,                           \
                                   .RegNum     = 4,                            \
                                   .FgGpPara_p = PWM1FGParaList,               \
                                   .RegPara_p  = PWM1RegParaList,              \
                                   .List_p[0]  = &PWM1FgGpData_str,            \
                                   .List_p[1]  = &PWM1RegData_str};

/**
 * @brief   提供PWM3硬體初始化所需之設定佈局
 * @ingroup pwm_set_macro
 */
#define PWM3_HW_LAY()                                                          \
    static HardWareSet_t PWM3HWSet_str = {0};                                  \
    {                                                                          \
        static HWFgGpPara_t PWM3FGParaList[10] = {                             \
            {&TCCR3A, 0x03, 0}, {&TCCR3B, 0x18, 3}, {&TCCR3B, 0x07, 0},        \
            {&TCCR3A, 0xc0, 6}, {&TCCR3A, 0x30, 4}, {&TCCR3A, 0x0c, 2},        \
            {&DDRB, 0x08, 3},   {&DDRB, 0x10, 4},   {&DDRB, 0x20, 5},          \
            {&ETIMSK, 0x04, 2}};                                               \
                                                                               \
        static HWRegPara_t PWM3RegParaList[4] = {                              \
            {&ICR3L, 2}, {&OCR3AL, 2}, {&OCR3BL, 2}, {&OCR3CL, 2}};            \
                                                                               \
        static PWM1_3FgGpData_t PWM3FgGpData_str = PWM1_3_FG_DATA_INT;         \
                                                                               \
        static PWM1_3RegData_t PWM3RegData_str = PWM1_3_REG_DATA_INT;          \
        PWM3HWSet_str.FgGpNum                  = 10;                           \
        PWM3HWSet_str.RegNum                   = 4;                            \
        PWM3HWSet_str.FgGpPara_p               = PWM3FGParaList;               \
        PWM3HWSet_str.RegPara_p                = PWM3RegParaList;              \
        PWM3HWSet_str.List_p[0]                = &PWM3FgGpData_str;            \
        PWM3HWSet_str.List_p[1]                = &PWM3RegData_str;             \
    }
/*-- hardwareinit section end ------------------------------------------------*/

/*-- pipeline section start --------------------------------------------------*/

/**
 * @brief   Pipeline 結構體
 * @ingroup SysPipeline_struct
 * 
 * @param MaxTask Maximum capacity for tasks to be Registed
 * @param Total Total number of Registed Tasks
 * @param TaskId Task Identification gotten
 * @param HWIStr_ssp pointer of HW interrupt structure
 * @param Task_p pointer of Task List
 * @param FifoBF_p pointer of FIFO buffer structure
 * @param ScopeBF_p pointer of scope buffer structure
 */
typedef struct {
    uint8_t MaxTask;
    uint8_t Total;
    volatile uint8_t TaskId;
    HWIntStr_t* HWIStr_ssp;
    TaskBlock_t* Task_p;
    FifoBFStr_t* FifoBF_p;
    ScopeBFStr_t* ScopeBF_p;
} PipelineStr_t;

/**
 * @brief
 * 供使用者呼叫將功能方塊登錄成可由管道工作執行器代執行的工作方塊。執行工作包括:
 *          1. 取得新工作編號。
 *          2. 將功能方塊結構體指標，步級函式，存進管道工作結構體相應編號位置。
 *          3. 將管道工作結構體，工作編號，存進功能方塊結構體。
 *          4. 試跑工作方塊，求取執行需時，送到人機顯示。
 *
 * @param PLStr_ssp     管道工作執行器管理用結構體的住址指標。
 * @param FbFunc_p      管道工作執行器執行之工作函式之住址指標。
 * @param FbPara_p      管道工作執行器執行之工作函式專用結構體之住址指標。
 * @param TaskName_p    pointer to Name of the task to be registered 工作名稱。
 * @return              工作識別碼。
 */
uint8_t Pipeline_reg(PipelineStr_t* PLStr_ssp, Func_t FbFunc_p, void* FbPara_p,
                     uint8_t* TaskName_p);

/**
 * @brief
 * 能夠由先進先出緩衝器取得工作識別碼，據以由工作方塊表中取得工作方塊並予以執行。
 *        本身亦與管道工作結構體組成工作方塊並登錄於計時中斷中執行。
 *        完成執行之工作識別碼會存入移動區段緩衝區做為記錄，若工作方塊回應有錯誤時，
 *        除了會將錯誤訊息工作名稱送往人機之外，也將先前執行的工作識別碼送往人機，以協助除錯。
 *
 * @param void_p 己被無形化之管道工作執行器管理用結構體。
 */
void Pipeline_step(void* void_p);

/**
 * @brief   提供管道工作執行器初始化所需之設定佈局
 * @ingroup pipeline_set_macro
 *
 * @param TASKNUM       Maximum Number of Tasks allow to be registed into
 * Pipeline executor.
 * @param FBDEPTH       FIFO Buffer element number.
 * @param SBDEPTH       Scope Buffer element number.
 */
#define PIPELINE_LAY(TASKNUM, FBDEPTH, SBDEPTH)                                \
    FIFOBUFF_LAY(PipelineFifo_str, FBDEPTH);                                   \
    SCOPEBUFF_LAY(PipelineScope_str, SBDEPTH, SBDEPTH);                        \
    {                                                                          \
        static TaskBlock_t Pipeline_TASKLIST[TASKNUM];                         \
        SysPipeline_str.MaxTask   = TASKNUM;                                   \
        SysPipeline_str.Task_p    = Pipeline_TASKLIST;                         \
        SysPipeline_str.FifoBF_p  = &PipelineFifo_str;                         \
        SysPipeline_str.ScopeBF_p = &PipelineScope_str;                        \
    }

/**
 * @brief
 * 提供前級工作方塊，於完成本身工作之後，叫用以便將其後級的後續工作指標，放進先進出緩衝區等待管道工作執行器執行。
 * @ingroup trig_next_task_macro
 *
 * @param NEXTTASKINDEX Index of next Tasks.
 */
#define TRIG_NEXT_TASK(NEXTTASKINDEX)                                          \
    {                                                                          \
        SysPipeline_str.TaskId = NEXTTASKINDEX % (SysPipeline_str.MaxTask);    \
        SysPipeline_str.Task_p[SysPipeline_str.TaskId].state++;                \
        FifoBF_put((FifoBFStr_t*)(SysPipeline_str.FifoBF_p),                   \
                   (void*)&SysPipeline_str.TaskId, 1);                         \
    }


extern PipelineStr_t SysPipeline_str;
/*-- pipeline section end ----------------------------------------------------*/


#endif // C4MLIB_H
