int btn = A5;  // 기존 버튼 (A5) - "START" 출력
const int lineSensor = 2;  // 라인트레이싱 센서 (기존 btn1(A3) 대신 사용)
int pushBtn = A7; // 카운트다운 중지 버튼 - "PUSH" 출력

int num1 = 0, num2 = 3;  // 카운트다운 숫자
int counter = 0;         // 카운터 변수
int FND2[7] = {41, 42, 43, 44, 45, 46, 47};  // 10의 자리
int FND1[7] = {34, 35, 36, 37, 38, 39, 40};  // 1의 자리
int FND_data[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

int isCounting = 0;  // 카운트다운 상태 플래그
int isDisplayingSerial = 0; // 시리얼 출력 중일 때 인터랙션 방지

unsigned long countdownStartTime = 0; // 카운트다운 시작 시간
unsigned long buttonPressStart = 0;  // 버튼 눌림 시간
unsigned long pushButtonPressStart = 0;  // Push 버튼 눌림 시간

bool btn1Pressed = false; // 라인트레이싱 센서가 감지되었는지 확인
bool pushBtnPressed = false; // Push 버튼 눌림 상태
bool isCounterDisplayed = false; // 카운터가 표시 중인지 확인

void setup() {
    Serial.begin(9600); // 시리얼 모니터 초기화

    // 입력 핀 설정
    pinMode(btn, INPUT_PULLUP);
    pinMode(lineSensor, INPUT);
    pinMode(pushBtn, INPUT_PULLUP);

    // 7세그먼트 FND 핀 설정
    for (int i = 0; i < 7; i++) {
        pinMode(FND1[i], OUTPUT);
        pinMode(FND2[i], OUTPUT);
        digitalWrite(FND1[i], HIGH);
        digitalWrite(FND2[i], HIGH);
    }

    // 초기 카운터 값 표시
    displayNumber(counter / 10, counter % 10);
}

void loop() {
    if (isDisplayingSerial || isCounterDisplayed) {
        return; // 시리얼 출력 중 또는 카운트다운 중이면 실행 안함
    }

    // 📌 라인트레이싱 센서 감지 시 (LOW일 때 감지됨)
    if (digitalRead(lineSensor) == LOW && !pushBtnPressed) {
        if (!btn1Pressed) {
            btn1Pressed = true;
            buttonPressStart = millis();
        }
        if (millis() - buttonPressStart >= 200) {
            btn1Pressed = false;
            counter++;
            if (counter > 7) counter = 7;  // counter 최대 7 제한
            displayNumber(counter / 10, counter % 10);

            if (counter >= 2) {
                isDisplayingSerial = 1;
                Serial.print("ONE MORE? Counter: ");
                Serial.println(counter);
                delay(2000);
                isDisplayingSerial = 0;
            }
        }
        return;
    } else {
        btn1Pressed = false;
    }

    // 📌 btn(A5) 버튼을 누르면 "START" 출력
    if (!digitalRead(btn)) {
        Serial.println("START");
        delay(200);  // 버튼 바운스 방지
    }

    // 📌 pushBtn(A7) 버튼을 누르면 "PUSH" 출력
    if (!digitalRead(pushBtn)) {
        Serial.println("PUSH");
        delay(200);  // 버튼 바운스 방지
    }

    // 📌 기존 카운트다운 시작 버튼 동작 (btn(A5) 동작과 별개)
    if (counter > 0 && !digitalRead(btn) && !pushBtnPressed) {
        if (millis() - buttonPressStart >= 200) { // 0.2초 이상 눌렀을 때만 동작
            while (!digitalRead(btn)); // 버튼이 떼어질 때까지 대기

            resetCountdown(); // 카운트다운 초기화
            isCounting = 1; // 카운트다운 시작
            countdownStartTime = millis();
        }
    }

    // 📌 카운트다운 로직 실행
    if (isCounting) {
        displayNumber(num2, num1);
        delay(1000);

        if (num1 == 0) {
            if (num2 > 0) {
                num1 = 9;
                num2--;
            } else {
                displayNumber(0, 0); // `00` 표시
                delay(3000); // 3초간 유지

                // 3초 후 counter 감소 및 새로운 값 표시
                counter--;
                if (counter < 0) counter = 0; // 음수 방지
                displayNumber(counter / 10, counter % 10);
                isCounting = 0; // 카운트다운 정지
            }
        } else {
            num1--;
        }
    }
}

// 📌 카운트다운 중지 후 중단된 값 표시
void stopCountdown() {
    isCounting = 0;
    displayNumber(num2, num1);
    delay(3000);
    counter--;
    if (counter < 0) counter = 0;
    displayNumber(counter / 10, counter % 10);
}

// 📌 카운트다운 초기화 (3초로 재설정)
void resetCountdown() {
    num1 = 0;
    num2 = 3;
}

// 📌 7세그먼트에 숫자 표시
void displayNumber(int tens, int ones) {
    for (int i = 0; i < 7; i++) {
        digitalWrite(FND2[i], bitRead(FND_data[tens], i));
        digitalWrite(FND1[i], bitRead(FND_data[ones], i));
    }
}

// 📌 7세그먼트 디스플레이 끄기
void displayOff() {
    for (int i = 0; i < 7; i++) {
        digitalWrite(FND2[i], HIGH);
        digitalWrite(FND1[i], HIGH);
    }
}
