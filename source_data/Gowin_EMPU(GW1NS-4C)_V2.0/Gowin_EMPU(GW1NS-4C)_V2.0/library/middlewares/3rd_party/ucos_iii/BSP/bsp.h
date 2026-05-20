/*
 * *****************************************************************************************
 *
 * 		Copyright (C) 2014-2019 Gowin Semiconductor Technology Co.,Ltd.
 * 		
 * @file			bsp.h
 * @author		Embedded Development Team
 * @version		V1.3.0
 * @date			2019-10-1 09:00:00
 * @brief			Board support package.
 ******************************************************************************************/

#ifndef  BSP_PRESENT
#define  BSP_PRESENT

#ifdef   BSP_MODULE
#define  BSP_EXT
#else
#define  BSP_EXT  extern
#endif

/* Include---------------------------------------------------------------------- */
#include  <stdio.h>
#include  <stdarg.h>
#include  <cpu.h>
#include  <cpu_core.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include "system_gw1ns4c.h"

#endif  /* BSP_PRESENT */
