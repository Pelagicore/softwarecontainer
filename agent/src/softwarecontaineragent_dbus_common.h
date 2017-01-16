
#pragma once
#include <iostream>
#include "glibmm.h"
#include "giomm.h"


class SoftwareContainerAgentCommon {
    public:
        template<typename T>
        static void unwrapList(std::vector<T> &list, const Glib::VariantContainerBase &wrapped) {
            for (uint i = 0; i < wrapped.get_n_children (); i++) {
                Glib::Variant<T> item;
                wrapped.get_child(item, i);
                list.push_back(item.get());
            }
        }

        static std::vector<Glib::ustring> stdStringVecToGlibStringVec(const std::vector<std::string> &strv) {
            std::vector<Glib::ustring> newStrv;
            for (uint i = 0; i < strv.size(); i++) {
                newStrv.push_back(strv[i]);
            }

            return newStrv;
        }

        static std::vector<std::string> glibStringVecToStdStringVec(const std::vector<Glib::ustring> &strv) {
            std::vector<std::string> newStrv;
            for (uint i = 0; i < strv.size(); i++) {
                newStrv.push_back(strv[i]);
            }

            return newStrv;
        }

        static std::map<std::string, std::string> glibStringMapToStdStringMap(const std::map<Glib::ustring, Glib::ustring> &strm) {
            std::map<std::string, std::string> newStrm;
            for (std::pair<Glib::ustring, Glib::ustring> pair : strm) {
                std::pair<std::string, std::string> newPair(pair.first, pair.second);
                newStrm.insert(newPair);
            }

            return newStrm;
        }

        template <typename T0, typename T1>
        static std::vector<T1> mapVector(const std::vector<T0> &vector0, std::function<T1(T0)> mapFunction) {
            std::vector<T1> vector1;
            for (T0 &e : vector0) {
                vector1.push_back(mapFunction(e));
            }

            return vector1;
        }
};
class SoftwareContainerAgentMessageHelper {
public:
    SoftwareContainerAgentMessageHelper (const Glib::RefPtr<Gio::DBus::MethodInvocation> msg) :
        m_message(msg) {}

    const Glib::RefPtr<Gio::DBus::MethodInvocation> getMessage() {
        return m_message;
    }

void returnValue()
{
    Glib::VariantContainerBase empty;
    m_message->return_value(empty);
}

template <typename T0>
void returnValue(T0 p0)
{
    std::vector<Glib::VariantBase> vlist;
    vlist.push_back(Glib::Variant<T0>::create(p0));

    m_message->return_value(Glib::Variant<Glib::VariantBase>::create_tuple(vlist));
}

template <typename T0,typename T1>
void returnValue(T0 p0, T1 p1)
{
    std::vector<Glib::VariantBase> vlist;
    vlist.push_back(Glib::Variant<T0>::create(p0));
    vlist.push_back(Glib::Variant<T1>::create(p1));

    m_message->return_value(Glib::Variant<Glib::VariantBase>::create_tuple(vlist));
}

void returnError(const std::string &errorMessage)
{
    Gio::DBus::Error error(Gio::DBus::Error::FAILED, errorMessage);
    m_message->return_error(error);
}

private:
    Glib::RefPtr<Gio::DBus::MethodInvocation> m_message;
};
