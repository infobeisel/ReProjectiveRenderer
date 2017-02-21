// CatmullRomSpline.cpp


#include <utils/CatmullRomSpline.h>


using namespace Framework::Math;

CatmullRomSpline::CatmullRomSpline()
{
}

void CatmullRomSpline::AddPositionToFront(const QVector3D& position, float time)
{
#ifdef _DEBUG
	if (m_Points.size() > 0 && time > m_Points[0].Time)
	{
        qDebug() << "First time value is more than the next one in Catmull-Rom spline.";

	}
#endif

	m_Points.insert(m_Points.begin(), PointData());
	m_Points[0].Position = position;
	m_Points[0].Time = time;
}

void CatmullRomSpline::AddPosition(const QVector3D& position, float time)
{
#ifdef _DEBUG
	if (m_Points.size() > 0 && time < m_Points.back().Time)
	{
        qDebug() << "Next time value is less than the previous one in Catmull-Rom spline.";
	}
#endif

	m_Points.push_back(PointData());
	auto& element = m_Points.back();
	element.Position = position;
	element.Time = time;
}

void CatmullRomSpline::FinalizeCreation()
{
#ifdef _DEBUG
	if (m_Points.size() < 2)
	{
        qDebug() << "Catmull-Rom spline has less than 2 points: cannot create start and end points.";
	}
#endif

    QVector3D startPosition = m_Points[0].Position;
    QVector3D endPosition = m_Points.back().Position;

	AddPositionToFront(startPosition, m_Points[0].Time);
	AddPosition(endPosition, m_Points.back().Time);
}

void CatmullRomSpline::SetStartTimeToZero()
{
	float startTime = m_Points[1].Time;
	for (size_t i = 0; i < m_Points.size(); i++)
	{
		m_Points[i].Time -= startTime;
	}
}

float CatmullRomSpline::GetStartTime() const
{
    Q_ASSERT(m_Points.size() >= 4);

	return m_Points[1].Time;
}

float CatmullRomSpline::GetEndTime() const
{
    Q_ASSERT(m_Points.size() >= 4);

	return m_Points[m_Points.size() - 2].Time;
}

unsigned CatmullRomSpline::GetCountControlPoints() const
{
	return static_cast<unsigned>(m_Points.size());
}

unsigned CatmullRomSpline::GetCountSegments() const
{
	return static_cast<unsigned>(m_Points.size() - 3);
}

float CatmullRomSpline::GetSegmentStartTime(int index) const
{
	return m_Points[index + 1].Time;
}

float CatmullRomSpline::GetSegmentEndTime(int index) const
{
	return m_Points[index + 2].Time;
}

void CatmullRomSpline::GetSegmentStartData(int index, QVector3D& position, float& time) const
{
	int pointIndex = index + 1;
	position = m_Points[pointIndex].Position;
	time = m_Points[pointIndex].Time;
}

void CatmullRomSpline::GetSegmentEndData(int index, QVector3D& position, float& time) const
{
	int pointIndex = index + 2;
	position = m_Points[pointIndex].Position;
	time = m_Points[pointIndex].Time;
}

QVector3D CatmullRomSpline::GetPosition(float time, int* segmentPtr) const
{
#ifdef _DEBUG
	if (m_Points.size() < 4)
	{
        qDebug() <<"Catmull-Rom spline has less than 4 points: cannot get a position.";
	}
#endif

	float startTime = GetStartTime();
	float endTime = GetEndTime();

    QVector3D position;
	if (time < startTime)
	{
		// Returning the exact position.

		position = m_Points[0].Position;
		if (segmentPtr != nullptr)
		{
			*segmentPtr = InvalidSegmentIndexBefore;
		}
	}
	else if (time > endTime)
	{
		// Returning the exact position.

		position = m_Points.back().Position;
		if (segmentPtr != nullptr)
		{
			*segmentPtr = InvalidSegmentIndexAfter;
		}
	}
	else
	{
		unsigned index = GetStartPointIndex(time);

		float t_1 = m_Points[index - 1].Time;
		float t0 = m_Points[index].Time;
		float t1 = m_Points[index + 1].Time;
		float t2 = m_Points[index + 2].Time;

		float tDiff = t1 - t0;
		float t = (tDiff > 0.0f ? (time - t0) / tDiff : 0.0f);
		float t_2 = t * t;
		float t_3 = t_2 * t;
		float h00 = 2.0f * t_3 - 3.0f * t_2 + 1;
		float h10 = t_3 - 2.0f * t_2 + t;
		float h01 = -2.0f * t_3 + 3.0f * t_2;
		float h11 = t_3 - t_2;

		const auto& p_1 = m_Points[index - 1].Position;
		const auto& p0 = m_Points[index].Position;
		const auto& p1 = m_Points[index + 1].Position;
		const auto& p2 = m_Points[index + 2].Position;
	
        QVector3D m0 = (p1 - p_1) / (t1 - t_1);
        QVector3D m1 = (p2 - p0) / (t2 - t0);

		position = h00 * p0 + h10 * tDiff * m0 + h01 * p1 + h11 * tDiff * m1;

		if (segmentPtr != nullptr)
		{
			*segmentPtr = index - 1;
		}
	}
	return position;
}

