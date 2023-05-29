// Define pin connections & motor's steps per revolution
const int dirPin1 = 2;
const int stepPin1 = 3;
const int testPin1 = 10;
const int dirPin2 = 4;
const int stepPin2 = 5;
const int xLimitPin = 6; //limit switch for each direction, on the side away from the motor
const int yLimitPin = 7;
const int magnetPin = 8;
const int microstepPin = 9;
const int XstepsPerRevolution = 400;
const int YstepsPerRevolution = 200;
//distanceConversion is in units of (rotations)/(square length)
//Y is short way, X is long way of the board frame
const float distanceConversionX = 0.95;
const float distanceConversionY = 0.97;
const float maxSpeed = 5; //maximum safe speed for the motors in rotations per second
const float maxX = 12;
const float maxY = 8;
float defaultSpeed = 3.5;
float posX;
float posY;
int microstep = 1;
//void turnOffPower() {
//  digitalWrite(powerPin, LOW);
//}
//void turnOnPower() {
//  digitalWrite(powerPin, HIGH);
//  delay(50);
//}
void setMicrostep(int newMicrostep) {
  microstep = newMicrostep;
  if (newMicrostep == 4) {
    digitalWrite(microstepPin, HIGH);
  } else {
    digitalWrite(microstepPin, LOW);
  }
}
void raiseMagnet() {
  digitalWrite(magnetPin, LOW);
}
void lowerMagnet() {
  digitalWrite(magnetPin, HIGH);
}

bool limitX() {
  return (digitalRead(xLimitPin) == 1);
}
bool limitY() {
  return (digitalRead(yLimitPin) == 1);
}
int spin(float spd, float amt, int pin, float dir, int spr, int axis, bool force = false) { //spr = steps per revolution, axis = 0: x, 1: y
  float currentSpeed = 0.2;
  if (spd < 3) {
      currentSpeed = spd; //no ramp up
  }
  for(int x = 0; x < spr * amt * microstep; x++)
  {
    if (currentSpeed < spd) {
      currentSpeed += 0.1/currentSpeed;
    }
    digitalWrite(pin, HIGH);
    delayMicroseconds((int)((500000.0/currentSpeed)/((float)spr)));
    digitalWrite(pin, LOW);
    delayMicroseconds((int)((500000.0/currentSpeed)/((float)spr)));
    if (!force && limitX() && dir < 0 && axis == 0) {
      return -1;
    }
    if (!force && limitY() && dir < 0 && axis == 1) {
      return -1;
    }
  }
  return 1;
}

//truly around 0.7 times as fast as it should be if it were even speed
int spin(float spd, float amt, int pinX, float dirX, int pinY, float dirY, bool force = false) {
  float currentSpeed = 0.2;
  if (spd < 3) {
      currentSpeed = spd; //no ramp up
  }
  int spr = YstepsPerRevolution;
  for(int x = 0; x < spr * amt * microstep; x++)
  {
    if (currentSpeed < spd) {
      currentSpeed += 0.1/currentSpeed;
    }
    digitalWrite(pinX, HIGH);
    digitalWrite(pinY, HIGH);
    delayMicroseconds((int)((500000.0/currentSpeed)/((float)spr)));
    digitalWrite(pinX, LOW);
    delayMicroseconds((int)((500000.0/currentSpeed)/((float)spr)));
    digitalWrite(pinX, HIGH);
    digitalWrite(pinY, LOW);
    delayMicroseconds((int)((500000.0/currentSpeed)/((float)spr)));
    digitalWrite(pinX, LOW);
    delayMicroseconds((int)((500000.0/currentSpeed)/((float)spr)));
    if (!force && limitX() && dirX < 0) {
      return -1;
    }
    if (!force && limitY() && dirY < 0) {
      return -1;
    }
  }
  return 1;
}
//distance is the number of chessboard squares
//pin1
int moveX(float distance, float spd, bool force = false) { //+ distance is towards the motors, long way
  if (distance > 0) {
    digitalWrite(dirPin1, LOW);
  } else {
    digitalWrite(dirPin1, HIGH);
  }
  float amt = distance * distanceConversionX;
  posX += distance;
  return spin(spd, abs(amt), stepPin1, distance, XstepsPerRevolution, 0, force);
}

//pin2
int moveY(float distance, float spd, bool force = false) { //+ distance is towards the single motor, short way
  if (distance > 0) {
    digitalWrite(dirPin2, HIGH);
  } else {
    digitalWrite(dirPin2, LOW);
  }
  float amt = distance * distanceConversionY;
  posY += distance;
  return spin(spd, abs(amt), stepPin2, distance, YstepsPerRevolution, 1, force);
}

