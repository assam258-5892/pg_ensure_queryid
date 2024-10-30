#include "postgres.h"
#include "tcop/utility.h"

#if PG_VERSION_NUM >= 160000
#include "utils/backend_status.h"
#include "nodes/queryjumble.h"
#elif PG_VERSION_NUM >= 140000
#include "utils/backend_status.h"
#include "utils/queryjumble.h"
#endif

#include "commands/explain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "pg_ensure_queryid.h"

PG_MODULE_MAGIC;

static ExecutorRun_hook_type prev_ExecutorRun = NULL;
static ProcessUtility_hook_type prev_ProcessUtility = NULL;

static bool use_query_id_tracking = true;

void _PG_init(void);

DECLARE_HOOK(void pgeq_ExecutorRun, QueryDesc *queryDesc, ScanDirection direction, uint64 count, bool execute_once);
DECLARE_HOOK(void pgeq_ProcessUtility, PlannedStmt *pstmt, const char *queryString,
			 bool readOnlyTree,
			 ProcessUtilityContext context,
			 ParamListInfo params, QueryEnvironment *queryEnv,
			 DestReceiver *dest,
			 QueryCompletion *qc);

void
_PG_init(void)
{
#if PG_VERSION_NUM >= 140000
	prev_ExecutorRun = ExecutorRun_hook;
	ExecutorRun_hook = HOOK(pgeq_ExecutorRun);

	prev_ProcessUtility = ProcessUtility_hook;
	ProcessUtility_hook = HOOK(pgeq_ProcessUtility);

	EnableQueryId();

	DefineCustomBoolVariable("pg_ensure_queryid.use_query_id_tracking",
							"If the value of pg_stat_activity.query_id is 0, assign the query_id for the SQL currently being executed by the executor.",
							NULL,
							&use_query_id_tracking,
							true,
							PGC_SIGHUP,
							0,
							NULL,
							NULL,
							NULL);
#endif
}

static void
pgeq_ExecutorRun(QueryDesc *queryDesc, ScanDirection direction, uint64 count,
				 bool execute_once)
{
#if PG_VERSION_NUM >= 140000
	if (use_query_id_tracking == true && queryDesc != NULL && queryDesc->plannedstmt != NULL)
		pgstat_report_query_id(queryDesc->plannedstmt->queryId, false);

	PG_TRY();
	{
		if (prev_ExecutorRun)
			prev_ExecutorRun(queryDesc, direction, count, execute_once);
		else
			standard_ExecutorRun(queryDesc, direction, count, execute_once);
	}
	PG_CATCH();
	{
		PG_RE_THROW();
	}
	PG_END_TRY();
#endif
}

static void
pgeq_ProcessUtility(PlannedStmt *pstmt, const char *queryString,
					  bool readOnlyTree,
					  ProcessUtilityContext context,
					  ParamListInfo params, QueryEnvironment *queryEnv,
					  DestReceiver *dest, QueryCompletion *qc)
{
#if PG_VERSION_NUM >= 140000
	if (use_query_id_tracking == true && pstmt != NULL)
		pgstat_report_query_id(pstmt->queryId, false);

	PG_TRY();
	{
		if (prev_ProcessUtility)
			prev_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, qc);
		else
			standard_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, qc);
	}
	PG_CATCH();
	{
		PG_RE_THROW();
	}
	PG_END_TRY();
#endif
}

