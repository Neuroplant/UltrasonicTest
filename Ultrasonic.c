#include <wiringPi.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <softPwm.h>
#include <stdio.h>


#define servoPin_US     6   	//Ultraschall-Servo     out PWM
#define trigPin         2   	//Ultraschall-Trigger   out digital
#define echoPin         0   	//Ultraschall-Echo      in  digital
#define	SERVO_MIN_US	5
#define SERVO_MAX_US	27


struct timespec Time1;
float SArray[30][2];
long StartTime, EndTime;   
int flip = TRUE;

int pulseIn(int pin, int level, int timeout)
{
   struct timeval tn, t0, t1;
   long micros;
   gettimeofday(&t0, NULL);
   micros = 0;
   while (digitalRead(pin) != level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(pin) == level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros = micros + (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros = micros + (tn.tv_usec - t1.tv_usec);
   return micros;
}


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
	clock_gettime(CLOCK_REALTIME, &Time1);
	StartTime  = Time1.tv_nsec;
	while (digitalRead(echoPin)) {
		clock_gettime(CLOCK_REALTIME, &Time1);
		EndTime = Time1.tv_nsec;
	}
}
		
float getSonar(){   // get the measurement results of ultrasonic module,with unit: cm
    long pingTime;
    float distance;
    digitalWrite(trigPin,HIGH); //trigPin send 10us high level 
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pingTime = pulseIn(echoPin,HIGH,13200);   //read plus time of echoPin
    distance = (float)pingTime * 340.0 / 2.0 / 10000.0; // the sound speed is 340m/s,and calculate distance
    return distance;
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
    };
	
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);
	pinMode(servoPin_US,OUTPUT);
	softPwmCreate(servoPin_US, 0, 200);
	
	//wiringPiISR (echoPin, INT_EDGE_RISING, &StartStopTimer) ;
	
	for (int i=SERVO_MIN_US;i<SERVO_MAX_US;i++) {
		SArray[i][0]= getSonarP(i);
	}
	for (int i=SERVO_MAX_US;i<SERVO_MIN_US;i--) {
		SArray[i][1]= getSonarP(i);
	}
	float Fehler =0, Max = 0 , Min = 0;
	int Minimum=0,Maximum=0;
	for (int i=0;i<30;i++) {
		Fehler = Fehler+abs(SArray[i][0]-SArray[i][1]);
		if (SArray[i][0]>Max) {
			Maximum = i;
			Max = SArray[i][0];
		}
		if ((SArray[i][0] < Min) && (SArray[i][0] > 0)) {
			Minimum = i;
			Min = SArray[i][0];
		}
	}
	printf("\nMessfehler gesammt: %3.2f \n Minimum %3.2f bei Pos. %2i\n Maximum %3.2f bei Pos. %2i\n", Fehler, Min, Minimum, Max, Maximum);	
}
