# 동시성 개선 기록

## 문제

현재 구현에서 병렬 요청 처리의 병목이 어디인지 정리한다.

예시:

- 읽기 요청까지 전체 락으로 막는 구조인지
- worker thread가 요청을 어떻게 가져가는지
- queue full 상황을 어떻게 처리하는지

## 학습

이 문서에서는 아래 개념을 정리한다.

- `mutex`와 `rwlock`의 차이
- producer-consumer queue의 동작
- worker pool에서 graceful shutdown을 하는 방법

## 해결

여기에 실제 적용한 방식을 적는다.

- 어떤 락을 선택했는지
- 어떤 공유 자원을 분리했는지
- 어떤 함수가 어떤 책임을 맡는지

## 검증

여기에 확인한 내용을 적는다.

- 동시 요청을 얼마나 보낼 수 있는지
- 읽기 요청이 병렬로 동작하는지
- 큐 포화 시 응답이 어떻게 되는지

## 남은 과제

- 더 큰 부하에서의 성능 측정
- race condition 점검
- timeout 및 shutdown 처리 보강

## 관련 링크

- [해결기록 홈](README.md)
- [Review 문서 홈](../README.md)