void CatmullRomSpline::GetTangentAndNormal(float time, QVector3D& tangent, QVector3D& normal) const
{
#ifdef _DEBUG
	if (m_Points.size() < 4)
	{
        qDebug() <<"Catmull-Rom spline has less than 4 points: cannot get a position.";
	}
#endif

	float startTime = GetStartTime();
	float endTime = GetEndTime();

	if (time < startTime)
	{
		time = startTime;
	}
	else if (time > endTime)
	{
		time = endTime;
	}

	unsigned index = GetStartPointIndex(time);

	float t_1 = m_Points[index - 1].Time;
	float t0 = m_Points[index].Time;
	float t1 = m_Points[index + 1].Time;
	float t2 = m_Points[index + 2].Time;

	const auto& p_1 = m_Points[index - 1].Position;
	const auto& p0 = m_Points[index].Position;
	const auto& p1 = m_Points[index + 1].Position;
	const auto& p2 = m_Points[index + 2].Position;

    QVector3D m0 = (p1 - p_1) / (t1 - t_1);
    QVector3D m1 = (p2 - p0) / (t2 - t0);

	float tDiff = t1 - t0;
	float t = (tDiff > 0.0f ? (time - t0) / tDiff : 0.0f);
	float t_2 = t * t;

	float th00 = 6.0f * (t_2 - t);
	float th10 = (3.0f * t_2 - 4.0f * t + 1.0f);
	float th01 = 6.0f * (t - t_2);
	float th11 = (3.0f * t_2 - 2.0f * t);

	float nh00 = 12.0f * t - 6.0f;
	float nh10 = 6.0f * t - 4.0f;
	float nh01 = 6.0f - 12.0f * t;
	float nh11 = 6.0f * t - 2.0f;

	tangent = (th00 * p0 + th10 * tDiff * m0 + th01 * p1 + th11 * tDiff * m1) / tDiff;
	normal = (nh00 * p0 + nh10 * tDiff * m0 + nh01 * p1 + nh11 * tDiff * m1) / (tDiff * tDiff);
}

void CatmullRomSpline::MakeCircular()
{
	if (m_Points.size() > 2 && m_Points[0].Time == m_Points[1].Time)
	{
		m_Points.erase(m_Points.begin());
	}
	if (m_Points.size() > 2 && m_Points.back().Time == (++m_Points.rbegin())->Time)
	{
		m_Points.erase(--m_Points.end());
	}

#ifdef _DEBUG
		if (m_Points.size() < 2)
		{
            qDebug() << "Catmull-Rom spline has less than 2 points: cannot make it circular.";
		}
#endif

	auto& data0 = m_Points[0];
	float time0 = data0.Time;
	auto position0 = data0.Position;

	auto& data1 = m_Points[1];
	float time1 = data1.Time;
	auto position1 = data1.Position;

	auto& dataN = m_Points.back();
	float timeN = dataN.Time;
	auto positionN = dataN.Position;

	float time0_1 = time1 - time0;
    float timeN_0 = time0_1 * (position0 - positionN).length() / (position1 - position0).length();
	float startNtime = time0 - timeN_0;
	float finish0Time = timeN + timeN_0;
	float finish1Time = finish0Time + time0_1;

	AddPositionToFront(positionN, startNtime);
	AddPosition(position0, finish0Time);
	AddPosition(position1, finish1Time);

	SetStartTimeToZero();
}

unsigned CatmullRomSpline::GetStartPointIndex(float time) const
{
    Q_ASSERT(m_Points.size() >= 4 && time >= m_Points[1].Time && time <= m_Points[m_Points.size() - 2].Time);

	// Searching start point with binary search.
	unsigned startIndex = 1;
	unsigned endIndex = static_cast<unsigned>(m_Points.size()) - 3;
	while (endIndex - startIndex > 1)
	{
        Q_ASSERT(m_Points[startIndex].Time <= time && m_Points[endIndex + 1].Time >= time);

		unsigned newIndex = (startIndex + endIndex) / 2;
		if (time <= m_Points[newIndex].Time)
		{
			endIndex = newIndex;
		}
		else
		{
			startIndex = newIndex;
		}
	}

	if (startIndex < endIndex && time >= m_Points[endIndex].Time)
	{
		startIndex = endIndex;
	}

    Q_ASSERT(startIndex >= 1 && startIndex < static_cast<unsigned>(m_Points.size()) - 2
		&& m_Points[startIndex].Time <= time && m_Points[startIndex + 1].Time >= time);

	return startIndex;
}

void CatmullRomSpline::DecreaseControlPointNumber(unsigned countNewControlPoints)
{
	float time = GetStartTime();
	float timeIncrement = (GetEndTime() - time) / (countNewControlPoints - 1);
	std::vector<PointData> newPoints;
	for (unsigned i = 0; i < countNewControlPoints; i++, time += timeIncrement)
	{
		newPoints.push_back({ GetPosition(time), time });
	}
	m_Points = std::move(newPoints);

	FinalizeCreation();
}

void CatmullRomSpline::RemovePositionDuplicates()
{
	std::vector<unsigned> indices;

	unsigned countPoints = static_cast<unsigned>(m_Points.size());
	for (unsigned i = 1; i < countPoints; i++)
	{
		if (m_Points[i].Position != m_Points[i - 1].Position)
		{
			indices.push_back(i);
		}
	}

	unsigned countIndices = static_cast<unsigned>(indices.size());
	for (size_t i = 0; i < countIndices; i++)
	{
		m_Points[i + 1] = m_Points[indices[i]];
	}
}

void CatmullRomSpline::RemoveControlPointRange(unsigned startIndex, unsigned endIndex)
{
	unsigned size = static_cast<unsigned>(m_Points.size());
	auto newSize = size - endIndex + startIndex;
	auto countElementsToCopy = size - endIndex;

	if (countElementsToCopy > 0)
	{
		memcpy(&m_Points[startIndex], &m_Points[endIndex], countElementsToCopy * sizeof(PointData));
	}

	m_Points.resize(newSize);
}
