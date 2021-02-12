#include <dbus-1.0/dbus/dbus.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <chrono>
#include <thread>

// http://www.matthew.ath.cx/articles/dbus
const char *const INTERFACE_NAME = "test.signal.Type";
const char *const SERVER_BUS_NAME = "test.signal.sink";
const char *const OBJECT_PATH_NAME = "/test/signal/Object";

/**
 * Listens for signals on the bus
 */
void consume()
{
    int count = 0;
    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    int ret;
    char* sigvalue;

    printf("Listening for signals\n");

    // initialise the errors
    dbus_error_init(&err);

    // connect to the bus and check for errors
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        exit(1);
    }

    // request our name on the bus and check for errors
    ret = dbus_bus_request_name(conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    //    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
    //        exit(1);
    //    }

    // add a rule for which messages we want to see

    std::string rule = std::string("type='signal',interface='") + std::string(INTERFACE_NAME) + "'";
    // dbus_bus_add_match(conn, "type='signal',interface='test.signal.Type'", &err); // see signals from the given interface
    dbus_bus_add_match(conn, rule.c_str(), &err); // see signals from the given interface
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }
    printf("Match rule sent\n");

    // loop listening for signals being emmitted
    while (true) {

        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);

        // loop again if we haven't read a message
        if (NULL == msg) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(msg, INTERFACE_NAME, "PublishingRawData")) {

            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message Has No Parameters\n");
            else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not string!\n");
            else
                dbus_message_iter_get_basic(&args, &sigvalue);

            printf("Got Signal with value %s\n", sigvalue);

            std::cout << "total msg sent = " << ++count << std::endl;
        }
        // free the message
        dbus_message_unref(msg);
    }
    // close the connection
    // dbus_connection_close(conn);
    dbus_connection_unref(conn);
}

int main() {
    consume();
    return 0;
}
