/**
 * History:
 * ================================================================
 * 2019-12-20 qing.zou created
 *
 */

#include "jansson.h"
#include "http_handler.h"

typedef struct ResponseCode
{
    int         err_code;
    char        *err_msg;
} ResponseCode;

static const ResponseCode code_mapping[] =
{
    { 5001, "param error" },
    { 5002, "system error" },
};

static const char *find_by_code(int code)
{
    unsigned int i = 0;
    for (i = 0; i < _countof(code_mapping); i++)
    {
        if (code == code_mapping[i].err_code)
            return code_mapping[i].err_msg;
    }
    return NULL;
}

int error_json_create(int ecode, char **data)
{
    json_t *json_root = json_object();

    json_object_set_new(json_root, "err_code", json_integer(ecode));
    json_object_set_new(json_root, "err_msg", json_string(find_by_code(ecode)));

    char *data_dump = json_dumps(json_root, 0);
    VMP_LOGD("response error:\n%s", data_dump);

    int length = strlen(data_dump);
    *data = calloc(1, length + 1);
    if (*data)
        strcpy(*data, data_dump);
    else
        VMP_LOGE("memory alloc failed: error_json_create");

    free(data_dump);
    json_decref(json_root);

    return length;
}
