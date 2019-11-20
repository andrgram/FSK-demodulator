/* 
 * Code for modulating a digital signal carried by a frequency modulated 
 * sine wave, oscillating between two frequencies.
 * It reads an analog signal on pin A0. and uses the built in LED
 * on pin 13 to indicate a high or low frequency signal.
 * As the Arduino only accepts voltages between 0 and 5 volts in,
 * the supplied signal needs to oscillate around 2.5 volts by the supply.
 *  
 * created November 19
 * by Andreas Gram
 * 
 */

#include <TimerOne.h>

// Global variables
volatile int sample;              // Variable storing the last sample
bool newSample;                   // Flag checking whether a sample has been taken

int sampleLow  = 0;               // Low pass filtered sample
int sampleHigh = 0;               // High pass filtered sample

const float aLow  =  0.5;         // Low pass filter characteristic
const float aHigh = -0.5;         // High pass filter characteristic

const int f_s = 2000;             // Sampling frequency in Hz
const int freqLow  = 200;         // Frequency of the low frequency signal
const int freqHigh = 850;         // Frequency of the high frequency signal

const int N = int(f_s/freqLow); // Normalization filter length. Set to twice the period of the lowest signal
int counter = 0;                  // Counter because arrays don´t work...
int sumLow = 0;                   // Sum of low pass filter samples. For normalization
int sumHigh = 0;                  // Sum of high pass filter samples. For normalization

int ledPin = 13;                  // Built in LED. For output. b(t)
int signalPin = 5;                // Indicates whether a signal is coming at all. u(t)


void setup() {
  // Setup of TimerOne interrupt:
  Timer1.initialize(int(1000000/f_s));    // Time set to 500 microseconds at f_s = 2000
  Timer1.attachInterrupt(takeSample);     // Run the takeSample at interrupts
  pinMode(ledPin, OUTPUT);                // Set the LED to output
  pinMode(signalPin, OUTPUT);             // Set the signal indicator to output
}

void loop() {
  
  if(newSample) {
    // Filter code

    // Runs the sample through low and high pass filters
    sampleLow = recursive(sample, sampleLow, aLow);
    sampleHigh = recursive(sample, sampleHigh, aHigh);
    
    // Adds the values measured
    sumLow += abs(sampleLow);
    sumHigh += abs(sampleHigh);

    // Increase counter until N samples are stored
    counter++;
    if (counter == N) {
      // Signal indicator high if sum of measurements exceeding 1000 for both
      // over one measurement period
      if(sumLow < 50 * N && sumHigh < 50 * N) {
        digitalWrite(signalPin, LOW);
        digitalWrite(ledPin, LOW);
      }
      else {
        digitalWrite(signalPin, HIGH);
        // LED on if low frequency signal. Off if otherwise
        if (sumLow < sumHigh) digitalWrite(ledPin, HIGH);
        else digitalWrite(ledPin, LOW);
      }
      
      // resets counting values
      sumLow = 0;
      sumHigh = 0;
      counter = 0;
      
    }
    // Resets newSample flag
    newSample = false;
  }
}

// Takes a sample, makes it oscillate around zero, and sets the newSample flag to true
void takeSample(void) {
  sample = (analogRead(0) - 512);
  newSample = true;
}

// Recursivve filter taking in the newest input, the previous output and the filter characteristic
// Normalizes the output value to be within the measurement´s max/min
int recursive(int sample, int prevSample, float a) {
  return int((sample + int(prevSample * a))*(1 - abs(a)));
}
