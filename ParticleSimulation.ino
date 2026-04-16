#include <Arduino.h>

#include <ESP32Servo.h>
const int SIZE = 8;
int particles[SIZE][SIZE][SIZE];
const int MAX_PARTICLES = 4;
int particleCount = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  createParticle(4,7,4);
  printThreeViews();
  simulateParticles();

  Serial.println();
  //Serial.print(", ");
  //Serial.println(particles[6][0][4]);
  delay(1000);
}

void printThreeViews() {
  for (int row = 0; row < SIZE; row++) {

    //show X vs Z (top-down)
    int z = row;
    for (int x = 0; x < SIZE; x++) {
      int visible = 0;
      for (int y = 0; y < SIZE; y++) {
        visible |= particles[x][y][z];
      }
      Serial.print(visible);
      Serial.print(" ");
    }

    Serial.print("   |   ");

    //show Y vs Z
    int z2 = row;
    for (int y = 0; y < SIZE; y++) {
      int visible = 0;
      for (int x = 0; x < SIZE; x++) {
        visible |= particles[x][y][z2];
      }
      Serial.print(visible);
      Serial.print(" ");
    }

    Serial.print("   |   ");

    //show X vs y
    int y2 = SIZE - 1 - row;  // print top y first
    for (int x = 0; x < SIZE; x++) {
      int visible = 0;
      for (int z3 = 0; z3 < SIZE; z3++) {
        visible |= particles[x][y2][z3];
      }
      Serial.print(visible);
      Serial.print(" ");
    }

    Serial.println();
  }

  Serial.println();
}
void simulateParticles(){
  for(int x = 0; x < SIZE; x++){
    for(int y = 1; y < SIZE; y++){
      for(int z = 0; z < SIZE; z++){
        if(particles[x][y][z] == 1){
          updateParticle(x,y,z);
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
  if(particles[x][y-1][z] == 0){
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

void moveParticle(int x1, int y1, int z1, int x2, int y2, int z2){
  if(particles[x2][y2][z2] == 1){
    throw std::runtime_error("Trying to move particle to non empty spot");
  }
  particles[x1][y1][z1] = 0;
  particles[x2][y2][z2] = 1;
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