
#include"Time_t.h"
#include <stdio.h>

  /*
      usage: start -> time_stamp(0, NULL)
             end   -> time_stamp(1, "name")
  */
#ifdef WIN32 
    #include <windows.h>
    void time_stamp(int is_end, const char *timeName)
    {
        static LARGE_INTEGER s_tm_freq, s_tm_start;
	    LARGE_INTEGER s_tm_end;
	    if(0==is_end)
	    {
	        QueryPerformanceFrequency(&s_tm_freq);	
	        QueryPerformanceCounter(&s_tm_start);
	    }
    	else
	    {
	        QueryPerformanceCounter(&s_tm_end);
	        printf("%s : %f ms \n",timeName, (double)(s_tm_end.QuadPart-s_tm_start.QuadPart)/s_tm_freq.QuadPart*1000);
	    }
    }
#else
    #include <sys/time.h>
    #include <unistd.h>
    void  time_stamp(int is_end, const char *timeName)  
    {
        static  struct timeval  start, end;
       	if(0==is_end)
	    {
	        gettimeofday(&start, NULL);
	    }
	    else
	    {
	        gettimeofday(&end, NULL);
	        printf("%s : %f ms \n",timeName, (double)(1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec))/1000);
	    }
    }

#endif
//// time debug
