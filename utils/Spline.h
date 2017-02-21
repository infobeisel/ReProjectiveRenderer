// Spline.h

#ifndef _FRAMEWORK_SPLINE_H_
#define _FRAMEWORK_SPLINE_H_
#include <QVector3D>



namespace Framework
{
	namespace Math
	{
		class Spline
		{
		public:

			static const int InvalidSegmentIndexBefore = -1;
			static const int InvalidSegmentIndexAfter = -2;

			virtual ~Spline(){}

            virtual void AddPosition(const QVector3D& position, float time) = 0;

			virtual void FinalizeCreation(){ }

			virtual void SetStartTimeToZero() = 0;

			virtual float GetStartTime() const = 0;
			virtual float GetEndTime() const = 0;

			virtual unsigned GetCountSegments() const = 0;
			virtual float GetSegmentStartTime(int index) const = 0;
			virtual float GetSegmentEndTime(int index) const = 0;
            virtual void GetSegmentStartData(int index, QVector3D& position, float& time) const = 0;
            virtual void GetSegmentEndData(int index, QVector3D& position, float& time) const = 0;

            virtual QVector3D GetPosition(float time, int* segmentPtr = nullptr) const = 0;
			virtual void GetTangentAndNormal(float time,
                QVector3D& tangent, QVector3D& normal) const = 0;

			virtual float GetTotalLength(unsigned countSamplePoints = 1000)
			{
				float time = GetStartTime();
				float increment = (GetEndTime() - time)
					/ static_cast<float>(countSamplePoints - 1);
                QVector3D p0 = GetPosition(time);
                QVector3D p1;
				float length = 0.0f;
				for (unsigned i = 1; i < countSamplePoints; i++)
				{
					time += increment;
					p1 = GetPosition(time);
                    length += (p1 - p0).length();
					p0 = p1;
				}
				return length;
			}


		private: // Boost serialization.

			
		};
	}
}


#endif
