//Defini��o de Bibliotecas
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"
#include "sensores.h"
#include "tela.h"
#include "bufduplo.h"
#include "referenciaT.h"
#include "referenciaH.h"

#define	NSEC_PER_SEC    (1000000000) 	// Numero de nanosegundos em um segundo
#define NUM_THREADS	8
#define N_AMOSTRAS 1000

double REF_TEMP = 0.0, NIVEL_REF = 0.0;

//funcoes
void ref_putT( double ref);
void ref_putH(double ref);
double ref_getT(void);
double ref_getH(void);
void sensor_put(double temp, double nivel, double tempar, double tempagua, double fluxoagua);
double sensor_get(char *s);
void sensor_alarmeT(double limite);

void thread_mostra_status (void){
	double ta, t, ti, no, h, ref_nivel, ref_temp;
	while(1){
		ta = sensor_get("ta");
		t = sensor_get("t");
		ti = sensor_get("ti");
		no = sensor_get("no");
		h = sensor_get("h");

		ref_temp = ref_getT();
		ref_nivel = ref_getH();

		aloca_tela();
		system("tput reset");
		printf("Temperatura de referencia--> %.2lf\n", ref_temp);
		printf("Nivel de referencia--> %.2lf\n", ref_nivel);
		printf("---------------------------------------\n");
		printf("Temperatura do ar (Ta)--> %.2lf\n", ta);
		printf("Temperatura (T)--> %.2lf\n", t);
		printf("Temperatura da agua (Ti)--> %.2lf\n", ti);
		printf("Fluxo de agua (No)--> %.2lf\n", no);
		printf("Nivel (H)--> %.2lf\n", h);
		printf("---------------------------------------\n");
		libera_tela();
		sleep(1);
		//								
	}	
		
}


void thread_le_sensor (void){ //Le Sensores periodicamente a cada 10ms
	char msg_enviada[1000];
	struct timespec t, t_fim;
	long periodo = 10e6; //10e6ns ou 10ms
	
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);
	while(1){
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		//Envia mensagem via canal de comunica��o para receber valores de sensores		
		sensor_put(
			msg_socket("st-0"), //Temperatura
			msg_socket("sh-0"), //Nivel
			msg_socket("sta0"), //Temperatura do ar
			msg_socket("sti0"), //Temperatura da agua
			msg_socket("sno0")//Fluxo da agua
		);
		
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}		
	}		
}

void thread_alarme (void){
	while(1){
		sensor_alarmeT(REF_TEMP);
		aloca_tela();
		printf("ALARME\n");
		libera_tela();
		sleep(1);	
	}
}

///Controle
void thread_controle_temperatura (void){
	char msg_enviada[1000];
	long atraso_fim;
	struct timespec t, t_fim;
	long periodo = 50e6; //50ms
	double temp, ref_temp;
    double nivel, ref_niv;
    double tol = 0.005;
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);
	t.tv_sec++;
	
    while(1){
		// Espera o inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		temp = sensor_get("t");
		ref_temp = ref_getT();
  	nivel = sensor_get("h");
		ref_niv = ref_getH();
		//printf("ref_temp %lf\n", &ref_temp);
		//printf("temp %lf\n", temp);

        
		if(temp > ref_temp+tol) { //se a temperatura estiver alta...     
			if(nivel > ref_niv+tol) { //...e o nivel estiver alto
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 0.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 100.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 0.0 );
				msg_socket(msg_enviada);
			} 

			if(ref_niv-tol <= nivel && nivel <= ref_niv+tol) { //...e o nivel estiver razoavel
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 100.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 100.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 0.0 );
				msg_socket(msg_enviada);
			}   

			if(nivel < ref_niv-tol) { //...e o nivel estiver baixo
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 100.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 0.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 0.0 );
				msg_socket(msg_enviada);            
			}             
		}


		if(ref_temp-tol <= temp && temp <= ref_temp+tol) { //se a temperatura estiver razoável...
			if(nivel > ref_niv+tol) { //...e o nivel estiver alto
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 0.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 100.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 0.0 );
				msg_socket(msg_enviada);            
			}

			if(ref_niv-tol <= nivel && nivel <= ref_niv+tol) { //...e o nivel estiver razoavel
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 0.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 0.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 0.0 );
				msg_socket(msg_enviada);            
			}   

			if(nivel < ref_niv-tol) { //...e o nivel estiver baixo
				//tem que entrar água por ni
				sprintf( msg_enviada, "ani%lf", 100.0);
				msg_socket(msg_enviada);
				//não pode sair água por nf		
				sprintf( msg_enviada, "anf%lf", 0.0);
				msg_socket(msg_enviada);
				//não é interessante entrar água por n0
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp não importa a princípio
				sprintf( msg_enviada, "aq-%lf", 0.0 );
				msg_socket(msg_enviada);            
			}  
		}


		if(temp < ref_temp-tol) { //se a temperatura estiver baixa...
			if(nivel > ref_niv+tol) { //...e o nivel estiver alto
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 0.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 100.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 1000000.0 );
				msg_socket(msg_enviada);                
			} 

			if(ref_niv-tol <= nivel && nivel <= ref_niv+tol) { //...e o nivel estiver razoavel
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 0.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 10.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 10.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 1000000.0 );
				msg_socket(msg_enviada);            
			}

			if(nivel < ref_niv-tol) { //...e o nivel estiver baixo
				//não pode entrar água por ni
				sprintf( msg_enviada, "ani%lf", 100.0);
				msg_socket(msg_enviada);
				//tem que sair água por nf		
				sprintf( msg_enviada, "anf%lf", 0.0);
				msg_socket(msg_enviada);
				//não pode entrar água por na
				sprintf( msg_enviada, "ana%lf", 0.0 );
				msg_socket(msg_enviada);
				//temp importa a princípio
				sprintf( msg_enviada, "aq-%lf", 1000000.0 );
				msg_socket(msg_enviada);
			}                
		}
               
		// Le a hora atual, coloca em t_fim
		clock_gettime(CLOCK_MONOTONIC ,&t_fim);
		
		// Calcula o tempo de resposta observado em microsegundos
		atraso_fim = 1000000*(t_fim.tv_sec - t.tv_sec)   +   (t_fim.tv_nsec - t.tv_nsec)/1000;
		
		bufduplo_insereLeitura(atraso_fim);
		
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC){
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
    }		
}

