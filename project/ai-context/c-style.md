---
name: c-style
description: C 코딩 스타일 컨벤션. C 코드를 작성·리뷰·리팩터링할 때 활성화. "C 코드", "malloc", "포인터", "헤더 파일", ".c 파일", ".h 파일"이 언급되거나 C 소스를 생성·수정할 때 이 문서를 따른다.
---

# C 코딩 스타일 컨벤션

> 목적: 팀원 간 코드 스타일을 통일하고, AI 도구가 코드를 생성할 때도 같은 규칙을 따르게 한다.
> 기반: Linux kernel style + K&R 변형. 크래프톤 정글 시스템 프로그래밍 환경에 맞춰 조정.

## 기본 원칙

- 읽기 쉬운 코드를 우선한다.
- 스타일보다 안전성과 정확성을 먼저 지킨다.
- 기존 파일의 구조와 이름 규칙을 가능한 한 유지한다.
- 경고가 발생하는 코드는 병합하지 않는다.
- 메모리, 인덱스, 포인터 연산은 항상 방어적으로 작성한다.

## 포맷팅

### 들여쓰기

- 4칸 스페이스를 사용한다.
- 탭은 사용하지 않는다.
- 한 줄 최대 80자를 권장하고, 불가피한 경우 100자까지 허용한다.

### 중괄호

함수 정의만 다음 줄, 나머지 `if`/`for`/`while`/`struct`는 같은 줄에 둔다.

```c
/* 함수 정의: 여는 중괄호를 다음 줄에 */
int find_fit(size_t size)
{
    /* 제어문: 여는 중괄호를 같은 줄에 */
    if (size == 0) {
        return -1;
    }

    for (int i = 0; i < MAX_SIZE; i++) {
        if (block[i] >= size) {
            return i;
        }
    }

    return -1;
}
```

### 단일 문장 블록

`if`/`for`/`while` 본문이 한 줄이어도 중괄호를 사용한다.

```c
/* 좋음 */
if (ptr == NULL) {
    return;
}

/* 나쁨 */
if (ptr == NULL)
    return;
```

### 공백

```c
/* 키워드 뒤 공백 */
if (condition)
for (int i = 0; i < n; i++)
while (running)

/* 함수 호출은 공백 없음 */
printf("hello");
malloc(size);

/* 이항 연산자 양쪽 공백, 단항 연산자는 붙임 */
int x = a + b;
int *p = &x;
size_t len = ~mask;

/* 포인터 선언: * 는 변수에 붙인다 */
int *ptr;       /* 좋음 */
int* ptr;       /* 나쁨 */
int * ptr;      /* 나쁨 */
```

### 빈 줄

- 함수 사이에는 빈 줄 1개를 둔다.
- 함수 내부에서는 논리적 단위가 바뀔 때만 빈 줄 1개를 둔다.
- 연속된 빈 줄 2개 이상은 금지한다.

## 네이밍

### 규칙

| 대상 | 형식 | 예시 |
|------|------|------|
| 변수, 함수 | `snake_case` | `block_size`, `find_fit` |
| 상수, 매크로 | `UPPER_SNAKE_CASE` | `MAX_HEAP`, `ALIGNMENT` |
| typedef 타입 | `snake_case_t` | `block_t`, `header_t` |
| struct 태그 | `snake_case` | `struct free_node` |
| enum 값 | `UPPER_SNAKE_CASE` | `STATUS_OK`, `STATUS_ERR` |
| 파일명 | `snake_case` | `mm.c`, `free_list.c` |

### 금지

- 헝가리안 표기법은 사용하지 않는다.
- 한 글자 변수는 루프 카운터(`i`, `j`, `k`)와 짧은 포인터 반복(`p`, `q`)만 허용한다.
- 의미가 불분명한 약어는 풀어 쓴다.
- `tmp`, `data`, `value` 같은 포괄적 이름은 매우 짧은 범위가 아니면 피한다.

허용되는 짧은 약어 예시:

- `ptr`
- `len`
- `idx`
- `cnt`
- `buf`
- `src`
- `dst`

## 타입과 상수

- 크기, 길이, 인덱스 성격의 값은 가능한 한 `size_t`를 사용한다.
- 불리언 의미를 갖는 값은 `bool`과 `true`/`false`를 우선 검토한다.
- 하드코딩된 숫자는 이름 있는 상수나 매크로로 치환한다.
- 비트 플래그는 주석으로 각 비트 의미를 남긴다.

## L-value / R-value

### 개념

- `L-value`: 메모리 위치를 가진 표현식. 대입의 왼쪽에 올 수 있다.
- `R-value`: 임시 값. 대입의 왼쪽에 올 수 없다.

### 규칙

