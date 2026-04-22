#!/bin/bash

cd /Users/ricky_lim/Desktop/All_Folder/jungle/project/api/week8-team2-network

WORKER_COUNTS=(4 6 8)
RESULTS_FILE="performance_results2.txt"
PORT=8080

pkill -f "db_server" 2>/dev/null
sleep 1

echo "=== 성능 비교 테스트 ===" > $RESULTS_FILE
echo "환경: CPU 8코어" >> $RESULTS_FILE
echo "테스트: 10초, 20 동시 연결" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

for WORKERS in "${WORKER_COUNTS[@]}"; do
    echo ""
    echo "워커 $WORKERS 테스트 중..."

    # server_main.c 수정
    sed -i '' "s/config.worker_count = [0-9]*;/config.worker_count = $WORKERS;/" server_main.c

    # 빌드
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1

    # 서버 시작
    ./db_server $PORT > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 2

    # ab 테스트 (10초 제한, 20 동시 연결)
    TEST_RESULT=$(ab -t 10 -c 20 http://localhost:$PORT/ 2>&1)

    # 결과 저장
    echo "=== 워커 $WORKERS ===" >> $RESULTS_FILE
    echo "$TEST_RESULT" >> $RESULTS_FILE
    echo "" >> $RESULTS_FILE

    # 결과 추출
    RPS=$(echo "$TEST_RESULT" | grep "Requests per second" | awk '{print $NF}')
    FAILED=$(echo "$TEST_RESULT" | grep "Failed requests" | awk '{print $NF}')
    COMPLETE=$(echo "$TEST_RESULT" | grep "Complete requests" | awk '{print $NF}')

    printf "%-12s RPS: %8s  완료: %6s  실패: %s\n" "워커 $WORKERS" "$RPS" "$COMPLETE" "$FAILED"

    # 서버 종료
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    sleep 1
done

echo ""
echo "=== 완료 (상세: $RESULTS_FILE) ==="
