/*
 * Copyright (C) 2016 Pelagicore AB
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

#include <sigc++/sigc++.h>
#include <glibmm.h>

#include <vector>
#include <functional>

#include <sys/types.h>

namespace softwarecontainer {

/**
 * @brief The SignalConnectionsHandler class contains references to sigc++ connections and
 * automatically disconnects them on destruction.
 */
class SignalConnectionsHandler
{

public:
    /**
     * Add a new connection
     */
    void addConnection(sigc::connection &connection);

    ~SignalConnectionsHandler();

private:
    std::vector<sigc::connection> m_connections;

};

/**
 * @brief addProcessListener Adds a glib child watch for a process.
 * @warning This is not thread safe!
 * @param connections Add the signal to this list of connections
 * @param pid The pid to watch for.
 * @param function A lambda/function pointer to run when the signal is sent for a process.
 * @param context glib context to attach the SignalChildWatch to.
 */
inline void addProcessListener(
    SignalConnectionsHandler &connections,
    pid_t pid,
    std::function<void(pid_t, int)> function,
    Glib::RefPtr<Glib::MainContext> context)
{
    Glib::SignalChildWatch watch = context->signal_child_watch();
    auto connection = watch.connect(function, pid);
    connections.addConnection(connection);
}

} // namespace softwarecontainer
