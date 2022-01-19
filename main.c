/*
 * lsbootstrap
 * author: Matt Jacobson
 * date: January 2022
 */

#include <stdio.h>
#include <assert.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/task_special_ports.h>
#include <mach/vm_map.h>
#include <servers/bootstrap.h>
#include <servers/bootstrap_defs.h>

int main(int argc, const char *argv[]) {
    kern_return_t kr;
    mach_port_t task_port, bootstrap_port;
    name_array_t service_names, server_names;
    bool_array_t service_actives;
    mach_msg_type_number_t i;
    mach_msg_type_number_t service_names_count, server_names_count, service_actives_count;

    task_port = mach_task_self();
    if (task_port == MACH_PORT_NULL) {
        fprintf(stderr, "no task port\n");
        return 1;
    }

    kr = task_get_bootstrap_port(task_port, &bootstrap_port);
    if (kr != KERN_SUCCESS) {
        mach_error("task_get_bootstrap_port", kr);
        return 1;
    }

    kr = bootstrap_info(bootstrap_port, &service_names, &service_names_count, &server_names, &server_names_count, &service_actives, &service_actives_count);
    if (kr != BOOTSTRAP_SUCCESS) {
        mach_error("bootstrap_info", kr);
        return 1;
    }

    assert(server_names_count == service_names_count);
    assert(service_actives_count == service_names_count);

    printf("%u services\n\n", service_names_count);

    for (i = 0; i < service_names_count; i++) {
        printf("%c %s", service_actives[i] ? '*' : ' ', service_names[i]);

        if (server_names[i][0] != '\0') {
            printf(" (%s)", server_names[i]);
        }

        printf("\n");
    }

    kr = vm_deallocate(task_port, (vm_address_t)service_names, service_names_count);
    if (kr != KERN_SUCCESS) {
        mach_error("vm_deallocate", kr);
        return 1;
    }

    kr = vm_deallocate(task_port, (vm_address_t)server_names, server_names_count);
    if (kr != KERN_SUCCESS) {
        mach_error("vm_deallocate", kr);
        return 1;
    }

    kr = vm_deallocate(task_port, (vm_address_t)service_actives, service_actives_count);
    if (kr != KERN_SUCCESS) {
        mach_error("vm_deallocate", kr);
        return 1;
    }

    return 0;
}
