#include "UserDefinedEmitStrategyAggregatingTransform.h"

#include <Processors/Transforms/convertToChunk.h>

#include <algorithm>

namespace DB
{
namespace Streaming
{

UserDefinedEmitStrategyAggregatingTransform::UserDefinedEmitStrategyAggregatingTransform(Block header, AggregatingTransformParamsPtr params_)
    : UserDefinedEmitStrategyAggregatingTransform(std::move(header), std::move(params_), std::make_unique<ManyAggregatedData>(1), 0, 1, 1)
{
}

UserDefinedEmitStrategyAggregatingTransform::UserDefinedEmitStrategyAggregatingTransform(
    Block header,
    AggregatingTransformParamsPtr params_,
    ManyAggregatedDataPtr many_data_,
    size_t current_variant_,
    size_t max_threads_,
    size_t temporary_data_merge_threads_)
    : AggregatingTransform(
        std::move(header),
        std::move(params_),
        std::move(many_data_),
        current_variant_,
        max_threads_,
        temporary_data_merge_threads_,
        "UserDefinedAggregatingTransform",
        ProcessorID::UserDefinedEmitStrategyAggregatingTransformID)
{
    assert(params->params.group_by == Aggregator::Params::GroupBy::USER_DEFINED);
}

void UserDefinedEmitStrategyAggregatingTransform::finalize(const ChunkContextPtr & chunk_ctx)
{
    /// We don't need care other data variants, just finalize what we have
    auto & variants = *many_data->variants[current_variant];
    if (variants.empty())
        return;

    Block block;
    if (params->final)
    {
        auto results = params->aggregator.convertToBlocksFinal(variants, ConvertAction::STREAMING_EMIT, 1);
        assert(results.size() == 1);
        block = std::move(results.back());

        if (params->emit_version)
            emitVersion(block);
    }
    else
    {
        auto results = params->aggregator.convertToBlocksIntermediate(variants, ConvertAction::STREAMING_EMIT, 1);
        assert(results.size() == 1);
        block = std::move(results.back());
    }

    if (block)
        setCurrentChunk(convertToChunk(block), chunk_ctx);
}
}
}
