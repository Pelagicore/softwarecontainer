/*
 * Copyright (C) 2016-2017 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */



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
        log_warn() << "profiler " << s << " start " << format(m_time1);
        m_s = s;
    };

    ~PProfiler() {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_time2); 
        timespec d = diff(m_time1, m_time2);
        log_warn() << "profiler " << m_s << " end " << format(m_time2) << " " << format(d);
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
