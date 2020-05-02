/* Hshmat Sahak and Gabriel Luo
   TER4M1 Summative Project
   January 22, 2019
   Mr.Jay
*/

// Include necessary libraries and set up code
#include <Servo.h>

// Sensors
// definning Inputs for line tracking sensor
int LS = 2; // attach left sensor to 2
int RS = 3; // attach right sensor to 3

int IRfinderpin = A5;  // Infrared range finder pin
int touchsensorpin = 8; // attach touch sensor to pin 8

// Arm is controlled by several servo motors
Servo armbase;  // Controls arm base
Servo army; // Control vertical motion of arm
Servo armx; // Control horizontal motion of arm
Servo armclaw;  // Control claw
Servo IReyes; // Controls IR finder, used in obstacle detection

// Connections for arm
int arm_x = A1; // potentiometer controlling horizontal direction connected to analog pin 1
int arm_y = A2; // potentiometer controlling vertical direction connected to analog pin 2
int arm_down = 12;  // button on joystick, pressing moves arm down

boolean drawing = true; // tells robot whether to draw or not
//variables to make sure messages are displayed once
boolean flag = false;
boolean flag2 = false;
boolean flag3 = false;
boolean temp = false;

char notes[] = {'e', 'f', 'g', 'a', 'B', 'C'}; // notes for piano

//Standard PWM DC control
int E1 = 5;     //M1 Speed Control
int E2 = 6;     //M2 Speed Control
int M1 = 4;     //M1 Direction Control
int M2 = 7;     //M2 Direction Control

void setup() {
  Serial.begin(9600); // allow access to serial monitor
  Serial1.begin(9600); //allows access to the bluetooth serial monitor

  // Servos for arm control
  armbase.attach(9); // attach base to analog 0
  army.attach(10); // attach army to analog 1
  armx.attach(15); // attach armx to analog 3
  armclaw.attach(11); // attach claw to analog 3

  IReyes.attach (14); // attach IR eyes to pin 16

  // declare sensors as input
  pinMode(LS, INPUT); // set left tracking sensor as input
  pinMode(RS, INPUT); // set right tracking sensor as input
  pinMode(touchsensorpin, INPUT); // declare metal touch sensor as input
  pinMode(arm_down, INPUT_PULLUP);  // declare arm down button as input
  stop(); // stop motion
  armbase.write(90);  // set shaft to angle 90
  armx.write(20); // set arm x to angle 20
  army.write(150);  // set arm y to angle 150
  armclaw.write(0); // set arm claw to angle 0
  IReyes.write(90); // set servo for infrared finder to angle 90
}

// loop function commands car to visit and complete tasks in each station
void loop() {
  if (flag2 == false) { // if message hasn't been displayed
    Serial1.println("Press start to begin");  // display start message on the bluetooth serial monitor
    flag2 = true; // set boolean variable to true, message is not displayed again
  }
  if (Serial1.read() == 56) // if user presses start button on the bluetooth serial monitor
    temp = true;  // set boolean variable to true
  if (temp == true) // if boolean variable is true
    movebetweenstations();  // calls the movebetweenstations function
}

// Movement
void stop(void) // stops car
{
  digitalWrite(E1, LOW);
  digitalWrite(E2, LOW);
}
void advance (char a, char b) // moves car forward at a given speed
{
  analogWrite (E1, a);
  digitalWrite(M1, HIGH);
  analogWrite (E2, b);
  digitalWrite(M2, HIGH);
}
void back (char a, char b) // moves car backwards at a given speed
{
  analogWrite (E1, a);
  digitalWrite(M1, LOW);
  analogWrite (E2, b);
  digitalWrite(M2, LOW);
}
void turn_L (char a, char b) // turns car left at a given speed
{
  analogWrite (E1, a);
  digitalWrite(M1, LOW);
  analogWrite (E2, b);
  digitalWrite(M2, HIGH);
}
void turn_R (char a, char b) // turns car right at a given speed
{
  analogWrite (E1, a);
  digitalWrite(M1, HIGH);
  analogWrite (E2, b);
  digitalWrite(M2, LOW);
}

// Turn left or right depending on angle passed
void turn (int angle) {
  if (angle < 90) { // If angle is less than 90
    turn_R (160, 230);  // turn right
    delay ((long)(90 - angle) * 2800 / 360);  // completes one circle in 2800 milliseconds, so turn will take this much time
  }
  else if (angle > 90) {  // If angle is more than 90
    turn_L (240, 170); // turn left
    delay ((long)(angle - 90) * 2800 / 360); // completes one circle in 2800 milliseconds, so turn will take this much time
  }
}

