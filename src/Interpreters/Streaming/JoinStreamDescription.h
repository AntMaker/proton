#pragma once

#include <Core/Block.h>
#include <Core/Joins.h>
#include <Core/Streaming/DataStreamSemantic.h>
#include <Storages/IStorage_fwd.h>

namespace DB
{
struct TableWithColumnNamesAndTypes;

namespace Streaming
{

DataStreamSemanticEx getDataStreamSemantic(StoragePtr storage);

struct JoinStreamDescription
{
    JoinStreamDescription(const TableWithColumnNamesAndTypes & table_with_columns_, Block input_header_, DataStreamSemanticEx data_stream_semantic_, UInt64 keep_versions_)
        : table_with_columns(table_with_columns_)
        , input_header(std::move(input_header_))
        , data_stream_semantic(data_stream_semantic_)
        , keep_versions(keep_versions_)
    {
    }

    JoinStreamDescription(JoinStreamDescription && other) noexcept
        : table_with_columns(other.table_with_columns)
        , input_header(std::move(other.input_header))
        , data_stream_semantic(other.data_stream_semantic)
        , keep_versions(other.keep_versions)
        , primary_key_column_positions(std::move(other.primary_key_column_positions))
        , version_column_position(other.version_column_position)
    {
    }

    bool hasPrimaryKey() const noexcept { return primary_key_column_positions.has_value() && !primary_key_column_positions->empty(); }
    bool hasVersionColumn() const noexcept { return version_column_position.has_value(); }
    bool hasDeltaColumn() const noexcept { return delta_column_position.has_value(); }
    const String & deltaColumnName() const;

    void calculateColumnPositions(JoinStrictness strictness);

    const TableWithColumnNamesAndTypes & table_with_columns;

    Block input_header;

    /// The input stream data semantic
    DataStreamSemanticEx data_stream_semantic;

    UInt64 keep_versions;

    /// Header's properties. Pre-calculated and cached. Used during join
    /// Primary key columns and version columns could be a performance enhancement
    /// during join.
    /// For example, assuming `versioned_kv` has primary key `(k, k1)` and version column as `_tp_time`,
    /// instead of inserting `ChangelogTransform`, we rewrite this join
    /// `SELECT versioned_kv.i, versioned_kv.k1, append.j, append.k
    ///  FROM append JOIN
    ///       versioned_kv
    /// ON append.k = versioned_kv.k`
    /// =>
    /// `SELECT versioned_kv.i, versioned_kv.k1, append.j, append.k
    ///  FROM append JOIN
    ///    (SELECT i, k1, k AS __tp_pk_k, k1 AS __tp_pk_k1, __tp_time AS __tp_v_tp_time FROM versioned_kv) AS versioned_kv
    ///  ON append.k = versioned_kv.k`
    /// With this rewrite, we don't need insert `ChangelogTransform` step. Instead we do all of this changelog stuff
    /// in HashJoin. Essentially, we push `ChangelogTransform` step to `HashJoin` step.
    /// This could have higher performance since we only need index primary key / join key and can save comparing value columns
    /// for retraction.
    std::optional<std::vector<size_t>> primary_key_column_positions;
    std::optional<size_t> version_column_position;

    /// `delta_column` is expected to be in input header if we insert `ChangelogTransform` before the HashJoin.
    /// In this mode, when `delta_column` is `-1`, we will need do retraction:
    /// 1. First find the last row(s) by using join key
    /// 2. Loop the value entries in list and compare the value to find a match (there has to be a match)
    ///    do retraction and garbage collection if necessary.
    std::optional<size_t> delta_column_position;

private:
    void assertValid() const;
};

using JoinStreamDescriptionPtr = std::shared_ptr<JoinStreamDescription>;
}
}
