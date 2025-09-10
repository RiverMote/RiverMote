#include "random"
/*
#define BUFF_SIZE 100


double avgBuffer[] = {};

void fill_buffer(double new_val) {
    //if(avgBuffer.length < BUFF_SIZE) {
        //
    //}
    // add new value to buffer
    //buffer.pop();	
    for(int i=BUFF_SIZE-1; i >= 1; i--) {
        // shuffle everything right by 1
        double curr_val = avgBuffer[i-1]; // move everything one to the left
        avgBuffer[i] =  curr_val;
    }
    avgBuffer[0] = new_val; // add new val to the beginning of the array
}

double get_average(double new_val) {
    // get average 
	double avg = 0;
    double sum = 0;
    for(int i=0; i < BUFF_SIZE; i++){
		//val = avgBuffer[i];
        sum = sum + avgBuffer[i];
        avg = sum/BUFF_SIZE;
    }
    return avg; 
}


void test_avg() {
	long test_val = random();
    fill_buffer(test_val);
	double avg_test = get_average(test_val); Serial.println(avg_test);
    //return avg_test;
}
*/