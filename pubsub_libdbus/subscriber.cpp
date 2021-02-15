#include "dbus-common.h"
#include <iostream>
#include <sstream>
#include <thread>

#define RECEIVE_TIMEOUT_MS 100

car* sample_car;

int main(int argc, char **argv) {
    unsigned char* payload;
    size_t payload_size;

    /* Needed for establishing connections. */
    DBusConnection* bus;

    DBusMessage* msg;
    DBusMessageIter args;

    DBusError err;
    dbus_error_init(&err);

    /*Connecting to the System D-Bus.*/
    bus = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    terminateOnError("Failed to open bus.", &err);

    /* Register us on bus, terminate if errors. */
    int ret = dbus_bus_request_name(bus,
                                    SERVER_BUS_NAME,
                                    DBUS_NAME_FLAG_ALLOW_REPLACEMENT,
                                    &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    /* Add rule for the signal. */
    std::stringstream rule_ss;
    rule_ss << "type='signal',interface='" << INTERFACE_NAME << "'";

    /* See/Accept signals from the given interface. */
    dbus_bus_add_match(bus,
                       rule_ss.str().c_str(),
                       &err);

    // Alternative way:-
    // dbus_bus_add_match(conn,
    //                    "type='signal',interface='test.signal.Type'",
    //                    &err); // see signals from the given interface

    dbus_connection_flush(bus);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }
    printf("Match rule sent\n");

    /* loop listening for signals being emitted */
    while (true) {

        /* non blocking read of the next available message */
        dbus_connection_read_write(bus, 0);
        msg = dbus_connection_pop_message(bus);

        /* Loop again if we haven't read a message. */
        if (msg == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RECEIVE_TIMEOUT_MS));
            continue;
        }

        /* Check if the message is a signal from the correct interface and with the correct name. */
        if (dbus_message_is_signal(msg, INTERFACE_NAME, SIGNAL_NAME)) {
//            /* read the parameters */
//            if (!dbus_message_iter_init(msg, &args)){
//                std::cerr << "Message had no parameters\n";
//            } else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
//                std::cerr << "Payload is not a string.\n";
//            } else {
//                dbus_message_iter_get_basic(&args, &payload);
//            }

            if(FALSE == dbus_message_get_args(msg,
                                              &err,
                                              DBUS_TYPE_ARRAY,
                                              DBUS_TYPE_BYTE,
                                              &payload,
                                              &payload_size,
                                              DBUS_TYPE_INVALID)
                                              && 0 != payload_size) {

                std::cerr << "Receive Data Error: Payload Size = " << payload_size << "\n";
                return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
            }

            /* Mapping the payload to the car structure. */

            sample_car = reinterpret_cast<car*>(payload);

            std::cout << "Cost of " << sample_car->model << " is " << sample_car->cost << std::endl;
        }
        // free the message
        dbus_message_unref(msg);
    }

    dbus_connection_unref(bus);

    return 0;
}