#ifndef SENSORES_H
#define SENSORES_H

void sensor_put(double temp, double nivel, double tempar, double tempagua, double fluxoagua);
double sensor_get(char *s);
void sensor_alarmeT(double limite);

#endif



