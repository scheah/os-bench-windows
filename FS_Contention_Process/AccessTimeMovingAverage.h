// Utility to provide support for averaging timestamps
#include <windows.h>

template<unsigned int Size>
class AccessTimeMovingAverage
{
public:
	// Constructor
	AccessTimeMovingAverage();

	// Take a new start time stamp
	void AddNewStartTimeStamp();

	// Take a new end time stamp and adds the elapsed time to the buffer.
	void AddNewEndTimeStamp();

	// Output the average
	ULONGLONG Average();

private:
	void AddSample(ULONGLONG elapsedTime);
	unsigned int m_sampleCnt;
	unsigned int m_bufferHead;
	ULONGLONG m_buffer[Size];
	LARGE_INTEGER m_startTimeStamp;
};

// Constructor
template <unsigned int Size>
AccessTimeMovingAverage<Size>::AccessTimeMovingAverage()
{
	m_sampleCnt = 0;
	m_bufferHead = 0;
}

// AddNewStartTimeStamp
template <unsigned int Size>
void AccessTimeMovingAverage<Size>::AddNewStartTimeStamp()
{
	QueryPerformanceCounter(&m_startTimeStamp);
}

// AddNewEndTimeStamp
template <unsigned int Size>
void AccessTimeMovingAverage<Size>::AddNewEndTimeStamp()
{
	LARGE_INTEGER endTimeStamp;
	LARGE_INTEGER frequency;
	QueryPerformanceCounter(&endTimeStamp);
	QueryPerformanceFrequency(&frequency);
	AddSample(((ULONGLONG)endTimeStamp.QuadPart - (ULONGLONG)m_startTimeStamp.QuadPart) * 1000000000U / frequency.QuadPart);
}

// AddSample
template <unsigned int Size>
void AccessTimeMovingAverage<Size>::AddSample(ULONGLONG elapsedTime)
{
	// Add the elapsed time to the circular buffer. When the index reaches the 
	// specified size the buffer rolls over.
	m_buffer[m_bufferHead] = elapsedTime;

	if (++m_bufferHead >= Size)
		m_bufferHead = 0;

	if (++m_sampleCnt >= Size)
		m_sampleCnt = Size;
}

// Average
template <unsigned int Size>
ULONGLONG AccessTimeMovingAverage<Size>::Average()
{
	ULONGLONG outputAvg = 0;
	int bufferIdx = m_bufferHead;
	for (unsigned int i = 1; i <= m_sampleCnt; i++)
	{
		// Index throught the circular buffer and average the samples.
		if (bufferIdx == 0)
			bufferIdx = Size - 1;
		else
			bufferIdx--;
		if (m_buffer[bufferIdx] > outputAvg)
			outputAvg += (m_buffer[bufferIdx] - outputAvg) / i;
		else
			outputAvg -= (outputAvg - m_buffer[bufferIdx]) / i;
	}
	return outputAvg;
}