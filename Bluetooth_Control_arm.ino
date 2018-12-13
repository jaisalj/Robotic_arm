#include<SoftwareSerial.h>

#include <VarSpeedServo.h> 
SoftwareSerial EEBlue(10, 11);
VarSpeedServo servo_0;
VarSpeedServo servo_1;
VarSpeedServo servo_2;
VarSpeedServo servo_3;
const int leftForward = 3;
const int leftBackward = 5;
const int rightForward = 7;
const int rightBackward = 9;
int val,z[4];
char x;
int mn;int va=0,j,k,rep=0;
float i;
String a;
int  pre1=90,pre2=90,pre3=90,pre4=90;
    
int count0, arrayStep, arrayMax, countverz, Taster, stepsMax, steps, time = 1000, del = 1000, temp;
// arraystep = memory what pos in the array
// arrayMax = max steps we safed to array
// countverz = seems to be something to calculate the delay between complete moves
// Taster = Button
// stepsMax = longest way a servo have to travel
// steps = single steps for a move between stored positions
unsigned int  verz = 0;

long previousMillis1 = 0;
long previousMillis2 = 0;
long previousMillis3 = 0;
long previousMillis4 = 0;
long previousMicros = 0;
unsigned long currentMillis = millis();
unsigned long currentMicros = micros();

// arrays
int Delay[7] = {0,0,1,3,15,60,300}; // array to map gripper pot to delay in seconds
int SensVal[4]; // sensor value
float dif[4], ist[4], sol[4],  dir[4]; // difference between stored position and momentary position
int joint0[180];// array for servo(s)
int joint1[180];
int joint2[180];
int joint3[180];
int top = 179; // we should not write over the end from a array
// status 
boolean playmode = false, Step = false,butt=false;

void setup()
{
 //inMode(4, INPUT);  // sets the digital pin 4 as input
  pinMode(6, INPUT);
  pinMode(13, OUTPUT);  // sets the digital pin 13 as outtput
  //digitalWrite(13, HIGH);   // sets the LED on
  servo_0.attach(31); // attaches the servo
  servo_1.attach(33);
  servo_2.attach(34);
  servo_3.attach(36);
  pinMode(leftForward , OUTPUT);
  pinMode(leftBackward , OUTPUT);
  pinMode(rightForward , OUTPUT);
  pinMode(rightBackward , OUTPUT);
  EEBlue.begin(38400);
  Serial.begin(38400); // Baudrate
       
  //delay(1000);
  //digitalWrite(13, LOW);
}

void loop() 
{
  currentMillis = millis(); 
  currentMicros = micros();
  
  // read the button  
  Button();
  
  if(!playmode)
  {        
    if(currentMillis - previousMillis1 > 25) // 25miliseconds until next manual mode update
    {
      if (arrayStep < top) 
      {
        previousMillis1 = currentMillis; //reset
        readPot(); // get the value from potentiometers
        mapping(); // map to milliseconds for servos
        move_servo(); // setz newservo position
        //record();   
      } // end counter < max
    } // end step check
  } // ende manualy move
   
  else if(playmode) // play
  {
    if (Step) // next step read from array
    {
      digitalWrite(13, HIGH); //LED
      if (arrayStep < arrayMax) // we not reach the end from stored data
      {
        arrayStep += 1; // next array pos
        Read(); // from the arrays
        calculate(); // find biggest travel distance and calculate the other 3 servos (the have to do smaler steps to be finished at same time!)
        Step = 0;
        digitalWrite(13, LOW);  
      }
      else // array read finished > start over
      {
        arrayStep = 0; // 
        calc_pause(); // delay between moves read from potentiometer
        countverz = 0; // used for the delay
        /*while(countverz < verz) // verz = time getting from calc_pause();
        { // here we do loop and wait until next start over
          countverz += 1;
          calc_pause();
          digitalWrite(13, HIGH); delay(25);   
          digitalWrite(13, LOW); delay(975); 
        }*/
      }
      //Serial.println(arrayStep);
    }
    else // do the servos!
    {
      if (currentMicros - previousMicros > time) // here we do a single micro step
      { // 
        previousMicros = currentMicros;
        play_servo(); 
      }
    }
  }// ende playmode

// ---------------------------------------------------------------------------------Hardware pause switch PIN 6
    /*while (digitalRead(4) == true)
      { 
        digitalWrite(13, HIGH); delay(500);   
        digitalWrite(13, LOW); delay(500);
      }*/
// ---------------------------------------------------------------------------------- Textout serial
    // serial ausgabe 1 sek
    /*if(currentMillis - previousMillis2 > 5000)
    { 
      previousMillis2 = currentMillis;
      /*count0 = 0;
      while(count0 < 4)
      {
        int val = SensVal[count0];
      // val = map(val, 142, 888, 0, 180);
        Serial.println(val);
        //Serial.println("test");
        count0 += 1;
      }
      Serial.println(playmode); 
      Serial.println(arrayStep);    
      Serial.println(arrayMax);    
      Serial.println(" ");    
    }*/
}

