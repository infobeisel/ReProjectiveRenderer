#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <fstream>
#include <sstream>
#include <QString>

class Configuration
{

public:
    float CameraTourDurationInSeconds;
    float CameraTourStartOffsetInSeconds;
    float NearClippingPlane;
    float FoV;
    float FarClippingPlane;
    float MaxEyeSeparation;
    int RenderType;
    int WindowWidth;
    int WindowHeight;

    float SpecularError;
    float LodError;
    float DepthThreshold;


    static Configuration& instance() {
        static Configuration instance;
        return instance;
    }

    void load(QString path) {

        qDebug() << "load config file: " <<path;
        std::ifstream in(path.toStdString());
        std::string contents((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
        QString c = QString::fromStdString(contents);

        QStringList lines = c.split("\n");
        lines.removeLast();

        foreach(auto l , lines) {
            l = l.simplified();
            l = l.remove(QChar(' '), Qt::CaseInsensitive);
            if(!l.startsWith("#") && !l.isEmpty()) {
                QStringList parts = l.split("=");
                bool ok = true;
                if(parts[0] == "CameraTourDurationInSeconds")
                    CameraTourDurationInSeconds = parts[1].toFloat(&ok);
                else if(parts[0] ==  "CameraTourStartOffsetInSeconds")
                    CameraTourStartOffsetInSeconds = parts[1].toFloat(&ok);
                else if(parts[0] ==  "NearClippingPlane")
                    NearClippingPlane = parts[1].toFloat(&ok);
                else if(parts[0] ==  "FoV")
                    FoV = parts[1].toFloat(&ok);
                else if(parts[0] ==  "FarClippingPlane")
                    FarClippingPlane = parts[1].toFloat(&ok);
                else if(parts[0] ==  "NearClippingPlane")
                    NearClippingPlane = parts[1].toFloat(&ok);
                else if(parts[0] ==  "MaxEyeSeparation")
                    MaxEyeSeparation = parts[1].toFloat(&ok);
                else if(parts[0] ==  "RenderType")
                    RenderType = parts[1].toInt(&ok);
                else if(parts[0] ==  "WindowWidth")
                    WindowWidth = parts[1].toInt(&ok);
                else if(parts[0] ==  "WindowHeight")
                    WindowHeight = parts[1].toInt(&ok);
                else if(parts[0] ==  "SpecularError")
                    SpecularError = parts[1].toFloat(&ok);
                else if(parts[0] ==  "LodError")
                    LodError = parts[1].toFloat(&ok);
                else if(parts[0] ==  "DepthThreshold")
                    DepthThreshold = parts[1].toFloat(&ok);
                qDebug() << "Configuration: read " << parts[1];
                if(!ok) {
                   qWarning() << "configuration value " << parts[0] << " could not be casted to the right type";
                }
            }

        }
    }




    //delete assign operator and copy constructor
    Configuration(Configuration const&) = delete;
    void operator=(Configuration const&)  = delete;

private:
    Configuration()
    {
        CameraTourDurationInSeconds
        = CameraTourStartOffsetInSeconds
        = NearClippingPlane
        = FoV
        = FarClippingPlane
        = MaxEyeSeparation
        = SpecularError
        = LodError
        = DepthThreshold = 0.0;

        RenderType = WindowWidth= WindowHeight = 0;


    }
};



#endif // CONFIGURATION_H
