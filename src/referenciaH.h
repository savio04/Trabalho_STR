/* Monitor sensor, no arquivo sensor.c */ 
#include <math.h>
#include <pthread.h>

static pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; 
static double ref_entrada = 0; 
static double ref_entrada_nivel = 0;
static double limite_atual = HUGE_VAL;

void ref_putH(double ref) {
	pthread_mutex_lock( &exclusao_mutua); 
	ref_entrada_nivel = ref; 
	pthread_mutex_unlock( &exclusao_mutua); 
}

double ref_getH(void) {
	double aux; 
	pthread_mutex_lock( &exclusao_mutua); 
	aux = ref_entrada_nivel; 
	pthread_mutex_unlock( &exclusao_mutua); 
	return aux;
}