#!/bin/bash

cd /Users/ricky_lim/Desktop/All_Folder/jungle/project/api/week8-team2-network

WORKER_COUNTS=(4 6 8)
RESULTS_FILE="performance_results.txt"
PORT=8080

# 이전 프로세스 종료
pkill -f "db_server" 2>/dev/null

echo "=== 로컬 성능 테스트 ===" > $RESULTS_FILE
echo "환경: CPU 8코어" >> $RESULTS_FILE
echo "테스트 설정: 1000 요청, 50 동시 연결" >> $RESULTS_FILE
echo "================================" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

for WORKERS in "${WORKER_COUNTS[@]}"; do
    echo ""
    echo "=== 워커 $WORKERS 테스트 ==="
    echo "" >> $RESULTS_FILE
    echo "=== 워커 $WORKERS ===" >> $RESULTS_FILE

    # server_main.c 수정
    sed -i '' "s/config.worker_count = [0-9]*;/config.worker_count = $WORKERS;/" server_main.c

    echo "1. 빌드 중..."
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "빌드 실패!"
        echo "빌드 실패" >> $RESULTS_FILE
        continue
    fi

    echo "2. 서버 시작 (워커 $WORKERS)..."
    ./db_server $PORT > /dev/null 2>&1 &
    SERVER_PID=$!

    # 서버가 준비될 때까지 대기
    sleep 2

    # 서버가 실행 중인지 확인
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo "서버 시작 실패!"
        echo "서버 시작 실패" >> $RESULTS_FILE
        continue
    fi

    echo "3. 부하 테스트 실행..."
    # ab 테스트 실행
    TEST_RESULT=$(ab -n 1000 -c 50 -t 30 http://localhost:$PORT/ 2>&1)

    # 전체 결과 저장
    echo "$TEST_RESULT" >> $RESULTS_FILE
    echo "" >> $RESULTS_FILE

    # 결과 추출 및 출력
    RPS=$(echo "$TEST_RESULT" | grep "Requests per second" | awk '{print $NF}')
    AVG_TIME=$(echo "$TEST_RESULT" | grep "Time per request:" | head -1 | awk '{print $NF}')
    FAILED=$(echo "$TEST_RESULT" | grep "Failed requests" | awk '{print $NF}')

    echo "   RPS: $RPS req/s"
    echo "   평균 응답시간: ${AVG_TIME}ms"
    echo "   실패: $FAILED"

    # 서버 종료
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    sleep 1
done

echo ""
echo "=== 테스트 완료 ==="
echo ""
echo "== 결과 요약 =="
grep -A 2 "^=== 워커" $RESULTS_FILE | grep -E "(워커|Requests per second|Time per request:|Failed)"
