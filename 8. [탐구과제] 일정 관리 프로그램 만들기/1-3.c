/**
 * @name 포인터로 그룹 지정하기
 * @author 박민혁
 */

#include <stdio.h> // Standard I/O
#include <stdlib.h> // 동적 할당을 위해 stdlib.h 사용
#include <string.h>


// 리눅스 getch 폴리필 | 출처: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux {
#include <termios.h>

static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  current = old; /* make new settings same as old settings */
  current.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      current.c_lflag |= ECHO; /* set echo mode */
  } else {
      current.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}
// }

#define MIN_SCH 8 // 초기 그룹 당 일정 수
#define GROUPS 5 // 그룹 수

#define UP 65
#define DOWN 66

char *groupLabel[5] = {"수업듣기", "예제보기", "자유시간", "도전과제", "학교공부"};

// 일정 구조체
typedef struct {
  struct { int year, month, date } date; // 날짜 저장
  struct { int hour, minute } time; // 시간 저장
  int important; // 중요도 저장
  char title[127], description[127]; // 제목 & 설명 저장
} Schedule;

// 그룹 구조체
struct {
  Schedule *schedules; // 일정 저장 / 포인터 배열인데 동적 할당을 위해 이중 포인터
  int memLimit;
  int length; // 일정 배열에 일정이 몇개 있는지 저장
} group[GROUPS];

void renderSchedules (), refresh (); // 일정 출력 함수 / 파일 읽기 함수

int curserSelect = 0, seletedGroup = 0, seletedSchedule = 0, totalSchedules = 0, displayMod = 0; // 선택된 일정 번호 / 선택된 일정의 그룹 / 선택된 일정의 그룹속 일정 번호 / 화면 번호

// 최초 실행시 동작하는 함수
int main () {
  for (int c = 0; c < GROUPS; c++) {
    group[c].schedules = (Schedule *) malloc(MIN_SCH * sizeof(Schedule)); // 그룹수에 맞춰 동적 할당
  }
  refresh(); // 파일 읽기

  int key = 0;
  while (1) {
    
    renderSchedules(); // 일정 출력
    key = getch();
    if (key == 10) refresh(); // 엔터를 누르면 파일 다시 읽기
    if (key == 113) displayMod = 0; // q를 누르면 다시 돌아가기
    if (key == 32) displayMod = 1; // 스페이스 바를 누르면 정보 창으로 이동
    if (key == 105) displayMod = 2; // i를 누르면 일정 추가 창으로 이동
    if (key == 27) key = getch();
    if (key == 91) key = getch();
    switch (key) {
      case UP:
        if (curserSelect > 0) curserSelect--;
        break;
      
      case DOWN:
        if (curserSelect < totalSchedules - 1) curserSelect++;
        break;
    }
  }
  return 0; // 정상 종료
}

