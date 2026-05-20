/*
 ******************************************************************************************
 * @file      task_app.h
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     uC/OS-III task application.
 ******************************************************************************************
 */
 
#ifndef	__TASK_APP_H_
#define	__TASK_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

/* Macros-------------------------------------------------------------------------------- */
#define PRINTF_USE

#ifdef	PRINTF_USE
#define printf_d(args...)					printf(args)
#endif	//PRINTF_USE

/* Declarations-------------------------------------------------------------------------- */
extern void task_app(void);


#ifdef __cplusplus
}  //extern "C"
#endif //__cplusplus

#endif    /* __TASK_APP_H_ */
