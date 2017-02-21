// CatmullRomSpline.h

#ifndef _FRAMEWORK_CATMULLROMSPLINE_H_
#define _FRAMEWORK_CATMULLROMSPLINE_H_

#include <utils/Spline.h>


#include <vector>

namespace Framework
{
	namespace Math
	{
		class CatmullRomSpline : public Spline
		{
			struct PointData
			{
                QVector3D Position;
				float Time;

			private: // Boost serialization.

			};

			std::vector<PointData> m_Points;

            void AddPositionToFront(const QVector3D& position, float time);

			unsigned GetStartPointIndex(float time) const;

		public:

			CatmullRomSpline();

            void AddPosition(const QVector3D& position, float time);

			void FinalizeCreation();

			void SetStartTimeToZero();

			float GetStartTime() const;
			float GetEndTime() const;

			unsigned GetCountControlPoints() const;
			unsigned GetCountSegments() const;
			float GetSegmentStartTime(int index) const;
			float GetSegmentEndTime(int index) const;
            void GetSegmentStartData(int index, QVector3D& position, float& time) const;
            void GetSegmentEndData(int index, QVector3D& position, float& time) const;

            QVector3D GetPosition(float time, int* segmentPtr = nullptr) const;
			void GetTangentAndNormal(float time,
                QVector3D& tangent, QVector3D& normal) const;

			void MakeCircular();

			void DecreaseControlPointNumber(unsigned countNewControlPoints);

			void RemovePositionDuplicates();
			void RemoveControlPointRange(unsigned startIndex, unsigned endIndex);

		private: // Boost serialization.

			
		};

		
	}
}

#endif