// Determines distance to nearest object (uses infrared range finder)
int distance () {
  return (analogRead(IRfinderpin) / 10); // divide by 10 for convenience- it is better to work with smaller values (0-70)
}

// instructions for robot to follow line, this will be carried out as part of movebetweenstations method
void readLine() {
  if (!digitalRead (LS) && !digitalRead (RS)) { //if left and right sensor are both black
    stop ();  // end of station, stop
  }
  else if (!digitalRead (LS) && digitalRead (RS)) { //if only left sensor reads black
    turn (88); // adjust slightly to the left to get car back to its original trajectory
  }
  else if (digitalRead (LS) && !digitalRead (RS)) {// if only right sensor reads black
    turn (93);  // adjust slightly to the right to get car back to its original trajectory
  }
  else //otherwise
    advance(200, 200); // both sensors read white, keep moving forward
}

// function returns a boolean variable that determines whether the car has reached its final destination
boolean stationReached () {
  if (!digitalRead (LS) && !digitalRead (RS)) // if both sensors read black
    return true;  // next station has been reached, return true
  return false; // otherwise, return false
}

// instructions to move between stations
void movebetweenstations () {
  do { // instructions to move between stations
    advance(200, 200);  // move forward
    readLine ();  // follow line to get from one station to next
  }
  while (!stationReached());// stop moving when line ends
  stop ();  // robot is at the next station!
  if (flag3 == false) { // if message hasn't been displayed
    Serial1.print("Please indicate which station the robot is at:");  // display message on the bluetooth serial monitor - ask user to indicate robot station
    flag3 = true; // set as true, ensures message is displayed once
  }
  int command = Serial1.read(); // read user input
  if (command == 48)  // if user presses the sort button on the bluetooth serial monitor
    sort(); // user selected sort station, execute sort station commands
  else if (command == 49) // if user presses the piano button on the bluetooth serial monitor
    pianostation(); // user selected piano station, play piano
  else if (command == 50) // if user presses the drawing button on the bluetooth serial monitor
    drawingstation(); // allow for joystick controlled drawing
}

// place properly will place object at the right place depending on boolean variable passed to it
void placeproperly (bool reading) {
  int delaytime = 500;  // declare delaytime as 500 milliseconds
  back(210, 255); // move back
  delay(500); // for 500 milliseconds
  stop(); // stop motion
  // grab object, position arm properly to grasp the object.
  armbase.write(100); // set arm base to angle 100
  delay(delaytime); // short delay to ensure smooth execution
  armclaw.write(0); // open claw
  delay(delaytime); // short delay to ensure smooth execution
  army.write(110);  // set vertical position of arm to right position
  delay(delaytime); // short delay to ensure smooth execution
  armx.write(180);  // extend arm to grasp object
  delay(delaytime); // short delay to ensure smooth execution
  armclaw.write(90);  // use claw to grab object
  delay(delaytime); // short delay to ensure smooth execution
  if (reading) {  // if metal
    armbase.write(180); // place to one side
    delay(delaytime); // short delay to ensure smooth execution
    armclaw.write(0); // open claw to release object
    delay(delaytime); // short delay to ensure smooth execution
  }
  else {  // otherwise, if object is not metal
    armbase.write(0); // place on opposite side of metal objects
    delay(delaytime); // short delay to ensure smooth execution
    armclaw.write(0); // open claw to release object
    delay(delaytime); // short delay to ensure smooth execution
  }
  armx.write(20); // pull arm back from fully extended state
  delay(delaytime);  // delay to ensure smooth execution
  army.write(150);  // reset vertical arm position
  delay(delaytime); // short delay to ensure smooth execution
  armbase.write(90);  // reset arm base to angle 90
  delay(delaytime); // short delay to ensure smooth execution
  armclaw.write(0); // reset claw
  delay(delaytime); // short delay to ensure smooth execution
}

void sort () { // sort function will sort conductive objects
  boolean reading = false;  // metal touch sensor reading
  boolean result = false; // result of reading
  int current = 0;  // set current time as 0
  IReyes.write(90); // set IR finder to angle 90
  Serial1.println("Press ""stop"" when all items are sorted");  // display appropriate message
  do { // keep sorting till all objects are sorted
    if (distance() > 55) {  // if distance read is greater than 55
      detectObject(); //detect object to determine conductivity (hit object against sensor)
      delay(500); // short delay for smooth execution
      current = millis(); // set current to current time
      while (millis() - current < 2500) { // while elapsed time is less than 2500 milliseconds, allowing the robot to have 2.5 seconds to determine a reading of the object
        reading = touchsensorvalue(); // determine reading from metal touch sensor
        if (reading)  // if reading is true
          result = true;  // set result to true
      }
      delay(500); //short delay for smooth execution
      placeproperly(result);  // place object to the right side, depending on the result (metal or non-metal)
    }

  }
  while (Serial1.read() != 52);// as long as user does not press the stop button on the bluetooth serial
  turn(0); // when finished, reposition car
  movebetweenstations();  // move to next station
}

