const int LDR = 0;
const int LM35 = 1;
const int R = 9;
const int G = 10;
const int B = 11;


float temp;
float analVoltage;
float ldr;
int i;

void setup() {
  Serial.begin(9600);
  
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  analVoltage = 0;
  for (i = 0; i < 100; i++) {
    analVoltage += analogRead(LM35);
    ldr += analogRead(LDR);
  }
  
  analVoltage = analVoltage / 100;
  analVoltage = (analVoltage / 1023) * 5000;
  temp = analVoltage / 10.0;
  ldr = ldr /100;
  /*
  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.print("LDR: ");
  Serial.println(ldr);*/
  lightRGB(ldr, temp);
  
  
}


void lightRGB(float l, float t) {
  int luxMul = 0;
  int r = 0;
  int g = 0;
  int b = 0;

  /*
  if (l < 400) {
      luxMul = int(l) % 255;
      //luxMul = ;
  }
  */
  luxMul = map((int) l, 0, 1023, 255, 0);
  
  if (t < 22) {
    b = 255;
  } else if (t < 30) {
    b = constrain(map(t, 22, 27, 64, 128), 0, 128);
    r = constrain(map(t, 25, 30, 64, 128), 128, 255);
    
  } else {
    r = 255;
  }
Serial.print("R: ");
Serial.print(r);
Serial.print(" B: ");
Serial.println(b);

  
  analogWrite(R, r);
  analogWrite(G, 255 - luxMul);
  analogWrite(B, b);
  
      
}

