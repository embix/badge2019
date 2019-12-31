#include "font.h"
#include "display.h"
#include "leds.h"
#include "timer.h"
#include "power.h"
#include "events.h"

#define FGColor 0xCB0
#define BGColor 0x000

enum events
{
	EV_TICK=17
};

void
helloworld(void)
{
	dp_fill(0, 0, 240, 240, 0x000);

	dp_puts(0, 0, FGColor, BGColor, ":P");

	led1_on();
	led2_on();
	led3_on();	

	dp_puts(55, 50,      0x00F, 0x00, "Hold POWER");
	dp_puts(75, 50 + 28, 0x00F, 0x00, "to exit");	

	struct ticker tick250;
	unsigned int count = 0;
	ticker_start(&tick250, 250, EV_TICK);

	while(1) {
		switch ((enum events) event_wait()) {
			case EV_TICK:
				if(power_pressed()) {
					++count;
					if (count == 3) {
						ticker_stop(&tick250);
						led1_off();
						led2_off();
						led3_off();
						return;
					}				
				}
				break;
		}		
	}	
}