```c
/* L-value: 대입 가능 */
int x = 10;
arr[i] = 5;
*ptr = 42;
node->data = 7;

/* R-value: 대입 불가 */
10 = x;           /* 컴파일 에러 */
a + b = c;        /* 컴파일 에러 */
get_value() = 3;  /* 컴파일 에러 */
```

### 실수하기 쉬운 패턴

```c
/* 포인터 역참조는 L-value다 */
*(ptr + i) = value;

/* 후위 증가의 결과는 R-value다 */
int *p = arr;
*p++ = 10;

/* const 변수는 L-value이지만 수정 불가 */
const int max = 100;
max = 200;        /* 컴파일 에러 */

/* 캐스트 결과는 R-value다 */
(int)x = 5;       /* 컴파일 에러 */
```

## 조건문 비교 순서

### 규칙

변수를 왼쪽, 상수를 오른쪽에 둔다.

```c
/* 좋음 */
if (ptr == NULL)
if (size > 0)
if (count != MAX_SIZE)
if (status == STATUS_OK)

/* 나쁨 */
if (NULL == ptr)
if (0 < size)
if (MAX_SIZE != count)
```

### 대입 실수 방지

```c
/* 위험 */
if (x = 0) { ... }

/* 의도적 대입 */
if ((ptr = malloc(size)) != NULL) {
    /* 할당 성공 */
}
```

### NULL / 포인터 비교

```c
if (ptr == NULL)       /* 권장 */
if (ptr != NULL)       /* 권장 */

if (!ptr)              /* 허용 */
if (ptr)               /* 허용 */

if (count == 0)        /* 권장 */
if (!count)            /* 지양 */
```

### 범위 비교

```c
if (0 <= index && index < size)    /* 권장 */
if (index >= 0 && index < size)    /* 허용 */
if (size > index && index >= 0)    /* 지양 */
```

## 헤더 파일

### Include Guard

```c
#ifndef MM_H
#define MM_H

/* 선언부 */

#endif /* MM_H */
```

### Include 순서

```c
/* 1. 대응하는 헤더 */
#include "mm.h"

/* 2. 시스템/표준 라이브러리 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 3. 프로젝트 내부 헤더 */
#include "memlib.h"
#include "config.h"
```

### 원칙

- 헤더에는 선언만 두고 구현은 넣지 않는다. 예외는 짧은 `static inline`뿐이다.
- 필요한 헤더만 최소한으로 include한다.
- 전역 변수는 헤더에 `extern` 선언만 두고, 정의는 `.c` 파일 한 곳에만 둔다.
- 헤더가 독립적으로 include 가능한지 확인한다.

## 함수

### 원칙

- 한 함수는 50줄 이내를 권장한다.
- 파라미터는 4개 이하를 권장한다.
- 에러를 반환하는 함수는 성공과 실패 규칙을 문서화한다.
- 함수 선두에 역할을 설명하는 짧은 주석을 둔다.

```c
/* 가용 리스트에서 size 이상인 첫 번째 블록을 찾는다 */
static void *find_fit(size_t asize)
{
    ...
}
```

### static 사용

- 파일 외부에 노출할 필요가 없는 함수와 전역 변수는 반드시 `static`으로 선언한다.
- 헤더에 선언하지 않는 함수는 `static`을 기본으로 한다.

### 함수 설계 권장사항

- 함수 하나가 자료구조 탐색, 검증, 출력까지 동시에 하지 않게 나눈다.
- 출력 파라미터가 필요하면 이름으로 의도를 분명히 한다.
- 사이드 이펙트가 큰 함수는 이름에 동작을 드러낸다.

## 메모리 관리

### malloc / free 규칙

```c
void *ptr = malloc(size);
if (ptr == NULL) {
    return NULL;
}

int *arr = malloc(n * sizeof(*arr));   /* 좋음 */
int *bad = malloc(n * sizeof(int));    /* 타입 변경 시 위험 */

free(ptr);
ptr = NULL;

int *zeroed = calloc(n, sizeof(*zeroed));
```

### 포인터 안전

- 사용 전 `NULL` 여부를 확인한다.
- 범위를 벗어난 접근을 금지한다.
- `free` 후에는 즉시 `NULL`을 대입해 dangling pointer 사용을 막는다.
- 포인터 연산은 가능한 한 인덱스 기반 접근보다 더 명확할 때만 사용한다.
- 메모리 소유권이 이동하면 주석이나 함수명으로 드러낸다.

### 메모리 관련 금지 사항

- 할당 결과를 확인하지 않고 바로 역참조하지 않는다.
- 이미 해제한 포인터를 다시 `free`하지 않는다.
- 다른 모듈이 소유한 메모리를 임의로 해제하지 않는다.
- 구조체 복사 시 shallow copy 문제가 없는지 확인한다.

## 배열, 문자열, 버퍼

