## Unreleased

### Functionality

* 17114:Improved audit logging for user management operations. The audit logs now include details about user modification actions such as password changes, user blocking, and unblocking, making it easier to troubleshoot login issues. [#17114](https://github.com/ydb-platform/ydb/pull/17114) ([flown4qqqq](https://github.com/flown4qqqq))
* 18352:Added database audit logs in console's tablet.[#18352](https://github.com/ydb-platform/ydb/pull/18352) ([flown4qqqq](https://github.com/flown4qqqq))
* 18298:Limited the creation of ReassignerActor to only one active instance to prevent [SelfHeal](https://ydb.tech/docs/ru/maintenance/manual/selfheal) from overloading BSC. [#18298](https://github.com/ydb-platform/ydb/pull/18298) ([Sergey Belyakov](https://github.com/serbel324))
* 18294:Changed version format from Year.Major.Minor.Hotfix to Year.Major.Minor.Patch.Hotfix [#18294](https://github.com/ydb-platform/ydb/pull/18294) ([Sergey Belyakov](https://github.com/serbel324))

### Bug fixes

* 17313:Fixed CopyTable operation to allow copying tables with all column types present in the source table, regardless of feature flag settings. This resolves an issue where copying tables with certain decimal types would fail after version downgrades. [#17313](https://github.com/ydb-platform/ydb/pull/17313) ([azevaykin](https://github.com/azevaykin))
* 17122:Fixed an rare issue that caused client applications to hang during commit operations. The problem occurred because the `TEvDeletePartition` message could arrive before the `TEvApproveWriteQuota` message. The batch did not send TEvConsumed and this blocked the queue of write quota requests. [#17122](https://github.com/ydb-platform/ydb/pull/17122) ([Alek5andr-Kotov](https://github.com/Alek5andr-Kotov))
* 18362:Table auto partitioning: Fixed crash when selecting split key from access samples containing a mix of full key and key prefix operations (e.g. exact/range reads). [#18362](https://github.com/ydb-platform/ydb/pull/18362) ([ijon](https://github.com/ijon))
* 18301:Optimized memory usage in transactions with a large number of participants by changing the storage and resending mechanism for TEvReadSet messages. [#18302](https://github.com/ydb-platform/ydb/pull/18301) ([Alek5andr-Kotov](https://github.com/Alek5andr-Kotov))
* 18296:Fixed replication continuing to consume disk space when storage was low, which caused VDisks to become read-only. [#18296](https://github.com/ydb-platform/ydb/pull/18296) ([Sergey Belyakov](https://github.com/serbel324))
* 18271:Fix replication bug #10650 [#18271](https://github.com/ydb-platform/ydb/pull/18271) ([Alexander Rutkovsky](https://github.com/alexvru))
* 18231:Fix segfault that could happen while retrying Whiteboard requests. [#18231](https://github.com/ydb-platform/ydb/pull/18231) ([Andrei Rykov](https://github.com/StekPerepolnen))
* 20567:Changes from #20547

Added a check. KQP returns SCHEME_ERROR if there are unknown partitions in the request. [#20567](https://github.com/ydb-platform/ydb/pull/20567) ([Alek5andr-Kotov](https://github.com/Alek5andr-Kotov))
* 20543:Fixed memory travel when consumer commit offset to the topic with autopartitioning enabled [#20543](https://github.com/ydb-platform/ydb/pull/20543) ([Nikolay Shestakov](https://github.com/nshestakov))
* 20422:fixes timeout on healthcheck, closes #20420 [#20422](https://github.com/ydb-platform/ydb/pull/20422) ([Alexey Efimov](https://github.com/adameat))
* 20395:Fixed reporting of gRPC metrics of serverless databases. [#20395](https://github.com/ydb-platform/ydb/pull/20395) ([Ilnaz Nizametdinov](https://github.com/CyberROFL))
* 20390:[Ticket](https://github.com/ydb-platform/ydb/issues/20152)
[PR to main](https://github.com/ydb-platform/ydb/pull/20166) [#20390](https://github.com/ydb-platform/ydb/pull/20390) ([Evgenik2](https://github.com/Evgenik2))
* 20355:Allows to remove data cleanup freezes in case of counter discrepancies [#20355](https://github.com/ydb-platform/ydb/pull/20355) ([Andrey Molotkov](https://github.com/molotkov-and))

