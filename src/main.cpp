/*******************************************************************
 * (c) Copyright 2016-2017 Microsemi SoC Products Group. All rights reserved.
 * Simple ray tracer written in ~200 lines of C++
 * version 1.0 - 01/12/2017 by anton.krug@microsemi.com
 * Using UART at 115200 baudrate with MiV Core running 50MHz
 * References:
 * https://www.ics.uci.edu/~gopi/CS211B/RayTracing%20tutorial.pdf
 * https://tmcw.github.io/literate-raytracer/
 * http://www.3dcpptutorials.sk/index.php?id=16
 * http://en.cppreference.com/w/cpp/language/operators
 * http://en.cppreference.com/w/c/numeric/math
 *******************************************************************/
#include <stdio.h>
#include <float.h>
#include <cmath>
#include "../tests/test-utils.h"

#define WIDTH  80             // Terminal's columns
#define HEIGHT 40             // Terminal's rows
#define SMOOTHNESS 20.5f      // Affects shininess of the sphere

#ifndef ITERATIONS
#define ITERATIONS 1          // How many times the same the frame should be repeated
#endif

#ifndef ROTATION_STEPS
#define ROTATION_STEPS 11.0f  // More steps will make the movement smoother
#endif

#define M_PI_F 3.14159265f    // Single floating point version of M_PI

// https://coderwall.com/p/nb9ngq/better-getting-array-size-in-c
template<size_t SIZE, class T> inline size_t array_size(T (&arr)[SIZE]) {
  return SIZE;
}

struct Shade {
  float value;

  // https://stackoverflow.com/questions/926752/why-should-i-prefer-to-use-member-initialization-list
  Shade(): value(0.0f) {
  }

  Shade(float valueInit): value(valueInit) {
  }

  Shade operator *(float scalar) {
    return Shade(this->value * scalar);
  }

  Shade operator +(Shade secondShade) {
    return Shade(this->value + secondShade.value);
  }

  // Normalize
  Shade operator ~() {
    this->value = fminf(1.0f, fmaxf(0.0f, this->value)); // Make value within range 0 >= value <=1
    return *this;
  }

  // Casting Shade to "char" will trigger equivalent of a toString() method
  // http://en.cppreference.com/w/cpp/language/cast_operator
  operator char() {
    const char shades[] = {' ', '.', ',', '-', ':', '+', '=', '*', '%', '@', '#'};
    return shades[(int) ((~*this).value * (array_size(shades)-1))];
  }
};

class Vector3 {
  float x, y, z;

public:
  Vector3(): x(0.0f), y(0.0f), z(0.0f) {
  }

  Vector3(float xInit, float yInit, float zInit): x(xInit), y(yInit), z(zInit) {
  }

  Vector3 operator +(Vector3 secondVector) {
    return Vector3(x + secondVector.x, y + secondVector.y, z + secondVector.z);
  }

  Vector3 operator -(Vector3 secondVector) {
    return Vector3(x - secondVector.x, y - secondVector.y, z - secondVector.z);
  }

  Vector3 operator *(float scalar) {
    return Vector3(x * scalar, y * scalar, z * scalar);
  }

  Vector3 operator /(float scalar) {
    return Vector3(x / scalar, y / scalar, z / scalar);
  }

  // Dot product
  float operator%(Vector3 secondVector) {
    return (x * secondVector.x + y * secondVector.y + z * secondVector.z);
  }

  // Normalize
  Vector3 operator~() {
    const float magnitude = sqrtf(x * x + y * y + z * z);
    return Vector3(x / magnitude, y / magnitude, z / magnitude);
  }
};

struct Light: public Vector3 {
  Shade shade;
  Light(Vector3 source, Shade shadeInit): Vector3(source), shade(shadeInit) {
  }
};

struct Ray {
  Vector3 source;
  Vector3 direction;

  Ray(Vector3 sourceInit, Vector3 directionInit): source(sourceInit), direction(directionInit) {
  }
};

class Sphere {
  Vector3 center;
  float   radius;

public:
  Sphere(Vector3 centerInit, float radiusInit): center(centerInit), radius(radiusInit) {
  }

  // Get normalized normal vector from sphere's surface point
  Vector3 operator ^ (Vector3 pointOnSurface) {
    return ~(pointOnSurface - this->center);
  }

