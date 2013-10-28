#ifndef _pulse_h_
#include <pulse/pulseaudio.h>

typedef struct pulse_con {
	pa_mainloop_api * api;
	pa_context * context;
	pa_threaded_mainloop * mainloop;
	char * socket;
	int module_idx;
} pulse_con_t;

void pulse_startup(pulse_con_t *p, char * socket);

#endif /* _pulse_h_*/