#include "dbus-common.h"
#include <iostream>

const car tesla = {"Tesla Model S", 1729 };

int main(int argc, char **argv) {
    /* Message to publish. */
    const char* payload = reinterpret_cast<const char*>(&tesla);

    /*Structure representing the connection to a bus.*/
    DBusConnection* bus = nullptr;

    /* 4 message types in dbus:
     * - method call messages
     * - method return messages
     * - error return messages
     * - signal messages
     * */
    DBusMessage* msg = nullptr;
    DBusMessageIter args;

    /* dbus will report problems & exceptions using DBusError struct. */
    DBusError err;

    /*clean the error state*/
    dbus_error_init(&err);

    /*Connecting to the System D-Bus.*/
    bus = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    terminateOnError("Failed to open bus.", &err);

    /* Register as primary owner on bus, terminate if errors. */
    int ret = dbus_bus_request_name(bus,
                                    SERVER_BUS_NAME,
                                    DBUS_NAME_FLAG_REPLACE_EXISTING,
                                    &err);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    /* Terminate if not a primary owner. */
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        exit(1);
    }

    /* Construct a DBusMessage that represents a signal. */
    msg = dbus_message_new_signal(OBJECT_PATH_NAME,
                                  INTERFACE_NAME,
                                  SIGNAL_NAME);

    if (msg == nullptr) {
        std::cerr << "Ran out of memory while creating a dbus signal message." << std::endl;
        throw std::bad_alloc(); // out of memory
    }

    /* Append arguments to the signal message. */
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args,
                                        DBUS_TYPE_STRING,
                                        &payload)) {
        std::cerr << "Ran out of memory while constructing args." << std::endl;
        throw std::bad_alloc(); // out of memory
    }

    /* If the no-reply flag is set, the D-Bus daemon makes sure that the
     possible reply is discarded and not sent to us. */
    dbus_message_set_no_reply(msg, true);

    std::cout << "Adding message to publish queue." << "\n";
    if (!dbus_connection_send(bus, msg, nullptr)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }

    std::cout << "Waiting for publish-queue to be sent out.\n";
    dbus_connection_flush(bus);
    std::cout << "Published, queue is now empty." << std::endl;

    /* Clean up allocated message. */
    dbus_message_unref(msg);
    msg = nullptr;

    /*
     * Free up the connection, libdbus attempts to share existing
     * connections for the same client, so instead of closing down
     * a connection object, it is unreferenced. The D-Bus library
     * will keep an internal reference to each shared connection,
     * to prevent accidental closing of shared connections before
     * the library is finalized.
     * */
    dbus_connection_unref(bus);
    bus = nullptr;

    std::cout << "Quitting (success)\n" ;

    return 0;
}