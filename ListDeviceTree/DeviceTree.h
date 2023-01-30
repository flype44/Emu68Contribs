#ifndef __DEVICETREE_H__
#define __DEVICETREE_H__

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/execbase.h>
#include <exec/libraries.h>

#define APP_NAME    "DeviceTree"
#ifdef VERSION_STRING
#define APP_VSTRING VERSION_STRING " Philippe CARPENTIER";
#else
#define APP_VSTRING "$VER: " APP_NAME " 1.0 (14.1.2023) Philippe CARPENTIER\n"
#endif

#define COLOR0 "\033[30m"
#define COLOR1 "\033[31m"
#define COLOR2 "\033[32m"
#define COLOR3 "\033[33m"
#define COLOR4 "\033[34m"
#define COLOR5 "\033[35m"
#define COLOR6 "\033[36m"
#define COLOR7 "\033[37m"

typedef struct of_property {
    struct of_property *op_next;
    const char *        op_name;
    ULONG               op_length;
    const void *        op_value;
} of_property_t;

typedef struct of_node {
    struct of_node *    on_next;
    struct of_node *    on_parent;
    const char *        on_name;
    struct of_node *    on_children;
    of_property_t *     on_properties;
} of_node_t;

struct DeviceTreeBase {
    struct Library      dt_Node;
    struct ExecBase *   dt_ExecBase;
    of_node_t *         dt_Root;
    CONST_STRPTR        dt_StrNull;
    ULONG *             dt_Data;
    CONST_STRPTR        dt_Strings;
};

#endif
