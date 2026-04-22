#include <Arduino.h>
#include <SSD1306Wire.h>

#define button1 34 // speed up button pin
#define button2 0// direction button pin
#define button3 35 // step mode button pin

const int SIZE = 8;
const int MAX_PARTICLES = 100;
const int SPEED = 100;
int particles[SIZE][SIZE][SIZE];
int particleCount = 0;

bool moved[SIZE][SIZE][SIZE];

//screen on makerboard
SSD1306Wire lcd(0x3C, SDA, SCL);
char text[200];

//can be 1 or 0 or -1
int yGrav = -1;
int xGrav = 0;
int zGrav = 0 ;

void setup() {
  lcd.init();
  lcd.flipScreenVertically();
  lcd.setFont(ArialMT_Plain_16);
  lcd.clear();
  lcd.display();
  Serial.begin(115200);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
}

void loop() {
  checkInput();
  createParticle(4,7,4);
  simulateParticles();
  printThreeViews();
  //Serial.print(", ");
  //Serial.println(particles[6][0][4]);
  delay(SPEED);
}


void checkInput(){
   if(digitalRead(button1)==0){
    if(xGrav <=0){
      xGrav++;
    }
    else{
      xGrav = -1;
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