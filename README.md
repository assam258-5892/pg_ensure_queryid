# pg_ensure_queryid 확장 불필요 안내

## 요약

`pg_ensure_queryid` 확장은 **PostgreSQL 14.14, 15.9, 16.5, 17.0 이상 버전에서 불필요**합니다.

Extended Query Protocol에서 query_id가 보고되지 않던 문제가 PostgreSQL core에서 수정되었습니다.

---

## 패치 정보

### 커밋
- **해시**: `b36ee879c54cf42f21c9374aceb5baf65efecbc3`
- **날짜**: 2024-09-18
- **저자**: Sami Imseih
- **리뷰어**: Jian He, Andrei Lepikhov, Michael Paquier

### 제목
```
Add missing query ID reporting in extended query protocol
```

### 설명
Extended Query Protocol 메시지 처리 시 query_id 보고가 누락되던 문제 수정:
- **Bind 메시지**: 캐시된 쿼리에서 첫 번째 Query의 queryId 보고
- **Execute 메시지**: Portal에 저장된 첫 번째 PlannedStmt의 queryId 보고

### 관련 토론
- https://postgr.es/m/CA+427g8DiW3aZ6pOpVgkPbqK97ouBdf18VLiHFesea2jUk3XoQ@mail.gmail.com
- https://postgr.es/m/CA+TgmoZxtnf_jZ=VqBSyaU8hfUkkwoJCJ6ufy4LGpXaunKrjrg@mail.gmail.com
- https://postgr.es/m/1391613709.939460.1684777418070@office.mailbox.org

---

## 적용된 버전

| PostgreSQL 버전 | 첫 적용 버전 | 릴리스 날짜 |
|----------------|-------------|-------------|
| **14** | 14.14 | 2024-11-14 |
| **15** | 15.9 | 2024-11-14 |
| **16** | 16.5 | 2024-11-14 |
| **17** | 17.0 | 2024-09-26 |

---

## Diff

### src/backend/executor/execMain.c

```diff
@@ -130,10 +130,12 @@ void
 ExecutorStart(QueryDesc *queryDesc, int eflags)
 {
 	/*
-	 * In some cases (e.g. an EXECUTE statement) a query execution will skip
-	 * parse analysis, which means that the query_id won't be reported.  Note
-	 * that it's harmless to report the query_id multiple time, as the call
-	 * will be ignored if the top level query_id has already been reported.
+	 * In some cases (e.g. an EXECUTE statement or an execute message with the
+	 * extended query protocol) the query_id won't be reported, so do it now.
+	 *
+	 * Note that it's harmless to report the query_id multiple times, as the
+	 * call will be ignored if the top level query_id has already been
+	 * reported.
 	 */
 	pgstat_report_query_id(queryDesc->plannedstmt->queryId, false);
```

### src/backend/tcop/postgres.c (exec_bind_message)

```diff
@@ -1609,6 +1609,7 @@ exec_bind_message(StringInfo input_message)
 	char		msec_str[32];
 	ParamsErrorCbData params_data;
 	ErrorContextCallback params_errcxt;
+	ListCell   *lc;

 	/* ... */

 	pgstat_report_activity(STATE_RUNNING, psrc->query_string);

+	foreach(lc, psrc->query_list)
+	{
+		Query	   *query = lfirst_node(Query, lc);
+
+		if (query->queryId != UINT64CONST(0))
+		{
+			pgstat_report_query_id(query->queryId, false);
+			break;
+		}
+	}
+
 	set_ps_display("BIND");
```

### src/backend/tcop/postgres.c (exec_execute_message)

```diff
@@ -2067,6 +2079,7 @@ exec_execute_message(const char *portal_name, long max_rows)
 	char		msec_str[32];
 	ParamsErrorCbData params_data;
 	ErrorContextCallback params_errcxt;
+	ListCell   *lc;

 	/* ... */

 	pgstat_report_activity(STATE_RUNNING, sourceText);

+	foreach(lc, portal->stmts)
+	{
+		PlannedStmt *stmt = lfirst_node(PlannedStmt, lc);
+
+		if (stmt->queryId != UINT64CONST(0))
+		{
+			pgstat_report_query_id(stmt->queryId, false);
+			break;
+		}
+	}
+
 	set_ps_display(GetCommandTagName(portal->commandTag));
```

---

## 결론

### pg_ensure_queryid가 불필요한 버전
- PostgreSQL 14.14+
- PostgreSQL 15.9+
- PostgreSQL 16.5+
- PostgreSQL 17.0+

### pg_ensure_queryid가 필요한 버전
- PostgreSQL 14.0 ~ 14.13
- PostgreSQL 15.0 ~ 15.8
- PostgreSQL 16.0 ~ 16.4

### 권장 사항
위 버전 이상을 사용하는 경우 `pg_ensure_queryid` 확장을 제거하고,
`shared_preload_libraries`에서도 삭제하는 것을 권장합니다.

---
---

# pg_ensure_queryid (Legacy Documentation)

# Table of contents
- [Installation](#installation)
- [GUC](#guc)

# Installation
1. Build
```bash
$ make
$ make install
```

2. Modify postgresql.conf
```bash
shared_preload_libraries = 'pg_ensure_queryid'
```

3. Restart postgres
```bash
pg_ctl restart
```

# GUC
pg_ensure_queryid.use_query_id_tracking
- on : enable query_id tracking on extended PreferQueryMode (default)
- off : disable query_id tracking on extended PreferQueryMode

Note: This option does not require a server restart
