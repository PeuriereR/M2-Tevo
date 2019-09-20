#include "mat.h"
#include <math.h>

#define UP 0
#define SIDE 1

class Camera{
public:
    Point position;
    Point center;

    float pitch;
    float yaw; // not used
    float roll; // not used

    float distanceFromCenter;
    float angleAroundCenter;

    Camera():position(Point(0,0,50)),center(Point(0,0,0)),pitch(0),yaw(0),roll(0),distanceFromCenter(50),angleAroundCenter(0){}
    Camera(float _pitch, float _yaw, float _roll):position(Point(120,0,-120)),center(Point(0,0,0)),pitch(_pitch),yaw(_yaw),roll(_roll),distanceFromCenter(50),angleAroundCenter(0){}

    void move(float,float,float);

    Point getPosition();
    float getPitch();
    float getYaw();
    float getRoll();
    Vector getUpVector();

    void moveSide(float step);
    void moveFront(float step);

    void calculateZoom(float mouse2value); // wheel value
    void calculatePitch(float mouse1value); //my value
    void calculateAngleAroundCenter(float mouse3value); // mx value

    float calculationHorizontalDistance();
    float calculationVerticalDistance();

    void calculateCameraPosition(float hD, float vD);

    void invertPitch();
};
