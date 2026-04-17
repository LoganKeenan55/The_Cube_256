#include <Arduino.h>
#include <SSD1306Wire.h>

#define button1 34 // speed up button pin
#define button2 0// direction button pin
#define button3 35 // step mode button pin

const int SIZE = 8;
const int MAX_PARTICLES = 20;
const int SPEED = 300;
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
    zGrav = 0;
    yGrav = -1;
    xGrav = 0;
    delay(200); //debounce :D
   }
   if(digitalRead(button2)==0){
    zGrav = -1;
    yGrav = 0;
    xGrav = 0;
    delay(200); //debounce :D
   }
   if(digitalRead(button3)==0){
    zGrav = 0;
    yGrav = 0;
    xGrav = -1;
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

  for(int x = 0; x < SIZE; x++){
    for(int y = 0; y < SIZE; y++){
      for(int z = 0; z < SIZE; z++){
        if(particles[x][y][z] == 1 && !moved[x][y][z]){
          updateParticleBasedOnGravity(x,y,z);
        }
      }
    }
  }

}

void updateParticle(int x, int y, int z){
  //on ground (bottem of cube)
  if(y == 0){
    return;
  }
  //in air
  if(y >= 1 && particles[x][y-1][z] == 0){
    moveParticle(x,y,z,x,y-1,z);
    return;
  }

  //on particle, where to move
  
  //1
  if(x > 0 && particles[x-1][y-1][z] == 0){
    moveParticle(x,y,z,x-1,y-1,z);
    return;
  }
  //2
  else if(x < 7 && particles[x+1][y-1][z] == 0){
    moveParticle(x,y,z,x+1,y-1,z);
    return;
  }
  //3
  else if(z > 0 && particles[x][y-1][z-1] == 0){
    moveParticle(x,y,z,x,y-1,z-1);
    return;
  }
  //4
  else if(z < 7 && particles[x][y-1][z+1] == 0){
    moveParticle(x,y,z,x,y-1,z+1);
    return;
  }
  else{
    //no open spot below, don't move
    return;
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

void updateParticleBasedOnGravity(int x, int y, int z){

  moved[x][y][z] = true;

  int possibleMoves[7][3];
  int moveCount = 0;

  possibleMoves[moveCount][0] = x + xGrav;
  possibleMoves[moveCount][1] = y + yGrav;
  possibleMoves[moveCount][2] = z + zGrav;
  moveCount++;

  if(yGrav != 0){
    possibleMoves[moveCount][0] = x - 1;       possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z;         moveCount++;
    possibleMoves[moveCount][0] = x + 1;       possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z;         moveCount++;
    possibleMoves[moveCount][0] = x;           possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z - 1;     moveCount++;
    possibleMoves[moveCount][0] = x;           possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z + 1;     moveCount++;
    possibleMoves[moveCount][0] = x - 1;       possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z - 1;     moveCount++;
    possibleMoves[moveCount][0] = x + 1;       possibleMoves[moveCount][1] = y + yGrav; possibleMoves[moveCount][2] = z + 1;     moveCount++;
  }
  else if(xGrav != 0){
    possibleMoves[moveCount][0] = x + xGrav;   possibleMoves[moveCount][1] = y - 1;     possibleMoves[moveCount][2] = z;         moveCount++;
    possibleMoves[moveCount][0] = x + xGrav;   possibleMoves[moveCount][1] = y + 1;     possibleMoves[moveCount][2] = z;         moveCount++;
    possibleMoves[moveCount][0] = x + xGrav;   possibleMoves[moveCount][1] = y;         possibleMoves[moveCount][2] = z - 1;     moveCount++;
    possibleMoves[moveCount][0] = x + xGrav;   possibleMoves[moveCount][1] = y;         possibleMoves[moveCount][2] = z + 1;     moveCount++;
    possibleMoves[moveCount][0] = x + xGrav;   possibleMoves[moveCount][1] = y - 1;     possibleMoves[moveCount][2] = z - 1;     moveCount++;
    possibleMoves[moveCount][0] = x + xGrav;   possibleMoves[moveCount][1] = y + 1;     possibleMoves[moveCount][2] = z + 1;     moveCount++;
  }
  else if(zGrav != 0){
    possibleMoves[moveCount][0] = x - 1;       possibleMoves[moveCount][1] = y;         possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x + 1;       possibleMoves[moveCount][1] = y;         possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x;           possibleMoves[moveCount][1] = y - 1;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x;           possibleMoves[moveCount][1] = y + 1;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x - 1;       possibleMoves[moveCount][1] = y - 1;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;
    possibleMoves[moveCount][0] = x + 1;       possibleMoves[moveCount][1] = y + 1;     possibleMoves[moveCount][2] = z + zGrav; moveCount++;
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