int moveDiag(float distanceX, float distanceY, float spd, bool force = false) { //distance is the horizontal and vertical distance
  if (distanceX > 0) {
    digitalWrite(dirPin1, LOW);
  } else {
    digitalWrite(dirPin1, HIGH);
  }
  if (distanceY > 0) {
    digitalWrite(dirPin2, HIGH);
  } else {
    digitalWrite(dirPin2, LOW);
  }
  float amt = distanceX * distanceConversionX;
  posX += distanceX;
  posY += distanceY;
  return spin(spd, abs(amt), stepPin1, distanceX, stepPin2, distanceY, force);
}

int moveTo(float x, float y, bool onEdge = false) {
  //limits x and y
  if (x < 0 || x > maxX || y < 0 || y > maxY) {
    return -1;
  }
  if (abs(x-posX) == abs(y-posY)) {
    moveDiag(x-posX, y-posY, defaultSpeed);
  } else if (onEdge) {
    float littleXChange = (abs(x-posX)/(x-posX)) * 0.5;
    float littleYChange = (abs(y-posY)/(y-posY)) * 0.5;
    //move to edge of square
    moveDiag(littleXChange, littleYChange, defaultSpeed);
    //move to target location
    moveX(x-posX-littleXChange, defaultSpeed);
    moveY(y-posY-littleYChange, defaultSpeed);
    //move back to center of square
    moveDiag(littleXChange, littleYChange, defaultSpeed);
    setMicrostep(1);
  } else {
    moveX(x-posX, defaultSpeed);
    moveY(y-posY, defaultSpeed, true);
  }
}
void callibrate() {
  lowerMagnet();
  setMicrostep(4);
  moveX(0.1, defaultSpeed, true);
  moveY(0.1, defaultSpeed, true);
  setMicrostep(1);
  moveX(-12, defaultSpeed);
  posX = -0.15;
  moveX(1, defaultSpeed, true);
  moveY(-8, defaultSpeed);
  posY = -0.2;
  moveY(1, defaultSpeed, true);
}
bool parseMove(String moveName, int* fromTo) { //returns whether the pieces need to be moved between others
  fromTo[0] = moveName[0]-'a';
  fromTo[1] = moveName[1]-'1';
  if (moveName[3] == '-') { //remove piece
    fromTo[2] = 9;
    fromTo[3] = fromTo[1];
    return true;
  } else {
    fromTo[2] = moveName[2]-'a';
    fromTo[3] = moveName[3]-'1';
  }
  if (moveName.length() > 4) {
    if (moveName[4] == 'N' || moveName[4] == 'n') {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}
void setup()
{
//  defaultSpeed = 3;
  // Declare pins as Outputs
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(magnetPin, OUTPUT);
  pinMode(microstepPin, OUTPUT);
  pinMode(xLimitPin, INPUT);
  pinMode(yLimitPin, INPUT);
  
  
  Serial.begin(9600);
  callibrate();
}
void loop()
{
  moveTo(0,0);
  raiseMagnet();
  while (Serial.available() == 0) {
    delay(10);
  }
  String s = Serial.readString();
  int * moveArr = new int[4];
  bool betweenSquares = parseMove(s, moveArr);
  lowerMagnet();
  moveTo(moveArr[0], moveArr[1]);
  raiseMagnet();
  delay(50);
  moveTo(moveArr[2], moveArr[3], betweenSquares);
  delay(50);
  Serial.println("Move complete");

//  raiseMagnet();
//  moveTo(2,2);
//  delay(1000);
//  moveTo(4,0);
//  delay(3000);
  
//  Serial.println("First pattern");
//  lowerMagnet();
//  moveTo(0,0);
//  raiseMagnet();
//  delay(1000);
//  moveTo(3,5);
//  delay(1000);
//  moveTo(6,3);
//  delay(1000);
//  moveTo(2,4);
//  delay(1000);
//  moveTo(0,0);
//  delay(1000);
//  lowerMagnet();
//  delay(5000);
//  Serial.println("Second pattern");
//  moveTo(5,0);
//  raiseMagnet();
//  delay(1000);
//  moveTo(3,5);
//  delay(1000);
//  moveTo(6,3);
//  delay(1000);
//  moveTo(2,4);
//  delay(1000);
//  moveTo(5,0);
//  delay(5000);
  callibrate();
}
