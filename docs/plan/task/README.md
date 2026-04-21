# Task Folder Guide

## Purpose

This folder is for task cards that break implementation work into small, actionable pieces.

## Rules

- One file equals one task card
- Keep cards small
- Prefer titles that end with `~함`
- Always include completion criteria and validation steps
- Always include the responsible persona and touched files
- Always write the immediate predecessor and successor tasks
- If a card crosses persona boundaries, write the interface explicitly
- Do not mix implementation, testing, and docs work in one card unless the card is intentionally small

## Recommended Template

```md
# 작업 제목함

## 목적

## 범위

## 완료 기준

## 테스트/검증

## 의존성

## 메모
```

## Split Rules

Split a card further if any of these apply:

- It will take a long time
- It needs multiple tests
- Someone else could work on it in parallel
- It mixes implementation and refactoring
- It also includes documentation changes
- It touches more than one persona without a clear interface

## Workflow

1. Define the big direction in `plan`
2. Break implementation into small cards in `task`
3. Write completion criteria for each card
4. Mark cards done as you work
5. Update both the card and `07-test-and-quality-plan.md` when test coverage changes
