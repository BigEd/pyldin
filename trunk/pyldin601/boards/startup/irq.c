/*****************************************************************************
 *   irq.c: Interrupt handler C file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.13  ver 1.00    Prelimnary version, first Release
 *
 ******************************************************************************/ 

/* TODO mthomas - volatiles for vect_addr, vect_cntl? */

#include "config.h"
#include <inttypes.h>
#include "irq.h"

/* Initialize the interrupt controller */
/******************************************************************************
** Function name:		init_VIC
**
** Descriptions:		Initialize VIC interrupt controller.
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void Init_VIC(void) 
{
    uint32_t i = 0;
    uint32_t *vect_addr, *vect_prio;

    /* Initialize VIC*/
    VICIntEnClr = 0xffffffff;
    VICVectAddr = 0;
    VICIntSelect = 0;

    /* Set all the vector and vector control register to 0 */
    for (i = 0; i < VIC_SIZE; i++)
    {
	vect_addr = (uint32_t *) (VIC_BASE_ADDR + VECT_ADDR_INDEX + i * 4);
	vect_prio = (uint32_t *) (VIC_BASE_ADDR + VECT_PRIO_INDEX + i * 4);
	*vect_addr = 0x0;
	*vect_prio = 0xF;
    }

    return;
}

/******************************************************************************
** Function name:		install_irq
**
** Descriptions:		Install interrupt handler
** parameters:			Interrupt number, interrupt handler address, 
**						interrupt priority
** Returned value:		true or false, return false if IntNum is out of range
** 
******************************************************************************/
uint32_t Install_IRQ( uint32_t IntNumber, void *HandlerAddr, uint32_t Priority )
{
    uint32_t *vect_addr;
    uint32_t *vect_prio;

    VICIntEnClr = (1 << IntNumber); /* Disable Interrupt */
    if ( IntNumber >= VIC_SIZE )  {
	return (FALSE);
    } else {
	/* Find first un-assigned VIC address for the handler */
	/* vect_addr is a var to keep address of interrupt service routine */
	vect_addr = (uint32_t *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber * 4);
	/* vect_prio is a var to keep priority of interrupt service routine */
	vect_prio = (uint32_t *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + IntNumber * 4);
	/* set the address of the routine passed as a parameter */
	*vect_addr = (uint32_t) HandlerAddr;
	/* set the priority of the routine passed as a parameter */
	*vect_prio = Priority;

	VICIntEnable = (1 << IntNumber);	/* Enable Interrupt */
	return(TRUE);
    }
}

/******************************************************************************
**                            End Of File
******************************************************************************/
