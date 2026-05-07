#include <Arduino.h>
#include <SSD1306Wire.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <FastLED.h>
#include <cstdlib>
#include <cmath>
#include <WiFi.h>
#include <WebServer.h>

Adafruit_MPU6050 mpu;

#define button1 34 // speed up button pin
#define button2 0// direction button pin
#define button3 35 // step mode button pin

//ACCELEROMETER
#define GPIO12 12

//LEDS
#define LED_PIN 13
#define NUM_LEDS 320

//SERVER
const char* ssid = "TheCube";
const char* pass = "HelloWorld";
WebServer server(80);

CRGB leds[NUM_LEDS];

const int SIZE = 8;
const int MAX_PARTICLES = 180;
const int SPEED = 25;
int currentSpeed = SPEED;


int particles[SIZE][SIZE][SIZE];
int particleCount = 0;

int currentColor = 0;
int currentTime = 0;


bool moved[SIZE][SIZE][SIZE];
bool startupFinished = false;


//screen on makerboard
SSD1306Wire lcd(0x3C, SDA, SCL);
char text[200];

//can be 1 or 0 or -1
int yGrav = -1;
int xGrav = 0;
int zGrav = 0 ;



String getCurrentColorHex() {
  switch (currentColor) {
    case 0: return "#00ff64";
    case 1: return "#ff3200";
    case 2: return "#00ff32";
    case 3: return "#0032ff";
    case 4: return "#ffffff";
    case 5: return "#aaaaaa";
    case 6: return "#ff0000";
  }
  return "#ffffff";
}

const char* controlPage = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <title>The Cube</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root { --accent: #00ff64; }
    body {
      font-family: Arial, sans-serif;
      background: #111;
      color: #eee;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 30px;
    }
    h1 { color: var(--accent); }
    .group { margin: 20px 0; text-align: center; }
    .group h2 { margin-bottom: 10px; font-size: 16px; color: #aaa; }
    .btn {
      padding: 14px 28px;
      margin: 6px;
      font-size: 16px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      background: var(--accent);
      color: #111;
      font-weight: bold;
    }
    #status { margin-top: 24px; color: var(--accent); font-size: 14px; }
  </style>
</head>
<body>
  <h1>The Cube&#8482;</h1>

  <div class="group">
    <h2>Color Mode</h2>
    <button class="btn" onclick="cycleColor()">Next Color</button>
  </div>

  <div class="group">
    <h2>Speed</h2>
    <button class="btn" onclick="cmd('/speed/faster')">Faster</button>
    <button class="btn" onclick="cmd('/speed/slower')">Slower</button>
  </div>

  <p id="status">Ready.</p>

  <script>
    function setAccent(hex) {
      document.documentElement.style.setProperty('--accent', hex);
    }

    function cmd(path) {
      fetch(path)
        .then(r => r.text())
        .then(msg => document.getElementById('status').innerText = msg)
        .catch(() => document.getElementById('status').innerText = 'No response');
    }

    function cycleColor() {
      fetch('/color/cycle')
        .then(r => r.text())
        .then(hex => {
          setAccent(hex);
          document.getElementById('status').innerText = 'Color changed!';
        })
        .catch(() => document.getElementById('status').innerText = 'No response');
    }

    fetch('/color/current')
      .then(r => r.text())
      .then(hex => setAccent(hex));
  </script>
</body>
</html>
)rawhtml";

void on_home() {
  server.send(200, "text/html", controlPage);
}

void on_color_cycle() {
  if(currentColor <= 5){
    currentColor++;
  }
  else{
    currentColor = 0;
  }
  server.send(200, "text/plain", getCurrentColorHex());
}

void on_color_current() {
  server.send(200, "text/plain", getCurrentColorHex());
}

void on_speed_faster() {
  if(currentSpeed > 10){
    currentSpeed -= 10;
  }
  char msg[40];
  sprintf(msg, "Delay: %dms", currentSpeed);
  server.send(200, "text/plain", msg);
}

void on_speed_slower() {
  if(currentSpeed < 200){
    currentSpeed += 10;
  }
  char msg[40];
  sprintf(msg, "Delay: %dms", currentSpeed);
  server.send(200, "text/plain", msg);
}

void initWiFiServer() {
  Serial.println("Starting WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", on_home);
  server.on("/color/cycle", on_color_cycle);
  server.on("/color/current", on_color_current);
  server.on("/speed/faster", on_speed_faster);
  server.on("/speed/slower", on_speed_slower);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });

  server.begin();
  Serial.println("Web server started.");
}

