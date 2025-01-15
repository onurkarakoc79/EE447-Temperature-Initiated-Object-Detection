

#ifndef RADAR_H
#define RADAR_H
#define step_count 			4
#define step_motor_angle  1024
#define STEP_MOTOR_PIN_1 PE2 
#define STEP_MOTOR_PIN_2 PE3
#define STEP_MOTOR_PIN_3 PE4
#define STEP_MOTOR_PIN_4 PE5

#define MAX_ANGLES          180
#define RENDER_RATE 		5

void RADAR_GPIO_ENABLE(void);



void Run_Motor(int step_no);



void RADAR_DISABLE(void);



void RADAR_ENABLE(void);



void SysTick_Handler(void);


typedef struct {
     float distance[MAX_ANGLES];
     float angle[MAX_ANGLES];
     float avg_distance;
     float avg_angle;
} RADAR_Struct;

extern RADAR_Struct* const radar;

#endif 