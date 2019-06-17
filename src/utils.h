#ifndef UTILS_H_
#define UTILS_H_

extern unsigned int timer_flag;

void set_duration(double duration); // Time in seconds
void hpas_sleep(double sleeptime);  // Time in seconds

long int parse_size(char *input);

#endif /* UTILS_H_ */
