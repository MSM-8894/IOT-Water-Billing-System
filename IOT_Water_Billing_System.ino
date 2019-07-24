/*
 * 
 http://www.iothitechnology.co.in/waterusage.aspx
 http://www.iothitechnology.co.in/services/Update_RDNO.asmx/SetWaterLevel?CID=HI44&S1=1.5&S2=52
 login 8850495043   pass 12345
*/

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

volatile int  flow_frequency;  // Measures flow meter pulses
unsigned int  l_hour_sensor_1;          // Calculated litres/hour   

unsigned char flowmeter = 2;  
unsigned long currentTime;
unsigned long cloopTime;

String Mobile1 = "+919920724217";

int Buzzer =3;
int Relay = 13;

float Recharge=0;
float Water_balance=0;// 30rs=1 liter 
int cost=30;
int a,b,c,d;

   
LiquidCrystal lcd(5, 6, 7, 8, 9, 10);

SoftwareSerial serial_connection(A4,A3);//12
char SM_ARU;

char sms_data[25];
int temp=0,u=0,A=0;
char inChar;
int  sms_received_bit=0;
int sms_send_bit=0;

void flow ()                  // Interruot function
{ 
  if(Recharge>0)
  {
   flow_frequency++;
   if(flow_frequency>=50)
   {
     flow_frequency=0;
     Recharge--;
     Water_balance=Recharge/cost;
   }
  }
  if(Recharge==0)
    digitalWrite(Relay, LOW);
} 

void setup()
{ 

  Serial.begin(9600);  
  Serial.println("Connecting...");
  serial_connection.begin(9600);
  pinMode(Buzzer, OUTPUT);
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, LOW);
  digitalWrite(Buzzer, HIGH);

  lcd.begin(6, 2);
  lcd.setCursor(0, 0);
  //Serial.println("SM_ARU@3327"); 
  lcd.print(" Water Billing");  
  lcd.setCursor(0, 1);
  lcd.print("  System"); 
  delay(5000);
  serial_connection.println("AT");
  delay(100);
  serial_connection.println("AT+CNMI=1,2,0,0,0");
  delay(100);
  serial_connection.println("AT&W");
  delay(100);
  serial_connection.println("AT+CMGF=1");
  lcd.clear();
  init_GPRS();
   pinMode(flowmeter, INPUT);
   attachInterrupt(0, flow, RISING);        
   sei();                           
 //  currentTime = millis();
  // cloopTime = currentTime;
} 

void loop ()    
{
 // while(1)
//SMS_Received();

if(sms_received_bit==1)
{
    sms_received_bit=0; 
    lcd.clear();
    lcd.print("SMS Received...     ");
    digitalWrite(Buzzer, LOW);
    delay(2000);
    digitalWrite(Buzzer, HIGH);
    lcd.clear();
    lcd.print("Recharge: ");
    
    a=sms_data[0]-0x30;
    b=sms_data[1]-0x30;
    c=sms_data[2]-0x30;
    d=sms_data[3]-0x30;
    Recharge=(a*1000+b*100+c*10+d);
    Water_balance=Recharge/cost;
    
    lcd.print(Recharge);
    lcd.print("Rs");   
     
    lcd.setCursor(0, 1); 
    lcd.print("Water: ");    
    lcd.print(Water_balance);
    lcd.print("Liter");
    delay(2000);sms_send_bit=0;
    web_update();sms_send();
}
  if(Recharge==0)
  {
    lcd.clear();
    lcd.print("Balance Amount 0");
    lcd.setCursor(0, 1); 
    lcd.print("Please Recharge");
    digitalWrite(Relay, LOW); 
    delay(100);
    if(sms_send_bit==0)
    {web_update();sms_send();sms_send_bit=1;}
  }
  
   if(Recharge<20 && Recharge>0)
  {
    lcd.clear();
    lcd.print("Balance is Low");
    lcd.setCursor(0, 1); 
    lcd.print("Please Recharge");
    digitalWrite(Buzzer, LOW);
    delay(100);
    digitalWrite(Buzzer, HIGH);
    delay(100);
  }
  if(Recharge>0) 
  {
    lcd.clear();
    lcd.print("Balance ");
    lcd.print(" Pump On ");
    digitalWrite(Relay, HIGH); 
    lcd.setCursor(0, 1); 
    lcd.print(Recharge);
    lcd.print("Rs  ");
    lcd.setCursor(9, 1); 
    lcd.print(Water_balance);
    lcd.print("L");
    delay(1000);
  }
}

