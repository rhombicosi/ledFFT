/*
fft_adc_serial.pde
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            6
#define NUMPIXELS      64

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int delayval = 500; // delay for half a second
int sum[8];

void setup() {
  
  pixels.begin(); 
  pixels.setBrightness(8);
  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    //Serial.println("start");
    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      //Serial.println(fft_log_out[i]); // send out the data
    }

    for(int k = 0; k <8;k++){
      for (int kk = 0; kk<16;kk++){
          sum[k] += fft_log_out[kk+8*k];
        }
        sum[k]=map(sum[k]/16,0,128,0,7);
        Serial.println(sum[k]);
      }
    

  /*  for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,fft_log_out[i])); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(delayval); // Delay for a period of time (in milliseconds).

  }
*/
    
    for(int i = 0; i < 8; i++) {
      for(int j = 0; j < sum[i]; j++){
           pixels.setPixelColor(i*8+j, pixels.Color(map(i,0,7,0,255),map(i,0,7,0,255),150));
           pixels.show(); 
           //delay(delayval);
        }

      for(int jj=sum[i]; jj < 8; jj++){
           pixels.setPixelColor(i*8+jj, pixels.Color(0,0,0));
           pixels.show(); 
           //delay(delayval);
        }
      }

  
  
}
