#include "systim.h"

uint32_t getSYSTIM(void)
{
	return millis();
}

uint8_t chk4TimeoutSYSTIM(uint32_t btime, uint32_t period)
{
	uint32_t time = millis();
	
	if(time >= btime)
	{
		if((time - btime) >= period)
		{
			return (SYSTIM_TIMEOUT);
		}
		else
		{
			return (SYSTIM_KEEP_ALIVE);
		}
	}
	else
	{
		uint32_t utmp32 = 0xFFFFFFFF - btime;
		if((time + utmp32) >= period)
		{
			return (SYSTIM_TIMEOUT);
		}
		else
		{
			return (SYSTIM_KEEP_ALIVE);
		}
	}
}

uint32_t getElapsedTimeSYSTIM(uint32_t t_beg)
{
	uint32_t time = millis();
	t_beg = t_beg;
	
	if(time >= t_beg)
	{
		return (time - t_beg);
	}
	else
	{
		return (time + (0xFFFFFFFF - t_beg));
	}
}