  bool detectHit(Ray ray, Vector3 &hitPoint) {
    // http://mathforum.org/mathimages/index.php/Ray_Tracing
    // All points at sphere's surface meet this equation:
    // (point - center).(point - center) - radius^2 = 0
    // While any point on the ray's path can be calculated:
    // point = source + direction * distance
    // Source and direction are known, reversing the equations to find if there
    // is a distance on the path which meets sphere's equation.
    Vector3 inRef   = ray.source - this->center;
    float   dotDir  = ray.direction % ray.direction;
    float   temp1   = ray.direction % inRef;
    float   temp2   = (inRef % inRef) - (this->radius * this->radius);
    float   tempAll = (temp1 * temp1) - (dotDir * temp2);

    if (tempAll < 0.0f) return false; // The ray didn't hit the sphere at all

    // 2 points are intersecting the sphere, chose the closest point to the camera
    const float distance = fminf( (-temp1 + sqrtf(tempAll)) / dotDir,
                                  (-temp1 - sqrtf(tempAll)) / dotDir );

    hitPoint = ray.source + ray.direction * distance;
    return true;
  }
};

Shade calculateShadeOfTheRay(Ray ray, Light light) {
  Sphere  sphere(Vector3(0.0f, 0.0f, HEIGHT), HEIGHT/2.0f);
  Shade   ambient = 0.1f; // implicit vs http://en.cppreference.com/w/cpp/language/explicit
  Shade   shadeOfTheRay;
  Vector3 hitPoint;

  if (sphere.detectHit(ray, hitPoint)) {
    // The ray hit the sphere, let's find the bounce angle and shade it
    // https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
    Vector3 hitNormal    = sphere ^ hitPoint;
    Vector3 hitReflected = ray.direction - (hitNormal * 2.0f * (ray.direction % hitNormal));
    Vector3 hitLight     = ~(light - hitPoint);
    float   diffuse      = fmaxf(0.0f, hitLight % hitNormal);    // How similar are they?
    float   specular     = fmaxf(0.0f, hitLight % hitReflected); // How similar are they?

    // diffuse  = similarity (dot product) of hitLight and hitNormal
    // specular = similarity (dot product) of hitLight and hitReflected
    // https://youtu.be/KDHuWxy53uM
    // And use the diffuse and specular only when they are positive
    // simplifiedPhongShading = specular + diffuse + ambient
    // https://en.wikipedia.org/wiki/Phong_reflection_model
    shadeOfTheRay = light.shade * powf(specular, SMOOTHNESS) + light.shade * diffuse + ambient;
  }
  testAddToChecksumFloat(shadeOfTheRay.value); // Calculating checksums for automated tests
  return shadeOfTheRay;
}

int main() {
  for (float zoom = 12.0f; zoom <= 32.0f; zoom+=10.0f) {
    for (float lightRotate = 0.0f; lightRotate < 2.0f * M_PI_F; lightRotate += M_PI_F / ROTATION_STEPS) {
      for (int iteration = 0; iteration < ITERATIONS; iteration++) {
        printf("Zoom=%1.1f, lighRotate=%3.1f\n", logf(zoom), (180.0f * lightRotate) / M_PI_F);
        Light light(Vector3(2.0f * WIDTH  *  cosf(lightRotate),
                            3.0f * HEIGHT * (sinf(lightRotate)-0.5f), -100.0f), Shade(0.7f));
        // Calculate ray for each pixel on the scene
        for (int y = 2; y < HEIGHT; y++) { // dedicate few lines for top/bottom margins
          for (int x = 0; x < WIDTH; x++) {
            Ray rayForThisPixel( Vector3(0.0f,            0.0f,             0.0f),
                                ~Vector3(x - (WIDTH / 2), y - (HEIGHT / 2), zoom));
            putchar(calculateShadeOfTheRay(rayForThisPixel, light));
          }
          printf("\n"); // print break after each row
        }
#ifdef SERIAL_TERMINAL_ANIMATION
      printf("\033[0;0H"); // http://www.termsys.demon.co.uk/vtansi.htm
#endif
      }
    }
  }

  testValidate(ITERATIONS, 1); // if GDB testing is enabled, it will validate the checksums

#ifndef EXIT_FROM_THE_INFINITE_LOOP
  while(1);
#endif
  return 0;
}
