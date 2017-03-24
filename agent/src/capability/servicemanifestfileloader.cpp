/*
 * Copyright (C) 2017 Pelagicore AB
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

#include "servicemanifestfileloader.h"
#include <glibmm/fileutils.h>

namespace softwarecontainer {

ServiceManifestFileLoader::ServiceManifestFileLoader(const std::string &source) :
    ServiceManifestLoader(source)
{
}

std::vector<json_t *> ServiceManifestFileLoader::loadContent()
{
    std::string errorMessage;

    if (m_source.empty()) {
        //File path is empty, this is ok
        log_debug() << "Path to service manifests is empty";
    } else if (m_source.compare("/") == 0) {
        errorMessage = "Searching for configuration files from root dir not allowed";
        log_error() << errorMessage;
        throw ServiceManifestPathError(errorMessage);
    } else if (isDirectory(m_source)) {
        log_debug() << "Path to service manifests is a directory";
        loadServiceManifestDir();
    } else if (isFile(m_source)) {
        log_debug() << "Path to service manifest is a file";
        if (isJsonFile(m_source)) {
            loadServiceManifestFile(m_source);
        } else {
            errorMessage = "Path to service manifest is not a json file";
            log_debug() << errorMessage;
            throw ServiceManifestPathError(errorMessage);
        }
    } else {
        errorMessage = "The path to the service manifest(s) is not a directory or file: \""
            + m_source + "\"";
        log_error() << errorMessage;
        throw ServiceManifestPathError(errorMessage);
    }
    return m_content;
}

void ServiceManifestFileLoader::loadServiceManifestDir()
{
    std::string errorMessage;

    if (isDirectoryEmpty(m_source)) {
        log_warning() << "The service manifest directory is empty: " << m_source;
        return;
    }

    std::vector<std::string> files = fileList();

    if (files.empty()) {
        log_info() << "No configuration files found: " << m_source;
        return;
    }

    for (std::string file : files) {
        std::string filePath = buildPath(m_source, file);
        loadServiceManifestFile(filePath);
    }
}

void ServiceManifestFileLoader::loadServiceManifestFile(const std::string &filePath)
{
    std::string errorMessage;

    log_debug() << "Loading service manifest file: " << filePath;

    json_error_t error;
    json_t *fileroot = json_load_file(filePath.c_str(), 0, &error);

    if (nullptr == fileroot) {
        errorMessage = "Could not parse the service manifest: "
            + filePath + ":" + std::to_string(error.line) + " : " + std::string(error.text);
        log_error() << errorMessage;
        throw ServiceManifestParseError(errorMessage);
    } else if (!json_is_object(fileroot)) {
        errorMessage = "The service manifest root is not a json object: "
            + filePath + ":" + std::to_string(error.line) + " : " + std::string(error.text);
        log_error() << errorMessage;
        throw ServiceManifestParseError(errorMessage);
    }

    log_debug() << "Loaded service manifest: " << filePath;

    m_content.push_back(fileroot);
}


/****************
 * Help functions
 */

std::vector<std::string> ServiceManifestFileLoader::fileList()
{
    std::vector<std::string> files;
    try {
        Glib::Dir dir(m_source);

        std::string filename;
        while ((filename = dir.read_name()) != "") {
            if (isJsonFile(filename)) {
                files.push_back(filename);
            }
        }
        dir.close();
    } catch (Glib::FileError &err) {
        log_error() << "Could not read files in directory: " << m_source;
    }

    return files;
}

bool ServiceManifestFileLoader::isJsonFile(const std::string &filename)
{

     if (filename.size() <= 5) {
         // Even if the file name is ".json" we will not classify this as a json file
         return false;
     }
     return filename.substr(filename.size()-5) == ".json";
}

} // namespace softwarecontainer
