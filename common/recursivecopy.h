#pragma once
#include "softwarecontainer-common.h"


/**
 * @brief The RecursiveCopy class is a singleton class used to copy files recursively from a
 * source to a destination.
 */
class RecursiveCopy {
    LOG_DECLARE_CLASS_CONTEXT("RECO", "Recursive Copy");
public:
    /**
     * @brief getInstance Gets the RecursiveCopy instance.
     * @return The RecursiveCopy instance.
     */
    static RecursiveCopy &getInstance();

    /**
     * Disallow copy and operator constructors.
     */
    RecursiveCopy(RecursiveCopy &s) = delete;
    void operator=(RecursiveCopy const &s) = delete;


    /**
     * @brief copy Copy files from src to dst
     *
     * The function is protected by a mutex because it uses some non thread-safe C functions.
     * @param src The source path to copy from
     * @param dst The destination path to copy to
     * @return ReturnCode::SUCCESS on success
     * @return ReturnCode::FAILURE on failure
     */
    ReturnCode copy(std::string src, std::string dst);

private:
    RecursiveCopy();
    ~RecursiveCopy();

    /**
     * @brief m_copyLock lock used to protect global variables in the copy() method.
     */
    std::mutex m_copyLock;
};
