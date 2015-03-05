#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include "math.h"

#include "visualizer.hpp"
#include "setupAide.hpp"
#include "occa.hpp"

#if COMPILED_WITH_GL
visualizer vis;
int click;
#endif

setupAide setup;

string model;
int dev, plat;

int width, height;
int Bx, By;

datafloat heightScale;

datafloat currentTime;
datafloat dx, dt;

int stencilRadius;

int mX, mY;

datafloat freq;
datafloat minU, maxU;

vector<datafloat> u1, u2;
vector<datafloat> xyz;

void run();
void setupMesh();

void setupSolver();
void solve();

#if COMPILED_WITH_GL
void glRun();
void update();
void drawMesh();
#endif

int main(int argc, char **argv){
#if OCCA_USE_OPENCL==1
  cl_list_all_devices();
#endif

#if COMPILED_WITH_GL
  vis.setup("OCL Visualizer", argc, argv);
  glRun();
#else
  run();
#endif

  return 0;
}

#if COMPILED_WITH_GL
void glRun(){
  setupMesh();
  setupSolver();

  click = 0;

  vis.setExternalFunction(update);
  vis.createViewports(1,1);

  vis.setOutlineColor(0, 0, 0);

  vis.fitBox(0, 0, -dx*width/2, dx*width/2, -dx*height/2, dx*height/2);

  vis.setBackground(0, 0,
                    (GLfloat) 0, (GLfloat) 0, (GLfloat) 0);

  vis.pause(0, 0);

  vis.start();
}
#endif

double totalIters = 0;
double totalFlops = 0;
double totalBW    = 0;
double totalNS    = 0;

void run(){
  setupMesh();
  setupSolver();

  for(int i = 0; i < 50; i++)
    solve();

  ofstream avgFile("avg.dat");

  avgFile << totalFlops/totalIters << " " << totalBW/totalIters << " " << totalNS/totalIters << '\n';

  avgFile.close();
  exit(0);
}

void setupMesh(){
  setup.read("setuprc");

  setup.getArgs("MODEL"   , model);
  setup.getArgs("PLATFORM", plat);
  setup.getArgs("DEVICE"  , dev);

  setup.getArgs("STENCIL RADIUS" , stencilRadius);

  setup.getArgs("WIDTH" , width);
  setup.getArgs("HEIGHT", height);

  setup.getArgs("BX", Bx);
  setup.getArgs("BY", By);

  setup.getArgs("DX", dx);
  setup.getArgs("DT", dt);

  setup.getArgs("MIN U", minU);
  setup.getArgs("MAX U", maxU);

  setup.getArgs("FREQUENCY", freq);

  heightScale = 10000.0;

  currentTime = 0;

  std::cout << "MODEL          = " << model << '\n'
            << "STENCIL RADIUS = " << stencilRadius << '\n'
            << "WIDTH          = " << width << '\n'
            << "HEIGHT         = " << height << '\n'
            << "DX             = " << dx << '\n'
            << "DT             = " << dt << '\n'
            << "MIN U          = " << minU << '\n'
            << "MAX U          = " << maxU << '\n'
            << "FREQUENCY      = " << freq << '\n';

  u1.resize(width*height, 0);
  u2.resize(width*height, 0);

  xyz.resize(2*width*height);

  for(int h = 0; h < height; h++){
    for(int w = 0; w < width; w++){
      xyz[2*(h*width + w) + 0] = (w - width/2)*dx;
      xyz[2*(h*width + w) + 1] = (h - height/2)*dx;
    }
  }

  mX = width/2;
  mY = height/2;
}

#if COMPILED_WITH_GL
void update(){
  if( vis.keyIsPressed(' ') ){
    if(!click){
      solve();
      click = 1;
    }
  }
  else
    click = 0;

  vis.placeViewport(0,0);

  if(!vis.isPaused())
    solve();

  drawMesh();
}

const datafloat colorRange[18] = {1.0, 0.0, 0.0,
                              1.0, 1.0, 0.0,
                              0.0, 1.0, 0.0,
                              0.0, 1.0, 1.0,
                              0.0, 0.0, 1.0};

void getColor(datafloat *ret, datafloat scale, datafloat value, datafloat min){
  datafloat c = (value - min)*scale;

  int b = ((int) 5.0*c) - ((int) c/1.0);

  datafloat ratio = 5.0*c - b;

  ret[0] = colorRange[3*b]   + ratio*(colorRange[3*(b+1)]   - colorRange[3*b]);
  ret[1] = colorRange[3*b+1] + ratio*(colorRange[3*(b+1)+1] - colorRange[3*b+1]);
  ret[2] = colorRange[3*b+2] + ratio*(colorRange[3*(b+1)+2] - colorRange[3*b+2]);
}

void getGrayscale(datafloat *ret, datafloat scale, datafloat value, datafloat min){
  datafloat v = (maxU - value)*scale;

  ret[0] = v;
  ret[1] = v;
  ret[2] = v;
}

