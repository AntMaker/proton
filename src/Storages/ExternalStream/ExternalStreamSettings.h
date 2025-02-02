#pragma once

#include <Core/BaseSettings.h>
#include <Core/Settings.h>

namespace DB
{
class ASTStorage;

#define EXTERNAL_STREAM_RELATED_SETTINGS(M) \
    M(String, type, "", "External stream type", 0) \
    /* those are kafka related settings */ \
    M(String, brokers, "", "A comma-separated list of brokers, for example Kafka brokers.", 0) \
    M(String, topic, "", "topic, for example Kafka topic name.", 0) \
    M(String, security_protocol, "plaintext", "The protocol to connection external logstore", 0) \
    M(String, username, "", "The username of external logstore", 0) \
    M(String, password, "", "The password of external logstore", 0) \
    /* those are log related settings */ \
    M(String, log_files, "", "A comma-separated list of log files", 0) \
    M(String, log_dir, "", "log root directory", 0) \
    M(String, timestamp_regex, "", "Regex to extract log timestamp", 0) \
    M(UInt64, hash_bytes, 1024, "File bytes to hash to decide if the same file", 0) \
    /* those are mapped to format factory settings */ \
    M(String, data_format, "", "The message format, for example JSONEachRow", 0) \
    M(String, row_delimiter, "\n", "The string to be considered as a delimiter in raw message.", 0) \
    M(UInt64, max_row_length, 4096, "Max row length", 0) \
    M(String, data_schema, "", "Schema identifier (used by schema-based formats)", 0)

#define LIST_OF_EXTERNAL_STREAM_SETTINGS(M) \
    EXTERNAL_STREAM_RELATED_SETTINGS(M) \
    FORMAT_FACTORY_SETTINGS(M)

DECLARE_SETTINGS_TRAITS(ExternalStreamSettingsTraits, LIST_OF_EXTERNAL_STREAM_SETTINGS)

/** Settings for the ExternalStream engine.
  * Could be loaded from a CREATE EXTERNAL STREAM query (SETTINGS clause).
  */
struct ExternalStreamSettings : public BaseSettings<ExternalStreamSettingsTraits>
{
    void loadFromQuery(ASTStorage & storage_def);
};

}
