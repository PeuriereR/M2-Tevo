#include "camera.h"

Point Camera::getPosition(){
    return position;
}

float Camera::getPitch(){
    return pitch;
}

float Camera::getYaw(){
    return yaw;
}

float Camera::getRoll(){
    return roll;
}


Vector Camera::getUpVector(){
    return RotationY(-angleAroundCenter)(RotationX(-pitch)(Vector(0,1,0)));
}

void Camera::moveSide(float step){
    center.x += step;
    position.x += step;
}

void Camera::moveFront(float step){
    center.z += step;
    position.z += step;
}


void Camera::calculateCameraPosition(float hD, float vD){
    // a changer 0 0 0 pos du centre=
    float offsetX = hD * sin(angleAroundCenter * M_PI/180);
    float offsetZ = hD * cos(angleAroundCenter * M_PI/180);
    position.x = center.x - offsetX;
    position.y = center.y + vD;
    position.z = center.z + offsetZ;
}

void Camera::move(float mouse1value, float mouse2value,float mouse3value){
    // a changer
    calculateZoom(mouse2value);
    calculatePitch(mouse1value);
    calculateAngleAroundCenter(mouse3value);

    float horizontalDistance = calculationHorizontalDistance();
    float verticalDistance = calculationVerticalDistance();
    calculateCameraPosition(horizontalDistance,verticalDistance);
    yaw = 180 - angleAroundCenter;
}

void Camera::calculateZoom(float mouse2value){
    if (mouse2value > 0 && distanceFromCenter < 10 ) return;
    distanceFromCenter -= mouse2value;
}

void Camera::calculatePitch(float mouse1value){
    pitch -= mouse1value * 0.1;
}

void Camera::calculateAngleAroundCenter(float mouse3value){
    angleAroundCenter -= mouse3value * 0.3;
}

float Camera::calculationHorizontalDistance(){
    return distanceFromCenter * cos(pitch*M_PI/180);
}

float Camera::calculationVerticalDistance(){
    return distanceFromCenter * sin(pitch*M_PI/180);
}
void Camera::invertPitch(){
    pitch = -pitch;
    // move(0,0,0);
}