void startupAnimation(){
  for(int i = 0; i < NUM_LEDS; i++){
    int r =  rand() % (255 + 1);
    int g =  rand() % (255 + 1);
    int b =  rand() % (255 + 1);
    leds[i] = CRGB(r, g, b);
    FastLED.show();
    delay(5);
  }
  startupFinished = true;
}

CRGB getCurrentColor() {
  switch (currentColor) {
    case 0:
      return CRGB(0,255,100);
    case 1:
      return CRGB(255, 50, 0);

    case 2:
      return CRGB(0, 255, 50);

    case 3:
      return CRGB(0, 50, 255);

    case 4: {
      int r = rand() % 256;
      int g = rand() % 256;
      int b = rand() % 256;
      return CRGB(r, g, b);
    }

    case 5: {
      int randomValue = rand() % 256;
      return CRGB(randomValue, randomValue, randomValue);
    }

    case 6: {
      double timeDivided = sin(currentTime % 34)*137;
      return CRGB(timeDivided, 0, 0);
    }
    case 7:{
      
    }
  }
}


void updateScreen(int i){
  int offset = (i - 1) * 64;

  for (int row = 0; row < SIZE; row++) {
    for (int col = 0; col < SIZE; col++) {
      int visible = 0;
      int closest = -1;

      if(i == 1){
        int z = row;
        int x = col;

        for (int y = 0; y < SIZE; y++) {
          if(particles[x][y][z]){
            visible = 1;
            if (y > closest) closest = y;
          }
        }
      }
    else if(i == 2){
      int y = SIZE - 1 - row;
      int z = SIZE - 1 - col;

      for (int x = 0; x < SIZE; x++) {
        if(particles[x][y][z]){
          visible = 1;
          if (x > closest) closest = x;
        }
      }
    }
    else if(i == 3){
      int y = SIZE - 1 - row;
      int z = col;

      for (int x = 0; x < SIZE; x++) {
        if(particles[x][y][z]){
          visible = 1;
          if (SIZE - 1 - x > closest) closest = SIZE - 1 - x;
        }
      }
    }
      else if(i == 4){
        int y = SIZE - 1 - row;
        int x = col;

        for (int z = 0; z < SIZE; z++) {
          if(particles[x][y][z]){
            visible = 1;
            if (z > closest) closest = z;
          }
        }
      }
      else if(i == 5){
        int y = SIZE - 1 - row;
        int x = SIZE - 1 - col;

        for (int z = 0; z < SIZE; z++) {
          if(particles[x][y][z]){
            visible = 1;
            if (SIZE - 1 - z > closest) closest = SIZE - 1 - z;
          }
        }
      }

      int index;
      if (row % 2 == 0) {
        index = offset + col + row * SIZE;
      } else {
        index = offset + (SIZE - 1 - col) + row * SIZE;
      }

      if (visible) {
        int alpha = map(closest, 0, SIZE - 1, 0, 255);
        leds[index] = getCurrentColor().nscale8(alpha);
      } else {
        leds[index] = CRGB(0, 0, 0);
      }
    }
  }
}
void updateAllScreens(){
  FastLED.clear();
  for(int i = 1; i <= 5; i++){
    updateScreen(i);
  }
  FastLED.show();
}




void initAccelerometer(){
  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050");

  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);


}


void initLEDMatrix(){
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
  FastLED.clear();
  FastLED.show();
}



void setup() {
  initAccelerometer();
  initLEDMatrix();
  
  lcd.init();
  lcd.flipScreenVertically();
  lcd.setFont(ArialMT_Plain_16);
  lcd.clear();
  lcd.display();
  Serial.begin(115200);
  initWiFiServer();
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  startupAnimation();
}

