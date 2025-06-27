/*
 * $Id: debug.hpp,v 1.0.0 2024/12/26 20:55:23 Jaya Wikrama Exp $
 *
 * Copyright (c) 2024 Jaya Wikrama
 * jayawikrama89@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file
 * @brief Class for customize debug output.
 *
 * This file contains functions that can be used to print custom debug message
 * on terminal and automatically save last 1000 debug line in internal object. 
 *
 * @version 1.0.0
 * @date 2024-12-26
 * @author Jaya Wikrama
 */

#include <string>
#include <vector>
#include <pthread.h>

#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

class Debug {
    private:
        size_t maxLineLogs;
        std::vector <std::string> confidential;
        std::vector <std::string> history;
        pthread_mutex_t mutex;
    
    public:
        typedef enum _LogType_t {
            INFO,
            WARNING,
            ERROR,
            CRITICAL
        } LogType_t;

        Debug();

        Debug(size_t maxLineLogs);

        ~Debug();

        std::string logTypeToString(LogType_t type) const;

        void setConfidential(const std::string &confidential);

        void log(LogType_t type, const char* functionName, const char* format, ...);

        std::string getLogHistory();

        void clearLogHistory();
};

#endif /* __DEBUG_HPP__ */