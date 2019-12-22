#define         RL_VALUE                     (10)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
 
#define         LPG                      (0)         // Gas identity no.
#define         SMOKE                    (1)
#define         CO                       (2)
#define         CH4                      (3)
#define         Alcohol                  (4)
#define         H2                       (5)
#define         Propane                  (6)

#define         MQ2Sensor

float           LPGCurve[3]  = {2.3,0.20,-0.45};   //two points from LPG curve are taken point1:(200,1.6) point2(10000,0.26)
                                                    //take log of each point (lg200, lg 1.6)=(2.3,0.20)  (lg10000,lg0.26)=(4,-0.58)
                                                    //find the slope using these points. take point1 as reference   
                                                    //data format:{ x, y, slope};  

float           SmokeCurve[3] ={2.3,0.53,-0.43};    //two points from smoke curve are taken point1:(200,3.4) point2(10000,0.62) 
                                                    //take log of each point (lg200, lg3.4)=(2.3,0.53)  (lg10000,lg0.63)=(4,-0.20)
                                                    //find the slope using these points. take point1 as reference   
                                                    //data format:{ x, y, slope};
float           COCurve[3] = {2.4, 0.716, -0.317};

float           CH4Curve[3] = {2.3, 0.47, -0.394};
float           AlcoholCurve[3] = {2.3, 0.255, -0.293};
float           H2Curve[3] ={2.3, 0.34, -0.565};
float           PropaneCurve[3] = {2.3, 0.230, -0.460};

                                                                              
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
 
 
 int  GetPercentage(float rs_ro_ratio, float *pcurve);
 int  GetGasPercentage(float rs_ro_ratio, int gas_id);
 float ReadSensor();
 float ResistanceCalculation(int raw_adc);
 float SensorCalibration();

#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial s(5,6);
 
void setup() {
  //s.begin(9600);
  Serial.begin(9600);
 // Serial1.begin(9600);
  Serial.println("Calibrating...\n\r");                
  Ro = 10;//SensorCalibration();                       //Please make sure the sensor is in clean air 
                                                  //when you perform the calibration                    
  Serial.println("Calibration is done...\n\r"); 
  Serial.print("Ro=  ");
  Serial.print(Ro);
  Serial.println("kohm");
  Serial.print("\n\r");

}
 
void loop() {
 DynamicJsonDocument root(1000);

  root["LPG"] = GetGasPercentage(ReadSensor()/Ro,LPG);
  root["Smoke"] = GetGasPercentage(ReadSensor()/Ro,SMOKE);
  root["CO"] = GetGasPercentage(ReadSensor()/Ro,CO);
  root["CH4"] = GetGasPercentage(ReadSensor()/Ro,CH4);
  root["Alcohol"] = GetGasPercentage(ReadSensor()/Ro,Alcohol);
  root["H2"] = GetGasPercentage(ReadSensor()/Ro,H2);
  root["Propane"] = GetGasPercentage(ReadSensor()/Ro,Propane);
  serializeJson(root,Serial);
 // if(s.available()>0){
   // serializeJson(root, s);
 // }
}


float ResistanceCalculation(int raw_adc)
{                                                         // sensor and load resistor forms a voltage divider. so using analog value and load value 
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));     // we will find sensor resistor.
}
 


float SensorCalibration()
{
  int i;                                   // This function assumes that sensor is in clean air.
  float val=0;
 
  for (i=0;i<50;i++) {                   //take multiple samples and calculate the average value
    
    val += ResistanceCalculation(analogRead(A0));
    delay(500);
  }
  val = val/50;                  
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
 
  return val; 
}

 
float ReadSensor()
{
  int i;
  float rs=0;
 
  for (i=0;i<5;i++) {                                 // take multiple readings and average it.
    rs += ResistanceCalculation(analogRead(A0));   // rs changes according to gas concentration.
    delay(50);
  }
  
  rs = rs/5;
 
  return rs;  
}
 

int GetGasPercentage(float rs_ro_ratio, int gas_id)
{
//  if ( gas_id == LPG ) {
//     return GetPercentage(rs_ro_ratio,LPGCurve);
//   
//  } else if ( gas_id == SMOKE ) {
//     return GetPercentage(rs_ro_ratio,SmokeCurve);
//  }  else if(gas_id ==   
// 

    switch(gas_id){
        case LPG:
          return GetPercentage(rs_ro_ratio,LPGCurve);
          break;
        case SMOKE:
          return GetPercentage(rs_ro_ratio,SmokeCurve);
          break;
        case CO:
          return GetPercentage(rs_ro_ratio,COCurve);
        case CH4:
          return GetPercentage(rs_ro_ratio, CH4Curve);
          break;
        case Alcohol:
          return GetPercentage(rs_ro_ratio,AlcoholCurve);
          break;
        case H2:
          return GetPercentage(rs_ro_ratio,H2Curve);
          break;
        case Propane:
          return GetPercentage(rs_ro_ratio,PropaneCurve);
          break;
        default:
          break;
       }
  return 0;
}
 
 
int  GetPercentage(float rs_ro_ratio, float *curve)
{                                                                          //Using slope,ratio(y2) and another point(x1,y1) on line we will find  
  return (pow(10,( ((log(rs_ro_ratio)-curve[1])/curve[2]) + curve[0])));   // gas concentration(x2) using x2 = [((y2-y1)/slope)+x1]
                                                                          // as in curves are on logarithmic coordinate, power of 10 is taken to convert result to non-logarithmic. 
}