// ---------------------------------------------------------------------------------------- sub routinen
void calc_pause() // read pot and map to usable delay time after a complete move is done
{
    readPot();
    temp = SensVal[3];
    if (temp < 0) temp = 0;
    temp = map(temp, 0, 680, 0 ,6); 
    verz = Delay[temp]; // verz = delay in second
    /*Serial.print(temp);
          Serial.print(" ");
          Serial.print(verz);
          Serial.print(" ");
          Serial.println(countverz);*/
}

void readPot() // read analog inputs and add some offsets (mechanical corrections)
{va=0;
 
  // Feed any data from bluetooth to Terminal.
  if (Serial.available())
       a=Serial.readString();
  else if(EEBlue.available())
       a=EEBlue.readString();      
       Serial.println(a);
       //fun(a);
       if(a=="f")
        forward();
      else if(a=="b")
          back( );
      else if (a=="l")
            left();
      else if (a=="r")
             right();
      else if(a=="s")
              Stop();
      else{
       va=a.toInt();
       rep=va;
       mn=va%10;
       va=va/10;
       va=map(va,0,180,0,1023);
      if(mn==1)//motor 1
      {SensVal[0]=va;
       SensVal[0]+=-10;
      }
      if(mn==2)//motor 2
      {SensVal[1]=va;
       SensVal[1]+=280;
      }
      if(mn==3)//motor 2
      {SensVal[2]=va;
       SensVal[2]+=-50;
      } 
      if(mn==4)//motor 2
      {SensVal[3]=va;
       SensVal[3]+=0;
      }
      if(mn==5 || rep==6)//button 
      {butt=true;
        }
       
      }
   //SensVal[0] = analogRead(sensorPin0); SensVal[0] += -10; // rotate
   //SensVal[1] = analogRead(sensorPin1); SensVal[1] += 280; // Shoulder
   //SensVal[2] = analogRead(sensorPin2); SensVal[2] += -50; // hand
   //SensVal[3] = analogRead(sensorPin3); // SensVal[3] += 0;// gripper
   //Serial.print(SensVal[2]);Serial.print(" "); // CHECK
}
void mapping() // we need microsecond for the servos instead potentiometer values
{
  ist[0] = map(SensVal[0], 150, 900, 600, 2400);z[0]=ist[0];
  ist[1] = map(SensVal[1], 1000, 100, 550, 2400);z[1]=ist[1];
  ist[2] = map(SensVal[2], 120, 860, 400, 2500);z[2]=ist[2];
  ist[3] = map(SensVal[3], 1023, 0, 500, 2500);z[3]=ist[3];
  //Serial.println(ist[2]); // CHECK
}
void record()
{
    joint0[arrayStep] = ist[0]; // write positions in servo array
    joint1[arrayStep] = ist[1];
    joint2[arrayStep] = ist[2];
    joint3[arrayStep] = ist[3];
}
void Read()
{
    sol[0] = joint0[arrayStep]; // read from the array
    sol[1] = joint1[arrayStep];
    sol[2] = joint2[arrayStep];
    sol[3] = joint3[arrayStep];
}
void move_servo()
{ z[0]=map(z[0],600,2400,0,180);
z[1]=map(z[1],550,2400,0,180);
z[2]=map(z[2],400,2500,0,180);
z[3]=map(z[3],500,2500,0,180);
  servo_0.write(z[3],30); // send milissecond values to servos
  servo_1.write(z[2],30); 
  servo_2.write(z[0],30); 
  servo_3.write(z[1],30); 
}

