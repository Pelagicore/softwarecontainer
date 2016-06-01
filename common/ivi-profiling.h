
#pragma once 

#include <ivi-logging.h>
#include <iostream>
#include <time.h>
#include <iomanip>

typedef logging::DefaultLogContext LogContext;

// Profile a single point in time. 
#ifdef PROFILING_ENABLED
#define profilepoint(s) \
    PProfiler::point(s); 
#else // PROFILING_ENABLED
#define profilepoint(s)
#endif

// If profiling is enabled, create an instance of Profiler. This will be 
// destroyed when leaving scope 
#ifdef PROFILING_ENABLED
#define profilefunction(s) \
    PProfiler prof(s);
#else // PROFILING_ENABLED
#define profilefunction(s)
#endif

class PProfiler {
public:
    LOG_DECLARE_CLASS_CONTEXT("PPRO", "Profiler context");

    PProfiler(std::string s) { 
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_time1); 
        log_error() << "profiler " << s << " start " << format(m_time1);
        m_s = s;
    };

    ~PProfiler() {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_time2); 
        timespec d = diff(m_time1, m_time2);
        log_error() << "profiler " << m_s << " end " << format(m_time2) << " " << format(d); 
    };

    static void point(std::string s)
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t); 
        log_warn() << "profilerPoint " << s << " " << PProfiler::format(t);
    }

    static std::string format(timespec t)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(1) << t.tv_sec << "." << std::setw(9) << t.tv_nsec;
        return ss.str();
    }

    timespec diff(timespec start, timespec end)
    {
        timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0) {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
    }

private:
    std::string m_s;
    timespec m_time1, m_time2;
};