- 버퍼 크기는 상수 또는 계산식으로 명확히 표현한다.
- 문자열 함수는 목적에 맞는 안전한 길이 검사를 동반한다.
- `snprintf`처럼 길이 제한이 있는 API를 우선 사용한다.
- 버퍼를 채운 뒤 널 종료 보장이 필요한지 항상 검토한다.

## 주석

### 스타일

```c
/* 한 줄 주석 */

/*
 * 여러 줄 주석은
 * 이 스타일을 사용한다
 */
```

### 원칙

- `무엇`보다 `왜`를 적는다.
- 코드와 주석이 불일치하면 주석을 즉시 수정한다.
- 자명한 코드에는 주석을 달지 않는다.
- TODO/FIXME에는 작성자와 날짜를 남긴다.

예시:

```c
/* TODO(woonyong, 2026-04): 경계 조건 처리 추가 */
```

### 함수 문서화

공개 함수는 입력, 동작, 반환값, 실패 조건을 함께 적는다.

```c
/*
 * mm_malloc - size 바이트 이상의 정렬된 블록을 할당한다.
 *
 * 가용 리스트를 first-fit으로 탐색하고, 실패 시 힙을 확장한다.
 * 반환값: 할당된 블록의 payload 포인터. 실패 시 NULL.
 */
void *mm_malloc(size_t size)
{
    ...
}
```

## 매크로

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define PACK(size, alloc) ((size) | (alloc))

#define LOG_ERROR(msg) do { \
    fprintf(stderr, "ERROR: %s\n", (msg)); \
    exit(1); \
} while (0)

#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
```

### 원칙

- 매크로 인자는 반드시 괄호로 감싼다.
- 여러 문장 매크로는 `do { ... } while (0)` 형태로 감싼다.
- 함수로 대체 가능한 매크로는 `static inline`을 우선 검토한다.
- 부작용이 있는 표현식을 여러 번 평가하는 매크로는 피한다.

## 에러 처리

- 에러 경로를 먼저 처리하는 early return을 선호한다.
- 중첩된 `if`를 줄이고 실패 조건을 위로 올린다.
- 라이브러리 호출 실패 시 원인과 전파 방식을 명확히 한다.

```c
void *mm_malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    size_t asize = align(size + OVERHEAD);
    void *ptr = find_fit(asize);
    if (ptr == NULL) {
        ptr = extend_heap(asize);
        if (ptr == NULL) {
            return NULL;
        }
    }

    place(ptr, asize);
    return ptr;
}
```

## 디버깅과 검증

- 개발 중에는 `assert`를 적극 활용하되, 릴리즈 빌드 영향은 확인한다.
- 경계 조건, 빈 입력, 최대 크기 입력을 먼저 테스트한다.
- 메모리 관련 변경 후에는 누수, 이중 해제, out-of-bounds 여부를 반드시 점검한다.
- 가능하면 `valgrind`, `asan`, `ubsan` 같은 도구로 검증한다.

## 컴파일

### 컴파일 플래그

```bash
gcc -Wall -Wextra -Werror -std=c99 -g -O0
gcc -Wall -Wextra -Werror -std=c99 -O2
```

### 권장 추가 옵션

```bash
gcc -Wall -Wextra -Werror -Wshadow -Wconversion -std=c99 -g -O0
gcc -fsanitize=address,undefined -g -O0
```

### Makefile 필수

- `make`: 기본 빌드
- `make clean`: 산출물 제거
- `make test`: 테스트 실행

프로젝트에 테스트 실행 규칙이 있다면 `README` 또는 별도 문서에 함께 명시한다.

## 코드 리뷰 체크리스트

- 메모리 누수, 이중 해제, use-after-free가 없는가
- 포인터와 인덱스 경계 검사가 충분한가
- 함수 이름과 실제 동작이 일치하는가
- `static`, `const`를 더 좁게 적용할 수 없는가
- 헤더 의존성과 include 순서가 정리되어 있는가
- 에러 반환 경로가 누락되지 않았는가

## 빠른 참조

| 항목 | 규칙 |
|------|------|
| 들여쓰기 | 4칸 스페이스 |
| 줄 길이 | 80자 권장, 최대 100자 |
| 네이밍 | `snake_case`, 상수는 `UPPER_SNAKE_CASE` |
| 포인터 | `int *p` |
| 중괄호 | 함수만 다음 줄, 나머지는 같은 줄 |
| 조건문 순서 | 변수 왼쪽, 상수 오른쪽 |
| 범위 비교 | `0 <= i && i < n` |
| 단일문 블록 | 항상 중괄호 |
| malloc | NULL 체크, `sizeof(*var)`, `free` 후 NULL |
| 함수 크기 | 50줄 이내 권장 |
| static | 외부 노출 불필요하면 `static` |
| 경고 | `-Wall -Wextra -Werror` |