// ------------------------------------------------------------ single steps calculating
void calculate()
{
      // travel distance for each servo
      dif[0] = abs(ist[0]-sol[0]);
      dif[1] = abs(ist[1]-sol[1]);
      dif[2] = abs(ist[2]-sol[2]);
      dif[3] = abs(ist[3]-sol[3]);

      // biggest travel way from all 4 servos
      stepsMax = max(dif[0],dif[1]);
      stepsMax = max(stepsMax,dif[2]);
      stepsMax = max(stepsMax,dif[3]);
      // stepsMax is the biggest distance a servo have to do beween momentary position and new pos read from the array
      
      //Serial.println(stepsMax); 
      
      if (stepsMax < 500) // del(ay) between a single step is bigger is move is smaler. just looks cool
        del = 1200;
      else
        del = 600;
      
       // calculating single (micro) step for each servo
       // need that to do move all servos in a loop (stepsMax times done) with different values.
       // This makes all servos have done the traveling distance at same time
      if (sol[0] < ist[0]) dir[0] = 0-dif[0]/stepsMax; else dir[0] = dif[0]/stepsMax;
      if (sol[1] < ist[1]) dir[1] = 0-dif[1]/stepsMax; else dir[1] = dif[1]/stepsMax;
      if (sol[2] < ist[2]) dir[2] = 0-dif[2]/stepsMax; else dir[2] = dif[2]/stepsMax;
      if (sol[3] < ist[3]) dir[3] = 0-dif[3]/stepsMax; else dir[3] = dif[3]/stepsMax;
        //Serial.println(dir4); 

}
void play_servo()
{
    steps += 1;
    if (steps < stepsMax) // sure we not reach the end from a move
    {
      //time = del*5;// anfahr rampe
      if(steps == 20) time = del*4;         // ramp up 
      else if(steps == 40) time = del*3;    // time is the delay in microsecns we wait in the mainloop until
      else if(steps == 80) time = del*2;    // a micro step will be done
      else if(steps == 100) time = del-1;    // cannot explain here is not del*1
      
      if(steps == stepsMax-200) time = del*2;        // stop ramp down (200 microsteps before end time will be increased
      else if(steps == stepsMax-80) time = del*3;
      else if(steps == stepsMax-40) time = del*4;
      else if(steps == stepsMax-20) time = del*5;
      
      ist[0] += dir[0]; // set new pos
      ist[1] += dir[1];
      ist[2] += dir[2];
      ist[3] += dir[3];

      servo_0.writeMicroseconds(ist[3]); 
      servo_1.writeMicroseconds(ist[2]); 
      servo_2.writeMicroseconds(ist[0]); 
      servo_3.writeMicroseconds(ist[1]); 
    }
    else
    {
      Step = 1; 
      steps = 0; 
    }
}

void data_out() // just to write the recorded data to serial
{
  int i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint0[i]); Serial.print(", ");
  }
  Serial.println("Joint0");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint1[i]); Serial.print(", ");
  }
  Serial.println("Joint1");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint2[i]); Serial.print(", ");
  }
  Serial.println("Joint2");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint3[i]); Serial.print(", ");
  }
  Serial.println("Joint3");
}

void Button() // check buttons for single and doubleclick
{
  
    if (butt == true) 
    {
     /* if (Taster == 0)
      {
        Taster = 1;
        previousMillis3 = currentMillis;
        //Serial.print("Status Record "); Serial.println(Taster); 
      }*/
      Taster++;
      if ((Taster == 1) && (rep==6))
      {
        Taster = 2;
        //Serial.println(Taster); 
      }
      else if ((Taster == 2) && (currentMillis - previousMillis3 < 500))
      {
        Taster = 3;
        Serial.println(Taster); 
      }
    }
  
    
   if ((Taster == 1) && mn==5) // write to array
    {
      arrayStep += 1;
      arrayMax = arrayStep;
      record();
      Taster = 0;
      playmode = false;
      Serial.print("Record Step: "); Serial.println(arrayStep);
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
    }
    else if (Taster == 2)
    {
      arrayStep = 0;
      playmode = true;
      Taster = 0;
      Step = 1;
      Serial.println("playmode ");
  //    data_out();
      delay(250);   
      digitalWrite(13, LOW);    
    }
    if (Taster == 3)
    {
      // ++ arrayStep
      // playmode = 1;
      Taster = 0;
      Serial.println("Clear ");
    }
    if (currentMillis - previousMillis3 > 2000) // button Status clear
    {
      Taster = 0;
      
    }
butt=false;
rep=0;
mn=0;
}
void forward(){
analogWrite(leftForward , 0);
  analogWrite(leftBackward , 150 );
  analogWrite(rightForward , 0);
  analogWrite(rightBackward , 150);}
void right(){
analogWrite(leftForward , 0);
 analogWrite(leftBackward , 120);
  analogWrite(rightForward , 0);
  analogWrite(rightBackward , 0);}
 void left(){
analogWrite(leftForward , 0);
  analogWrite(leftBackward , 0);
  analogWrite(rightForward , 0);
  analogWrite(rightBackward , 120);}
 void back(){
analogWrite(leftForward , 150);
  analogWrite(leftBackward , 0);
  analogWrite(rightForward , 150);
  analogWrite(rightBackward , 0);}
  void Stop()
  {digitalWrite(leftForward , LOW);
  digitalWrite(leftBackward , LOW);
  digitalWrite(rightForward , LOW);
  digitalWrite(rightBackward , LOW);
}
