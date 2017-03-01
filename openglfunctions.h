#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H
#include <typeinfo>
#define GL OpenGLFunctions::instance(typeid(this).name())

#include <QOpenGLFunctions_4_1_Core>
#include <QDebug>
#include <sstream>
class OpenGLFunctions : public QOpenGLFunctions_4_1_Core
{

public:

    void CheckGLError(std::string tClassName)
    {
        auto errorCode = this->glGetError();

        if (errorCode != GL_NO_ERROR)
        {
            std::stringstream ss;
            std::string sourceString, typeString, severityString, errorIdString;

            //GetSourceString(source, sourceString);
            //GetTypeString(type, typeString);
            //GetSeverityString(severity, severityString);
            GetErrorIdString(errorCode, errorIdString);
            qDebug() << "An OpenGL error has been occured: " << QString::fromStdString(tClassName) << " " <<QString::fromStdString(errorIdString) << "\n";
        }
    }

    static OpenGLFunctions& instance(std::string className) {
        static OpenGLFunctions instance;
        instance.CheckGLError(className);
        return instance;
    }





    inline void GetSourceString(GLenum source, std::string& str)
    {
        switch (source)
        {
        case GL_DEBUG_CATEGORY_API_ERROR_AMD:
        case GL_DEBUG_SOURCE_API: str = "API"; break;
        case GL_DEBUG_CATEGORY_APPLICATION_AMD:
        case GL_DEBUG_SOURCE_APPLICATION: str = "Application"; break;
        case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: str = "Window System"; break;
        case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
        case GL_DEBUG_SOURCE_SHADER_COMPILER: str = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: str = "Third Party"; break;
        case GL_DEBUG_CATEGORY_OTHER_AMD:
        case GL_DEBUG_SOURCE_OTHER: str = "Other"; break;
        default: str = "Unknown"; break;
        }
    }

    inline void GetTypeString(GLenum type, std::string& str)
    {
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR: str = "Error"; break;
        case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: str = "Deprecated Behavior"; break;
        case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: str = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB: str = "Portability"; break;
        case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
        case GL_DEBUG_TYPE_PERFORMANCE: str = "Performance"; break;
        case GL_DEBUG_CATEGORY_OTHER_AMD:
        case GL_DEBUG_TYPE_OTHER: str = "Other"; break;
        default: str = "Unknown"; break;
        }
    }

    inline void GetSeverityString(GLenum severity, std::string& str)
    {
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH: str = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: str = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW: str = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: str = "Notification"; break;
        default: str = "Unknown"; break;
        }
    }

    inline void GetErrorIdString(GLuint id, std::string& str)
    {
        switch (id)
        {
        case GL_INVALID_VALUE: str = "Invalid value"; break;
        case GL_INVALID_ENUM: str = "Invalid enum"; break;
        case GL_INVALID_OPERATION: str = "Invalid operation"; break;
        case GL_STACK_OVERFLOW: str = "Stack overflow"; break;
        case GL_STACK_UNDERFLOW: str = "Stack underflow"; break;
        case GL_OUT_OF_MEMORY: str = "Out of memory"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: str = "Invalid framebuffer operation"; break;
        case GL_CONTEXT_LOST: str = "Context lost"; break;
        case GL_TABLE_TOO_LARGE: str = "Table too large"; break;
        default: std::stringstream ss; ss << "Unknow: " << id; str = ss.str(); break;
        }
    }





    //delete assign operator and copy constructor
    OpenGLFunctions(OpenGLFunctions const&) = delete;
    void operator=(OpenGLFunctions const&)  = delete;

private:
    OpenGLFunctions()
    {
        qDebug() << "initialize GL Functions";
        initializeOpenGLFunctions();
    }
};

#endif // OPENGLFUNCTIONS_H