void thread_controle_nivel (void){
	char msg_enviada[1000];
	long atraso_fim;
	struct timespec t, t_fim;
	long periodo = 70e6; //70ms
	double nivel, ref_niv;
    double temp, ref_temp;
    double tol = 0.005;
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);
	t.tv_sec++;
	while(1){
		// Espera o inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
        temp = sensor_get("t");
		ref_temp = ref_getT();		
        nivel = sensor_get("h");
		ref_niv = ref_getH();
		//printf("ref_temp %lf\n", &ref_niv);
		//printf("nivel %lf\n", nivel);
		
        if(nivel > ref_niv+tol) { //se o nível estiver alto...   
            if(temp > ref_temp+tol){ //...e a temperatura estiver alta
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 0.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 100.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 0.0 );
			    msg_socket(msg_enviada);
            }     
            if(ref_temp-tol <= temp && temp <= ref_temp+tol){ //...e a temperatura estiver razoável
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 0.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 100.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 0.0 );
			    msg_socket(msg_enviada);
            }   
            if(temp < ref_temp-tol){ //...e a temperatura estiver baixa
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 0.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 100.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 1000000.0 );
			    msg_socket(msg_enviada);
            }       
		}


        if(ref_niv-tol <= nivel && nivel <= ref_niv+tol){ //se o nivel estiver razoável...
            if(temp > ref_temp+tol){ //...e a temperatura estiver alta
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 100.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 100.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 0.0 );
			    msg_socket(msg_enviada);
            }     
            if(ref_temp-tol <= temp && temp <= ref_temp+tol){ //...e a temperatura estiver razoável
                //tem que entrar água por ni
                sprintf( msg_enviada, "ani%lf", 0.0);
			    msg_socket(msg_enviada);
			    //não pode sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 0.0);
			    msg_socket(msg_enviada);
			    //não é interessante entrar água por n0
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp não importa a princípio
                sprintf( msg_enviada, "aq-%lf", 0.0 );
			    msg_socket(msg_enviada);
            }   
            if(temp < ref_temp-tol){ //...e a temperatura estiver baixa
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 0.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 10.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 10.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 1000000.0 );
			    msg_socket(msg_enviada);
            }      
        }


        if(nivel < ref_niv-tol) { //e se o nivel estiver baixo...   
             if(temp > ref_temp+tol){ //...e a temperatura estiver alta
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 100.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 0.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 0.0 );
			    msg_socket(msg_enviada);
                }     
            if(ref_temp-tol <= temp && temp <= ref_temp+tol){ //...e a temperatura estiver razoável
                //tem que entrar água por ni
                sprintf( msg_enviada, "ani%lf", 100.0);
			    msg_socket(msg_enviada);
			    //não pode sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 0.0);
			    msg_socket(msg_enviada);
			    //não é interessante entrar água por n0
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp não importa a princípio
                sprintf( msg_enviada, "aq-%lf", 0.0 );
			    msg_socket(msg_enviada);
            }   
            if(temp < ref_temp-tol){ //...e a temperatura estiver baixa
                //não pode entrar água por ni
                sprintf( msg_enviada, "ani%lf", 100.0);
			    msg_socket(msg_enviada);
			    //tem que sair água por nf		
			    sprintf( msg_enviada, "anf%lf", 0.0);
			    msg_socket(msg_enviada);
			    //não pode entrar água por na
			    sprintf( msg_enviada, "ana%lf", 0.0 );
			    msg_socket(msg_enviada);
                //temp importa a princípio
                sprintf( msg_enviada, "aq-%lf", 1000000.0 );
			    msg_socket(msg_enviada);
            }                      
		}

        
		// Le a hora atual, coloca em t_fim
		clock_gettime(CLOCK_MONOTONIC ,&t_fim);
		
		// Calcula o tempo de resposta observado em microsegundos
		atraso_fim = 1000000*(t_fim.tv_sec - t.tv_sec)   +   (t_fim.tv_nsec - t.tv_nsec)/1000;
		
		bufduplo_insereLeitura(atraso_fim);
		
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
    }
}

