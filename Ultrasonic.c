#include <wiringPi.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <softPwm.h>


#define servoPin_US     6   	//Ultraschall-Servo     out PWM
#define trigPin         4   	//Ultraschall-Trigger   out digital
#define echoPin         5   	//Ultraschall-Echo      in  digital
#define	SERVO_MIN_US	7+OFFSET_ST
#define SERVO_MAX_US	23+OFFSET_ST


struct timespec Time1, Time2;

void servoWriteMS(int pin, int ms){     //specific the unit for pulse(5-25ms) with specific duration output by servo pin: 0.1ms
    if(ms > SERVO_MAX_US) {
        printf("Pin: %i ms: %i too big\n",pin,ms);
        ms = SERVO_MAX_US;
    };
    if(ms < SERVO_MIN_US) {
        printf("Pin: %i ms: %i too small\n",pin,ms);
        ms = SERVO_MIN_US;
    };
    softPwmWrite(pin,ms);
	delay(10);
}


void StartStopTimer (void) {
	if (digitalRead(echoPin)==HIGH) {
		clock_gettime(CLOCK_REALTIME, &Time1);
		StartTime = rTime.tv_nsec;
	}else{
		clock_gettime(CLOCK_REALTIME, &Time2);
		EndTime = rTime.tv_nsec;
	}
}
		
float getSonar(void) {
	// send trigger signal
	digitalWrite(trigPin,HIGH);
    	delayMicroseconds(10);
	digitalWrite(trigPin,LOW);
    	delayMicroseconds(10);
	
	float puls = Time1.tv_nsec - Time2.tv_nsec;
	return (puls * 340.0 / 2.0 / 10000.0);
}
	
float getSonarP(int angle) {
	float distance;
	servoWriteMS(servoPin_US,angle);
	delay(100);
	distance = getSonar();
	printf(" %3.2f cm an Position %i \n",distance,angle);
	return distance;
}

void main(void) {
	if(wiringPiSetup() == -1){ 
        printf("setup wiringPi faiservo !");
        return 1; 
    };
	
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);
	pinMode(servoPin_US,OUTPUT);
	softPwmCreate(servoPin_US, 0, 200);
	
	wiringPiISR (echoPin, INT_EDGE_BOTH, &StartStopTimer) ;
	
	for (i=SERVO_MIN_US;i<SERVO_MAX_US;i++) {
		printf("Position %i, Distance %f cm",i, getSonarP(i));
	}
	for (i=SERVO_MAX_US;i<SERVO_MIN_US;i--) {
		printf("Position %i, Distance %f cm",i, getSonarP(i));
	}
		
}
