//
// Created by kaushal on 15/02/21.
//

#ifndef DBUSTEST_DBUS_COMMON_H
#define DBUSTEST_DBUS_COMMON_H

#include <dbus/dbus.h>
#include <assert.h>
#include <iostream>
#include <stdlib.h>

#define SERVER_BUS_NAME     "test.signal.sink"
#define OBJECT_PATH_NAME    "/test/signal/Object"
#define INTERFACE_NAME      "test.signal.Type"
#define SIGNAL_NAME         "PublishingSerializedData"

/*
 * Will use the following struct as payload.
 * */
struct car {
    char    model[20];
    int     cost;
    double  fuel_capacity;
};

/*
 * Utility to terminate if given DBusError is set.
 * Will print out the message and error before terminating.
 *
 * If error is not set, will do nothing.
 * */
static void terminateOnError(const char* msg, DBusError* err) {
    assert(msg != nullptr);
    assert(err != nullptr);

    if (dbus_error_is_set(err)) {
        std::cerr << "DBusError.name: " << err->name << "\n";
        std::cerr << "DBusError.message: " << err->message << "\n";

        dbus_error_free(err);
//        exit(EXIT_FAILURE);
    }
}

#endif //DBUSTEST_DBUS_COMMON_H