void loop() {
  server.handleClient();

  if(!startupFinished){
    return;
  }
  updateAllScreens();
  checkGravity();
  checkInput();
  createParticle(4,7,4);
  simulateParticles(); 
  printThreeViews();
  //Serial.print(", ");
  //Serial.println(particles[6][0][4]);
  currentTime++;
  delay(currentSpeed);
}

void checkGravity(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float ax = a.acceleration.x;   //remapped
  float ay = -a.acceleration.z;  //remapped
  float az = a.acceleration.y;   //remapped

  float threshold = 6.0;

  if(ax > threshold){
    xGrav = 1;
  }
  else if(ax < -threshold){
    xGrav = -1;
  }
  else if(ax <threshold && ax > -threshold){
    xGrav = 0;
  }

  if(ay > threshold){
    yGrav = 1;
  }
  else if(ay < -threshold){
    yGrav = -1;
  }
  else if(ay <threshold && ay > -threshold){
    yGrav = 0;
  }

  if(az > threshold){
    zGrav = 1;
  }
  else if(az < -threshold){
    zGrav = -1;
  }
  else if(az <threshold && az > -threshold){
    zGrav = 0;
  }

}

void checkInput(){
   if(digitalRead(button1)==0){
    if(currentColor <= 5){
      currentColor++;
    }
    else{
      currentColor = 0;
    }
    delay(200); //debounce :D
   }
   if(digitalRead(button2)==0){
    if(yGrav <=0){
      yGrav++;
    }
    else{
      yGrav = -1;
    }
    delay(200); //debounce :D
   }
   if(digitalRead(button3)==0){
    if(zGrav <=0){
      zGrav++;
    }
    else{
      zGrav = -1;
    }
    delay(200); //debounce :D
   }

    lcd.clear();
    sprintf(text, "Xgrav: %d", xGrav);
    lcd.drawString(0, 0, text);
    sprintf(text, "Ygrav: %d", yGrav);
    lcd.drawString(0, 16, text);
    sprintf(text, "Zgrav: %d", zGrav);
    lcd.drawString(0, 32, text);
    lcd.display();

}

void printThreeViews()
{
  Serial.println();
  Serial.print("\033[2J");
  Serial.print("\033[H");

  for (int row = 0; row < SIZE; row++){

    // show X vs Z (top-down)
    int z = row;
 for (int x = 0; x < SIZE; x++){
   int visible = 0;
      for (int y = 0; y < SIZE; y++){
        visible |= particles[x][y][z];
      }
      if (visible) Serial.print("1 ");
      else Serial.print("  ");
    }

    Serial.print("   |   ");

    // show Y vs Z (side view)
    int y = SIZE - 1 - row; // top row = highest y
    for (int z2 = 0; z2 < SIZE; z2++){
      int visible = 0;
      for (int x = 0; x < SIZE; x++){
        visible |= particles[x][y][z2];
      }
      if (visible) Serial.print("1 ");
      else Serial.print("  ");
    }

    Serial.print("   |   ");

    // show X vs y (side view)
    int y2 = SIZE - 1 - row; // print top y first
    for (int x = 0; x < SIZE; x++){
      int visible = 0;
      for (int z3 = 0; z3 < SIZE; z3++)
      {
        visible |= particles[x][y2][z3];
      }
      if (visible) Serial.print("1 ");
      else Serial.print("  ");
    }

    Serial.println();
  }

  Serial.println();
  Serial.println();
}


void clearMoved(){
  for(int x = 0; x < SIZE; x++){
    for(int y = 0; y < SIZE; y++){
      for(int z = 0; z < SIZE; z++){
        moved[x][y][z] = false;
      }
    }
  }
}

