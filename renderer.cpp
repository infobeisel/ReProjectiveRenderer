#include "renderer.h"
Renderer::Renderer(){}
Renderer::~Renderer(){}

void Renderer::draw(Scene* s) {}
void Renderer::initialize() {}
void Renderer::initialize(int w, int h) {}
void Renderer::setNormalizedEyeSeparation(float e) {}
void Renderer::toggleDebugMode() {}
float Renderer::getNormalizedEyeSeparation() {return 0.0f;}
QString Renderer::configTags() {return "";}

void Renderer::setProjectionMatrix(float fov,float aspect, float near, float far) {}

void Renderer::setViewMatrix(QMatrix4x4 v) {}
void Renderer::setCameraPosition(QVector3D p) {}
void Renderer::setCameraOrientation(QQuaternion q) {}
void Renderer::toggleZPrepass(){}

QOpenGLShaderProgram* Renderer::getShaderProgram() {return &shaderProgram;}
