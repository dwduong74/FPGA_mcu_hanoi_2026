/*
struct tree
	main.c: initial and main loop function
	PFPA.c: unused
	GPIO.c: initial gpio pin
	 WDT.c: initial watchdog with 250ms reset while do nothing
  ct16b0.c:	 support ms timer
     adc.c: measure potentiomter voltage
 utility.c: delay
 
 segment.c: display adc value
 
 function:
	1. ADC sample P2.0 voltage with potentionmter.
	2. segment tube display the adc value ,range 0 ~ 4095
	   actually display value 0~4088, due to  low pass filter.
	3. segment tube scan frequency used CT16B0 timer 1ms.   
*/