void simulateParticles(){

  clearMoved();

  int xStart = 0, xEnd = SIZE, xStep = 1;
  int yStart = 0, yEnd = SIZE, yStep = 1;
  int zStart = 0, zEnd = SIZE, zStep = 1;

  if (xGrav > 0) {
    xStart = SIZE - 1; xEnd = -1; xStep = -1;
  }
  else if (xGrav < 0) {
    xStart = 0; xEnd = SIZE; xStep = 1;
  }

  if (yGrav > 0) {
    yStart = SIZE - 1; yEnd = -1; yStep = -1;
  }
  else if (yGrav < 0) {
    yStart = 0; yEnd = SIZE; yStep = 1;
  }

  if (zGrav > 0) {
    zStart = SIZE - 1; zEnd = -1; zStep = -1;
  }
  else if (zGrav < 0) {
    zStart = 0; zEnd = SIZE; zStep = 1;
  }

  for(int x = xStart; x != xEnd; x += xStep){
    for(int y = yStart; y != yEnd; y += yStep){
      for(int z = zStart; z != zEnd; z += zStep){
        if(particles[x][y][z] == 1 && !moved[x][y][z]){
          updateParticleBasedOnGravity(x,y,z);
        }
      }
    }
  }
}

void validateParticleCount(){
  int actualCount = 0;
  for(int x = 0; x < SIZE; x++){
    for(int y = 0; y < SIZE; y++){
      for(int z = 0; z < SIZE; z++){
        if(particles[x][y][z] == 1){
          actualCount++ ;
        }
      }
    }
  }
  if(actualCount != particleCount){
    throw std::runtime_error("Incorrect Particle Count!");
  }
}

bool isInBounds(int x, int y, int z){
  if(x >= SIZE || x < 0){
    return false;
  }
  if(y >= SIZE || y < 0){
    return false;
  }
  if(z >= SIZE || z < 0){
    return false;
  }
  return true;
}

void updateMultiGravityParticle(int x, int y, int z){

  int possibleMoves[8][3];
  int scores[8];
  int moveCount = 0;

  int xOptions[2] = {0, xGrav};
  int yOptions[2] = {0, yGrav};
  int zOptions[2] = {0, zGrav};

  int xCount = (xGrav == 0) ? 1 : 2;
  int yCount = (yGrav == 0) ? 1 : 2;
  int zCount = (zGrav == 0) ? 1 : 2;

  for(int xi = 0; xi < xCount; xi++){
    for(int yi = 0; yi < yCount; yi++){
      for(int zi = 0; zi < zCount; zi++){
        int dx = xOptions[xi];
        int dy = yOptions[yi];
        int dz = zOptions[zi];

        if(dx == 0 && dy == 0 && dz == 0){
          continue;
        }

        possibleMoves[moveCount][0] = x + dx;
        possibleMoves[moveCount][1] = y + dy;
        possibleMoves[moveCount][2] = z + dz;
        scores[moveCount] = abs(dx) + abs(dy) + abs(dz);
        moveCount++;
      }
    }
  }

  for(int i = 0; i < moveCount - 1; i++){
    for(int j = i + 1; j < moveCount; j++){
      if(scores[j] > scores[i]){
        int tempScore = scores[i];
        scores[i] = scores[j];
        scores[j] = tempScore;

        int tempX = possibleMoves[i][0];
        int tempY = possibleMoves[i][1];
        int tempZ = possibleMoves[i][2];

        possibleMoves[i][0] = possibleMoves[j][0];
        possibleMoves[i][1] = possibleMoves[j][1];
        possibleMoves[i][2] = possibleMoves[j][2];

        possibleMoves[j][0] = tempX;
        possibleMoves[j][1] = tempY;
        possibleMoves[j][2] = tempZ;
      }
    }
  }

  for(int i = 0; i < moveCount; i++){
    int nx = possibleMoves[i][0];
    int ny = possibleMoves[i][1];
    int nz = possibleMoves[i][2];

    if(!isInBounds(nx, ny, nz)){
      continue;
    }

    if(particles[nx][ny][nz] == 0){
      moveParticle(x, y, z, nx, ny, nz);
      return;
    }
  }
}

void updateParticleBasedOnGravity(int x, int y, int z){

  moved[x][y][z] = true;

  int activeAxes = 0;
  if(xGrav != 0) activeAxes++;
  if(yGrav != 0) activeAxes++;
  if(zGrav != 0) activeAxes++;


  //if there is one axis we can do basic particle simulation with piling.
  if(activeAxes <= 1){
    updateSingleGravityParticle(x, y, z);
  }
  //if there is more then one axis active we need to alter the algorithm, it does not support piling, they
  //just fall in the direction of the gravities
  else{
    updateMultiGravityParticle(x, y, z);
  }
}