void drawMesh(){
  glBegin(GL_QUADS);

  datafloat color1[3];
  datafloat color2[3];
  datafloat color3[3];
  datafloat color4[3];

  datafloat ratio;

  if(minU == maxU)
    ratio = 0;
  else
    ratio = 1.0/(maxU - minU);

  for(int h = 0; h < (height-1); h++){
    for(int w = 0; w < (width-1); w++){
#if 0 // Matlab-like colors
      getColor(color1, ratio, u1[w     + (h)*width]    , minU);
      getColor(color2, ratio, u1[(w+1) + (h)*width]    , minU);
      getColor(color3, ratio, u1[(w+1) + ((h+1)*width)], minU);
      getColor(color4, ratio, u1[w     + ((h+1)*width)], minU);
#else // Grayscale
      getGrayscale(color1, ratio, u1[w     + (h)*width]    , minU);
      getGrayscale(color2, ratio, u1[(w+1) + (h)*width]    , minU);
      getGrayscale(color3, ratio, u1[(w+1) + ((h+1)*width)], minU);
      getGrayscale(color4, ratio, u1[w     + ((h+1)*width)], minU);
#endif

      glColor3f(color1[0], color1[1], color1[2]);
      glVertex3f(xyz[2*(w + (h)*width) + 0],
                 xyz[2*(w + (h)*width) + 1],
                 heightScale*u1[w + (h)*width]);

      glColor3f(color2[0], color2[1], color2[2]);
      glVertex3f(xyz[2*((w+1) + (h)*width) + 0],
                 xyz[2*((w+1) + (h)*width) + 1],
                 heightScale*u1[(w+1) + (h)*width]);

      glColor3f(color3[0], color3[1], color3[2]);
      glVertex3f(xyz[2*((w+1) + ((h+1)*width)) + 0],
                 xyz[2*((w+1) + ((h+1)*width)) + 1],
                 heightScale*u1[(w+1) + ((h+1)*width)]);

      glColor3f(color4[0], color4[1], color4[2]);
      glVertex3f(xyz[2*(w + ((h+1)*width)) + 0],
                 xyz[2*(w + ((h+1)*width)) + 1],
                 heightScale*u1[w + ((h+1)*width)]);
    }
  }

  glEnd();
}
#endif

/*
  Width : Number of nodes in the x direction
  Height: Number of nodes in the y direction

  dx: Spacing between nodes

  dt: Timestepping size

  u1, u2: Recordings

  mX/mY: Wavelet source point
 */

occa helper;
occaMemory o_u1, o_u2, o_u3;
occaKernel fd2d;

void setupSolver(){
  helper.setup(model.c_str(), plat, dev);

  o_u1 = helper.createBuffer(u1);
  o_u2 = helper.createBuffer(u1);
  o_u3 = helper.createBuffer(u2);

  size_t dims      = 2;
  size_t local[2]  = {(size_t) Bx, (size_t) By};
  size_t global[2] = {local[0]*( (width  + local[0] - 1)/local[0] ),
                      local[1]*( (height + local[1] - 1)/local[1] )};

  string defs;

  helper.addDefine(defs, "sr"  , stencilRadius);
  helper.addDefine(defs, "w"   , width);
  helper.addDefine(defs, "h"   , height);
  helper.addDefine(defs, "dx"  , dx);
  helper.addDefine(defs, "dt"  , dt);
  helper.addDefine(defs, "freq", freq);
  helper.addDefine(defs, "mX"  , mX);
  helper.addDefine(defs, "mY"  , mY);
  helper.addDefine(defs, "Bx"  , Bx);
  helper.addDefine(defs, "By"  , By);

  if(sizeof(datafloat) == sizeof(float))
     helper.addDefine(defs, "datafloat", "float");
  else
     helper.addDefine(defs, "datafloat", "double");

  fd2d = helper.buildKernel("fd2d.occa", "fd2d", defs);

  fd2d.setThreadArray(global, local, dims);
}

void solve(){
  const int iterations = 20;

  if(currentTime > 1){
    dt = -dt;
  }
  if(currentTime < 0){
    dt = -dt;
  }
  occaEvent ev_start;
  occaEvent ev_end;

  for(int i = 0; i < iterations; i++){
    currentTime += dt;

    fd2d(o_u1, o_u2, o_u3, currentTime);
    o_u2.swap(o_u3);
    o_u1.swap(o_u2);
  }

  const int stencilDiameter = 2*stencilRadius + 1;

  const datafloat timeTakenPerIteration = fd2d.elapsed(); // Just measure 1 iteration
  const datafloat flops = width*height*(stencilDiameter*4 + 5);
  const datafloat bw    = sizeof(datafloat)*width*height*(2*stencilDiameter + 3);
  const datafloat ns    = width*height;

#if 1
  cout << "Time Taken: " << timeTakenPerIteration << '\n';
  cout << "GFLOPS    : " << flops/(1.0e9*timeTakenPerIteration) << '\n';
  cout << "BW        : " <<    bw/(1.0e9*timeTakenPerIteration) << '\n';
  cout << "NS        : " <<    ns/(1.0e6*timeTakenPerIteration) << '\n';
#endif

  totalFlops += flops/(1.0e9*timeTakenPerIteration);
  totalBW    += bw/(1.0e9*timeTakenPerIteration);
  totalNS    += ns/(1.0e6*timeTakenPerIteration);
  totalIters += 1;

  o_u1.toHost(u1);

#if 0
  std::cout << "Time: " << currentTime << '\n';
  std::cout << "Min : " << *std::min_element(&u1[0], &u1.back()) << '\n';
  std::cout << "Max : " << *std::max_element(&u1[0], &u1.back()) << '\n';
#endif
}
