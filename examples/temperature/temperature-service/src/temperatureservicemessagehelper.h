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

#pragma once
#include <iostream>
#include "glibmm.h"
#include "giomm.h"

class TemperatureServiceMessageHelper {
public:
    TemperatureServiceMessageHelper (const Glib::RefPtr<Gio::DBus::MethodInvocation> msg) :
        m_message(msg) {}

    const Glib::RefPtr<Gio::DBus::MethodInvocation> getMessage() {
        return m_message;
    }

    void ret()
    {
        std::vector<Glib::VariantBase> vlist;

        m_message->return_value(Glib::Variant<Glib::VariantBase>::create_tuple(vlist));
    }

    void ret(double p0)
    {
        std::vector<Glib::VariantBase> vlist;
        vlist.push_back(Glib::Variant<double >::create((p0)));

        m_message->return_value(Glib::Variant<Glib::VariantBase>::create_tuple(vlist));
    }

    void ret(std::string p0)
    {
        std::vector<Glib::VariantBase> vlist;
        vlist.push_back(Glib::Variant<Glib::ustring >::create((p0)));

        m_message->return_value(Glib::Variant<Glib::VariantBase>::create_tuple(vlist));
    }


private:
    Glib::RefPtr<Gio::DBus::MethodInvocation> m_message;
};

