

#include <Wire.h>
int channel = 0;
int capdac =2;
double  dist = 0;
double area = 0.0157;
const double avg = 15;
const double ofs = 0.4;
double dev = 5;
double fct = 0.5;

int *fct_ptr; 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  Wire.begin();
  delay(1000);

  Configure_meas();
  Trigger_Meas();

}
bool autoCal(double target,double deviation)
{
 double current = measure();
 double mod = current;
 while((mod>(target+deviation)||(mod<(target-deviation)) //current reading outside acceptable deviation
 {
 if(mod>(10*target)) //if current reading is more than 10x the target range, increase the factor to reduce the range
 {
  *fct +=.01;
  mod = measure();
 }
 if(mod>(10*target)) //if current reading is less than  the target range, decrease the factor to reduce the range
 {
  *fct -=.01;
  mod = measure();
 }
}
return true;
}
void printDouble( double val, unsigned int precision) {
  // prints val with number of decimal places determine by precision
  // NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
  // example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

  Serial.print (int(val));  //prints the int part
  Serial.print("."); // print the decimal point
  unsigned int frac;
  if (val >= 0)
    frac = (val - int(val)) * precision;
  else
    frac = (int(val) - val ) * precision;
  int frac1 = frac;
  while ( frac1 /= 10 )
    precision /= 10;
  precision /= 10;
  while (  precision /= 10)
    Serial.print("0");
  
  Serial.print(frac, DEC);
  Serial.println(" cm");

}
double measure()
{

  double sum = 0;
 // double filtered_sum = 0;
 // double sec_filter = 1;
  //double target = 50;
   // for(int j = 0; j<sec_filter;j++)
 // {
 for (int i = 0; i < avg; i++)
  {
    if (Check_read())
    {
      sum += read_val();
      delayMicroseconds(1);
    }
  }
  sum = sum/avg;
 // filtered_sum +=sum;
 
  // Serial.println("distance: ");
  //Serial.print(8.854e-12);
  //Serial.println(sum);;
  
  // printDouble(sum, 1000);
  //Serial.print( sum/avg);
 // Serial.println(" cm");
 // Serial.print(" pf ");
//}
//sum = filtered_sum/sec_filter;
 
  //sum = (sum*1.8)-.435;
 // sum-=0.435;
//  sum = (sum)*1E-12;
  dist = ((8.854) * 1.0005 * area)/((sum*fct)-ofs) *10 ; //distance in cm
//  if(dist<target)
//  {
//    
//    while(true)
//    {
//      Serial.println("ERROR ERROR ERROR");
//    }
//  }
  return dist;
}

void Configure_meas()
{

  //configure measurment
  uint16_t configuration_data = 0;
  configuration_data |= ((uint16_t)channel) << 13; //CHA
  configuration_data |= ((uint16_t)0x04) << 10; //CHB disable / CAPDAC enable
  configuration_data |= ((uint16_t)capdac) << 5; //CAPDAC value

  Wire.beginTransmission(80);
  Wire.write(0x08); //send address
  Wire.write( (uint8_t) (configuration_data >> 8));
  Wire.write( (uint8_t) configuration_data);
  Wire.endTransmission();
  //Serial.println("Config Done");
}

void Trigger_Meas()
{
  //trigger measurment
  int measurement = 0;
  uint16_t trigger_data = 0;
  uint16_t rate = 0x01;
  trigger_data = ((uint16_t)rate) << 10; // sample rate
  trigger_data |= 1 << 8; //repeat disabled
  trigger_data |= (1 << (7 - measurement));

  Wire.beginTransmission(80);
  Wire.write(0x0C); //send address
  Wire.write( (uint8_t) (trigger_data >> 8));
  Wire.write( (uint8_t) trigger_data);
  Wire.endTransmission();
  //Serial.println("Trigger Done");
}

int Check_read()
{
  // check conversion is done
  Wire.beginTransmission(80);
  Wire.write(0x0C);
  Wire.endTransmission();
  //delay(40);
  uint16_t value = 0;
  while (value == 1416)
  {
    Wire.beginTransmission(80);
    Wire.requestFrom(80, (uint8_t)2);
    value = Wire.read();
    value <<= 8;
    value |= Wire.read();
    Wire.endTransmission();
    if (value != 0 && value != 65535)
    {
      Serial.println(value);
    }
  }
  return 1;
}

float read_val()
{
  // read value
  Wire.beginTransmission(80);
  Wire.write(0x00);
  Wire.endTransmission();
  //delay(40);
  uint16_t value_msb;
  Wire.beginTransmission(80);
  Wire.requestFrom(80, (uint8_t)2);
  value_msb = Wire.read();
  value_msb <<= 8;
  value_msb |= Wire.read();
  Wire.endTransmission();
  //Serial.println(value_msb);

  Wire.beginTransmission(80);
  Wire.write(0x01);
  Wire.endTransmission();
  //delay(40);
  uint16_t value_lsb;
  Wire.beginTransmission(80);
  Wire.requestFrom(80, (uint8_t)2);
  value_lsb = Wire.read();
  value_lsb <<= 8;
  value_lsb |= Wire.read();
  Wire.endTransmission();
  //Serial.println(value_lsb);

  

  //Lib Conversion
  int32_t capacitance = ((int32_t)457) * ((int32_t)value_msb); //in attofarads
  double cap_double = capacitance / 1000; //femto farads
  double cap_double_fm = cap_double / 1000; //in picofarads

  //Serial.print("Capacitance=");
  //Serial.print(cap_double_fm);
  //Serial.println(" pf");
  return cap_double_fm;
}


void loop() {
  unsigned long StartTime = millis();
 
 dist = measure();
    unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  
//Serial.println(ElapsedTime);

  printDouble(dist, 100000);

}