void detectObject () { // moves towards object, hits it against metal touch sensor
  advance(200, 230); // go towards object
  delay (distance() * 11); // until you hit it
  stop(); // stop motion
}

// reads touch sensor value
boolean touchsensorvalue () {
  if (digitalRead (touchsensorpin) == 1) // if sensor detects metal object
    return true;  // set boolean value as true
  return false; // otherwise, set as false
}

void pianostation () { // allows robot to play piano, each hit of a key corresponds to a note
  char note = 'p';  // declare note of char type which will be used to determine which note is to be hit
  int temp = 0; //variable used to set the position at which the armbase will be turned to in order to hit the right note
  armclaw.write(90);  //set claw to closed position

  do {// plays sequence of notes
    if (Serial1.available() > 0) { // if the readings comming from the bluetooth serial monitor are greater than 0 (the serial monitor will send -1 if nothing is inputted), this filters out any unwanted readings
      note = Serial1.read();  // read note off bluetooth serial monitor
      for (int x = 0; x < 6 ; x++) {  // check all 6 notes
        if (notes[x] == note) { // cycle through to see which note is selected
          temp = x; // set temp varaible to this note
        }
      }
    }
    // if note is not p, which is the character that is sent when the button on the bluetooth serial monitor is let go, allowing the arm to retract
    if (note != 'p') {
      armbase.write(109 - temp * 8);  // move armbase to appropriate position
      delay(500); // short delay to ensure smooth execution
      // hit corresponding key on keyboard, extending arm will push a key
      armx.write(173);
      army.write(180);
    }
    // otherwise, if note is p, this retracts the arm and stops hitting the note
    else {
      army.write(40); //retracts arm
    }
  }
  while (note != 52); // stop when user commands so by pressing the stop button on the bluetooth serial monitor
  turn (0);   // reset car position
  movebetweenstations();  // move to next station
}

// User-controlled arm x and y movement
void drawingstation () {
  // reset arm positioning
  armclaw.write(0);
  army.write(90);
  armx.write(90);
  if (flag == false) {  // if message hasn't been dispalyed
    // Dispaly messages/instructions on how to draw
    Serial1.println("Press ""Draw"" when ready to draw");
    Serial1.println("Push in the joystick to lift up the pen");
    Serial1.println("Press ""Stop"" when you are done drawing");
    flag = true;  // set flag to true, ensures that message is displayed only once
  }
  // waits for user the press the draw button on the bluetooth serial monitor
  while (Serial1.read() != 51) {
    if (Serial1.read() == 51) //if the draw button is pressed
      break;//break the loop and execute the drawing
  }
  // set arm positions
  armclaw.write(90);
  army.write(75);
  armx.write(180);
  // while the user does not press the stop button on the bluetooth serial monitor
  while (Serial.read() != 52) {
    int readingx = analogRead (arm_x); //store the reading on the x axis of the joystick
    int readingy = analogRead (arm_y); //store the reading on the y axis of the joystick
    int readingbutton = digitalRead (arm_down); // read joystick button
    if (readingbutton == LOW) { // if button is pressed
      drawing = !drawing; // toggle drawing status
    }
    //convert readings to number from 0-180
    readingx = map(readingx, 0, 1023, 53, 129); //maps the readings from 0-1023 to 0-180
    readingy = map(readingy, 0, 1023, -253, 255); //maps the readings from 0-1023 to 0-180
    Serial.print(readingx);
    // if readingy is 0
    if (readingy == 0) {
      stop(); // stop
    }
    // if readingy is less than 0
    else if (readingy < 0) {
      readingy = readingy * -1; // negative sign means backwards
      back(readingy, readingy); // move back
      delay(50);  // for 50 milliseconds
    }
    // otherwise, if reading is greater than 0
    else {
      advance(readingy, readingy);  // move forward
      delay(50);  // for 50 milliseconds
    }

    // readingx reports to car how much to turn
    turn(readingx); // turn to specified angle
    delay(50);  // for 50 milliseconds

    if (drawing) {  // if pen is down (drawing)
      armx.write(180);
      army.write(75);
      delay(200);
    }
    else {  // if pen is up (not drawing)
      armx.write(90);
      army.write(90);
      delay(200);
    }
  }
  turn (0);   // reset car position
  movebetweenstations();  // move to next station
}