void thread_grava_temp_resp(void){
	FILE* dados_f;
	dados_f = fopen("dados.txt", "w");
	if(dados_f == NULL){
		printf("Erro, nao foi possivel abrir o arquivo\n");
		exit(1);    
	}
	int amostras = 1;
	while(amostras++<=N_AMOSTRAS/200){
		long * buf = bufduplo_esperaBufferCheio();		
		int n2 = tamBuf();
		int tam = 0;
		while(tam<n2)
			fprintf(dados_f, "%4ld\n", buf[tam++]); 
		fflush(dados_f);
		aloca_tela();
		printf("Gravando no arquivo...\n");
		
		libera_tela();		
	}
	
	fclose(dados_f);	
}


void thread_altera_valores (void){
	char input;
	while(1){
		input = getchar();
		if(input == 'm') {
			aloca_tela();
			printf("Digite um valor para a temperatura de referencia maior que 0: ");
			scanf("%lf", &REF_TEMP);
			ref_putT(REF_TEMP);

			printf("Digite um valor para o nível de referencia maior que 0: ");
			scanf("%lf", &NIVEL_REF);
			ref_putH(NIVEL_REF);
			libera_tela();
			input = '1';
		}
		sleep(1);		
	}
}

void thread_grava_temp_nivel (void) {
	FILE* dados_f;
	dados_f = fopen("valores_por_segundo.txt", "w");
	if(dados_f == NULL){
		printf("Erro, nao foi possivel abrir o arquivo\n");
		exit(1);    
	}

	struct timespec time;
	long periodo = 1000e6; //1s
	double temp, nivel;

	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&time);
	time.tv_sec++;
	double t ,h; 

	while(1){
		// Espera o inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);

		temp = sensor_get("t");
		nivel = sensor_get("h");

		fprintf(dados_f, "_________Medições__________\n");
		fprintf(dados_f, "Valor temperatura: %lf\n", temp);
		fprintf(dados_f, "Valor nivel: %lf\n\n", nivel);
		fflush(dados_f);
		sleep(1);

		time.tv_nsec += periodo;
		while (time.tv_nsec >= NSEC_PER_SEC){
			time.tv_nsec -= NSEC_PER_SEC;
			time.tv_sec++;
		}
	}
	
}

void main( int argc, char *argv[]) {
	ref_putT(20.0);
  ref_putH(2.2);
	REF_TEMP = 30.0;
	NIVEL_REF = 2.2;

	cria_socket(argv[1], atoi(argv[2]) );
    
	int ord_prio[NUM_THREADS]={99,99,59,1,1,1,55,66};
	pthread_t threads[NUM_THREADS];
	pthread_attr_t pthread_custom_attr[NUM_THREADS];
	struct sched_param priority_param[NUM_THREADS];
	
	//Configura escalonador do sistema
	for(int i=0;i<NUM_THREADS;i++){
		pthread_attr_init(&pthread_custom_attr[i]);
		pthread_attr_setscope(&pthread_custom_attr[i], PTHREAD_SCOPE_SYSTEM);
		pthread_attr_setinheritsched(&pthread_custom_attr[i], PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&pthread_custom_attr[i], SCHED_FIFO);
		priority_param[i].sched_priority = ord_prio[i];
		if (pthread_attr_setschedparam(&pthread_custom_attr[i], &priority_param[i])!=0)
			fprintf(stderr,"pthread_attr_setschedparam failed\n");
	}

	pthread_create(&threads[0], &pthread_custom_attr[0], (void *) thread_controle_temperatura, NULL);
	pthread_create(&threads[0], &pthread_custom_attr[0], (void *) thread_controle_nivel, NULL);
	pthread_create(&threads[1], &pthread_custom_attr[1], (void *) thread_le_sensor, NULL);
	pthread_create(&threads[2], &pthread_custom_attr[2], (void *) thread_alarme, NULL);
	pthread_create(&threads[3], &pthread_custom_attr[3], (void *) thread_mostra_status, NULL);
	pthread_create(&threads[4], &pthread_custom_attr[4], (void *) thread_grava_temp_resp, NULL);
	pthread_create(&threads[5], &pthread_custom_attr[5], (void *) thread_altera_valores, NULL);
	pthread_create(&threads[6], &pthread_custom_attr[6], (void *) thread_grava_temp_nivel, NULL);

	for(int i=0;i<NUM_THREADS;i++){
		pthread_join(threads[i], NULL);
	}
}
