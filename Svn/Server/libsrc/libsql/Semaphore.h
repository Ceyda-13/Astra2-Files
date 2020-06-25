#ifndef __INC_METIN_II_SEMAPHORE_H__
#define __INC_METIN_II_SEMAPHORE_H__

#ifndef _WIN32
#include <semaphore.h>
#else

#endif

class CSemaphore
{
	private:
#ifndef _WIN32
		sem_t *	m_hSem;
#else
		HANDLE m_hSem;
#endif

	public:
		CSemaphore();
		~CSemaphore();

		int32_t	Initialize();
		void	Clear();
		void	Destroy();
		int32_t	Wait();
		int32_t	Release(int32_t count = 1);
};
#endif
