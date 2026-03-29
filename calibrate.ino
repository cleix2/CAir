


#include <MQ7.h>
#include <GP2Y1010AU0F.h>

  #define MQ7pin 34           //MQ7
  #define MQ7volage 5.0
  #define GP2Ypin 32         // GP2Y1010AU0F
  #define GP2Ydigital 14

  float MQ7rl 1000.0;   // Load resistor in ohms (1k)
  float MQ7vc 5.0;       // Supply voltage

  float GP2Yvc 5.0; 

  int MQ7count = 0; float CLEANair 27.0; 

  MQ7 mq7(MQ7pin, MQ7volage);
  GP2Y1010AU0F dustSensor(GP2Ydigital, GP2Ypin);

unsigned long TIMEstart;
bool switch5V = true;
bool MQ7state = false;    const unsigned long MQ7pretime = 180000; //3 mins
bool GP2Ystate = false;  const unsigned long GP2Ypretime = 280; // 28 Microseconds

float MQ7out = 0, GP2Yout = 0;
float MQ7sum = 0, GP2Ysum = 0;
int MQ7count = 0, GP2Ycount = 0;

void setup() {
  Serial.begin(115200);

  pinMode(MQ7heater, OUTPUT);
     mq7.setR0(MQ7ro);

  MQ7ro = MQ7calibrate();
  
  Serial.print("R0 = ");
  Serial.println(MQ7ro);

  dustSensor.begin();
  pinMode(GP2Ydigital, OUTPUT);

    TIMEstart = millis(); // Capture start time
}


void loop() {
  unsigned long TIMEcurrent = millis();

//For MQ7
    static unsigned long MQ7last = 0;
    if (switch5V) {// --- PHASE 1: 5V Cleaning (60 Seconds) ---
      analogWrite(MQ7pin, 255);
      if (TIMEcurrent - MQ7last >= 60000) {
        switch5V = false; // Switch to Low Heat
        MQ7last = TIMEcurrent;
        Serial.println("Switching to 1.4V (Measurement phase)...");
      }
    }
    else {// --- PHASE 2: 1.4V Measuring (90 Seconds) ---
      analogWrite(MQ7pin, 71);
      if (TIMEcurrent - MQ7last >= 85000) {
        if (TIMEcurrent - MQ7last >= 85000 && TIMEcurrent - MQ7last < 86000){
          MQ7calibrate();
        }
        
        if (now - lastTime >= 90000) { // End of 90s
            switch5V = true;
            MQ7last = TIMEcurrent;
            MQ7count++;
          
            Serial.print("Cycle ");
            Serial.print(MQ7count);
            Serial.println(" complete. Restarting Cleaning Phase...");
        }
        Serial.println("Switching to 5V (Cleaning phase)...");
      }
    }
  
//For GP2Y
  digitalWrite(GP2Ydigital, LOW);
  delayMicroseconds(280);
  
  // Read the raw voltage
  float GP2Yvo = analogRead(GP2Ypin) * (5.0 / 1024.0);
  
  delayMicroseconds(40);
    digitalWrite(GP2Ydigital, HIGH);
  delay(100); // Faster sampling for calibration

  // Logic: The lowest voltage ever recorded in clean air is your Voc
  if (GP2Yvo < GP2Yvc && Vo > 0.1) { 
    GP2Yvc = GP2Yvo;
  }

  Serial.print("Current Vo: "); Serial.print(Vo);
  Serial.print("V | Lowest Vo (Potential Voc): ");
  Serial.println(GP2Yvo);
  delay(1000);

}

void MQ7calibrate() {
  float RStot = 0;

  Serial.println("Sampling for R0...");
  for (int i = 0; i < 20; i++) {
    float RSval = RScalc();
    if (RSval > 0) RStot += RSval;
    delay(50);
  }

  float RSavg = RStot / 20;
  float R0final = RSavg / CLEANair;

  Serial.print("Current Rs: ");
  Serial.print(RSavg);
  Serial.print(" | CALIBRATED R0: ");
  Serial.println(R0final);
  Serial.println("---------------------------------------");
}

  float RScalc() {
    int ADCval = analogRead(MQ7pin);
    float VRLval = ADCval * (MQ7vc / 4095.0);
    if (VRLval <= 0.01) return -1; // Avoid division by zero
    
    float Rs = ((MQ7vc / VRLval) - 1.0) * MQ7rl;
    return Rs;
  }