void init_GPRS()
{
  
    lcd.clear();
    lcd.print("Init GPRS... ");
 serial_connection.println("AT+CGATT=1");
 delay(200);
 printSerialData();
 
 serial_connection.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR,connection type is GPRS
 delay(1000);
 printSerialData();
 
 serial_connection.println("AT+SAPBR=3,1,\"APN\",\"\"");//setting the APN,2nd parameter empty works for all networks 
 delay(5000);
 printSerialData();
 
 serial_connection.println();
 serial_connection.println("AT+SAPBR=1,1");
 delay(5000);
 serial_connection.println("AT+SAPBR=2,1");  /* Query the GPRS context */
 delay(5000);
 printSerialData();  
  
}


void printSerialData()
{
// while(serial_connection.available()!=0)
// Serial.write(serial_connection.read());
}
void web_update()
{ 
  lcd.clear();
  lcd.print("Update On Web    ");
      
serial_connection.println("AT+HTTPINIT"); //init the HTTP request
 delay(3000);
 serial_connection.println("AT+HTTPPARA=\"CID\",1"); //init the HTTP request
 delay(3000); 
 printSerialData();
 
 //http://www.iothitechnology.co.in/services/Update_RDNO.asmx/SetWaterLevel?CID=HI44&S1=1.5&S2=52
 serial_connection.print("AT+HTTPPARA=\"URL\",\"www.iothitechnology.co.in/services/");// setting the httppara, 
 delay(500); printSerialData();
 serial_connection.print("Update_RDNO.asmx/SetWaterLevel?CID=HI44");// setting the httppara, 
 delay(500);
 serial_connection.print("&S1=");
 serial_connection.print(Water_balance);
 serial_connection.print("&S2=");
 serial_connection.print(Recharge);

 delay(500);
 delay(10);
    
 serial_connection.println("\"");  
 delay(2000);
 printSerialData();
 serial_connection.println("AT+HTTPACTION=0");//submit the GET request 
 printSerialData();
 //serial_connection.println("AT+HTTPREAD=0,20");// read the data from the website you access
 //serial_connection.println("AT+HTTPREAD");// read the data from the website you access
 
 delay(3000);
 printSerialData();
 
 //serial_connection.println("");
 delay(1000);
 serial_connection.println("AT+HTTPTERM");// terminate HTTP service
 printSerialData();
  delay(1000);

  serial_connection.println("");
 delay(5000);

}



void sms_send()
{
  lcd.clear();
  lcd.print("Sending SMS    ");
  serial_connection.println("AT&W");
  delay(1000);
  serial_connection.println("AT+CMGF=1");
  delay(1000); 
  serial_connection.println("AT+CMGS=\""+Mobile1+"\"\r");
  lcd.setCursor(0, 1);
  lcd.print(Mobile1);
  
  delay(1000);
  if(Recharge==0)
  {
     serial_connection.println("Please Recharge");
  }
  else
  {
    serial_connection.println("Recharge Done!!!");   
  }
  
  serial_connection.print("Balance Amount ");
  serial_connection.print(Recharge);
  serial_connection.println("Rs");
  
  serial_connection.print("Water ");
  serial_connection.print(Water_balance);
  serial_connection.println("Liter");
    
  delay(100);   
  serial_connection.println((char)26);
  delay(5000);
  
}  


void serialEvent()
{
  while (Serial.available()) 
  {
      SM_ARU = Serial.read();
      
      if (SM_ARU == '#') 
      {     
        Serial.println("SMS Received...     ");
        delay(10);
        while (Serial.available()) 
          {
             inChar=Serial.read();
            sms_data[u++]=inChar;
            if(inChar=='*')
              {
                sms_received_bit=1;
                inChar='S';
                delay(10);sms_data[u--]='\n';
                Serial.print(sms_data);
                //return;
              } 
          }
          
        }
  }
}
