# Personas Guide

## 목적

이 폴더는 6개 페르소나가 같은 구현을 병렬로 하더라도 서로 충돌하지 않도록 돕는 안내서다.

## 권장 읽는 순서

1. [`00-overview.md`](00-overview.md)
2. [`01-coordinator-integrator.md`](01-coordinator-integrator.md)
3. 자신의 담당 문서
4. [`07-handoff-and-merge.md`](07-handoff-and-merge.md)

## 폴더 구조

```text
docs/plan/personas/
  README.md
  00-overview.md
  01-coordinator-integrator.md
  02-http-api.md
  03-db-sql.md
  04-concurrency-runtime.md
  05-testing-qa.md
  06-docs-demo.md
  07-handoff-and-merge.md
  08-ownership-matrix.md
```

## 사용 원칙

- 한 페르소나는 한 책임을 우선적으로 맡는다
- 같은 파일을 두 페르소나가 동시에 수정하지 않는다
- 구현 전에 계약을 먼저 고정한다
- 카드 단위로 작업한다
- 완료 후에는 반드시 handoff note를 남긴다
