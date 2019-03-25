#include "legato.h"
#include "interfaces.h"
#include <jansson.h>

/// Input resource path
#define RES_PATH_VALUE        "bme/value"
#define RES_PATH_CONFIG       "bme/config"
#define VALUE_EXAMPLE "{\"timestamp\":1,\"iaqValue\":1,\"iaqAccuracy\":1,\"co2EquivalentValue\":1,"\
                      "\"co2EquivalentAccuracy\":1,\"breathVocValue\":1,\"breathVocAccuracy\":1,"\
                      "\"pressure\":1,\"temperature\":1,\"humidity\":1 }"

static void BmeConfigPushHandler(double timestamp, const char *jsonStr, void *context)
{
    /*
     * Expect jsonStr in the form:
     * {
     *   "samplingRate": "SAMPLING_RATE_LP",
     *   "enableIaq": true,
     *   "enableCo2Equivalent": true,
     *   "enableBreathVoc": true,
     *   "enablePressure": true,
     *   "enableTemperature": true,
     *   "enableHumidity": true
     * }
     */

    json_error_t loadError;
    const size_t loadFlag = 0;
    json_t *json = json_loads(jsonStr, loadFlag, &loadError);
    if (json == NULL)
    {
        LE_ERROR("Received invalid JSON: \"%s\". Parsing gave error: %s", jsonStr, loadError.text);
        return;

    }
    const char *samplingRate;
    int enableIaq;
    int enableCo2Equivalent;
    int enableBreathVoc;
    int enablePressure ;
    int enableTemperature;
    int enableHumidity;
    int samplingRateEnum;

    const int  unpackRes =  json_unpack(
        json, "{s:s, s:b, s:b, s:b, s:b, s:b, s:b}", "samplingRate", &samplingRate, "enableIaq",
        &enableIaq, "enableCo2Equivalent", &enableCo2Equivalent, "enableBreathVoc",
        &enableBreathVoc, "enablePressure", &enablePressure, "enableTemperature",
        &enableTemperature, "enableHumidity", &enableHumidity);

    if (unpackRes)
    {
        LE_ERROR("Valid JSON has unexpected format: \"%s\"", jsonStr);
        goto cleanup;
    }

    if (strcmp(samplingRate, "SAMPLING_RATE_DISABLED") == 0)
    {
        samplingRateEnum = MANGOH_BME680_SAMPLING_RATE_DISABLED;
    }
    else if (strcmp(samplingRate, "SAMPLING_RATE_LP") == 0)
    {
        samplingRateEnum = MANGOH_BME680_SAMPLING_RATE_LP;
    }
    else if (strcmp(samplingRate, "SAMPLING_RATE_ULP") == 0)
    {
        samplingRateEnum = MANGOH_BME680_SAMPLING_RATE_ULP;
    }
    else
    {
        LE_ERROR("Bad sampling rate \"%s\"", samplingRate);
        goto cleanup;
    }

    LE_ASSERT_OK(mangOH_bme680_Configure(
       samplingRateEnum,
       enableIaq,
       enableCo2Equivalent,
       enableBreathVoc,
       enablePressure,
       enableTemperature,
       enableHumidity));

    cleanup:
        json_decref(json);
}


static void SensorReadingHandler
(
    const mangOH_bme680_Reading_t *reading,
    void* context
)
{
    json_t *jsonBme = json_object();

    json_object_set_new(jsonBme, "timestamp", json_real(reading->timestamp));

    if (reading->iaq.valid)
    {
        json_object_set_new(jsonBme, "iaqValue", json_real(reading->iaq.value));
        json_object_set_new(jsonBme, "iaqAccuracy", json_integer(reading->iaq.accuracy));
    }

    if (reading->co2Equivalent.valid)
    {
        json_object_set_new(jsonBme, "co2EquivalentValue", json_real(reading->co2Equivalent.value));
        json_object_set_new(jsonBme, "co2EquivalentAccuracy",
                            json_integer(reading->co2Equivalent.accuracy));
    }

    if (reading->breathVoc.valid)
    {
        json_object_set_new(jsonBme, "breathVocValue", json_real(reading->breathVoc.value));
        json_object_set_new(jsonBme, "breathVocAccuracy", json_integer(reading->breathVoc.accuracy));
    }

    if (reading->pressure.valid)
    {
        json_object_set_new(jsonBme, "pressure", json_real(reading->pressure.value));
    }

    if (reading->temperature.valid)
    {
        json_object_set_new(jsonBme, "temperature", json_real( reading->temperature.value));
    }

    if (reading->humidity.valid)
    {
        json_object_set_new(jsonBme, "humidity", json_real(reading->humidity.value));
    }

    char *value = json_dumps(jsonBme, JSON_COMPACT);
    LE_ASSERT(value != NULL);
    json_decref(jsonBme);
    io_PushJson(RES_PATH_VALUE, IO_NOW, value);
    free(value);
}


COMPONENT_INIT
{
    le_result_t res = io_CreateOutput(RES_PATH_CONFIG, IO_DATA_TYPE_JSON, "");
    LE_FATAL_IF(res != LE_OK, "Couldn't create config data hub output - %s", LE_RESULT_TXT(res));
//    io_MarkOptional(RES_PATH_CONFIG);
    io_AddJsonPushHandler(RES_PATH_CONFIG, BmeConfigPushHandler, NULL);

    LE_ASSERT_OK(io_CreateInput(RES_PATH_VALUE, IO_DATA_TYPE_JSON, ""));
    io_SetJsonExample(RES_PATH_VALUE, VALUE_EXAMPLE);

    mangOH_bme680_AddSensorReadingHandler(SensorReadingHandler, NULL);
}
