#include "kqp_opt_impl.h"

namespace NKikimr::NKqp::NOpt {

using namespace NYql;
using namespace NYql::NNodes;

namespace {

template<typename T>
TExprBase ProjectColumnsInternal(const TExprBase& input, const T& columnNames, TExprContext& ctx) {
    const auto rowArgument = Build<TCoArgument>(ctx, input.Pos())
        .Name("row")
        .Done();

    TVector<TExprBase> columnGetters;
    columnGetters.reserve(columnNames.size());
    for (const auto& column : columnNames) {
        const auto tuple = Build<TCoNameValueTuple>(ctx, input.Pos())
            .Name().Build(column)
            .template Value<TCoMember>()
                .Struct(rowArgument)
                .Name().Build(column)
                .Build()
            .Done();

        columnGetters.emplace_back(std::move(tuple));
    }

    return Build<TCoMap>(ctx, input.Pos())
        .Input(input)
        .Lambda()
            .Args({rowArgument})
            .Body<TCoAsStruct>()
                .Add(columnGetters)
                .Build()
            .Build()
        .Done();
}

} // namespace

bool IsKqpPureLambda(const TCoLambda& lambda) {
    return !FindNode(lambda.Body().Ptr(), [](const TExprNode::TPtr& node) {
        if (TMaybeNode<TKqlReadTableBase>(node)) {
            return true;
        }

        if (TMaybeNode<TKqlReadTableRangesBase>(node)) {
            return true;
        }

        if (TMaybeNode<TKqlLookupTableBase>(node)) {
            return true;
        }

        if (TMaybeNode<TKqlTableEffect>(node)) {
            return true;
        }

        return false;
    });
}

bool IsKqpPureInputs(const TExprList& inputs) {
    return !FindNode(inputs.Ptr(), [](const TExprNode::TPtr& node) {
        if (TMaybeNode<TKqpCnStreamLookup>(node)) {
            return true;
        }

        if (auto source = TExprBase(node).Maybe<TDqSource>()) {
            if (source.Cast().Settings().Maybe<TKqpReadRangesSourceSettings>()) {
                return true;
            }
        }

        return false;
    });
}

bool IsKqpEffectsStage(const TDqStageBase& stage) {
    return stage.Program().Body().Maybe<TKqpEffects>().IsValid();
}

bool NeedSinks(const TKikimrTableDescription& table, const TKqpOptimizeContext& kqpCtx) {
    return (kqpCtx.IsGenericQuery() || (kqpCtx.IsDataQuery() && table.Metadata->Kind != EKikimrTableKind::Olap))
        && (table.Metadata->Kind != EKikimrTableKind::Olap || kqpCtx.Config->EnableOlapSink)
        && (table.Metadata->Kind != EKikimrTableKind::Datashard || kqpCtx.Config->EnableOltpSink);
}

TExprBase ProjectColumns(const TExprBase& input, const TVector<TString>& columnNames, TExprContext& ctx) {
    return ProjectColumnsInternal(input, columnNames, ctx);
}

TExprBase ProjectColumns(const TExprBase& input, const THashSet<TStringBuf>& columnNames, TExprContext& ctx) {
    return ProjectColumnsInternal(input, columnNames, ctx);
}

TKqpTable BuildTableMeta(const TKikimrTableMetadata& meta, const TPositionHandle& pos, TExprContext& ctx) {
    return Build<TKqpTable>(ctx, pos)
        .Path().Build(meta.Name)
        .PathId().Build(meta.PathId.ToString())
        .SysView().Build(meta.SysView)
        .Version().Build(meta.SchemaVersion)
        .Done();
}

TKqpTable BuildTableMeta(const TKikimrTableDescription& tableDesc, const TPositionHandle& pos, TExprContext& ctx) {
    YQL_ENSURE(tableDesc.Metadata);
    return BuildTableMeta(*tableDesc.Metadata, pos, ctx);
}

TVector<TString> ExtractSortingKeys(const NYql::NNodes::TCoLambda& keySelector) {
    auto lambdaBody = keySelector.Body();

    TVector<TString> sortingKeys;

    auto extractSortKey = [keySelector] (NYql::NNodes::TExprBase key) -> TString {
        if (!key.Maybe<TCoMember>()) {
            return TString();
        }

        auto member = key.Cast<TCoMember>();
        if (member.Struct().Raw() != keySelector.Args().Arg(0).Raw()) {
            return TString();
        }

        return TString(member.Name().Value());
    };

    if (auto maybeTuple = lambdaBody.Maybe<TExprList>()) {
        auto tuple = maybeTuple.Cast();
        sortingKeys.reserve(tuple.Size());
        for (size_t i = 0; i < tuple.Size(); ++i) {
            sortingKeys.emplace_back(extractSortKey(tuple.Item(i)));
        }

    } else {
        sortingKeys.emplace_back(extractSortKey(lambdaBody));
    }

    return sortingKeys;
};

bool IsSortKeyPrimary(
    const NYql::NNodes::TCoLambda& keySelector,
    const NYql::TKikimrTableDescription& tableDesc,
    const TMaybe<THashSet<TStringBuf>>& passthroughFields,
    const ui64 skipPointKeys
) {
    YQL_ENSURE(skipPointKeys <= tableDesc.Metadata->KeyColumnNames.size());
    std::deque<TString> unsortedKeyColumns(tableDesc.Metadata->KeyColumnNames.begin() + skipPointKeys, tableDesc.Metadata->KeyColumnNames.end());
    if (unsortedKeyColumns.empty()) {
        return true;
    }

    THashSet<TString> sortedPointKeysSet(tableDesc.Metadata->KeyColumnNames.begin(), tableDesc.Metadata->KeyColumnNames.begin() + skipPointKeys);

    auto extractSortKey = [keySelector] (NYql::NNodes::TExprBase key) -> TString {
        if (!key.Maybe<TCoMember>()) {
            return TString();
        }

        auto member = key.Cast<TCoMember>();
        if (member.Struct().Raw() != keySelector.Args().Arg(0).Raw()) {
            return TString();
        }

        return TString(member.Name().Value());
    };

    auto checkKey = [keySelector, &extractSortKey, &sortedPointKeysSet, &passthroughFields] (NYql::NNodes::TExprBase key, TString unsorted) -> std::optional<bool> {
        auto column = extractSortKey(key);

        if (!sortedPointKeysSet.contains(column)) {
            if (column != unsorted) {
                return false;
            }
        }

        if (passthroughFields && !passthroughFields->contains(column)) {
            return false;
        }

        if (sortedPointKeysSet.contains(column)) {
            return std::nullopt;
        }

        return true;
    };

    auto lambdaBody = keySelector.Body();
    if (auto maybeTuple = lambdaBody.Maybe<TExprList>()) {
        auto tuple = maybeTuple.Cast();
        for (size_t i = 0; i < tuple.Size(); ++i) {
            if (unsortedKeyColumns.empty()) {
                return true;
            }

            auto result = checkKey(tuple.Item(i), unsortedKeyColumns.front());
            if (!result.has_value()) {
                continue;
            } else if (!result.value()) {
                return false;
            } else {
                unsortedKeyColumns.pop_front();
            }
        }
    } else {
        if (unsortedKeyColumns.empty()) {
            return true;
        }

        auto result = checkKey(lambdaBody, unsortedKeyColumns.front());
        if (result.has_value() && !result.value()) {
            return false;
        }
    }

    return true;
}

ESortDirection GetSortDirection(const NYql::NNodes::TExprBase& sortDirections) {
    auto getDirection = [] (TExprBase expr) -> ESortDirection {
        if (!expr.Maybe<TCoBool>()) {
            return ESortDirection::Unknown;
        }

        if (!FromString<bool>(expr.Cast<TCoBool>().Literal().Value())) {
            return ESortDirection::Reverse;
        }

        return ESortDirection::Forward;
    };

    auto direction = ESortDirection::None;
    if (auto maybeList = sortDirections.Maybe<TExprList>()) {
        for (const auto& expr : maybeList.Cast()) {
            direction |= getDirection(expr);
        }
    } else {
        direction |= getDirection(sortDirections);
    }
    return direction;
};

bool IsBuiltEffect(const TExprBase& effect) {
    // Stage with effect output
    if (effect.Maybe<TDqOutput>()) {
        return true;
    }

    // Stage with sink effect
    if (effect.Maybe<TKqpSinkEffect>()) {
        return true;
    }

    return false;
}

} // namespace NKikimr::NKqp::NOpt