void renderSchedules () {
  system("clear");

  char curser;
  int no = 0;
  int date, time, important, thisgroup = -1;
  char title[127], description[127], groupStr[127];

  switch (displayMod) {
    case 0: // 화면 번호가 0번, 즉, 보기 화면인경우
      for (int c = 0; c < GROUPS; c++) {
        printf("\n-------%s(%02d개)--------------------------------------------------\n    중요도 | 날짜             | 시간  | 제목     | 내용\n-----------------------------------------------------------------------\n", groupLabel[c], group[c].length);
        for (int d = 0; d < group[c].length; d++) {
          if (curserSelect != no) curser = ' ';
          else {
            curser = '>';
            seletedGroup = c;
            seletedSchedule = d;
          }
          
          printf("%c | #%-5d | %02d년 %02d월 %02d일 | %02d:%02d | %-10s | %-20s\n",
            curser,
            group[c].schedules[d].important,
            group[c].schedules[d].date.year,
            group[c].schedules[d].date.month,
            group[c].schedules[d].date.date,
            group[c].schedules[d].time.hour,
            group[c].schedules[d].time.minute,
            group[c].schedules[d].title,
            group[c].schedules[d].description);
          no++;
        }
      }
      printf("\n[일정 선택: 위, 아래 | 자세히: Space | 일정 추가: i]");
      break;

    case 1: // 화면 번호가 1번, 즉, 정보 화면인경우
      printf("#%d %s\n---------------------\n%s\n\n중요도: %d\n일시: %04d년 %02d월 %02d일, %02d시 %02d분\n그룹: %s(%d)\n",
        curserSelect,
        group[seletedGroup].schedules[seletedSchedule].title,
        group[seletedGroup].schedules[seletedSchedule].description,
        group[seletedGroup].schedules[seletedSchedule].important,
        group[seletedGroup].schedules[seletedSchedule].date.year,
        group[seletedGroup].schedules[seletedSchedule].date.month,
        group[seletedGroup].schedules[seletedSchedule].date.date,
        group[seletedGroup].schedules[seletedSchedule].time.hour,
        group[seletedGroup].schedules[seletedSchedule].time.minute,
        groupLabel[seletedGroup],
        seletedGroup);
      printf("\n[돌아가기: q]");
      break;

    case 2: // 화면 번호가 2번, 즉, 일정 추가 화면인경우
      printf("일정추가\n---------------------\n날짜> ");
      
      // 일정 정보를 입력 받는다 {
      scanf("%d", &date);
      printf("시간> ");
      scanf("%d", &time);
      printf("중요도> ");
      scanf("%d", &important);
      printf("그룹> ");
      scanf("%s", &groupStr);
      printf("제목> ");
      scanf("%s", &title);
      printf("내용> ");
      scanf("%s", &description);
      // }

      // 입력 받은 그룹 레이블로 그룹 번호 구하기
      for (int c = 0; c < GROUPS; c++) {
        if (strcmp(groupStr, groupLabel[c]) == 0) thisgroup = c;
      }

      if (thisgroup < 0) { // 그룹 번호가 없는경우
        printf("\n%s는 존재하지 않는 그룹입니다\n아무키를 눌러 다시 시도합니다", groupStr);
        getchar();
      } else { // 있는경우
        displayMod = 0; // 화면 번호를 0번, 즉 보기 화면으로 바꾸기
        printf("\n%s에 \"%s\" 일정을 추가했습니다!\n아무키를 눌러 목록으로 돌아갑니다", groupStr, title);
        getchar();

        FILE *file = fopen("./input.txt", "a"); // 추가모드로 열기
        if (file == NULL) { // 만약 파일이 없을경우
          printf("input.txt가 없습니다"); // 파일이 없음을 출력
        }

        fprintf(file, "\n%d %d %d %d %s %s", date, time, important, thisgroup, title, description); // 새 일정을 파일에 출력
        fclose(file); // 파일 닫기

        refresh(); // 파일 다시 읽기
      }
      break;
  }
}

// 파일 읽기 함수
void refresh () {
  // 각 그룹 구조체에 초기값을 대입 {
  for (int c = 0; c < GROUPS; c++) {
    group[c].length = 0;
    group[c].memLimit = 1;
  }
  // }

  FILE *file = fopen("./input.txt", "r"); // 파일을 읽기 전용으로 열기
  if (file == NULL) { // 만약 파일이 없을경우
    printf("input.txt가 없습니다"); // 파일이 없음을 출력
  }

  totalSchedules = 0;
  int conti = 0; // fscanf의 반환값(성공 여/부)를 저장
  while (1) { // 무한반복
    Schedule schedule; // 일정을 임시로 저장할 구조체의 포인터
    int date; // 날짜를 임시로 저장할 yyyyMMdd 형식의 int
    int time; // 시간을 임시로 저장할 hhmm 형식의 int
    int groupNo; // 그룹 번호를 저장
    conti = fscanf(file, "%d %d %d %d %s %s", // 날짜, 시간, 중요도, 그룹 번호, 제목, 내용을 입력받는다
      &date,
      &time,
      &schedule.important,
      &groupNo,
      &schedule.title,
      &schedule.description );

    if (conti == EOF) break; // 만약 fscanf의 반환값(성공 여/부)이 EOF(파일의 끝)일때, 반복을 나온다

    schedule.date.date = date % 100; // 날짜의 년도 계산
    schedule.date.month = (int) date % 10000 / 100; // 날짜의 월 계산
    schedule.date.year = (int) date / 10000; // 날짜의 일 계산

    schedule.time.minute = time % 100; // 시간의 분 계산
    schedule.time.hour = (int) time / 100; // 시간의 시 계산

    if (group[groupNo].length == MIN_SCH) { // 그룹의 일정의 개수가 초기 개수와 같을 경우, 더 많은 크기로 재할당 필요
      group[groupNo].memLimit++;
      group[groupNo].schedules = realloc(group[groupNo].schedules, MIN_SCH * group[groupNo].memLimit * sizeof(Schedule)); // 그룹의 일정 포인터 배열을 재할당하고 동적 할당을 해제
    }

    group[groupNo].schedules[group[groupNo].length] = schedule;
    
    group[groupNo].length++; // 그룹의 일정 개수가 증가됨
    totalSchedules++;
  }

  fclose(file); // 파일 닫기
}