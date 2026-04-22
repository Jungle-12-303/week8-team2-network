#!/bin/bash

# 워커 수 성능 테스트 스크립트
# 4, 6, 8 워커로 각각 테스트

cd /Users/ricky_lim/Desktop/All_Folder/jungle/project/api/week8-team2-network

WORKER_COUNTS=(4 6 8)
RESULTS_FILE="test_results.txt"

echo "성능 테스트 시작..." > $RESULTS_FILE
echo "테스트 환경: CPU 8코어" >> $RESULTS_FILE
echo "부하: 1000 요청, 50 동시 연결" >> $RESULTS_FILE
echo "================================" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

for WORKERS in "${WORKER_COUNTS[@]}"; do
    echo ""
    echo "=== 워커 $WORKERS 테스트 시작 ==="
    echo "" >> $RESULTS_FILE
    echo "=== 워커 $WORKERS ===" >> $RESULTS_FILE

    # server_main.c 수정
    sed -i.bak "s/config.worker_count = [0-9]*;/config.worker_count = $WORKERS;/" server_main.c

    echo "1. 빌드 중..."
    docker-compose build --no-cache > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "빌드 실패!"
        echo "빌드 실패" >> $RESULTS_FILE
        continue
    fi

    echo "2. 컨테이너 시작..."
    docker-compose down > /dev/null 2>&1
    docker-compose up -d > /dev/null 2>&1

    # 서버가 시작될 때까지 대기
    sleep 2

    echo "3. 부하 테스트 실행..."
    # ab 테스트 실행
    TEST_RESULT=$(ab -n 1000 -c 50 -t 30 http://localhost:8080/ 2>&1)

    echo "$TEST_RESULT" >> $RESULTS_FILE

    # 결과 추출
    RPS=$(echo "$TEST_RESULT" | grep "Requests per second" | awk '{print $NF}')
    AVG_TIME=$(echo "$TEST_RESULT" | grep "Time per request:" | head -1 | awk '{print $NF}')

    echo "결과: RPS=$RPS, 평균 응답시간=${AVG_TIME}ms"

    # 컨테이너 정지
    docker-compose down > /dev/null 2>&1
    sleep 1
done

echo ""
echo "=== 테스트 완료 ==="
echo "상세 결과: $RESULTS_FILE"
cat $RESULTS_FILE