void updateSingleGravityParticle(int x, int y, int z){

  int possibleMoves[9][3];
  int moveCount = 0;

  //THE ULTIMATE FORMULA
  if(yGrav != 0){
    possibleMoves[moveCount][0] = x;     possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z;     moveCount++;

    possibleMoves[moveCount][0] = x - 1; possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z;     moveCount++;
    possibleMoves[moveCount][0] = x + 1; possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z;     moveCount++;
    possibleMoves[moveCount][0] = x;     possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z - 1; moveCount++;
    possibleMoves[moveCount][0] = x;     possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z + 1; moveCount++;

    possibleMoves[moveCount][0] = x - 1; possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z - 1; moveCount++;
    possibleMoves[moveCount][0] = x - 1; possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z + 1; moveCount++;
    possibleMoves[moveCount][0] = x + 1; possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z - 1; moveCount++;
    possibleMoves[moveCount][0] = x + 1; possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z + 1; moveCount++;
  }
  else if(xGrav != 0){
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y;     possibleMoves[moveCount][2] = z;     moveCount++;

    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y -1; possibleMoves[moveCount][2] = z;     moveCount++;
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y + 1; possibleMoves[moveCount][2] = z;     moveCount++;
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y;     possibleMoves[moveCount][2] = z - 1; moveCount++;
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y;     possibleMoves[moveCount][2] = z + 1; moveCount++;

    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y - 1; possibleMoves[moveCount][2] = z - 1; moveCount++;
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y - 1; possibleMoves[moveCount][2] = z + 1; moveCount++;
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y + 1; possibleMoves[moveCount][2] = z - 1; moveCount++;
    possibleMoves[moveCount][0] = x + xGrav; possibleMoves[moveCount][1] = y + 1; possibleMoves[moveCount][2] = z + 1; moveCount++;
  }
  else if(zGrav != 0){
    possibleMoves[moveCount][0] = x;     possibleMoves[moveCount][1] = y;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;

    possibleMoves[moveCount][0] = x - 1; possibleMoves[moveCount][1] = y;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x + 1; possibleMoves[moveCount][1] = y;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x;     possibleMoves[moveCount][1] = y -1; possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x;     possibleMoves[moveCount][1] = y + 1; possibleMoves[moveCount][2] = z + zGrav; moveCount++;

    possibleMoves[moveCount][0] = x - 1; possibleMoves[moveCount][1] = y - 1; possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x - 1; possibleMoves[moveCount][1] = y + 1; possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x + 1; possibleMoves[moveCount][1] = y - 1; possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x + 1; possibleMoves[moveCount][1] = y + 1; possibleMoves[moveCount][2] = z + zGrav; moveCount++;
  }
  else{
    return;
  }

  for(int i = 0; i < moveCount; i++){
    int nx = possibleMoves[i][0];
    int ny = possibleMoves[i][1];
    int nz = possibleMoves[i][2];

    if(!isInBounds(nx, ny, nz)){
      continue;
    }

    if(particles[nx][ny][nz] == 0){
      moveParticle(x, y, z, nx, ny, nz);
      return;
    }
  }
}

void moveParticle(int x1, int y1, int z1, int x2, int y2, int z2){
  if(particles[x2][y2][z2] == 1){
    throw std::runtime_error("Trying to move particle to non empty spot");
  }
  particles[x1][y1][z1] = 0;
  particles[x2][y2][z2] = 1;
  moved[x2][y2][z2] = true;
}

void createParticle(int x, int y, int z){
  /* not sure if this should be an error

   if(particles[x][y][z] == 1){
     throw std::runtime_error("Trying to create particle in non empty spot");
   }
  */
  if(particleCount >= MAX_PARTICLES){
    return;
  }
  if(particles[x][y][z] == 1){
    return;
  }
    particles[x][y][z] = 1;
    particleCount++;

}

void removeParticle(int x, int y, int z){
   
  /* not sure if this should be an error
  if(particles[x][y][z] == 0){
    throw std::runtime_error("Trying to remove particle in empy spot");
  }
  */
   particles[x][y][z] = 0;
   particleCount--;
}