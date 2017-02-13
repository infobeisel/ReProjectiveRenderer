#include "renderer.h"
Renderer::Renderer(){}
Renderer::~Renderer(){}

void Renderer::draw(Scene* s) {}
void Renderer::initialize() {}
void Renderer::setNormalizedEyeSeparation(float e) {}
float Renderer::getNormalizedEyeSeparation() {return 0.0f;}

void Renderer::setProjectionMatrix(QMatrix4x4 p) {}
void Renderer::setViewMatrix(QMatrix4x4 v) {}
void Renderer::setCameraPosition(QVector3D p) {}
void Renderer::setCameraOrientation(QQuaternion q) {}
QOpenGLShaderProgram* Renderer::getShaderProgram() {return &shaderProgram;}