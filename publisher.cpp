#include <dbus-1.0/dbus/dbus.h>
#include <iostream>
#include <chrono>
#include <thread>

// http://www.matthew.ath.cx/articles/dbus

const char *const INTERFACE_NAME = "test.signal.Type";
const char *const SERVER_BUS_NAME = "test.signal.source";
const char *const OBJECT_PATH_NAME = "/test/signal/Object";
int count = 0;
/**
 * Connect to the DBUS bus and send a broadcast signal
 */
void publish(char* sigvalue)
{
    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    int ret;
    dbus_uint32_t serial = 0;

    printf("Sending signal with value %s\n", sigvalue);

    // initialise the error value
    dbus_error_init(&err);

    // connect to the DBUS system bus, and check for errors
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        exit(1);
    }

    // register our name on the bus, and check for errors
    ret = dbus_bus_request_name(conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        exit(1);
    }

    // create a signal & check for errors
    msg = dbus_message_new_signal(OBJECT_PATH_NAME, // object name of the signal
                                  INTERFACE_NAME, // interface name of the signal
                                  "PublishingRawData"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &sigvalue)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    // send the message and flush the connection
    while (true) {
        if (!dbus_connection_send(conn, msg, &serial)) {
            fprintf(stderr, "Out Of Memory!\n");
            exit(1);
        }
        dbus_connection_flush(conn);

        std::cout << "total msg sent = " << ++count << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }



    // free the message and close the connection
    dbus_message_unref(msg);
    //dbus_connection_close(conn);
    dbus_connection_unref(conn);
}


int main() {
    char* raw_buff = "Hello! You can replace this message with a raw buffer.";

    publish(raw_buff);
//    while (true) {
//        publish(raw_buff);
//        std::cout << "total msg sent = " << ++count << std::endl;
////        std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    }
    return 0;
}
